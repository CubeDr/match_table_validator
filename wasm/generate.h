#ifndef MATCH_TABLE_VALIDATOR_WASM_GENERATE_H__
#define MATCH_TABLE_VALIDATOR_WASM_GENERATE_H__

#include <vector>
#include "types.h"
#include "match_table.h"

MatchTable generate_match_table(const std::vector<Team> teams, int num_courts, int num_games);

#endif