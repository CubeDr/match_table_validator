export interface DoesntMeet {
    player: string,
    doesntMeet: string[],
}

export function verifyMeets(table: string[][][]): DoesntMeet[] {
    const players = new Set(table.flat().flat());
    players.delete('');

    const result: DoesntMeet[] = [];
    for (const player of players) {
        const doesntMeet = getNotMeetingPlayers(table, players, player);
        if (doesntMeet.length === 0) continue;

        result.push({
            player,
            doesntMeet,
        });
    }

    return result
}

function getNotMeetingPlayers(table: string[][][], players: Set<string>, player: string): string[] {
    const allPlayers = new Set(players);
    allPlayers.delete(player);

    for (const row of table) {
        for (const game of row) {
            if (!game.includes(player)) {
                continue;
            }

            for (const other of game) {
                allPlayers.delete(other);
            }
        }
    }

    return Array.from(allPlayers);
}