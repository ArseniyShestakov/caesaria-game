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

#include "farm.hpp"
#include "core/position.hpp"
#include "core/exception.hpp"
#include "core/variant_map.hpp"
#include "game/resourcegroup.hpp"
#include "gfx/helper.hpp"
#include "good/stock.hpp"
#include "city/helper.hpp"
#include "good/helper.hpp"
#include "city/city.hpp"
#include "core/utils.hpp"
#include "gfx/tilemap.hpp"
#include "core/gettext.hpp"
#include "core/logger.hpp"
#include "constants.hpp"
#include "walker/locust.hpp"
#include "city/statistic.hpp"
#include "core/foreach.hpp"
#include "core/tilepos_array.hpp"
#include "game/gamedate.hpp"
#include "gfx/helper.hpp"
#include "events/clearland.hpp"
#include "objects_factory.hpp"

using namespace gfx;

REGISTER_CLASS_IN_OVERLAYFACTORY(object::fig_farm, FarmFruit)
REGISTER_CLASS_IN_OVERLAYFACTORY(object::wheat_farm, FarmWheat)
REGISTER_CLASS_IN_OVERLAYFACTORY(object::vinard, FarmGrape)
REGISTER_CLASS_IN_OVERLAYFACTORY(object::meat_farm, FarmMeat)
REGISTER_CLASS_IN_OVERLAYFACTORY(object::olive_farm, FarmOlive)
REGISTER_CLASS_IN_OVERLAYFACTORY(object::vegetable_farm, FarmVegetable)

class FarmTile : public Construction
{
public:
  FarmTile() : Construction( object::farmtile, 1 ) {}
  FarmTile(const good::Product outGood, const TilePos& farmpos);
  virtual ~FarmTile() {}
  Picture& getPicture();
  virtual void initTerrain(gfx::Tile&) {}
  virtual bool isFlat() const { return false; }
  virtual bool build(const city::AreaInfo &info);

  static Picture computePicture( const good::Product outGood, const int percent);

private:
  TilePos _farmpos;
};

REGISTER_CLASS_IN_OVERLAYFACTORY(object::farmtile, FarmTile)

FarmTile::FarmTile( const good::Product outGood, const TilePos& farmpos )
 : Construction( object::farmtile, 1 )
{
  _farmpos = farmpos;
  //_animation.load( ResourceGroup::commerce, picIdx, 5);
  setPicture( computePicture( outGood, 0 ) );
}

Picture FarmTile::computePicture( const good::Product outGood, const int percent)
{
  int picIdx = 0;
  int sequenceSize = 5;

  std::map<good::Product, int> good2pics;
  good2pics[ good::wheat ] = 13;
  good2pics[ good::vegetable ] = 18;
  good2pics[ good::fruit ] = 23;
  good2pics[ good::olive ] = 28;
  good2pics[ good::grape ] = 33;
  good2pics[ good::meat ] = 38;

  std::map<good::Product, int>::iterator rIt = good2pics.find( outGood );
  if( rIt != good2pics.end() )
  {
    picIdx = rIt->second;
  }
  else
  {
    Logger::warning( "Unexpected farmType in farm" + good::Helper::name( outGood ) );
  }

  picIdx += math::clamp<int>( (percent * sequenceSize) / 100, 0, sequenceSize-1);
  return Picture::load( ResourceGroup::commerce, picIdx );
}

bool FarmTile::build(const city::AreaInfo &info)
{
  return Construction::build( info );
}

class Farm::Impl
{
public:
  TilePosArray sublocs;
  TilesArray subtiles;
  int lastProgress;
};

Farm::Farm(const good::Product outGood, const object::Type type )
  : Factory( good::none, outGood, type, Size(3) ), _d( new Impl )
{
  outStockRef().setCapacity( 100 );

  _d->lastProgress = 0;
  _d->sublocs << TilePos( 0, 0) << TilePos( 1, 0)
              << TilePos( 2, 0) << TilePos( 2, 1) << TilePos( 2, 2);

  _fgPicturesRef().resize( _d->sublocs.size() );

  for(unsigned int n = 0; n<_d->sublocs.size(); ++n)
  {
    _fgPicture(n) = FarmTile::computePicture( outGood, 0 );
    _fgPicture(n).addOffset( tile::tilepos2screen( _d->sublocs[n] ) );
  }
  _fgPicturesRef().push_back( Picture::load( ResourceGroup::commerce, 12) );  // farm building
  _fgPicturesRef().back().addOffset( tile::tilepos2screen( TilePos( 0, 1)) );

  init();
}

