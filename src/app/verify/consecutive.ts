export interface Consecutive {
    player: string,
    rowIndices: number[],
}

export function verifyNotConsecutive(table: string[][][]): Consecutive[] {
    const players = new Set(table.flat(2));
    players.delete('');

    const result: Consecutive[] = [];
    for (const player of players) {
        const consecutives = getConsecutives(table, player);
        if (consecutives == null) continue;

        result.push({
            player,
            rowIndices: consecutives,
        });
    }

    return result;
}

function getConsecutives(table: string[][][], player: string): number[] | null {
    const participatingRowIndices = [];
    for (let i = 0; i < table.length; i++) {
        const row = table[i];
        if (!row.some((game) => game.includes(player))) {
            continue;
        }
        participatingRowIndices.push(i);
    }
    if (participatingRowIndices.length === 0) {
        return null;
    }

    const consecutives: number[][] = [];
    let currentConsecutive: number[] = [];
    for (let i = 0; i < participatingRowIndices.length; i++) {
      if (i === 0 || participatingRowIndices[i] === participatingRowIndices[i - 1] + 1) {
        currentConsecutive.push(participatingRowIndices[i]);
      } else {
        if (currentConsecutive.length > 1) {
          consecutives.push(currentConsecutive);
        }
        currentConsecutive = [participatingRowIndices[i]];
      }
    }

    if (currentConsecutive.length > 1) {
      consecutives.push(currentConsecutive);
    }

    if (consecutives.length === 0) {
        return null;
    }

    return consecutives.flat();
}
