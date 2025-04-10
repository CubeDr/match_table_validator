#include "generate.h"

#include <algorithm>
#include <atomic>
#include <mutex>
#include <numeric>
#include <random>
#include <sstream>
#include <thread>
#include <vector>
#include "score.h"
#include "types.h"

std::random_device rd1;
std::mt19937 gen1(rd1());

struct ThreadResult
{
    score_t best_score = std::numeric_limits<score_t>::max();
    int best_index1 = -1;
    int best_index2 = -1;
};

void find_best_swap_for_range(
    const MatchTable &match_table,
    int start_index1,
    int end_index1,
    int total_length,
    ThreadResult &result)
{
    score_t local_best_score = std::numeric_limits<score_t>::max();
    int local_best_index1 = -1;
    int local_best_index2 = -1;

    for (int index1 = start_index1; index1 < end_index1; ++index1)
    {
        for (int index2 = index1 + 1; index2 < total_length; ++index2)
        {
            score_t swapped_score = score_games(match_table, index1, index2);

            if (swapped_score < local_best_score)
            {
                local_best_score = swapped_score;
                local_best_index1 = index1;
                local_best_index2 = index2;
            }
        }
    }

    result.best_score = local_best_score;
    result.best_index1 = local_best_index1;
    result.best_index2 = local_best_index2;
}

size_t get_number_of_threads()
{
    size_t num_threads = std::thread::hardware_concurrency();

    if (num_threads == 0)
    {
        return 1;
    }

    return 8;
}

bool parallel_hill_climb_best_among_all(MatchTable &match_table, int num_threads, int total_length)
{
    score_t original_score = score_games(match_table);

    std::vector<std::thread> threads;
    std::vector<ThreadResult> results(num_threads);

    int range_size = total_length / num_threads;
    int remainder = total_length % num_threads;
    int current_start_index = 0;

    for (unsigned int i = 0; i < num_threads; ++i)
    {
        int current_end_index = current_start_index + range_size + (i < remainder ? 1 : 0);
        current_end_index = std::min(current_end_index, total_length);

        if (current_start_index < current_end_index)
        {
            threads.emplace_back(
                find_best_swap_for_range,
                std::cref(match_table),
                current_start_index,
                current_end_index,
                total_length,
                std::ref(results[i]));
        }
        current_start_index = current_end_index;
    }

    for (auto &t : threads)
    {
        if (t.joinable())
        {
            t.join();
        }
    }

    score_t overall_best_score = original_score;
    int overall_best_index1 = -1;
    int overall_best_index2 = -1;

    for (const auto &result : results)
    {
        if (result.best_score < overall_best_score)
        {
            overall_best_score = result.best_score;
            overall_best_index1 = result.best_index1;
            overall_best_index2 = result.best_index2;
        }
    }

    if (overall_best_score >= original_score || overall_best_index1 == -1)
    {
        return false;
    }

    std::cout << "Climbing to the next best game: " << overall_best_score << std::endl;
    match_table.swap(overall_best_index1, overall_best_index2);
    return true;
}

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

void hill_climb(MatchTable &match_table, int total_length, int max_iterations)
{
    size_t num_threads = get_number_of_threads();
    for (int iteration = 0; iteration < max_iterations; ++iteration)
    {
        // if (!parallel_hill_climb_best_among_all(match_table, num_threads, total_length))
        // if (!hill_climb_best_among_all(match_table, total_length))
        if (!hill_climb_best_among_random(match_table, total_length, 10000))
        {
            break;
        }
    }
}

MatchTable generate_match_table(const std::vector<Team> teams, int num_courts, int num_games)
{
    MatchTable match_table(num_courts, num_games, teams);

    hill_climb(match_table, num_courts * num_games * 4, 60);

    return match_table;
}