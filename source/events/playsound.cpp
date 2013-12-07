// This file is part of CaesarIA.
//
// CaesarIA is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// CaesarIA is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with CaesarIA.  If not, see <http://www.gnu.org/licenses/>.

#include "playsound.hpp"
#include "core/stringhelper.hpp"
#include "sound/engine.hpp"
#include "game/game.hpp"

namespace events
{

GameEventPtr PlaySound::create( std::string rc, int index, int volume )
{
  PlaySound* e = new PlaySound();
  e->_sound = StringHelper::format( 0xff, "%s_%05d.wav", rc.c_str(), index );
  e->_volume = volume;

  GameEventPtr ret( e );
  ret->drop();

  return ret;
}

GameEventPtr PlaySound::create(std::string filename, int volume)
{
  PlaySound* e = new PlaySound();
  e->_sound = filename;
  e->_volume = volume;

  GameEventPtr ret( e );
  ret->drop();

  return ret;
}

void PlaySound::exec(Game&)
{
  audio::Engine::instance().play( _sound, _volume );
}

}