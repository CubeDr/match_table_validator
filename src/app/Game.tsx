'use client';

import { useCallback, useRef } from 'react';

interface Props {
    players: string[];
    onPlayersUpdate: (players: string[]) => void;
}

export default function Game({ players, onPlayersUpdate }: Props) {
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
        <>
            <input ref={player1Ref} value={players[0] ?? ''} onChange={onValueUpdate} />
            <input ref={player2Ref} value={players[1] ?? ''} onChange={onValueUpdate} />
            &nbsp;vs&nbsp;
            <input ref={player3Ref} value={players[2] ?? ''} onChange={onValueUpdate} />
            <input ref={player4Ref} value={players[3] ?? ''} onChange={onValueUpdate} />
        </>
    );
}