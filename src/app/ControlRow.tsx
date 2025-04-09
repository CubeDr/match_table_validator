import { useState } from 'react';
import styles from './ControlRow.module.css';

interface Props {
    courts: number;
    rows: number;
    onUpdate: (courts: number, rows: number) => void;
}

export default function ControlRow({ courts, rows, onUpdate }: Props) {
    const [courtsInput, setCourtsInput] = useState(courts);
    const [rowsInput, setRowsInput] = useState(rows);

    return (
        <div className={styles.ControlRow}>
            <label>Number of courts: </label>
            <input
                type="number"
                value={courtsInput}
                className={styles.Input}
                onChange={(e) => setCourtsInput(Number(e.target.value))} />

            <label>Number of rows: </label>
            <input
                type="number"
                value={rowsInput}
                className={styles.Input}
                onChange={(e) => setRowsInput(Number(e.target.value))} />

            <button
                className={styles.ApplyButton}
                disabled={courts === courtsInput && rows === rowsInput}
                onClick={() => onUpdate(courtsInput, rowsInput)}>
                Apply
            </button>
        </div>
    );
}