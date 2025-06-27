#include "score.h"

#include <unordered_set>
#include <unordered_set>
#include <vector>
#include "types.h"

inline int partner_index(int i)
{
    return (i / 2) * 2 + (1 - (i % 2));
}

class SwappedMatchTable
{
public:
    SwappedMatchTable(const MatchTable &match_table, int index_1, int index_2)
        : match_table(match_table), index_1(index_1), index_2(index_2) {}

    const Player &at(int row, int court, int index) const
    {
        return at((row * courts() + court) * 4 + index);
    }

    const Player &at(int player_index) const
    {
        return match_table.at(swapped_index(player_index));
    }

    int rows() const
    {
        return match_table.rows();
    }

    int courts() const
    {
        return match_table.courts();
    }

private:
    const MatchTable &match_table;
    const int index_1;
    const int index_2;

    int swapped_index(int index) const
    {
        if (index == index_1)
        {
            return index_2;
        }
        else if (index == index_2)
        {
            return index_1;
        }
        else
        {
            return index;
        }
    }
};

score_t score_level_balance(const SwappedMatchTable &match_table, int row, int court)
{
    int team1 = match_table.at(row, court, 0).level + match_table.at(row, court, 1).level;
    int team2 = match_table.at(row, court, 2).level + match_table.at(row, court, 3).level;
    return std::abs(team1 - team2) * 20ull;
}

score_t score_duplicate_player(const SwappedMatchTable &match_table, int player_index_start, int player_index_end)
{
    std::unordered_set<std::string> names;
    for (int i = player_index_start; i < player_index_end; ++i)
    {
        names.insert(match_table.at(i).name);
    }

    int expected_size = player_index_end - player_index_start;
    return 100000000000000ull * (expected_size - names.size());
}

score_t score_group_mixing(const SwappedMatchTable &match_table, int row, int court)
{
    std::unordered_map<int, int> group_counts;
    const auto& player1 = match_table.at(row, court, 0);
    const auto& player2 = match_table.at(row, court, 1);
    const auto& player3 = match_table.at(row, court, 2);
    const auto& player4 = match_table.at(row, court, 3);

    if (player1.group_id != player2.group_id || player3.group_id != player4.group_id) {
        return 1000000000000000ull;
    }

    return 0;
}

score_t score_game(const SwappedMatchTable &match_table, int row, int court)
{
    score_t balance_score = score_level_balance(match_table, row, court);

    int player_start_index = (row * match_table.courts() + court) * 4;
    int player_end_index = (row * match_table.courts() + court + 1) * 4;
    score_t duplicate_player_score = score_duplicate_player(match_table, player_start_index, player_end_index);

    score_t group_mixing_score = score_group_mixing(match_table, row, court);

    return balance_score + duplicate_player_score + group_mixing_score;
}

score_t score_rows(const SwappedMatchTable &match_table, int row)
{
    int players_in_row = match_table.courts() * 4;
    score_t duplicate_player_score = score_duplicate_player(match_table, row * players_in_row, (row + 1) * players_in_row);
    return duplicate_player_score;
}

float get_average_level(const SwappedMatchTable &match_table, int row, int court)
{
    float average_level = 0;
    for (int i = 0; i < 4; i++)
    {
        average_level += match_table.at(row, court, i).level;
    }
    average_level /= 4;
    return average_level;
}

const float COMPETE_LEVEL_THRESHOLD = 2;

score_t score_players(const SwappedMatchTable &match_table)
{
    std::unordered_map<std::string, PlayerGameStats> stats;

    for (int row = 0; row < match_table.rows(); ++row)
    {
        for (int court = 0; court < match_table.courts(); ++court)
        {

            float average_level = get_average_level(match_table, row, court);
            for (int i = 0; i < 4; ++i)
            {
                const auto &player = match_table.at(row, court, i);
                const auto &partner = match_table.at(row, court, partner_index(i));
                auto &player_stats = stats[player.name];

                // Score game level difficulty.
                if (player.level >= average_level + COMPETE_LEVEL_THRESHOLD)
                {
                    player_stats.with_weak_count++;
                }

                if (player.level > partner.level)
                {
                    player_stats.with_weak_partner_level_diff_score += pow(1.3, player.level - partner.level);
                }

                // Score duplicate partners.
                player_stats.partner_count[partner.name]++;

                // Score duplicate players in same game.
                for (int j = 0; j < 4; ++j)
                {
                    if (i == j)
                    {
                        continue;
                    }
                    const auto &other = match_table.at(row, court, j);
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
        score += stat.with_weak_partner_level_diff_score;

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

score_t score_games(const MatchTable &match_table, int swap_player_index1, int swap_player_index2)
{
    SwappedMatchTable swapped_match_table(match_table, swap_player_index1, swap_player_index2);

    score_t score = 0;

    for (int row = 0; row < swapped_match_table.rows(); ++row)
    {
        score += score_rows(swapped_match_table, row);

        for (int court = 0; court < swapped_match_table.courts(); ++court)
        {
            score += score_game(swapped_match_table, row, court);
        }
    }

    score += score_players(swapped_match_table);

    return score;
}