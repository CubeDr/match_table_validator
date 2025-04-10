#include "conversion.h"

#include <emscripten/val.h>
#include <sstream>
#include <string>
#include <vector>
#include "types.h"

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
            p.group_id = i;

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