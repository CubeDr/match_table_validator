import { useState } from 'react';
import styles from './MatchTable.module.css';
import { Table } from './page';

interface Props {
    table: Table;
    deleteTable: () => void;
}

export default function MatchTable({ table, deleteTable }: Props) {
    const [hovered, setHovered] = useState<string | null>(null);

    function copy() {
        navigator.clipboard.writeText(JSON.stringify(table));
    }

    return (
        <>
            <div className={styles.ControlRow}>
                <button className={styles.Button} onClick={copy}>Copy</button>
                <button className={styles.Button} onClick={deleteTable}>Delete</button>
            </div>
            <div className={styles.TableWrapper}>
                <table className={styles.Table}>
                    <tbody>
                        {table.map((rowOfGames, rowIndex) => (
                            <tr key={`row-${rowIndex}`}>
                                {rowOfGames.flat().map((player, playerIndex) => (
                                    <td
                                        key={`player-${rowIndex}-${playerIndex}`}
                                        onPointerEnter={() => setHovered(player)}
                                        onPointerLeave={() => setHovered(null)}
                                        className={styles.TableCell
                                            + (playerIndex % 4 === 3 ? ' ' + styles.StrongBorderRight : '')
                                            + (playerIndex % 4 === 1 ? ' ' + styles.DashedBorderRight : '')
                                            + (hovered === player ? ' ' + styles.Hovered : '')}>
                                        {player}
                                    </td>
                                ))}
                            </tr>
                        ))}
                    </tbody>
                </table>
            </div>
        </>
    );
}
