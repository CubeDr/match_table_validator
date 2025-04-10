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

std::random_device rd;
std::mt19937 gen(rd());

template <typename T>
std::vector<T> flatten(std::vector<std::vector<T>> vector)
{
    std::vector<T> result;
    for (auto &v : vector)
    {
        result.insert(result.end(), v.begin(), v.end());
    }
    return result;
}

void prepare_matches(std::vector<Row> &games, int num_courts, int num_games)
{
    games.reserve(num_games);
    for (int i = 0; i < num_games; ++i)
    {
        Row row;
        row.reserve(num_courts);
        for (int j = 0; j < num_courts; j++)
        {
            row.push_back(/*game=*/{});
        }
        games.push_back(row);
    }
}

void place_players_randomly(std::vector<Row> &games, const std::vector<Team> teams)
{
    std::vector<Player> players = flatten(teams);

    std::shuffle(players.begin(), players.end(), gen);

    int playerIndex = 0;
    for (int rowIndex = 0; rowIndex < games.size(); ++rowIndex)
    {
        Row &row = games[rowIndex];
        for (int gameIndex = 0; gameIndex < row.size(); ++gameIndex)
        {
            Game &game = row[gameIndex];
            game.clear();
            game.reserve(4);
            for (int i = 0; i < 4; i++)
            {
                game.push_back(players[playerIndex++]);

                if (playerIndex == players.size())
                {
                    std::shuffle(players.begin(), players.end(), gen);
                    playerIndex = 0;
                }
            }
        }
    }
}

Player &at(std::vector<Row> &games, int index)
{
    for (auto &row : games)
    {
        int playersInThisRow = row.size() * 4;
        if (index >= playersInThisRow)
        {
            index -= playersInThisRow;
            continue;
        }

        return row[index / 4][index % 4];
    }
    throw std::runtime_error("Invalid index passed to at().");
}

void swap(std::vector<Row> &games, int index1, int index2)
{
    Player &player1 = at(games, index1);
    Player &player2 = at(games, index2);

    Player temp = player1;
    player1 = player2;
    player2 = temp;
}

struct ThreadResult
{
    score_t best_score = std::numeric_limits<score_t>::max();
    int best_index1 = -1;
    int best_index2 = -1;
};

void find_best_swap_for_range(
    const std::vector<Row> &original_games,
    int start_index1,
    int end_index1,
    int total_length,
    ThreadResult &result)
{
    std::vector<Row> local_games = original_games;
    score_t local_best_score = std::numeric_limits<score_t>::max();
    int local_best_index1 = -1;
    int local_best_index2 = -1;

    for (int index1 = start_index1; index1 < end_index1; ++index1)
    {
        for (int index2 = index1 + 1; index2 < total_length; ++index2)
        {
            swap(local_games, index1, index2);
            score_t swapped_score = score_games(local_games);

            if (swapped_score < local_best_score)
            {
                local_best_score = swapped_score;
                local_best_index1 = index1;
                local_best_index2 = index2;
            }

            swap(local_games, index1, index2);
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

bool parallel_hill_climb_best_among_all(std::vector<Row> &games, int num_threads, int total_length)
{
    score_t original_score = score_games(games);

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
                std::cref(games),
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
    swap(games, overall_best_index1, overall_best_index2);
    return true;
}

bool hill_climb_best_among_all(std::vector<Row> &games, int total_length)
{
    score_t original_score = score_games(games);
    score_t best_score = original_score;
    int best_index1 = 0;
    int best_index2 = 0;

    for (int index1 = 0; index1 < total_length; ++index1)
    {
        for (int index2 = index1 + 1; index2 < total_length; ++index2)
        {
            swap(games, index1, index2);

            score_t swapped_score = score_games(games);
            if (swapped_score < best_score)
            {
                best_score = swapped_score;
                best_index1 = index1;
                best_index2 = index2;
            }

            swap(games, index1, index2);
        }
    }

    if (best_score == original_score)
    {
        return false;
    }

    std::cout << "Climbing to the next best game: " << best_score << std::endl;
    swap(games, best_index1, best_index2);
    return true;
}

void hill_climb(std::vector<Row> &games, int total_length, int max_iterations)
{
    size_t num_threads = get_number_of_threads();
    for (int iteration = 0; iteration < max_iterations; ++iteration)
    {
        // if (!parallel_hill_climb_best_among_all(games, num_threads, total_length))
        // {
        //     break;
        // }
        if (!hill_climb_best_among_all(games, total_length))
        {
            break;
        }
    }
}

std::vector<Row> generate_matches(const std::vector<Team> teams, int num_courts, int num_games)
{
    std::vector<Row> games;
    prepare_matches(games, num_courts, num_games);

    place_players_randomly(games, teams);
    std::cout << "Initial score: " << score_games(games) << std::endl;

    hill_climb(games, num_courts * num_games * 4, 60);

    return games;
}