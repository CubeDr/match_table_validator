import React from 'react';
import { Player, Gender } from './player';
import styles from './PlayerInputs.module.css';

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
        const clampedLevel = Math.max(1, Math.min(10, newLevel));

        if (!isNaN(newLevel)) {
            const updatedPlayer = { ...player, level: clampedLevel };
            onPlayerUpdate(updatedPlayer);
        } else {
            const updatedPlayer = { ...player, level: player.level };
            onPlayerUpdate(updatedPlayer);
        }
    };

    const handleGenderChange = (event: React.ChangeEvent<HTMLSelectElement>) => {
        const selectedGenderString = event.target.value;
        const newGender = Gender[selectedGenderString as keyof typeof Gender];

        if (newGender !== undefined) {
            const updatedPlayer = { ...player, gender: newGender };
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
            />
            <input
                type="number"
                value={player.level}
                onChange={handleLevelChange}
                min="1"
                max="10"
            />
            <select
                value={Gender[player.gender]}
                onChange={handleGenderChange}
            >
                {Object.keys(Gender).filter(key => isNaN(Number(key))).map((genderKey) => (
                    <option key={genderKey} value={genderKey}>
                        {genderKey.charAt(0).toUpperCase() + genderKey.slice(1).toLowerCase()}
                    </option>
                ))}
            </select>
        </div>
    );
}
