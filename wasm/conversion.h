#ifndef MATCH_TABLE_VALIDATOR_WASM_CONVERSION_H__
#define MATCH_TABLE_VALIDATOR_WASM_CONVERSION_H__

#include <vector>
#include <string>
#include <emscripten/val.h>
#include "types.h"

void convert_teams(const emscripten::val &teams_val, std::vector<std::vector<Player>> &teams);

std::string to_string(const std::vector<Row> &games);

#endif