//    Kexx2
//    Copyright (C) 2001, 2013 Jon Häggblad
//
//    This file is part of Kexx2.
//
//    Kexx2 is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    Kexx2 is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with Kexx2.  If not, see <http://www.gnu.org/licenses/>.

#ifndef KEXX2_PLAYERSTATE_H
#define KEXX2_PLAYERSTATE_H

#include <cassert>
#include <iostream>
#include <array>
#include "Weapon.h"
#include "Defines.h"

struct KeySet {
    KeySet() = default;
    KeySet(int player) {
        if (player == 1) {
            up = SDLK_UP;
            left = SDLK_LEFT;
            down = SDLK_DOWN;
            right = SDLK_RIGHT;
            fire_main = SDLK_RCTRL;
            fire_extra = SDLK_RSHIFT;
        } else if (player == 2) {
            up = SDLK_w;
            left = SDLK_a;
            down = SDLK_s;
            right = SDLK_d;
            fire_main = SDLK_LSHIFT;
            fire_extra = SDLK_LCTRL;
            //fire_extra = SDLK_GREATER;
        } else {
            std::cout << "Warning: keys not set" << std::endl;
        }
    }

    int left = 0;
    int right = 0;
    int up = 0;
    int down = 0;
    int fire_main = 0;
    int fire_extra = 0;
};

struct Player {
    int energy = 0;
    int energy_max = 0;
    int score = 0;
    int main_weapon_level = 1;
    int extra_weapon_count = 0;
    std::string main_weapon = "Blaster Weapon";
    std::string extra_weapon = "none";
    KeySet keyset;
}; 

class PlayerState final {
public:
    int energy(int player);
    int set_energy(int player, int value);

    int energy_max(int player);
    int set_energy_max(int player, int value);

    int score(int player);
    int set_score(int player, int score);

    int main_weapon_level(int player);
    int set_main_weapon_level(int player, int level);

    int extra_weapon_count(int player);
    int set_extra_weapon_count(int player, int count);

    std::string main_weapon(int player);
    std::string set_main_weapon(int player, std::string s);
    std::string extra_weapon(int player);
    std::string set_extra_weapon(int player, std::string s);

    KeySet keyset(int player);
    KeySet set_keyset(int player, KeySet keyset_);

    bool anyone_alive();
    void kill_all();

private:
    std::array<Player,NUM_OF_POSSIBLE_PLAYERS> players_;
};

#endif // KEXX2_PLAYERSTATE_H
