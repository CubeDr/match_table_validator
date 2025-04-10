#include "match_table.h"

#include <random>

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

MatchTable::MatchTable(int num_courts, int num_games, const std::vector<Team> &teams)
{
    table_.reserve(num_games);
    for (int i = 0; i < num_games; ++i)
    {
        Row row;
        row.reserve(num_courts);
        for (int j = 0; j < num_courts; ++j)
        {
            row.push_back(/*game=*/{});
        }
        table_.push_back(row);
    }

    std::vector<Player> players = flatten(teams);

    std::shuffle(players.begin(), players.end(), gen);

    int playerIndex = 0;
    for (int rowIndex = 0; rowIndex < table_.size(); ++rowIndex)
    {
        Row &row = table_[rowIndex];
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

const Player &MatchTable::at(int row, int court, int index) const
{
    return table_[row][court][index];
}

const Player &MatchTable::at(int player_index) const
{
    int index = player_index % 4;
    player_index /= 4;

    int court = player_index % courts();
    player_index /= courts();

    int row = player_index;

    return table_[row][court][index];
}

Player &MatchTable::at_internal(int player_index)
{
    int index = player_index % 4;
    player_index /= 4;

    int court = player_index % courts();
    player_index /= courts();

    int row = player_index;

    return table_[row][court][index];
}

const int MatchTable::rows() const
{
    return table_.size();
}

const int MatchTable::courts() const
{
    return table_[0].size();
}

const std::vector<Row> &MatchTable::table() const
{
    return table_;
}

void MatchTable::swap(int player_index_1, int player_index_2)
{
    Player temp = at_internal(player_index_1);
    at_internal(player_index_1) = at(player_index_2);
    at_internal(player_index_2) = temp;
}