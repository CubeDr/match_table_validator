#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <unordered_set>
#include <unordered_map>
#include <random>
#include <stdexcept> // For std::runtime_error
#include <emscripten/bind.h>
#include <emscripten/val.h>

enum class Gender
{
    MALE = 0,
    FEMALE = 1
};

std::ostream &operator<<(std::ostream &os, Gender gender)
{
    switch (gender)
    {
    case Gender::MALE:
        os << "Male";
        break;
    case Gender::FEMALE:
        os << "Female";
        break;
    default:
        os << "Unknown";
        break;
    }
    return os;
}

struct Player
{
    std::string name;
    int level;
    Gender gender;

    friend std::ostream &operator<<(std::ostream &os, const Player &player)
    {
        os << "{ " << player.name << ", " << player.level << ", " << player.gender << " }";
        return os;
    }
};

typedef std::vector<Player> Team;
typedef std::vector<Player> Game;
typedef std::vector<Game> Row;

struct PlayerGameStats
{
    int with_weak_count;
    int with_similar_count;
    int with_strong_count;
    std::unordered_map<std::string, int> partner_count;
    std::unordered_map<std::string, int> players_in_same_game_count;
};

std::random_device rd;
std::mt19937 gen(rd());

void convert_teams(const emscripten::val &teams_val, std::vector<std::vector<Player>> &teams)
{
    if (!teams_val.isArray())
    {
        throw std::runtime_error("Input 'teams_val' is not an array.");
    }
    size_t num_teams = teams_val["length"].as<size_t>();

    for (size_t i = 0; i < num_teams; ++i)
    {
        emscripten::val team_val = teams_val[i];
        if (!team_val.isArray())
        {
            throw std::runtime_error("Input team element is not an array.");
        }
        size_t num_players = team_val["length"].as<size_t>();
        std::vector<Player> current_team;

        for (size_t j = 0; j < num_players; ++j)
        {
            emscripten::val player_val = team_val[j];

            std::string player_type = player_val.typeOf().as<std::string>();
            if (player_val.isNull() || player_type != "object" || player_val.isArray())
            {
                throw std::runtime_error("Input player element is not a non-null, non-array object (type was: " + player_type + ").");
            }

            Player p;
            p.name = player_val["name"].as<std::string>();
            p.level = player_val["level"].as<int>();

            if (!player_val["gender"].isUndefined())
            {
                int gender_int = player_val["gender"].as<int>();
                if (gender_int == 0)
                    p.gender = Gender::MALE;
                else if (gender_int == 1)
                    p.gender = Gender::FEMALE;
                else
                    throw std::runtime_error("Invalid gender value in player object.");
            }
            else
            {
                throw std::runtime_error("Missing 'gender' property in player object.");
            }

            current_team.push_back(p);
        }
        teams.push_back(current_team);
    }
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

int score_level_balance(const Game &game)
{
    int team1Level = game[0].level + game[1].level;
    int team2Level = game[2].level + game[3].level;
    return abs(team1Level - team2Level) * 20;
}

int score_duplicate_player(const std::vector<Player> &players)
{
    std::unordered_set<std::string> names;
    for (const auto &player : players)
    {
        names.insert(player.name);
    }
    if (names.size() != players.size())
    {
        return 100000 * (players.size() - names.size());
    }
    return 0;
}

int score_game(const Game &game)
{
    int balance_score = score_level_balance(game);
    int duplicate_player_score = score_duplicate_player(game);
    return balance_score + duplicate_player_score;
}

int score_rows(const Row &row)
{
    int duplicate_player_score = score_duplicate_player(flatten(row)) * 10;
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

int score_players(const std::vector<Row> &games)
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
                    stats.insert({player.name, {0, 0, 0, {}}});
                }
                auto &player_stats = stats[player.name];

                // Score game level difficulty.
                if (std::abs(player.level - average_level) <= COMPETE_LEVEL_THRESHOLD)
                {
                    player_stats.with_similar_count++;
                }
                else if (player.level < average_level)
                {
                    player_stats.with_strong_count++;
                }
                else
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

    int score = 0;

    for (const auto &it : stats)
    {
        const auto &stat = it.second;
        score += pow(1.5, stat.with_strong_count);
        score += pow(2, stat.with_weak_count);

        for (const auto &partner_it : stat.partner_count)
        {
            score += pow(3, partner_it.second - 1);
        }
        for (const auto &other_it : stat.players_in_same_game_count)
        {
            score += pow(1.5, other_it.second - 1);
        }
    }

    return score;
}

int score_games(const std::vector<Row> &games)
{
    int score = 0;

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

bool hill_climb_best_among_all(std::vector<Row> &games, int total_length)
{
    int original_score = score_games(games);
    int best_score = original_score;
    int best_index1 = 0;
    int best_index2 = 0;

    for (int index1 = 0; index1 < total_length; ++index1)
    {
        for (int index2 = index1 + 1; index2 < total_length; ++index2)
        {
            swap(games, index1, index2);

            int swapped_score = score_games(games);
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

bool hill_climb_best_among_random(std::vector<Row> &games, int total_length, int iterations)
{
    int original_score = score_games(games);
    int best_score = original_score;
    int best_index1 = 0;
    int best_index2 = 0;

    std::uniform_int_distribution<> distr(0, total_length - 1);
    for (int i = 0; i < iterations; ++i)
    {
        int index1 = distr(gen);
        int index2 = distr(gen);

        swap(games, index1, index2);

        int swapped_score = score_games(games);
        if (swapped_score < best_score)
        {
            best_score = swapped_score;
            best_index1 = index1;
            best_index2 = index2;
        }

        swap(games, index1, index2);
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
    for (int iteration = 0; iteration < max_iterations; ++iteration)
    {
        if (!hill_climb_best_among_random(games, total_length, 10000))
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

std::string to_string(const std::vector<Row> &games)
{
    std::ostringstream oss;

    oss << '[';

    for (size_t i = 0; i < games.size(); ++i)
    {
        const Row &row = games[i];
        if (i > 0)
        {
            oss << ',';
        }

        oss << '[';

        for (size_t j = 0; j < row.size(); ++j)
        {
            const Game &game = row[j];
            if (j > 0)
            {
                oss << ',';
            }

            oss << '[';

            for (size_t k = 0; k < game.size(); ++k)
            {
                const Player &player = game[k];
                if (k > 0)
                {
                    oss << ',';
                }

                oss << '"' << player.name << '"';
            }

            oss << ']';
        }

        oss << ']';
    }

    oss << ']';

    return oss.str();
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