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

#ifndef _KEXX2_GAMEOVER_H_
#define _KEXX2_GAMEOVER_H_

#include "IGameState.h"

class Options;
class PlayerState;

class GameOver : public IGameState {
public:
    GameOver();
    virtual ~GameOver() {};

    void run_logic(sdlc::Input& input, sdlc::Timer& timer, sdlc::Mixer& mixer,
                   PlayerState& player_state) override;
    void draw(sdlc::Screen& screen, sdlc::Font& font);
};

#endif
