#include <iostream>
#include <vector>
#include <string>
#include <stdexcept> // For std::runtime_error
#include <emscripten/bind.h>
#include <emscripten/val.h>
#include "conversion.h"
#include "score.h"
#include "types.h"
#include "generate.h"

void print_games(const std::vector<Row> &games)
{
    std::cout << "< score: " << score_games(games) << " >" << std::endl;
    for (const auto &row : games)
    {
        std::cout << "[ ";
        for (const auto &game : row)
        {
            std::cout << "[ ";
            for (const auto &player : game)
            {
                std::cout << player << " ";
            }
            std::cout << "] ";
        }
        std::cout << "]" << std::endl;
    }
}

std::string generate_matches_val(
    emscripten::val teams_val,
    int num_courts,
    int num_games)
{
    std::vector<std::vector<Player>> teams;

    try
    {
        convert_teams(teams_val, teams);
        std::cout << "C++ (using val): Successfully converted " << teams.size() << " teams." << std::endl;
    }
    catch (const std::exception &e)
    {
        // Catch standard C++ exceptions (e.g., from runtime_error)
        std::cerr << "C++ val Conversion/Processing Error: " << e.what() << std::endl;
        return "{ \"status\": \"error\", \"message\": \"C++ data processing failed (std::exception)\" }";
    }
    catch (...)
    {
        // Catch potential JS exceptions leaking from val API calls
        // Note: emscripten::val operations might throw JS exceptions
        std::cerr << "C++ val Conversion/Processing Error: Unknown exception (possibly from JS binding)" << std::endl;
        return "{ \"status\": \"error\", \"message\": \"C++ data processing failed (unknown exception)\" }";
    }

    std::vector<Row> games = generate_matches(teams, num_courts, num_games);
    std::cout << "Final Score: " << score_games(games) << std::endl;

    std::string games_json = to_string(games);
    return "{ \"status\": \"success\", \"result\": " + games_json + " }";
}

EMSCRIPTEN_BINDINGS(match_generator_module)
{
    emscripten::function("generateMatches", &generate_matches_val);
}