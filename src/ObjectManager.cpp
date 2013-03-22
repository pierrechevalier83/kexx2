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

#include "ObjectManager.h"

#include <cmath>
#include <list>
#include <iostream>
#include <algorithm>

#include "SDLc.h"

#include "Ship.h"
#include "EnemySideways.h"
#include "EnemyRammer.h"
#include "EnemyBonus.h"
#include "ShotRocket.h"
#include "WeaponBlaster.h"
#include "WeaponRocket.h"
#include "PlayerState.h"

// -----------------------------------------------------------------------------
// Construction/Destruction
// -----------------------------------------------------------------------------

ObjectManager::~ObjectManager()
{
    std::for_each(begin(list), end(list), [](Object* i) { delete i; });
    std::for_each(begin(queue), end(queue), [](Object* i) { delete i; });
}

// -----------------------------------------------------------------------------
// Member Functions
// -----------------------------------------------------------------------------

void ObjectManager::load_data(std::string data_path)
{
    // load objects
    obj[PLAYER1].load(data_path + "gfx/Ship1.png");
    obj[PLAYER2].load(data_path + "gfx/Ship2.png");
    obj[ENEMYSTD].load(data_path + "gfx/EnemyStd.png");
    obj[ENEMYSIDEWAYS].load(data_path + "gfx/EnemySideways.png");
    obj[ENEMYRAMMER].load(data_path + "gfx/EnemyRammer.png");
    obj[ENEMYBONUS].load(data_path + "gfx/EnemyBonus.png");
    obj[OBJECTBIGSHIP].load(data_path + "gfx/ObjectBigship.png");
    obj[BONUSBLASTER].load(data_path + "gfx/BonusBlaster.png");
    obj[BONUSROCKET].load(data_path + "gfx/BonusRocket.png");

    // shots (for the weapons)
    obj[SHOTBLASTER].load(data_path + "gfx/ShotBlaster.png");
    obj[SHOTBLASTERBIG].load(data_path + "gfx/ShotBlasterBig.png");
    obj[SHOTROCKET].load(data_path + "gfx/ShotRocket.png");
    obj[SHOTBOMBFRAGMENT].alloc(2, 2);
    obj[SHOTBOMBFRAGMENT].fill_rect(0, 0, 2, 2, 100, 100, 255);
    obj[SHOTENEMYSTD].load(data_path + "gfx/ShotStd2.png");
    snd[SND_SHOTBLASTER].load(data_path + "soundfx/shoot1.wav");
    snd[SND_SHOTROCKET].load(data_path + "soundfx/rocketshot.wav");

    // misc
    obj[SMOKETRAIL].load(data_path + "gfx/Flame.png");
}

void ObjectManager::update(sdlc::Timer& timer, FxManager& fx_manager, 
                           float world_y_pos, PlayerState& player_state)
{
    world_y_pos_ = world_y_pos;

    for (auto current : list) {
        if (current->energy()) {

            // main functions
            current->think(*this, fx_manager);
            current->update(timer);
            current->check_collisions(*this, fx_manager);

            // kill objects who move outside their allowed area
            if (current->y() > 480) {
                ObjType t = current->type();
                if (t == OBJ_ENEMY || t == OBJ_PASSIVE 
                        || t == OBJ_BONUS || t == OBJ_SHOT) {
                    current->set_energy(0);
                }
            } else if (current->type() == OBJ_SHOT) {
                if (current->getY() < 0 - current->getHeight() 
                        || current->getX() > 640 
                        || current->getX() < 0 - current->getWidth()) {
                    current->set_energy(0);
                }
            }
            // debug
            if (current->type() == OBJ_UNDEFINED) {
                std::cout << "warning: object with undefined " 
                    << "type detected!" << std::endl;
            }
            if (current->name == "Generic Object") {
                std::cout << "warning: object with undefined " 
                    << "name detected!" << std::endl;
            }
        }
    }

    // Run maintenance functions
    update_player_state(player_state);
    flush_list();               // flush dead objects
    add_from_queue();           // add the new objects
    update_enemy_count();       // update/count how many enemies
}

void ObjectManager::draw(sdlc::Screen& screen)
{
    for (Object* current : list) 
        screen.blit(*current);
}

