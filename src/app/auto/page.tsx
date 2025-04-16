'use client'

import { useEffect, useState, useRef, useCallback } from 'react';
import styles from './page.module.css';
import { Gender, Player } from './player';
import PlayerInput from './PlayerInput';
import MatchTable from './MatchTable';

export type Table = string[][][];

const NUM_WORKERS = 4;

export default function AutoPage() {
    const [players, setPlayers] = useState<Player[][]>([[{ name: '', level: 1, gender: Gender.MALE }]]);
    const [courts, setCourts] = useState(2);
    const [games, setGames] = useState(4);
    const [tables, setTables] = useState<Table[]>([]);
    const [generatingCount, setGeneratingCount] = useState(0);
    const [workerError, setWorkerError] = useState<string | null>(null);
    const workersRef = useRef<Worker[]>([]);
    const workerSetupStatusRef = useRef<Array<{ id: number; ready: boolean; error: string | null }>>(
        Array(NUM_WORKERS).fill(null).map((_, i) => ({ id: i, ready: false, error: null }))
    );
    const [isAnyWorkerReady, setIsAnyWorkerReady] = useState(false);

    useEffect(() => {
        console.log("Main: Initializing workers...");
        workersRef.current = [];

        for (let i = 0; i < NUM_WORKERS; i++) {
            const worker = new Worker('/match.worker.js');
            workersRef.current.push(worker);
            const workerId = i;

            worker.onmessage = (event: MessageEvent<{ type: string, payload: Table }>) => {
                const { type, payload } = event.data;
                console.log(`Main (Worker ${workerId}): Received message:`, type);

                switch (type) {
                    case 'WORKER_READY':
                        console.log(`Main (Worker ${workerId}): Worker is ready.`);
                        const currentStatus = workerSetupStatusRef.current[workerId];
                        if (currentStatus) {
                            currentStatus.ready = true;
                            currentStatus.error = null;
                        }
                        if (!isAnyWorkerReady) setIsAnyWorkerReady(true);
                        setWorkerError(prevError =>
                            prevError?.includes('Wasm') ? null : prevError
                        );
                        break;
                    case 'WASM_LOAD_ERROR':
                        console.error(`Main (Worker ${workerId}): Failed to load Wasm:`, payload);
                        const statusWithError = workerSetupStatusRef.current[workerId];
                        if (statusWithError) {
                            statusWithError.ready = false;
                            statusWithError.error = `Worker ${workerId}: Failed to load Wasm module: ${payload}`;
                        }
                        setWorkerError(prevError => prevError ?? `Worker ${workerId}: Failed to load Wasm module: ${payload}`);
                        setGeneratingCount((count) => count - 1);
                        break;
                    case 'GENERATION_SUCCESS':
                        console.log(`Main (Worker ${workerId}): Generation successful.`);
                        setTables((prevTables) => [...prevTables, payload]);
                        setGeneratingCount((count) => count - 1);
                        setWorkerError(prevError =>
                            prevError?.startsWith('Generation Error:') ? null : prevError
                        );
                        break;
                    case 'GENERATION_ERROR':
                        console.error(`Main (Worker ${workerId}): Generation failed:`, payload);
                        setWorkerError(`Worker ${workerId} Error: ${payload}`);
                        setGeneratingCount((count) => count - 1);
                        break;
                    default:
                        console.warn(`Main (Worker ${workerId}): Received unknown message type:`, type);
                }
            };

            worker.onerror = (error) => {
                console.error(`Main (Worker ${workerId}): Script error:`, error);
                const errorMessage = `Worker ${workerId} script error: ${error.message}`;
                const statusWithError = workerSetupStatusRef.current[workerId];
                if (statusWithError) {
                    statusWithError.ready = false;
                    statusWithError.error = errorMessage;
                }
                setWorkerError(prevError => prevError ?? errorMessage);
                setGeneratingCount((count) => count - 1);
            };
        }

        return () => {
            console.log("Main: Terminating all workers.");
            workersRef.current.forEach((worker, i) => {
                console.log(`Main: Terminating worker ${i}`);
                worker.terminate();
            });
            workersRef.current = [];
            setIsAnyWorkerReady(false);
            workerSetupStatusRef.current = Array(NUM_WORKERS).fill(null).map((_, i) => ({ id: i, ready: false, error: null }));
        };
        // eslint-disable-next-line react-hooks/exhaustive-deps
    }, []);

    function addPlayer(teamIndex: number) {
        const newPlayers = Array.from(players);
        newPlayers[teamIndex].push({ name: '', level: 1, gender: Gender.MALE });
        setPlayers(newPlayers);
    }

    function updatePlayer(teamIndex: number, playerIndex: number, updatedPlayer: Player) {
        const newPlayers = Array.from(players);
        newPlayers[teamIndex][playerIndex] = updatedPlayer;
        setPlayers(newPlayers);
    }

    function addTeam() {
        const newPlayers = Array.from(players);
        newPlayers.push([{ name: '', level: 1, gender: Gender.MALE }]);
        setPlayers(newPlayers);
    }

    function copy() {
        navigator.clipboard.writeText(JSON.stringify(players))
    }

    function paste() {
        const input = window.prompt('json');
        if (input == null) return;
        try {
            setPlayers(JSON.parse(input));
        } catch (e) {
            console.error("Failed to parse pasted JSON:", e);
            alert("Invalid JSON pasted.");
        }
    }

    const handleGenerate = useCallback(() => {
        if (workersRef.current.length !== NUM_WORKERS) {
            console.error("Workers not fully initialized yet.");
            setWorkerError("Match generator is not ready. Please wait or refresh.");
            return;
        }

        if (generatingCount > 0) {
            console.log("Generation already in progress.");
            return;
        }

        const criticalError = workerSetupStatusRef.current.find(status => status.error && (status.error.includes('Wasm') || status.error.includes('script error')));
        if (criticalError) {
            console.error("Cannot generate, a worker encountered a critical error:", criticalError.error);
            setWorkerError(criticalError.error);
            return;
        }

        if (!isAnyWorkerReady) {
            console.warn("No workers reported ready yet. Trying anyway...");
            setWorkerError("Workers initializing... Please try again shortly.");
        }


        console.log(`Main: Sending GENERATE_MATCHES message to ${NUM_WORKERS} workers.`);
        setGeneratingCount(NUM_WORKERS);
        setWorkerError(null);

        const payload = {
            players,
            courts,
            games
        };

        workersRef.current.forEach((worker, i) => {
            console.log(`Main: Posting to worker ${i}`);
            worker.postMessage({
                type: 'GENERATE_MATCHES',
                payload: payload
            });
        });
    }, [players, courts, games, generatingCount, isAnyWorkerReady]);

    const isGenerateDisabled = generatingCount > 0 ||
        !isAnyWorkerReady ||
        !!workerSetupStatusRef.current.find(status => status.error && (status.error.includes('Wasm') || status.error.includes('script error')));


    return (
        <div className={styles.Page}>
            <h1>Auto Match Generator</h1>
            <button className={styles.Control} onClick={copy}>Copy</button>
            <button className={styles.Control} onClick={paste}>Paste</button>
            <div className={styles.TeamsSection}>
                {players.map((team, teamIndex) =>
                    <div className={styles.PlayersSection} key={'PlayersSection' + teamIndex}>
                        {team.map((player, playerIndex) =>
                            <PlayerInput
                                player={player}
                                onPlayerUpdate={(updatedPlayer) => updatePlayer(teamIndex, playerIndex, updatedPlayer)}
                                key={`PlayerInput-${teamIndex}-${playerIndex}`}
                            />
                        )}
                        <button onClick={() => addPlayer(teamIndex)}>Add Player</button>
                    </div>
                )}
                {players.length === 1 && <button onClick={() => addTeam()}>Add Team</button>}
            </div>
            <hr />
            <div className={styles.GameSettings}>
                <label>Number of courts: </label>
                <input
                    className={styles.GameSettingsInput}
                    type='number'
                    value={courts}
                    onChange={(e) => setCourts(Number(e.target.value))}
                    disabled={generatingCount > 0}
                />
                <label>Number of games: </label>
                <input
                    className={styles.GameSettingsInput}
                    type='number'
                    value={games}
                    onChange={(e) => setGames(Number(e.target.value))}
                    disabled={generatingCount > 0}
                />
                <button
                    className={styles.GenerateButton}
                    onClick={handleGenerate}
                    disabled={isGenerateDisabled}
                >
                    {generatingCount > 0 ? `Generating ${generatingCount} tables...` : (isGenerateDisabled && !isAnyWorkerReady) ? 'Initializing...' : `Generate ${NUM_WORKERS} tables`}
                </button>
            </div>
            {workerError && <p style={{ color: 'red' }}>Error: {workerError}</p>}

            <h2 className={styles.GeneratedTablesTitle}>Generated Tables ({tables.length} results)</h2>
            {tables.map((table, i) => (
                <MatchTable key={'table' + i} table={table} deleteTable={() => {
                    setTables((prevTables) => [...prevTables.slice(0, i), ...prevTables.slice(i + 1)]);
                }} />
            ))}
        </div>
    );
}
