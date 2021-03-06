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

#ifndef KEXX2_OBJECTMANAGER_H
#define KEXX2_OBJECTMANAGER_H

#include <list>
#include <vector>
#include <memory>
#include "SDLc.h"
#include "Defines.h"
#include "IObject.h"
#include "EnumHash.h"

class FxManager;
class PlayerState;
class Weapon;

// Hash functions so that we can use enum classes as keys for
// std::unordered_map<T>
typedef EnumHash<ObjIndex> OHash;
typedef EnumHash<ObjSnd> SHash;

class ObjectManager final {
public:
    void load_data(std::string data_path);

    void update(sdlc::Timer&, FxManager&, float world_y_pos, PlayerState&);
    void draw(sdlc::Screen&);

    std::shared_ptr<IObject> create_object(int x, int y, float x_vel, 
                                          float y_vel, ObjIndex, Owner);
    std::shared_ptr<IObject> create_object(int x, int y, ObjIndex, float vel, 
                                          float angle, Owner);
    void create_ships(PlayerState&);

    int num_of_enemies();
    int num_of_players_alive();

    // The main list of objects.
    std::list<std::shared_ptr<IObject>> list;       // object list

    // Holds the shared graphics/sound data for the objects
    std::unordered_map<ObjIndex,sdlc::Surface,OHash,OHash> obj;    
    std::unordered_map<ObjIndex,sdlc::Surface,OHash,OHash> obj_hit;
    std::unordered_map<ObjSnd,sdlc::Sound,SHash,SHash> snd; 

private:
    std::shared_ptr<IObject> allocate_object(ObjIndex object, Owner owner);
    void create_formation(int x, int y, float x_vel, float y_vel, ObjIndex);

    std::unique_ptr<Weapon> create_main_weapon(int player, PlayerState);
    std::unique_ptr<Weapon> create_extra_weapon(int player, PlayerState);

    // Update the player_state to reflect the actual state
    void update_all_player_state(PlayerState&);
    void update_player_state(std::shared_ptr<IObject>&, PlayerState&);

    // Remove objects with energy <= 0
    void flush_list();

    // Add inactive objects (typically loaded from level file) to the list
    // of active objectes.
    void add_from_queue();

    void update_enemy_count();

    // TODO: should switch to std::priority_queue or something similar.
    std::list<std::shared_ptr<IObject>> queue;
    int enemies_in_list_ = 0;
    int players_alive_ = 0;
    float world_y_pos_ = 0;
};

#endif