Object* ObjectManager::create_object(int x, int y, float x_vel, float y_vel, 
                                     ObjIndex object, Owner owner)
{
    // TODO: Redo to use a Factory pattern.
    // TODO: Read all parameters from file.
    
    Object* new_obj = nullptr;

    // Create formation
    if (object >= ENEMYSTD_V_FORMATION)
        create_formation(x, y, x_vel, y_vel, object);

    // Create single entity
    else {
        Object* new_obj = allocate_object(object, owner);

        new_obj->set_x(x - new_obj->width() / 2);
        new_obj->set_y(y - new_obj->height() / 2 + world_y_pos_);
        new_obj->set_x_vel(x_vel);
        new_obj->set_y_vel(y_vel);

        queue.push_back(new_obj);
    }
    return new_obj;
}

Object* ObjectManager::create_object(int x, int y, ObjIndex object, 
                                     float vel, float angle, Owner owner)
{
    angle = angle * (3.1415927f / 180.0f);

    float x_vel = cos(angle) * vel;
    float y_vel = -sin(angle) * vel;
    return (create_object(x, y, x_vel, y_vel, object, owner));
}

void ObjectManager::create_ships(PlayerState& player_state)
{
    for (int i = 1; i <= NUM_OF_POSSIBLE_PLAYERS; i++) {
        if (player_state.energy_max(i)) {

            std::string name = "Player " + std::to_string(i);

            int s = player_state.score(i);
            int e = player_state.energy_max(i);
            player_state.set_energy(i, e);  // reset energy to max

            sdlc::Surface& gfx = obj[PLAYER1 + i - 1];

            auto w1 = create_main_weapon(i, player_state);
            auto w2 = create_extra_weapon(i, player_state);

            KeySet keyset(i);   // For now we dont set custom keys

            Object* new_ship = new Ship(name, e, s, gfx, w1, w2, keyset);

            new_ship->set_locked_to_screen(false);
            new_ship->set_x(213 * i - new_ship->width());
            new_ship->set_y(480);
            new_ship->set_vel(0, -200.0f);
            queue.push_back(new_ship);
        }
    }
}

int ObjectManager::num_of_enemies()
{
    return amount_of_enemies_in_list_;
}

int ObjectManager::num_of_players_alive()
{
    return players_alive_;
}

// -----------------------------------------------------------------------------
// Private Functions
// -----------------------------------------------------------------------------

Object* ObjectManager::allocate_object(ObjIndex object, Owner owner)
{
    Object* new_obj = nullptr;
    sdlc::Surface& gfx = obj[object];

    switch (object) {

    // enemies
    case ENEMYSTD:
        new_obj = new Object("Standard Enemy", 5, 52, gfx, OBJ_ENEMY, 
                             SCROLLING_SPEED);
        break;
    case ENEMYSIDEWAYS:
        new_obj = new EnemySideways("Sideways Enemy", 3, 71, gfx);
        break;
    case ENEMYRAMMER:
        new_obj = new EnemyRammer("Rammer Enemy", 2, 43, gfx);
        break;
    case ENEMYBONUS:
        new_obj = new EnemyBonus("Bonus Enemy", 10, gfx);
        new_obj->init_animation(50, 2, 0);
        break;

    // passive objects
    case OBJECTBIGSHIP:
        new_obj = new Object("Object Bigship", 5, 0, gfx, OBJ_PASSIVE,  10.0f);
        new_obj->init_animation(40, 2, 0);
        break;

    // bonuses
    case BONUSBLASTER:
        new_obj = new Object("Blaster Bonus", 1, 0, gfx, OBJ_BONUS,  20.0f);
        new_obj->init_animation(40, 3, 0);
        break;
    case BONUSROCKET:
        new_obj = new Object("Rocket Bonus", 1, 0, gfx, OBJ_BONUS,  20.0f);
        new_obj->init_animation(40, 3, 0);
        break;

    // shots
    case SHOTBLASTER:
        new_obj = new Shot("Blaster Shot", 1, gfx, owner);
        break;
    case SHOTBLASTERBIG:
        new_obj = new Shot("Big Blaster Shot", 1, gfx, owner);
        break;
    case SHOTROCKET:
        new_obj = new ShotRocket("Rocket Shot", 10, gfx, owner);
        break;
    case SHOTBOMBFRAGMENT:
        new_obj = new Shot("Bomb Fragment Shot", 4, gfx, owner);
        break;
    case SHOTENEMYSTD:
        new_obj = new Shot("Standard Enemy Shot", 1, gfx, owner);
        break;

    // misc
    case SMOKETRAIL:
        new_obj = new Object("Smoketrail", 1, 0, gfx, OBJ_PLAYERPASSIVE, 0);
        new_obj->set_owner(owner);
        new_obj->init_animation(40, 2, 0);
        break;

    default:
        std::cout << "Warning: undefined object." << std::endl;
        break;
    }
    return new_obj;
}

