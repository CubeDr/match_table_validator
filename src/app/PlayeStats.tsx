'use client';

import { useEffect, useState } from 'react';
import styles from './PlayerStats.module.css';

interface Props {
    table: string[][][];
    highlight: string;
    setHighlight: (player: string) => void;
}

export default function Playerstats({ table, highlight, setHighlight }: Props) {
    const [players, setPlayers] = useState<string[]>([]);

    useEffect(() => {
        const set = new Set(table.flat(2));
        set.delete('');
        setPlayers(Array.from(set));
    }, [table]);

    function count(table: string[][][], player: string) {
        return table.flat(2).filter(p => p === player).length;
    }

    return (
        <>
            {players.map((player) => (
                <div
                    key={'player stats ' + player}
                    className={styles.Player}
                    onPointerEnter={() => setHighlight(player)}
                    onPointerLeave={() => setHighlight('')}>
                    <b className={player === highlight ? styles.Highlight : ''}>
                        {player}
                    </b> : {count(table, player)} games
                </div>
            ))}
        </>
    );
}