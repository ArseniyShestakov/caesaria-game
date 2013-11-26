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

#include "infoboxmanager.hpp"
#include "gui/info_box.hpp"
#include "gui/environment.hpp"
#include "road.hpp"
#include "building/prefecture.hpp"
#include "gfx/tile.hpp"
#include "building/service.hpp"
#include "building/engineer_post.hpp"
#include "core/stringhelper.hpp"
#include "building/house.hpp"
#include "core/gettext.hpp"
#include "game/city.hpp"
#include "gui/infobox_warehouse.hpp"
#include "gui/infobox_market.hpp"
#include "core/logger.hpp"
#include "building/constants.hpp"
#include "walker/walker.hpp"
#include "gfx/tilemap.hpp"
#include "gui/infobox_house.hpp"
#include <map>

using namespace constants;
using namespace gui;

template< class T >
class BaseInfoboxCreator : public InfoboxCreator
{
public:
  gui::InfoBoxSimple* create( PlayerCityPtr city, gui::Widget* parent, TilePos pos )
  {
    return new T( parent, city->getTilemap().at( pos ) );
  }
};

class InfoBoxHouseCreator : public InfoboxCreator
{
public:
  gui::InfoBoxSimple* create( PlayerCityPtr city, gui::Widget* parent, TilePos pos )
  {
    HousePtr house = city->getOverlay( pos ).as<House>();
    if( house->getHabitants().count() > 0 )
    {
      return new InfoBoxHouse( parent, city->getTilemap().at( pos ) );
    }
    else
    {
      return new InfoBoxFreeHouse( parent, city->getTilemap().at( pos ) );
    }
  }
};


template< class T >
class CitizenInfoboxCreator : public InfoboxCreator
{
public:
  gui::InfoBoxSimple* create( PlayerCityPtr city, gui::Widget* parent, TilePos pos )
  {
    CityHelper helper( city );
    WalkerList walkers = helper.find<Walker>( walker::any, pos );

    if( walkers.empty() )
    {
      return new T( parent, city->getTilemap().at( pos ) );
    }
    else
    {
      return new InfoBoxCitizen( parent, walkers );
    }
  }

  PlayerCityPtr city;
};

class ServiceBaseInfoboxCreator : public InfoboxCreator
{
public:
  ServiceBaseInfoboxCreator( const std::string& caption,
                             const std::string& descr,
                             bool drawWorkers=false )
  {
    title = caption;
    text = descr;
    isDrawWorkers = drawWorkers;
  }

  gui::InfoBoxSimple* create( PlayerCityPtr city, gui::Widget* parent, TilePos pos )
  {
    Size  size = parent->getSize();
    WorkingBuildingPtr building = city->getOverlay( pos ).as<WorkingBuilding>();
    if( building.isValid() )
    {
      InfoBoxWorkingBuilding* infoBox = new InfoBoxWorkingBuilding( parent, building );
      infoBox->setPosition( Point( (size.getWidth() - infoBox->getWidth()) / 2, size.getHeight() - infoBox->getHeight()) );

      infoBox->setTitle( title );
      infoBox->setText( text );
      return infoBox;
    }
    
    return 0;
  }

  std::string title, text;
  bool isDrawWorkers;
};

class InfoBoxBasicCreator : public InfoboxCreator
{
public:
  InfoBoxBasicCreator( const std::string& caption,
                       const std::string& desc )
  {
    title = caption;
    text = desc;
  }

  gui::InfoBoxSimple* create( PlayerCityPtr city, gui::Widget* parent, TilePos pos )
  {
    Size  size = parent->getSize();
    InfoBoxSimple* infoBox = new InfoBoxSimple( parent, Rect( 0, 0, 510, 300 ) );
    infoBox->setPosition( Point( (size.getWidth() - infoBox->getWidth()) / 2, 
                                  size.getHeight() - infoBox->getHeight()) );

    infoBox->setTitle( title );
    infoBox->setText( text );
    return infoBox;
  }

  std::string title, text;
};

class InfoBoxManager::Impl
{
public:
    bool showDebugInfo;

    typedef std::map< TileOverlay::Type, InfoboxCreator* > InfoboxCreators;
    std::map< std::string, TileOverlay::Type > name2typeMap;

    InfoboxCreators constructors;
};