void ObjectManager::create_formation(int x, int y, float x_vel, float y_vel, 
                                     enum ObjIndex object)
{
    if (object == ENEMYSTD_V_FORMATION) {
        create_object(x - 80, y - 40, x_vel, y_vel, ENEMYSTD, OWNER_NONE);
        create_object(x - 40, y - 20, x_vel, y_vel, ENEMYSTD, OWNER_NONE);
        create_object(x     , y     , x_vel, y_vel, ENEMYSTD, OWNER_NONE);
        create_object(x + 40, y - 20, x_vel, y_vel, ENEMYSTD, OWNER_NONE);
        create_object(x + 80, y - 40, x_vel, y_vel, ENEMYSTD, OWNER_NONE);
    } else if (object == ENEMYSTD_3V_FORMATION) {
        create_object(300 - 80, y - 40, x_vel, y_vel, ENEMYSTD, OWNER_NONE);
        create_object(300 - 40, y - 20, x_vel, y_vel, ENEMYSTD, OWNER_NONE);
        create_object(300     , y     , x_vel, y_vel, ENEMYSTD, OWNER_NONE);
        create_object(300 + 40, y - 20, x_vel, y_vel, ENEMYSTD, OWNER_NONE);
        create_object(300 + 80, y - 40, x_vel, y_vel, ENEMYSTD, OWNER_NONE);

        create_object(300 - 260, y - 140, x_vel, y_vel, ENEMYSTD, OWNER_NONE);
        create_object(300 - 220, y - 120, x_vel, y_vel, ENEMYSTD, OWNER_NONE);
        create_object(300 - 180, y - 100, x_vel, y_vel, ENEMYSTD, OWNER_NONE);
        create_object(300 - 140, y - 120, x_vel, y_vel, ENEMYSTD, OWNER_NONE);
        create_object(300 - 100, y - 140, x_vel, y_vel, ENEMYSTD, OWNER_NONE);

        create_object(300 + 100, y - 140, x_vel, y_vel, ENEMYSTD, OWNER_NONE);
        create_object(300 + 140, y - 120, x_vel, y_vel, ENEMYSTD, OWNER_NONE);
        create_object(300 + 180, y - 100, x_vel, y_vel, ENEMYSTD, OWNER_NONE);
        create_object(300 + 220, y - 120, x_vel, y_vel, ENEMYSTD, OWNER_NONE);
        create_object(300 + 260, y - 140, x_vel, y_vel, ENEMYSTD, OWNER_NONE);
    } else if (object == ENEMYSTD_DIAGONAL_FORMATION) {
        create_object(0  , y    , x_vel, y_vel, ENEMYSTD, OWNER_NONE);
        create_object(50 , y - 10 , x_vel, y_vel, ENEMYSTD, OWNER_NONE);
        create_object(100, y - 20 , x_vel, y_vel, ENEMYSTD, OWNER_NONE);
        create_object(150, y - 30 , x_vel, y_vel, ENEMYSTD, OWNER_NONE);
        create_object(200, y - 40 , x_vel, y_vel, ENEMYSTD, OWNER_NONE);
        create_object(250, y - 50 , x_vel, y_vel, ENEMYSTD, OWNER_NONE);
        create_object(300, y - 60 , x_vel, y_vel, ENEMYSTD, OWNER_NONE);
        create_object(350, y - 70 , x_vel, y_vel, ENEMYSTD, OWNER_NONE);
        create_object(400, y - 80 , x_vel, y_vel, ENEMYSTD, OWNER_NONE);
        create_object(450, y - 90 , x_vel, y_vel, ENEMYSTD, OWNER_NONE);
        create_object(500, y - 100, x_vel, y_vel, ENEMYSTD, OWNER_NONE);
        create_object(550, y - 110, x_vel, y_vel, ENEMYSTD, OWNER_NONE);
        create_object(600, y - 120, x_vel, y_vel, ENEMYSTD, OWNER_NONE);
    } else if (object == ENEMYSTD_MASSIVE_FORMATION) {
        int i;
        for (i = 0; i < 20; i++)
            create_object((rand() % 560) + 40 , y - (rand() % 200), x_vel, 
                    y_vel, ENEMYSTD, OWNER_NONE);
    } else if (object == ENEMYSIDEWAYS_VLINE_FORMATION) {
        create_object(x     , y     , x_vel, y_vel, ENEMYSIDEWAYS, OWNER_NONE);
        create_object(x + 5 , y - 20, x_vel, y_vel, ENEMYSIDEWAYS, OWNER_NONE);
        create_object(x + 10, y - 40, x_vel, y_vel, ENEMYSIDEWAYS, OWNER_NONE);
        create_object(x + 15, y - 60, x_vel, y_vel, ENEMYSIDEWAYS, OWNER_NONE);
        create_object(x + 20, y - 80, x_vel, y_vel, ENEMYSIDEWAYS, OWNER_NONE);
    } else if (object == ENEMYSIDEWAYS_HLINE_FORMATION) {
        create_object(x - 160, y, x_vel, y_vel, ENEMYSIDEWAYS, OWNER_NONE);
        create_object(x - 80 , y, x_vel, y_vel, ENEMYSIDEWAYS, OWNER_NONE);
        create_object(x      , y, x_vel, y_vel, ENEMYSIDEWAYS, OWNER_NONE);
        create_object(x + 80 , y, x_vel, y_vel, ENEMYSIDEWAYS, OWNER_NONE);
        create_object(x + 160, y, x_vel, y_vel, ENEMYSIDEWAYS, OWNER_NONE);
    } else if (object == ENEMYSIDEWAYS_V_FORMATION) {
        create_object(x - 80, y - 40, x_vel, y_vel, ENEMYSIDEWAYS, OWNER_NONE);
        create_object(x - 40, y - 20, x_vel, y_vel, ENEMYSIDEWAYS, OWNER_NONE);
        create_object(x     , y     , x_vel, y_vel, ENEMYSIDEWAYS, OWNER_NONE);
        create_object(x + 40, y - 20, x_vel, y_vel, ENEMYSIDEWAYS, OWNER_NONE);
        create_object(x + 80, y - 40, x_vel, y_vel, ENEMYSIDEWAYS, OWNER_NONE);
    } else if (object == ENEMYSIDEWAYS_MASSIVE_FORMATION) {
        for (int i = 0; i < 20; i++) {
            create_object((rand() % 560) + 40 , y - (rand() % 200), x_vel, 
                    y_vel, ENEMYSIDEWAYS, OWNER_NONE);
        }
    } else if (object == ENEMYRAMMER_VLINE_FORMATION) {
        create_object(x, y     , x_vel, y_vel, ENEMYRAMMER, OWNER_NONE);
        create_object(x, y - 20, x_vel, y_vel, ENEMYRAMMER, OWNER_NONE);
        create_object(x, y - 40, x_vel, y_vel, ENEMYRAMMER, OWNER_NONE);
        create_object(x, y - 60, x_vel, y_vel, ENEMYRAMMER, OWNER_NONE);
        create_object(x, y - 80, x_vel, y_vel, ENEMYRAMMER, OWNER_NONE);
    } else if (object == ENEMYRAMMER_DIAGONAL_FORMATION) {
        create_object(x - 80, y   , x_vel, y_vel, ENEMYRAMMER, OWNER_NONE);
        create_object(x - 40, y - 20, x_vel, y_vel, ENEMYRAMMER, OWNER_NONE);
        create_object(x     , y - 40, x_vel, y_vel, ENEMYRAMMER, OWNER_NONE);
        create_object(x + 40, y - 60, x_vel, y_vel, ENEMYRAMMER, OWNER_NONE);
        create_object(x + 80, y - 80, x_vel, y_vel, ENEMYRAMMER, OWNER_NONE);
    } else if (object == ENEMYRAMMER_FULLDIAGONAL_FORMATION) {
        create_object(10 , y - 120, x_vel, y_vel, ENEMYRAMMER, OWNER_NONE);
        create_object(110, y - 100, x_vel, y_vel, ENEMYRAMMER, OWNER_NONE);
        create_object(210, y - 80 , x_vel, y_vel, ENEMYRAMMER, OWNER_NONE);
        create_object(310, y - 60 , x_vel, y_vel, ENEMYRAMMER, OWNER_NONE);
        create_object(410, y - 40 , x_vel, y_vel, ENEMYRAMMER, OWNER_NONE);
        create_object(510, y - 20 , x_vel, y_vel, ENEMYRAMMER, OWNER_NONE);
        create_object(610, y      , x_vel, y_vel, ENEMYRAMMER, OWNER_NONE);
    }
}

