'use client';

import { useEffect, useState } from 'react';
import Game from './Game';
import styles from './page.module.css';
import { verify } from './verify/verify';
import { DoesntMeet } from './verify/meet';
import { InARow } from './verify/inarow';
import { Consecutive } from './verify/consecutive';

export default function Home() {
  const [table, setTable] = useState<string[][][]>([
    [['', '', '', ''], ['', '', '', '']],
    [['', '', '', ''], ['', '', '', '']],
    [['', '', '', ''], ['', '', '', '']],
    [['', '', '', ''], ['', '', '', '']],
    [['', '', '', ''], ['', '', '', '']],
    [['', '', '', ''], ['', '', '', '']],
  ]);

  const [doesntMeets, setDoesntMeets] = useState<DoesntMeet[]>([]);
  const [inARows, setInARows] = useState<InARow[]>([]);
  const [consecutives, setConsecutives] = useState<Consecutive[]>([]);

  useEffect(() => {
    const result = verify(table);

    setDoesntMeets(result.doesntMeets);
    setInARows(result.inARows);
    setConsecutives(result.consecutives);
  }, [table]);

  return (
    <div>
      <table className={styles.Table}>
        <thead>
          <tr>
            <td>1코트</td>
            <td>2코트</td>
          </tr>
        </thead>
        <tbody>
          {
            table.map((row, rowIndex) => (
              <tr key={rowIndex + ' tr'} className={inARows.map((inARow) => inARow.row).includes(row) ? styles.InARow : ''}>
                {row.map((players, gameIndex) => (
                  <td key={rowIndex + ' ' + gameIndex + ' td'} className={styles.Game}>
                    <Game players={players} onPlayersUpdate={(newPlayers) => {
                      setTable(prevTable => {
                        const newTable = [...prevTable];
                        newTable[rowIndex] = [...newTable[rowIndex]];
                        newTable[rowIndex][gameIndex] = newPlayers;
                        return newTable;
                      });
                    }} />
                  </td>
                ))}
              </tr>
            ))
          }
        </tbody>
      </table>
      {
        doesntMeets.length > 0 && <>
          {doesntMeets.toSorted((a, b) => b.doesntMeet.length - a.doesntMeet.length)
            .map(({ player, doesntMeet }) => (
              <div key={'doesntMeets ' + player}>
                {player} doesn't meet {doesntMeet.join(', ')}
              </div>
            ))}
        </>
      }
      {
        consecutives.length > 0 && <>
          {
            consecutives.map(({player, rowIndices}) => (
              <div key={'consecutive ' + player}>
                {player} has consecutive games in row {rowIndices.join(', ')}
              </div>
            ))
          }
        </>
      }
    </div>
  );
}
