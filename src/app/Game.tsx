'use client';

import { useCallback, useRef } from 'react';
import styles from './Game.module.css';

interface Props {
    players: string[];
    onPlayersUpdate: (players: string[]) => void;
    highlight: string;
}

export default function Game({ players, onPlayersUpdate, highlight }: Props) {
    const player1Ref = useRef<HTMLInputElement>(null);
    const player2Ref = useRef<HTMLInputElement>(null);
    const player3Ref = useRef<HTMLInputElement>(null);
    const player4Ref = useRef<HTMLInputElement>(null);

    const onValueUpdate = useCallback(() => {
        onPlayersUpdate([
            player1Ref.current!.value.trim(),
            player2Ref.current!.value.trim(),
            player3Ref.current!.value.trim(),
            player4Ref.current!.value.trim(),
        ]);
    }, [onPlayersUpdate]);

    return (
        <div className={styles.Game}>
            <input ref={player1Ref} value={players[0] ?? ''} onChange={onValueUpdate} className={highlight && players[0] === highlight ? styles.Highlight : ''} />
            <input ref={player2Ref} value={players[1] ?? ''} onChange={onValueUpdate} className={highlight && players[1] === highlight ? styles.Highlight : ''} />
            &nbsp;vs&nbsp;
            <input ref={player3Ref} value={players[2] ?? ''} onChange={onValueUpdate} className={highlight && players[2] === highlight ? styles.Highlight : ''} />
            <input ref={player4Ref} value={players[3] ?? ''} onChange={onValueUpdate} className={highlight && players[3] === highlight ? styles.Highlight : ''} />
        </div>
    );
}