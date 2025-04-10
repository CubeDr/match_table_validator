#ifndef __MATCH_TABLE_VALIDATOR_WASM_MATCH_TABLE_H__
#define __MATCH_TABLE_VALIDATOR_WASM_MATCH_TABLE_H__

#include <vector>
#include "types.h"

typedef std::vector<Player> Game;
typedef std::vector<Game> Row;

class MatchTable
{
public:
    MatchTable(int num_courts, int num_rows, const std::vector<Team> &teams);

    const Player &at(int row, int court, int index) const;
    const Player &at(int player_index) const;
    const int rows() const;
    const int courts() const;

    const std::vector<Row> &table() const;

    void swap(int player_index1, int player_index2);

private:
    std::vector<Row> table_;

    Player &at_internal(int player_index);
};

#endif