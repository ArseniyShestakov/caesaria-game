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

#include "infobox_raw.hpp"
#include "good/goodhelper.hpp"
#include "image.hpp"
#include "core/stringhelper.hpp"
#include "label.hpp"
#include "core/gettext.hpp"
#include "objects/constants.hpp"
#include "game/settings.hpp"

using namespace constants;

namespace gui
{

InfoBoxRawMaterial::InfoBoxRawMaterial( Widget* parent, const Tile& tile )
  : InfoBoxSimple( parent, Rect( 0, 0, 510, 350 ), Rect( 16, 146, 510 - 16, 146 + 74 ) )
{
  Widget::setupUI( GameSettings::rcpath( "/gui/infoboxraw.gui" ) );
  FactoryPtr rawmb = tile.getOverlay().as<Factory>();
  Label* lbDamage = findChild<Label*>( "lbDamage", true );
  Label* lbProgress = findChild<Label*>( "lbProgress", true );
  Label* lbAbout = findChild<Label*>( "lbAbout", true );
  Label* lbProductivity = findChild<Label*>( "lbProductivity", true );

  if( rawmb->getOutGoodType() != Good::none )
  {
    Picture pic = GoodHelper::getPicture( rawmb->getOutGoodType() );
    new Image( this, Point( 10, 10 ), pic );
  }

  _updateWorkersLabel( Point( 32, 160 ), 542, rawmb->getMaxWorkers(), rawmb->getWorkersCount() );

  if( lbDamage != NULL )
  {
    std::string text = StringHelper::format( 0xff, "%d%% damage - %d%% fire",
                                            (int)rawmb->getState( Construction::damage ),
                                            (int)rawmb->getState( Construction::fire ) );
    lbDamage->setText( text );
  }

  if( lbProgress != NULL )
  {
    std::string text = StringHelper::format( 0xff, "%s %d%%", _("##rawm_production_complete##"), rawmb->getProgress() );
    lbProgress->setText( text );
  }

  if( lbAbout != NULL )
  {
    std::string desc, name;
    //GoodType goodType = G_NONE;
    switch( rawmb->getType() )
    {
    case building::wheatFarm:
      desc = _("##farm_description_wheat##");
      name = _("##farm_title_wheat##");
    break;

    case building::fruitFarm:
      desc.assign( _("##farm_description_fruit##") );
      name.assign( _("##farm_title_fruit##") );
    break;

    case building::oliveFarm:
      desc.assign( _("##farm_description_olive##") );
      name.assign( _("##farm_title_olive##") );
    break;

    case building::grapeFarm:
      desc.assign( _("##farm_description_vine##") );
      name.assign( _("##farm_title_vine##") );
    break;

    case building::pigFarm:
      desc.assign( _("##farm_description_meat##") );
      name.assign( _("##farm_title_meat##") );
    break;

    case building::vegetableFarm:
      desc.assign( _("##farm_description_vegetable##") );
      name.assign( _("##farm_title_vegetable##") );
    break;

    default:
    break;
    }

    lbAbout->setText( desc );
    setTitle( name );
  }

  if( lbProductivity != NULL )
  {
    std::string text = _("##farm_working_normally##");
    if( rawmb->getWorkersCount() == 0 )
    {
      text = _("##farm_have_no_workers##");
    }
    else if( rawmb->getWorkersCount() <= rawmb->getMaxWorkers() / 2 )
    {
      text = _("##farm_working_bad##");
    }

    lbProductivity->setText( text );
  }
}

InfoBoxRawMaterial::~InfoBoxRawMaterial()
{
}

}//end namespace gui