bool Farm::canBuild( const city::AreaInfo& areaInfo ) const
{
  bool is_constructible = Construction::canBuild( areaInfo );
  bool on_meadow = false;

  TilesArray area = areaInfo.city->tilemap().getArea( areaInfo.pos, size() );
  foreach( tile, area )
  {
    on_meadow |= (*tile)->getFlag( Tile::tlMeadow );
  }

  Farm* non_const_this = const_cast< Farm* >( this );
  non_const_this->_setError( on_meadow ? "" : _("##farm_need_farmland##") );

  return (is_constructible && on_meadow);
}

void Farm::destroy()
{
  foreach( it, _d->subtiles )
  {
    OverlayPtr ov = (*it)->overlay();
    if( ov.isValid() && ov->type() == object::farmtile )
    {
      events::GameEventPtr e = events::ClearTile::create( ov->pos() );
      e->dispatch();
    }
  }

  Factory::destroy();
}

void Farm::computeRoadside()
{
  Factory::computeRoadside();

  foreach( it, _d->subtiles )
  {
    ConstructionPtr ov = ptr_cast<Construction>( (*it)->overlay() );
    if( ov.isValid() && ov->type() == object::farmtile )
    {
      _roadside().append( ov->roadside() );
    }
  }
}

void Farm::init()
{
  _fgPicturesRef().resize(5+1);
  computePictures();
}

void Farm::computePictures()
{
  int amount = progress();
  int percentTile;

  for(unsigned int n = 0; n<_d->subtiles.size(); ++n)
  {
    if (amount >= 20)   // 20 = 100 / nbSubTiles
    {
      // this subtile is at maximum
      percentTile = 100;  // 100%
      amount -= 20;  // for next subTiles
    }
    else
    {
      // this subtile is not at maximum
      percentTile = 5 * amount;
      amount = 0;  // for next subTiles
    }

    SmartPtr<FarmTile> ft = ptr_cast<FarmTile>( _d->subtiles[n]->overlay() );
    if( ft.isValid() )
      ft->setPicture( FarmTile::computePicture( produceGoodType(), percentTile ));
  }
}

void Farm::timeStep(const unsigned long time)
{
  Factory::timeStep(time);

  if( game::Date::isDayChanged() && mayWork()
      && progress() < 100 && _d->lastProgress != progress() )
  {
    _d->lastProgress = progress();
    computePictures();
  }
}

bool Farm::build( const city::AreaInfo& info )
{
  setSize( 2 );
  city::AreaInfo upInfo = info;
  if( !info.city->getOption( PlayerCity::forceBuild ) ) //it flag use on load only
  {
    upInfo.pos += TilePos(0,1);

    TilePosArray locations;
    foreach( it, _d->sublocs )
    {
      city::AreaInfo tInfo = info;
      tInfo.pos += *it;
      OverlayPtr farmtile( new FarmTile( produceGoodType(), upInfo.pos ) );
      farmtile->drop();

      farmtile->build( tInfo );
      info.city->addOverlay( farmtile );
      locations << farmtile->pos();
      _d->subtiles.push_back( &farmtile->tile() );
    }

    _d->sublocs = locations;
  }

  _fgPicturesRef().resize( 0 );
  Factory::build( upInfo );

  setPicture( ResourceGroup::commerce, 12 );
  computePictures();

  return true;
}

void Farm::save( VariantMap& stream ) const
{
  Factory::save( stream );
  stream[ "locations" ] = _d->sublocs.toVList();
}

void Farm::load( const VariantMap& stream )
{
  Factory::load( stream );
  _d->sublocs.fromVList( stream.get( "locations").toList() );

  foreach( it, _d->sublocs )
    _d->subtiles.push_back( &_city()->tilemap().at( *it ) );

  computePictures();
}

unsigned int Farm::produceQty() const
{
  return productRate() * getFinishedQty() * numberWorkers() / maximumWorkers();
}

Farm::~Farm() {}

FarmWheat::FarmWheat() : Farm(good::wheat, object::wheat_farm)
{
}

std::string FarmWheat::troubleDesc() const
{
  LocustList lc = city::statistic::findw<Locust>( _city(), constants::walker::locust, pos() );
  if( !lc.empty() )
  {
    return "##trouble_farm_was_blighted_by_locust##";
  }

  return Factory::troubleDesc();
}

bool FarmWheat::build( const city::AreaInfo& info )
{
  bool ret = Farm::build( info );
  if( info.city->climate() == game::climate::central )
  {
    setProductRate( productRate() * 2 );
  }

  return ret;
}

FarmOlive::FarmOlive() : Farm(good::olive, object::olive_farm)
{
}

FarmGrape::FarmGrape() : Farm(good::grape, object::vinard)
{
}

FarmMeat::FarmMeat() : Farm(good::meat, object::meat_farm)
{
}

FarmFruit::FarmFruit() : Farm(good::fruit, object::fig_farm)
{
}

FarmVegetable::FarmVegetable() : Farm(good::vegetable, object::vegetable_farm)
{
}