InfoBoxManager::InfoBoxManager() : _d( new Impl )
{
  _d->showDebugInfo = true;

  addInfobox( construction::road,         CAESARIA_STR_EXT(Road),        new CitizenInfoboxCreator<InfoBoxLand>() );
  addInfobox( building::reservoir,        CAESARIA_STR_EXT(Reservoir),   new InfoBoxBasicCreator( _("##reservoir_title##"), _("##reservoir_text##") ) );
  addInfobox( building::house,            CAESARIA_STR_EXT(House),       new InfoBoxHouseCreator() );
  addInfobox( building::prefecture,       CAESARIA_STR_EXT(Prefecture),  new ServiceBaseInfoboxCreator( "##prefecture_title##", "##prefecture_text##") );
  addInfobox( building::engineerPost,     CAESARIA_STR_EXT(EngineerPost),new ServiceBaseInfoboxCreator( "##engineering_post_title##", "##engineering_post_text##" ) );
  addInfobox( building::well,             CAESARIA_STR_EXT(Well),        new ServiceBaseInfoboxCreator( "##well_title##", "##well_text##" ) );
  addInfobox( building::doctor,           CAESARIA_STR_EXT(Doctor),      new ServiceBaseInfoboxCreator( "##doctor_title##", "##doctor_text##" ) );
  addInfobox( building::baths,            CAESARIA_STR_EXT(Baths),       new ServiceBaseInfoboxCreator( "##baths_title##", "##baths_text##" ) );
  addInfobox( building::barber,           CAESARIA_STR_EXT(Barber),      new ServiceBaseInfoboxCreator( "##barber_title##", "##barber_text##" ) );
  addInfobox( building::hospital,         CAESARIA_STR_EXT(B_HOSPITAL),  new ServiceBaseInfoboxCreator( "##hospital_title##", "##hospital_text##" ) );
  addInfobox( building::fountain,         CAESARIA_STR_EXT(B_FOUNTAIN),  new BaseInfoboxCreator<InfoBoxFontain>() );
  addInfobox( building::aqueduct,         CAESARIA_STR_EXT(B_AQUEDUCT),  new InfoBoxBasicCreator( "##aqueduct_title##", "##aqueduct_text##") );
  addInfobox( building::market,           CAESARIA_STR_EXT(B_MARKET),    new BaseInfoboxCreator<InfoBoxMarket>() );
  addInfobox( building::granary,          CAESARIA_STR_EXT(B_GRANARY),   new BaseInfoboxCreator<InfoBoxGranary>() );
  addInfobox( building::grapeFarm,        CAESARIA_STR_EXT(B_GRAPE_FARM),new BaseInfoboxCreator<InfoBoxRawMaterial>() );
  addInfobox( building::wheatFarm,        CAESARIA_STR_EXT(B_WHEAT_FARM),new BaseInfoboxCreator<InfoBoxRawMaterial>() );
  addInfobox( building::vegetableFarm,    CAESARIA_STR_EXT(B_VEGETABLE_FARM), new BaseInfoboxCreator<InfoBoxRawMaterial>() );
  addInfobox( building::oliveFarm,        CAESARIA_STR_EXT(B_OLIVE_FARM),new BaseInfoboxCreator<InfoBoxRawMaterial>() );
  addInfobox( building::fruitFarm,        CAESARIA_STR_EXT(B_FRUIT_FARM),new BaseInfoboxCreator<InfoBoxRawMaterial>() );
  addInfobox( building::warehouse,        CAESARIA_STR_EXT(B_WAREHOUSE), new BaseInfoboxCreator<InfoBoxWarehouse>() );
  addInfobox( building::pigFarm,          CAESARIA_STR_EXT(B_PIG_FARM),  new BaseInfoboxCreator<InfoBoxRawMaterial>() );
  addInfobox( building::templeCeres,      CAESARIA_STR_EXT(B_TEMPLE_CERES), new BaseInfoboxCreator<InfoBoxTemple>() );
  addInfobox( building::templeMars,       CAESARIA_STR_EXT(B_TEMPLE_MARS), new BaseInfoboxCreator<InfoBoxTemple>() );
  addInfobox( building::templeNeptune,    CAESARIA_STR_EXT(B_TEMPLE_NEPTUNE), new BaseInfoboxCreator<InfoBoxTemple>() );
  addInfobox( building::templeVenus,      CAESARIA_STR_EXT(B_TEMPLE_VENUS), new BaseInfoboxCreator<InfoBoxTemple>() );
  addInfobox( building::templeMercury,    CAESARIA_STR_EXT(B_TEMPLE_MERCURE), new BaseInfoboxCreator<InfoBoxTemple>() );
  addInfobox( building::B_BIG_TEMPLE_CERES, CAESARIA_STR_EXT(B_BIG_TEMPLE_CERES), new BaseInfoboxCreator<InfoBoxTemple>() );
  addInfobox( building::B_BIG_TEMPLE_MARS,  CAESARIA_STR_EXT(B_BIG_TEMPLE_MARS), new BaseInfoboxCreator<InfoBoxTemple>() );
  addInfobox( building::B_BIG_TEMPLE_NEPTUNE, CAESARIA_STR_EXT(B_BIG_TEMPLE_NEPTUNE), new BaseInfoboxCreator<InfoBoxTemple>() );
  addInfobox( building::B_BIG_TEMPLE_VENUS,   CAESARIA_STR_EXT(B_BIG_TEMPLE_VENUS), new BaseInfoboxCreator<InfoBoxTemple>() );
  addInfobox( building::B_BIG_TEMPLE_MERCURE, CAESARIA_STR_EXT(B_BIG_TEMPLE_MERCURE), new BaseInfoboxCreator<InfoBoxTemple>() );
  addInfobox( building::oracle,           CAESARIA_STR_EXT(B_TEMPLE_ORACLE), new BaseInfoboxCreator<InfoBoxTemple>() );
  addInfobox( building::school,         CAESARIA_STR_EXT(B_SCHOOL),    new ServiceBaseInfoboxCreator( _("##school_title##"), _("##school_text##") ));
  addInfobox( building::academy,        CAESARIA_STR_EXT(B_COLLEGE),   new ServiceBaseInfoboxCreator( _("##college_title##"), _("##college_text##") ));
  addInfobox( building::library,        CAESARIA_STR_EXT(B_LIBRARY),   new ServiceBaseInfoboxCreator( _("##library_title##"), _("##library_text##") ));
  addInfobox( construction::garden,       CAESARIA_STR_EXT(B_GARDEN),    new InfoBoxBasicCreator( _("##building_garden##"), _("##garden_desc##")) );
  addInfobox( building::smallStatue,        CAESARIA_STR_EXT(B_STATUE1),   new InfoBoxBasicCreator( _("##building_statue_small##"), _("##statue_desc##")) );
  addInfobox( building::middleStatue,        CAESARIA_STR_EXT(B_STATUE2),   new InfoBoxBasicCreator( _("##building_statue_middle##"), _("##statue_desc##")) );
  addInfobox( building::bigStatue,        CAESARIA_STR_EXT(B_STATUE3),   new InfoBoxBasicCreator( _("##building_statue_big##"), _("##statue_desc##")) );
  addInfobox( construction::plaza,        CAESARIA_STR_EXT(B_PLAZA),     new CitizenInfoboxCreator<InfoBoxLand>() );
  addInfobox( building::unknown,          CAESARIA_STR_EXT(unknown),     new CitizenInfoboxCreator<InfoBoxLand>() );
  addInfobox( building::pottery,          CAESARIA_STR_EXT(B_POTTERY),   new BaseInfoboxCreator<GuiInfoFactory>() );
  addInfobox( building::weaponsWorkshop, CAESARIA_STR_EXT(B_WEAPONS_WORKSHOP), new BaseInfoboxCreator<GuiInfoFactory>() );
  addInfobox( building::furnitureWorkshop,        CAESARIA_STR_EXT(B_FURNITURE), new BaseInfoboxCreator<GuiInfoFactory>() );
  addInfobox( building::clayPit,          CAESARIA_STR_EXT(B_CLAY_PIT),  new BaseInfoboxCreator<InfoBoxRawMaterial>() );
  addInfobox( building::timberLogger,     CAESARIA_STR_EXT(B_TIMBER_YARD), new BaseInfoboxCreator<InfoBoxRawMaterial>() );
  addInfobox( building::marbleQuarry,     CAESARIA_STR_EXT(B_MARBLE_QUARRY), new BaseInfoboxCreator<InfoBoxRawMaterial>() );
  addInfobox( building::ironMine,         CAESARIA_STR_EXT(B_IRON_MINE), new BaseInfoboxCreator<InfoBoxRawMaterial>() );
  addInfobox( building::winery,           CAESARIA_STR_EXT(B_WINE_WORKSHOP), new BaseInfoboxCreator<GuiInfoFactory>() );
  addInfobox( building::creamery,         CAESARIA_STR_EXT(B_OIL_WORKSHOP), new BaseInfoboxCreator<GuiInfoFactory>() );
  addInfobox( building::senate,           CAESARIA_STR_EXT(B_SENATE),    new BaseInfoboxCreator<InfoBoxSenate>() );
  addInfobox( building::theater,          CAESARIA_STR_EXT(Theater),     new ServiceBaseInfoboxCreator( _("##theater_title##"), _("##theater_text##")) );
  addInfobox( building::actorColony,      CAESARIA_STR_EXT(B_ACTOR_COLONY), new ServiceBaseInfoboxCreator( _("##actor_colony_title##"), _("##actor_colony_text##")) );
  addInfobox( building::amphitheater,     CAESARIA_STR_EXT(buildingAmphitheater), new ServiceBaseInfoboxCreator( _("##amphitheater_title##"), _("##amphitheater_text##")) );
  addInfobox( building::gladiatorSchool,  CAESARIA_STR_EXT(B_GLADIATOR_SCHOOL), new ServiceBaseInfoboxCreator( _("##gladiator_school_title##"), _("##gladiator_school_text##")) );
  addInfobox( building::colloseum,        CAESARIA_STR_EXT(B_COLLOSSEUM), new BaseInfoboxCreator<InfoBoxColosseum>() );
  addInfobox( building::lionHouse,        CAESARIA_STR_EXT(B_LION_HOUSE), new ServiceBaseInfoboxCreator( _("##lion_house_title##"), _("##lion_house_text##")) );
  addInfobox( building::hippodrome,       CAESARIA_STR_EXT(B_HIPPODROME), new ServiceBaseInfoboxCreator( _("##hippodrome_title##"), _("##hippodrome_text##")) );
  addInfobox( building::chariotSchool,    CAESARIA_STR_EXT(chariotSchool),new ServiceBaseInfoboxCreator( _("##chario_maker_title##"), _("##chario_maker_text##")) );
  addInfobox( building::forum,            CAESARIA_STR_EXT(forum),        new ServiceBaseInfoboxCreator( _("##forum_title##"), _("##forum_text##")) );
  addInfobox( building::governorHouse,    CAESARIA_STR_EXT(governorHouse),new ServiceBaseInfoboxCreator( _("##governor_house_title##"), _("##governonr_house_text##")) );
  addInfobox( building::governorVilla,    CAESARIA_STR_EXT(governorVilla),new ServiceBaseInfoboxCreator( _("##governor_villa_title##"), _("##governonr_villa_text##")) );
  addInfobox( building::governorPalace,   CAESARIA_STR_EXT(governorPalace), new ServiceBaseInfoboxCreator( _("##governor_palace_title##"), _("##governonr_palace_text##")) );
  addInfobox( building::highBridge,       CAESARIA_STR_EXT(highBridge),   new InfoBoxBasicCreator( _("##high_bridge_title##"), _("##high_bridge_text##")) );
  addInfobox( building::lowBridge,        CAESARIA_STR_EXT(lowBridge),    new InfoBoxBasicCreator( _("##low_bridge_title##"), _("##low_bridge_text##")) );
  addInfobox( building::wharf,            CAESARIA_STR_EXT(wharf),        new BaseInfoboxCreator<GuiInfoFactory>() );
}

