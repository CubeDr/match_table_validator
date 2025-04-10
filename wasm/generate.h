#ifndef MATCH_TABLE_VALIDATOR_WASM_GENERATE_H__
#define MATCH_TABLE_VALIDATOR_WASM_GENERATE_H__

#include <vector>
#include "types.h"

std::vector<Row> generate_matches(const std::vector<Team> teams, int num_courts, int num_games);

#endif