std::unique_ptr<Weapon> 
ObjectManager::create_main_weapon(int player, PlayerState player_state) 
{
    std::unique_ptr<Weapon> w;

    if (player_state.main_weapon(player) == "Blaster Weapon") {
        sdlc::Sound& sfx = snd[SND_SHOTBLASTER];
        Owner own = static_cast<Owner>(OWNER_PLAYER1 + player - 1);
        w = std::unique_ptr<Weapon>(new WeaponBlaster(sfx, own));
    }

    for (int j = 1; j < player_state.main_weapon_level(player); ++j)
        w->upgrade();

    return w;
}

std::unique_ptr<Weapon> 
ObjectManager::create_extra_weapon(int player, PlayerState player_state)
{
    std::unique_ptr<Weapon> w;

    if (player_state.extra_weapon(player) == "Rocket Weapon") {
        sdlc::Sound& sfx = snd[SND_SHOTROCKET];
        Owner own = static_cast<Owner>(OWNER_PLAYER1 + player - 1);
        w = std::unique_ptr<Weapon>(new WeaponRocket(sfx, own));
        w->set_count(player_state.extra_weapon_count(player));
    }

    return w;
}


void ObjectManager::update_player_state(PlayerState& player_state)
{
    for (auto object : list) {
        if (object->type() == OBJ_PLAYER) {

            // Get the player number from the name
            int player_id = object->name[object->name.length() - 1] - (int)'0';

            player_state.set_energy(player_id, object->energy());
            player_state.set_energy_max(player_id, object->energy_max());
            player_state.set_score(player_id, object->score());

            Ship* ship = (Ship*)object;
            if (ship->main_weapon_) {
                player_state.set_main_weapon(player_id, ship->main_weapon_->name);
                player_state.set_main_weapon_level(player_id, 
                        ship->main_weapon_->level());
            }
            if (ship->extra_weapon_) {
                player_state.set_extra_weapon(player_id, ship->extra_weapon_->name);
                player_state.set_extra_weapon_count(player_id, 
                        ship->extra_weapon_->count());
            } else player_state.set_extra_weapon(player_id, "none");

            if (object->energy() == 0)
                player_state.set_energy_max(player_id, 0);
        }
    }
}

