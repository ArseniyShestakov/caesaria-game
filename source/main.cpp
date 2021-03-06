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
//
// Copyright 2012-2014 Dalerank, dalerankn8@gmail.com

#include "core/utils.hpp"
#include "core/exception.hpp"
#include "vfs/directory.hpp"
#include "game/settings.hpp"
#include "game/game.hpp"
#include "gfx/helper.hpp"
#include "core/logger.hpp"
#include "core/stacktrace.hpp"
#include "core/osystem.hpp"

#ifdef CAESARIA_USE_STEAM
  #include "steam.hpp"
#endif

#if defined(CAESARIA_PLATFORM_WIN)
  #undef main
#endif

#if defined(CAESARIA_PLATFORM_ANDROID)
#include <SDL.h>
#include <SDL_system.h>
int SDL_main(int argc, char* argv[])
#else
int main(int argc, char* argv[])
#endif
{  
  crashhandler::install();

  vfs::Directory workdir;
#ifdef CAESARIA_PLATFORM_ANDROID
  workdir  = vfs::Path( SDL_AndroidGetExternalStoragePath() );
#else
  workdir = vfs::Path( argv[0] ).directory();
#endif
  game::Settings& options = game::Settings::instance();
  Logger::registerWriter( Logger::consolelog, "" );

  options.setwdir( workdir.toString() );
  options.checkwdir( argv, argc );
  Logger::registerWriter( Logger::filelog, workdir.toString() );

  Logger::warning( "Game: setting workdir to " + SETTINGS_VALUE( workDir ).toString()  );

  Logger::warning( "Game: load game settings" );
  options.load();
  options.checkCmdOptions( argv, argc );
  options.checkC3present();

  std::string systemLang = SETTINGS_VALUE( language ).toString();
#ifdef CAESARIA_USE_STEAM
  if( !steamapi::connect() )
    return EXIT_FAILURE;

  if( systemLang.empty() )
    systemLang = steamapi::language();
#endif

  options.changeSystemLang( systemLang );

  Logger::warning( "Game: setting language to " + SETTINGS_VALUE( language ).toString() );
  Logger::warning( "Game: using native C3 resources from " + SETTINGS_VALUE( c3gfx ).toString() );
  Logger::warning( "Game: set cell width %d", SETTINGS_VALUE( cellw ).toInt() );

  try
  {
    Game game;
    game.initialize();
    while( game.exec() );
  }
  catch( Exception& e )
  {
    Logger::warning( "Critical error: " + e.getDescription() );

    crashhandler::printstack();
  }

#ifdef CAESARIA_USE_STEAM
  steamapi::close();
#endif

  crashhandler::remove();

  return 0;
}
