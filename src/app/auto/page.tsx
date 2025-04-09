'use client'

import { useEffect, useState } from 'react';
import styles from './page.module.css';
import { Gender, Player } from './player';
import PlayerInput from './PlayerInput';
import MatchTable from './MatchTable';

export type Table = string[][][];

interface MatchGeneratorModule {
    generateMatches: (
        players: Player[][],
        courts: number,
        games: number
    ) => string;
}

export default function AutoPage() {
    const [players, setPlayers] = useState<Player[][]>([[{ name: '', level: 1, gender: Gender.MALE }]]);
    const [courts, setCourts] = useState(2);
    const [games, setGames] = useState(4);

    const [wasmModule, setWasmModule] = useState<MatchGeneratorModule | null>(null);
    const [isLoadingWasm, setIsLoadingWasm] = useState(true);
    const [tables, setTables] = useState<Table[]>([]);

    useEffect(() => {
        const loadWasm = async () => {
            try {
                setIsLoadingWasm(true);
                const moduleFactory = (await import('../../../public/wasm/match_generator.js')).default;

                const instance = await moduleFactory();
                setWasmModule(instance as MatchGeneratorModule);
                console.log("Wasm module loaded successfully.");

            } catch (err) {
                console.error("Error loading Wasm module:", err);
            } finally {
                setIsLoadingWasm(false);
            }
        };

        loadWasm();
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
        setPlayers(JSON.parse(input));
    }

    function handleGenerate() {
        if (!wasmModule) {
            console.error("Wasm module not loaded yet.");
            return;
        }
        if (isLoadingWasm) {
            console.log("Wasm module is still loading.");
            return; // Or show feedback
        }

        try {
            const startedAt = Date.now();
            const result = JSON.parse(wasmModule.generateMatches(players, courts, games));
            const elapsed = Date.now() - startedAt;
            console.log('Took ' + (elapsed / 1000) + 's.');

            if (result.status == 'success') {
                setTables((tables) => [...tables, result.result]);
            } else {
                console.error(result.message);
            }
        } catch (err) {
            console.error("Error calling Wasm function:", err);
        }
    };

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
                    onChange={(e) => setCourts(Number(e.target.value))} />
                <label>Number of games: </label>
                <input
                    className={styles.GameSettingsInput}
                    type='number'
                    value={games}
                    onChange={(e) => setGames(Number(e.target.value))} />
                <button
                    className={styles.GenerateButton}
                    onClick={handleGenerate}>
                    Generate
                </button>
            </div>
            <h2 className={styles.GeneratedTablesTitle}>Generated Tables</h2>
            {tables.map((table, i) => (
                <MatchTable key={'table' + i} table={table} />
            ))}
        </div>
    );
}