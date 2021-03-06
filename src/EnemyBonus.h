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

#ifndef KEXX2_ENEMYBONUS_H
#define KEXX2_ENEMYBONUS_H

#include "Object.h"

class EnemyBonus final : public Object {
public:
    EnemyBonus(std::string name, int energy, 
               sdlc::Surface& s, sdlc::Surface& hit_s);
    virtual ~EnemyBonus() {};

    void activate(ObjectManager& object_manager) override;
    void kill(ObjectManager& object_manager, FxManager& fx_manager) override;
};

#endif // KEXX2_ENEMYBONUS_H
