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

#include <cstdio>

#include "package_options_window.hpp"
#include "core/stringhelper.hpp"
#include "widget_helper.hpp"
#include "texturedbutton.hpp"
#include "game/settings.hpp"
#include "editbox.hpp"
#include "core/logger.hpp"

using namespace constants;

namespace gui
{

namespace dialog
{

class PackageOptions::Impl
{
public:
  EditBox* edResourcesPath;
  TexturedButton* btnApply;
};

PackageOptions::PackageOptions( Widget* parent, const Rect& rectangle )
  : Window( parent, rectangle, "" ), _d( new Impl )
{
  setupUI( ":/gui/packageopts.gui" );

  GET_DWIDGET_FROM_UI(_d,edResourcesPath)
  GET_DWIDGET_FROM_UI(_d,btnApply)

  if( _d->edResourcesPath ) { _d->edResourcesPath->setText( SETTINGS_VALUE( resourcePath ).toString() ); }

  CONNECT( _d->btnApply, onClicked(), this, PackageOptions::deleteLater );
  CONNECT( _d->edResourcesPath, onTextChanged(), this, PackageOptions::_setResourcesPath );
}

PackageOptions::~PackageOptions() {}

void PackageOptions::_setResourcesPath(std::string path)
{
  SETTINGS_SET_VALUE( resourcePath, Variant(path) );
}

}//end namespace dialog

}//end namespace gui