void ObjectManager::flush_list()
{
    // Delete and remove all objects with energy <= 0.
    auto last = std::stable_partition(begin(list), end(list), [](Object* o) {
        return o->energy() > 0;
    });
    for_each(last, end(list), [](Object* o) { delete o; });
    list.erase(last, end(list));
}

void ObjectManager::add_from_queue()
{
    // TODO: range based loop
    ObjectList::iterator i = queue.begin();
    while (i != queue.end()) {
        Object* current = *i;
        i++;

        if (world_y_pos_ < current->getY() + current->getHeight()) {
            current->setY(current->getY() - world_y_pos_);

            current->active(true);
            current->activate(*this);

            if (list.empty() || current->type() == OBJ_SHOT)
                list.push_back(current);
            else {
                unsigned j;
                // TODO: range based loop
                ObjectList::iterator iterator = list.begin();
                for (j = 0; j < list.size(); j++) {
                    Object* current_list = *iterator;
                    if (current->type() < current_list->type()) {
                        list.insert(iterator, current);
                        j = list.size(); // break;
                    }

                    // when we reach the end of the list
                    else if (j == list.size() - 1) {
                        list.push_back(current);
                        j = list.size(); // break;
                    }
                    iterator++;
                }
            }
            queue.remove(current);
        }
    }
}

void ObjectManager::update_enemy_count()
{
    amount_of_enemies_in_list_ = 0;
    players_alive_ = 0;

    // TODO: range based loop
    ObjectList::iterator iterator;
    for (iterator = list.begin(); iterator != list.end(); iterator++) {
        Object* current = *iterator;
        ObjType t = current->type();
        if (t == OBJ_ENEMY || t == OBJ_BONUS)
            amount_of_enemies_in_list_++;
        else if (t == OBJ_PLAYER)
            players_alive_++;
    }
    // TODO: range based loop
    for (iterator = queue.begin(); iterator != queue.end(); iterator++) {
        Object* current = *iterator;
        ObjType t = current->type();
        if (t == OBJ_ENEMY || t == OBJ_BONUS)
            amount_of_enemies_in_list_++;
        else if (t == OBJ_PLAYER)
            players_alive_++;
    }
}
