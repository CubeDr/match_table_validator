'use client'

import { useEffect, useState, useRef, useCallback } from 'react';
import styles from './page.module.css';
import { Gender, Player } from './player';
import PlayerInput from './PlayerInput';
import MatchTable from './MatchTable';

export type Table = string[][][];

export default function AutoPage() {
    const [players, setPlayers] = useState<Player[][]>([[{ name: '', level: 1, gender: Gender.MALE }]]);
    const [courts, setCourts] = useState(2);
    const [games, setGames] = useState(4);
    const [tables, setTables] = useState<Table[]>([]);
    const [isGenerating, setIsGenerating] = useState(false);
    const [workerError, setWorkerError] = useState<string | null>(null);

    const workerRef = useRef<Worker | null>(null);

    useEffect(() => {
        workerRef.current = new Worker('/match.worker.js');

        workerRef.current.onmessage = (event: MessageEvent<{ type: string, payload: any }>) => {
            const { type, payload } = event.data;
            console.log("Main: Received message from worker:", type);

            switch (type) {
                case 'WORKER_READY':
                    console.log("Main: Worker is ready and Wasm loaded.");
                    setWorkerError(null);
                    break;
                case 'WASM_LOAD_ERROR':
                    console.error("Main: Worker failed to load Wasm:", payload);
                    setWorkerError(`Failed to load Wasm module: ${payload}`);
                    setIsGenerating(false);
                    break;
                case 'GENERATION_SUCCESS':
                    console.log("Main: Generation successful.");
                    setTables((prevTables) => [...prevTables, payload]);
                    setIsGenerating(false);
                    setWorkerError(null);
                    break;
                case 'GENERATION_ERROR':
                    console.error("Main: Generation failed:", payload);
                    setWorkerError(`Generation Error: ${payload}`);
                    setIsGenerating(false);
                    break;
                default:
                    console.warn("Main: Received unknown message type from worker:", type);
            }
        };

        workerRef.current.onerror = (error) => {
            console.error("Main: Worker error:", error);
            setWorkerError(`Worker script error: ${error.message}`);
            setIsGenerating(false);
        };

        return () => {
            console.log("Main: Terminating worker.");
            workerRef.current?.terminate();
            workerRef.current = null;
        };
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
        if (!workerRef.current) {
            console.error("Worker not initialized yet.");
            setWorkerError("Match generator is not ready. Please try again later.");
            return;
        }
        if (isGenerating) {
            console.log("Generation already in progress.");
            return;
        }
        if (workerError && workerError.includes("Wasm module")) {
            console.error("Cannot generate, Wasm module failed to load.");
            return;
        }

        console.log("Main: Sending GENERATE_MATCHES message to worker.");
        setIsGenerating(true);
        setWorkerError(null);

        workerRef.current.postMessage({
            type: 'GENERATE_MATCHES',
            payload: {
                players,
                courts,
                games
            }
        });
    }, [players, courts, games, isGenerating, workerError]);

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
                                key={'PlayerInput' + playerIndex}
                            />
                        )}
                        <button onClick={() => addPlayer(teamIndex)}>Add</button>
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
                    disabled={isGenerating}
                />
                <label>Number of games: </label>
                <input
                    className={styles.GameSettingsInput}
                    type='number'
                    value={games}
                    onChange={(e) => setGames(Number(e.target.value))}
                    disabled={isGenerating}
                />
                <button
                    className={styles.GenerateButton}
                    onClick={handleGenerate}
                    disabled={isGenerating || !!workerError}
                >
                    {isGenerating ? 'Generating...' : 'Generate'}
                </button>
            </div>
            {workerError && <p style={{ color: 'red' }}>Error: {workerError}</p>}

            <h2 className={styles.GeneratedTablesTitle}>Generated Tables</h2>
            {tables.map((table, i) => (
                <MatchTable key={'table' + i} table={table} deleteTable={() => {
                    setTables((tables) => [...tables.slice(0, i), ...tables.slice(i + 1)]);
                }} />
            ))}
        </div>
    );
}