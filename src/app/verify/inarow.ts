export interface InARow {
    player: string,
    row: string[][],
}

export function verifyNotInARow(table: string[][][]): InARow[] {
    const players = new Set(table.flat().flat());
    players.delete('');

    const result: InARow[] = [];
    for (const player of players) {
        const row = getInARow(table, player);
        if (row == null) continue;

        result.push({
            player,
            row,
        });
    }

    return result;
}

function getInARow(table: string[][][], player: string): string[][] | null {
    for (const row of table) {
        const count = row.filter((game) => game.includes(player)).length;
        console.log('player', player, 'count', count);
        if (count > 1) {
            return row;
        }
    }
    return null;
}
