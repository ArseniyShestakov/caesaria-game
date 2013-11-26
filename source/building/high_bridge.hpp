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

#ifndef __OPENCAESAR3_HIGH_BRIDGE_H_INCLUDED__
#define __OPENCAESAR3_HIGH_BRIDGE_H_INCLUDED__

#include "game/construction.hpp"
#include "core/scopedptr.hpp"
#include "core/direction.hpp"

class HighBridge : public Construction
{
public:
  HighBridge();

  bool canBuild(PlayerCityPtr city, TilePos pos , const TilesArray& aroundTiles) const;
  void initTerrain( Tile& terrain );
  void build( PlayerCityPtr city, const TilePos& pos );
  void destroy();

private:
  void _computePictures( PlayerCityPtr city, const TilePos& startPos, const TilePos& endPos, constants::Direction dir );
  void _checkParams( PlayerCityPtr city, constants::Direction& direction, TilePos& start, TilePos& stop, const TilePos& curPos ) const;

  class Impl;
  ScopedPtr< Impl > _d;
};

#endif //__OPENCAESAR3_HIGH_BRIDGE_H_INCLUDED__