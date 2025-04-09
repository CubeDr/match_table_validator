'use client';

import { useEffect, useState } from 'react';
import Game from './Game';
import styles from './page.module.css';
import { verify } from './verify/verify';
import { DoesntMeet } from './verify/meet';
import { InARow } from './verify/inarow';
import { Consecutive } from './verify/consecutive';
import Playerstats from './PlayeStats';
import ControlRow from './ControlRow';

export default function Home() {
  const [courts, setCourts] = useState(2);
  const [rows, setRows] = useState(6);

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

  const [highlight, setHighlight] = useState<string>('');

  useEffect(() => {
    if (table.length === rows && table[0].length === courts) {
      return;
    }

    const newTable = table.slice(0, rows);
    while (newTable.length < rows) {
      newTable.push([]);
    }

    for (const rows of newTable) {
      while (rows.length > courts) {
        rows.pop();
      }
      while (rows.length < courts) {
        rows.push(['', '', '', '']);
      }
    }
    setTable(newTable);
  }, [courts, rows, table]);

  useEffect(() => {
    const result = verify(table);

    setDoesntMeets(result.doesntMeets);
    setInARows(result.inARows);
    setConsecutives(result.consecutives);
  }, [table]);

  const handleDragStart = (e: React.DragEvent<HTMLTableDataCellElement>, rowIndex: number, gameIndex: number) => {
    e.dataTransfer.setData('text/plain', JSON.stringify({ rowIndex, gameIndex }));
  };

  const handleDrop = (e: React.DragEvent<HTMLTableDataCellElement>, rowIndex: number, gameIndex: number) => {
    e.preventDefault();
    const source = JSON.parse(e.dataTransfer.getData('text/plain'));

    console.log('handleDrop ', rowIndex, gameIndex, source);

    setTable(prevTable => {
      const newTable = JSON.parse(JSON.stringify(prevTable));
      [newTable[source.rowIndex][source.gameIndex], newTable[rowIndex][gameIndex]] =
        [prevTable[rowIndex][gameIndex], prevTable[source.rowIndex][source.gameIndex]];
      return newTable;
    });
  };

  const handleDragOver = (e: React.DragEvent<HTMLTableDataCellElement>) => {
    e.preventDefault();
  };

  return (
    <div>
      <ControlRow courts={courts} rows={rows} onUpdate={(courts, rows) => {
        setCourts(courts);
        setRows(rows);
      }} />
      <table className={styles.Table}>
        <thead>
          <tr>
            {new Array(courts).fill(1).map((_, i) => <td>Court {i+1}</td>)}
          </tr>
        </thead>
        <tbody>
          {
            table.map((row, rowIndex) => (
              <tr key={rowIndex + ' tr'} className={inARows.map((inARow) => inARow.row).includes(row) ? styles.InARow : ''}>
                {row.map((players, gameIndex) => (
                  <td
                    key={rowIndex + ' ' + gameIndex + ' td'}
                    draggable
                    onDragStart={(e) => handleDragStart(e, rowIndex, gameIndex)}
                    onDrop={(e) => handleDrop(e, rowIndex, gameIndex)}
                    onDragOver={handleDragOver}>
                    <Game
                      players={players}
                      onPlayersUpdate={(newPlayers) => {
                        setTable(prevTable => {
                          const newTable = [...prevTable];
                          newTable[rowIndex] = [...newTable[rowIndex]];
                          newTable[rowIndex][gameIndex] = newPlayers;
                          return newTable;
                        });
                      }}
                      highlight={highlight} />
                  </td>
                ))}
              </tr>
            ))
          }
        </tbody>
      </table>
      <hr />
      <Playerstats table={table} highlight={highlight} setHighlight={setHighlight} />
      <hr />
      {
        consecutives.length > 0 && <>
          {
            consecutives.map(({ player, rowIndices }) => (
              <div key={'consecutive ' + player}>
                {player} has consecutive games in row {rowIndices.join(', ')}
              </div>
            ))
          }
        </>
      }
      <br />
      {
        doesntMeets.length > 0 && <>
          {doesntMeets.toSorted((a, b) => b.doesntMeet.length - a.doesntMeet.length)
            .map(({ player, doesntMeet }) => (
              <div key={'doesntMeets ' + player}>
                {player} doesn&apos;t meet {doesntMeet.join(', ')}
              </div>
            ))}
        </>
      }
      <button onClick={() => {
        navigator.clipboard.writeText(JSON.stringify(table));
      }}>Copy</button>
      <button onClick={() => {
        const input = window.prompt('json');
        if (input == null) return;
        setTable(JSON.parse(input));
      }}>Paste</button>
    </div>
  );
}
