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

#ifndef _CAESARIA_DESIRABILITY_UPDATER_H_INCLUDE_
#define _CAESARIA_DESIRABILITY_UPDATER_H_INCLUDE_

#include "cityservice.hpp"
#include "predefinitions.hpp"
#include "core/scopedptr.hpp"

namespace city
{

class DesirabilityUpdater : public Srvc
{
public:
  static SrvcPtr create(PlayerCityPtr city);
  virtual void timeStep(const unsigned int time);
  static std::string defaultName();
  virtual bool isDeleted() const;
  virtual void destroy();

  virtual void load(const VariantMap &stream);
  virtual VariantMap save() const;

private:
  DesirabilityUpdater( PlayerCityPtr city );

  class Impl;
  ScopedPtr<Impl> _d;
};

}

#endif //_CAESARIA_DESIRABILITY_UPDATER_H_INCLUDE_
