#ifndef MATCH_TABLE_VALIDATOR_WAS_TYPES_H__
#define MATCH_TABLE_VALIDATOR_WAS_TYPES_H__

#include <iostream>

enum class Gender
{
    MALE = 0,
    FEMALE = 1
};

std::ostream &operator<<(std::ostream &os, Gender gender);

struct Player
{
    std::string name;
    int level;
    Gender gender;
    int group_id;

    friend std::ostream &operator<<(std::ostream &os, const Player &player)
    {
        os << "{ " << player.name << ", " << player.level << ", " << player.gender << " }";
        return os;
    }
};

typedef std::vector<Player> Team;
typedef unsigned long long score_t;

struct PlayerGameStats
{
    int with_weak_count;
    std::unordered_map<std::string, int> partner_count;
    std::unordered_map<std::string, int> players_in_same_game_count;
};

#endif
