import React from 'react';
import { Player } from './player';
import styles from './PlayerInputs.module.css';

const MAX_LEVEL = 20;

interface Props {
    player: Player;
    onPlayerUpdate: (player: Player) => void;
}

export default function PlayerInput({ player, onPlayerUpdate }: Props) {
    const handleNameChange = (event: React.ChangeEvent<HTMLInputElement>) => {
        const newName = event.target.value;
        const updatedPlayer = { ...player, name: newName };
        onPlayerUpdate(updatedPlayer);
    };

    const handleLevelChange = (event: React.ChangeEvent<HTMLInputElement>) => {
        const newLevel = parseInt(event.target.value, 10) || 0;
        const clampedLevel = Math.max(1, Math.min(MAX_LEVEL, newLevel));

        if (!isNaN(newLevel)) {
            const updatedPlayer = { ...player, level: clampedLevel };
            onPlayerUpdate(updatedPlayer);
        } else {
            const updatedPlayer = { ...player, level: player.level };
            onPlayerUpdate(updatedPlayer);
        }
    };

    return (
        <div className={styles.PlayerInput}>
            <input
                type="text"
                value={player.name}
                onChange={handleNameChange}
                placeholder="Name"
                className={styles.NameInput}
            />
            <input
                type="number"
                value={player.level}
                onChange={handleLevelChange}
                min="1"
                max="20"
                className={styles.LevelInput}
            />
        </div>
    );
}
