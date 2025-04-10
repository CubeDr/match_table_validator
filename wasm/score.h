#ifndef MATCH_TABLE_VALIDATOR_WASM_SCORE_H__
#define MATCH_TABLE_VALIDATOR_WASM_SCORE_H__

#include <vector>
#include "match_table.h"

score_t score_games(const MatchTable &match_table, int swap_player_index1 = 0, int swap_player_index2 = 0);

#endif