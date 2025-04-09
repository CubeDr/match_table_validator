import styles from './MatchTable.module.css';
import { Table } from './page';

interface Props {
    table: Table;
}

export default function MatchTable({ table }: Props) {
    return (
        <div className={styles.TableWrapper}>
            <table className={styles.Table}>
                <tbody>
                    {table.map((rowOfGames, rowIndex) => (
                        <tr key={`row-${rowIndex}`}>
                            {rowOfGames.flat().map((player, playerIndex) => (
                                <td
                                    key={`player-${rowIndex}-${playerIndex}`}
                                    className={styles.TableCell
                                        + (playerIndex % 4 === 3 ? ' ' + styles.StrongBorderRight : '')
                                        + (playerIndex % 4 === 1 ? ' ' + styles.DashedBorderRight : '')}>
                                    {player}
                                </td>
                            ))}
                        </tr>
                    ))}
                </tbody>
            </table>
        </div>
    );
}
