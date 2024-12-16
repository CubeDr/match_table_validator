'use client';

import { useState } from 'react';
import Game from './Game';
import styles from './page.module.css';

export default function Home() {
  const [table, setTable] = useState<string[][][]>([
    [['', '', '', ''], ['', '', '', '']],
    [['', '', '', ''], ['', '', '', '']],
    [['', '', '', ''], ['', '', '', '']],
    [['', '', '', ''], ['', '', '', '']],
    [['', '', '', ''], ['', '', '', '']],
    [['', '', '', ''], ['', '', '', '']],
  ]);

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
              <tr>
                {row.map((players, gameIndex) => (
                  <td className={styles.Game}>
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
    </div>
  );
}
