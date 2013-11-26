// This file is part of openCaesar3.
//
// openCaesar3 is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// openCaesar3 is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with openCaesar3.  If not, see <http://www.gnu.org/licenses/>.
//
// Copyright 2012-2013 Gregoire Athanase, gathanase@gmail.com

#include "saver.hpp"
#include "core/variant.hpp"
#include "core/saveadapter.hpp"
#include "player.hpp"
#include "world/empire.hpp"
#include "city.hpp"
#include "gamedate.hpp"
#include "game.hpp"
#include "divinity.hpp"

void GameSaver::save(const vfs::Path& filename, const Game& game )
{
  VariantMap vm;
  vm[ "version" ] = Variant( 1 );
  vm[ "timemultiplier" ] = game.getTimeMultiplier();

  VariantMap vm_scenario;
  vm_scenario[ "date" ] = GameDate::current();
  vm[ "scenario" ] = vm_scenario;

  VariantMap vm_empire;
  game.getEmpire()->save( vm_empire );
  vm[ "empire" ] = vm_empire;

  VariantMap plm;
  game.getPlayer()->save( plm );
  vm[ "player" ] = plm;

  VariantMap vm_city;
  game.getCity()->save( vm_city );
  vm[ "city" ] = vm_city;

  VariantMap vm_pantheon;
  DivinePantheon::getInstance().save( vm_pantheon );
  vm[ "pantheon" ] = vm_pantheon;

  SaveAdapter::save( vm, filename );
}