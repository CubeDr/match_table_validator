#include "generate.h"

#include <algorithm>
#include <numeric>
#include <random>
#include <sstream>
#include <vector>
#include "score.h"
#include "types.h"

std::random_device rd1;
std::mt19937 gen1(rd1());

bool hill_climb_best_among_all(MatchTable &match_table, int total_length)
{
    score_t original_score = score_games(match_table);
    score_t best_score = original_score;
    int best_index1 = 0;
    int best_index2 = 0;

    for (int index1 = 0; index1 < total_length; ++index1)
    {
        for (int index2 = index1 + 1; index2 < total_length; ++index2)
        {
            score_t swapped_score = score_games(match_table, index1, index2);
            if (swapped_score < best_score)
            {
                best_score = swapped_score;
                best_index1 = index1;
                best_index2 = index2;
            }
        }
    }

    if (best_score == original_score)
    {
        return false;
    }

    std::cout << "Swapping " << best_index1 << " and " << best_index2 << " => score: " << best_score << std::endl;
    match_table.swap(best_index1, best_index2);
    return true;
}

bool hill_climb_best_among_random(MatchTable &match_table, int total_length, int iterations)
{
    score_t original_score = score_games(match_table);
    score_t best_score = original_score;
    int best_index1 = 0;
    int best_index2 = 0;

    std::uniform_int_distribution<> distr(0, total_length - 1);
    for (int i = 0; i < iterations; ++i)
    {
        int index1 = distr(gen1);
        int index2 = distr(gen1);

        if (index1 == index2)
        {
            --i;
            continue;
        }

        score_t swapped_score = score_games(match_table, index1, index2);
        if (swapped_score < best_score)
        {
            best_score = swapped_score;
            best_index1 = index1;
            best_index2 = index2;
        }
    }

    if (best_score == original_score)
    {
        return false;
    }

    std::cout << "Swapping " << best_index1 << " and " << best_index2 << " => score: " << best_score << std::endl;
    match_table.swap(best_index1, best_index2);
    return true;
}

void hill_climb(MatchTable &match_table, int total_length)
{
    int repeat = sqrt(total_length);

    std::cout << "Start climbing fast with low precision." << std::endl;
    for (int i = 0; i < repeat; ++i)
    {
        while (hill_climb_best_among_random(match_table, total_length, total_length))
        {
            // no-op
        }
    }

    std::cout << "Start climbing faster with higher precision." << std::endl;
    for (int i = 0; i < repeat; ++i)
    {
        while (hill_climb_best_among_random(match_table, total_length, total_length * (int)sqrt(total_length)))
        {
            // no-op
        }
    }
}

MatchTable generate_match_table(const std::vector<Team> teams, int num_courts, int num_games)
{
    MatchTable match_table(num_courts, num_games, teams);

    hill_climb(match_table, num_courts * num_games * 4);

    return match_table;
}