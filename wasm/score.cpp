#include "score.h"

#include <unordered_set>
#include <unordered_set>
#include <vector>
#include "types.h"

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

score_t score_level_balance(const Game &game)
{
    int team1Level = game[0].level + game[1].level;
    int team2Level = game[2].level + game[3].level;
    return std::abs(team1Level - team2Level) * 20ull;
}

score_t score_duplicate_player(const std::vector<Player> &players)
{
    std::unordered_set<std::string> names;
    for (const auto &player : players)
    {
        names.insert(player.name);
    }
    if (names.size() != players.size())
    {
        return 100000000000000ull * (players.size() - names.size());
    }
    return 0;
}

score_t score_group_mixing(const Game &game)
{
    score_t penalty = 0;

    std::unordered_map<int, int> groupCounts;
    for (const auto &player : game)
    {
        groupCounts[player.group_id]++;
    }

    for (const auto &pair : groupCounts)
    {
        int count = pair.second;
        if (count == 4)
        {
            penalty += 1000;
        }
        else if (count == 3)
        {
            penalty += 200;
        }
    }
    return penalty;
}

score_t score_game(const Game &game)
{
    score_t balance_score = score_level_balance(game);
    score_t duplicate_player_score = score_duplicate_player(game);
    score_t group_mixing_penalty = score_group_mixing(game);
    return balance_score + duplicate_player_score + group_mixing_penalty;
}

score_t score_rows(const Row &row)
{
    score_t duplicate_player_score = score_duplicate_player(flatten(row));
    return duplicate_player_score;
}

float get_average_level(const Game &game)
{
    float average_level = 0;
    for (const auto &player : game)
    {
        average_level += player.level;
    }
    average_level /= 4;
    return average_level;
}

const float COMPETE_LEVEL_THRESHOLD = 2;

score_t score_players(const std::vector<Row> &games)
{
    std::unordered_map<std::string, PlayerGameStats> stats;

    for (const auto &row : games)
    {
        for (const auto &game : row)
        {
            float average_level = get_average_level(game);
            for (int i = 0; i < game.size(); i++)
            {
                const auto &player = game[i];
                if (stats.find(player.name) == stats.end())
                {
                    stats.insert({player.name, {0, {}, {}}});
                }
                auto &player_stats = stats[player.name];

                // Score game level difficulty.
                if (std::abs(player.level - average_level) <= COMPETE_LEVEL_THRESHOLD)
                {
                    // Competing game
                }
                else if (player.level > average_level)
                {
                    player_stats.with_weak_count++;
                }

                // Score duplicate partners.
                const auto &partner = game[(i / 2) * 2 + (1 - (i % 2))];
                if (player_stats.partner_count.find(partner.name) == player_stats.partner_count.end())
                {
                    player_stats.partner_count.insert({});
                }
                player_stats.partner_count[partner.name]++;

                // Score duplicate players in same game.
                for (int j = 0; j < game.size(); ++j)
                {
                    if (i == j)
                    {
                        continue;
                    }
                    const auto &other = game[j];
                    if (player_stats.players_in_same_game_count.find(other.name) == player_stats.players_in_same_game_count.end())
                    {
                        player_stats.players_in_same_game_count.insert({});
                    }
                    player_stats.players_in_same_game_count[other.name]++;
                }
            }
        }
    }

    score_t score = 0;

    for (const auto &it : stats)
    {
        const auto &stat = it.second;
        score += pow(20, stat.with_weak_count);

        for (const auto &partner_it : stat.partner_count)
        {
            score += pow(5, partner_it.second - 1);
        }
        for (const auto &other_it : stat.players_in_same_game_count)
        {
            score += pow(3, other_it.second - 1);
        }
    }

    return score;
}

score_t score_games(const std::vector<Row> &games)
{
    score_t score = 0;

    for (const auto &row : games)
    {
        score += score_rows(row);
        for (const auto &game : row)
        {
            score += score_game(game);
        }
    }

    score += score_players(games);

    return score;
}