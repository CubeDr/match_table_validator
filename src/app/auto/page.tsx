'use client'

import { useState } from 'react';
import styles from './page.module.css';
import { Gender, Player } from './player';
import PlayerInput from './PlayerInput';

export default function AutoPage() {
    const [players, setPlayers] = useState<Player[][]>([[{ name: '', level: 1, gender: Gender.MALE }]]);
    const [courts, setCourts] = useState(2);
    const [games, setGames] = useState(4);

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
                <button className={styles.GenerateButton}>Generate</button>
            </div>
        </div>
    );
}