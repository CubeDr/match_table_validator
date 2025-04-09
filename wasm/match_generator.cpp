#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <random>
#include <stdexcept> // For std::runtime_error
#include <emscripten/bind.h>
#include <emscripten/val.h>

enum class Gender
{
    MALE = 0,
    FEMALE = 1
};

std::ostream& operator<<(std::ostream& os, Gender gender) {
    switch (gender) {
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

void generate_random_games(std::vector<Row> &games, const std::vector<Team> teams)
{
    std::random_device rd;
    std::mt19937 gen(rd());

    std::vector<Player> players = flatten(teams);

    for (const auto& player : players) {
        std::cout << player << ", ";
    }
    std::cout << std::endl;

    std::shuffle(players.begin(), players.end(), gen);

    for (const auto& player : players) {
        std::cout << player << ", ";
    }
    std::cout << std::endl;
}

std::vector<Row> generate_matches(const std::vector<Team> teams, int num_courts, int num_games)
{
    std::vector<Row> games;
    prepare_matches(games, num_courts, num_games);
    generate_random_games(games, teams);
    return games;
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

    generate_matches(teams, num_courts, num_games);

    return "success";
}

EMSCRIPTEN_BINDINGS(match_generator_module)
{
    emscripten::function("generateMatches", &generate_matches_val);
}