InfoBoxManager::~InfoBoxManager()
{

}

InfoBoxManager& InfoBoxManager::getInstance()
{
  static InfoBoxManager inst;
  return inst;
}

void InfoBoxManager::showHelp( PlayerCityPtr city, GuiEnv* gui, TilePos pos )
{
  Tile& tile = city->getTilemap().at( pos );
  TileOverlayPtr overlay = tile.getOverlay();
  TileOverlay::Type type;

  if( _d->showDebugInfo )
  {
    Logger::warning( "Tile debug info: dsrbl=%d", tile.getDesirability() );
  }

  type = overlay.isNull() ? building::unknown : overlay->getType();

  Impl::InfoboxCreators::iterator findConstructor = _d->constructors.find( type );

  InfoBoxSimple* infoBox = findConstructor != _d->constructors.end()
                                  ? findConstructor->second->create( city, gui->getRootWidget(), pos )
                                  : 0;
  
  if( infoBox && infoBox->isAutoPosition() )
  {
    Size rSize = gui->getRootWidget()->getSize();
    int y = ( gui->getCursorPos().getY() < rSize.getHeight() / 2 )
                ? rSize.getHeight() - infoBox->getHeight() - 5
                : 30;
    Point pos( ( rSize.getWidth() - infoBox->getWidth() ) / 2, y );

    infoBox->setPosition( pos );
  }
}

void InfoBoxManager::setShowDebugInfo( const bool showInfo )
{
  _d->showDebugInfo = showInfo;
} 

void InfoBoxManager::addInfobox( const TileOverlay::Type type, const std::string& typeName, InfoboxCreator* ctor )
{
  bool alreadyHaveConstructor = _d->name2typeMap.find( typeName ) != _d->name2typeMap.end();
  _CAESARIA_DEBUG_BREAK_IF( alreadyHaveConstructor && "already have constructor for this type");

  if( !alreadyHaveConstructor )
  {
    _d->name2typeMap[ typeName ] = type;
    _d->constructors[ type ] = ctor;
  }
}

bool InfoBoxManager::canCreate(const TileOverlay::Type type) const
{
  return _d->constructors.find( type ) != _d->constructors.end();   
}