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
// Copyright 2012-2014 dalerank, dalerankn8@gmail.com

#ifndef __CAESARIA_EMPIRE_OBJECTSFACTORY_H_INCLUDED__
#define __CAESARIA_EMPIRE_OBJECTSFACTORY_H_INCLUDED__

#include "core/scopedptr.hpp"
#include "predefinitions.hpp"
#include <string>

namespace world
{

class ObjectCreator
{
public:
  virtual ObjectPtr create( EmpirePtr city ) = 0;
};

class ObjectsFactory
{
public:
  static ObjectsFactory& instance();

  bool canCreate( const std::string& type ) const;

  void addCreator( const std::string& type, ObjectCreator* ctor );

  ObjectPtr create( const std::string& type, EmpirePtr empire );  // get an instance of the given type

  ~ObjectsFactory();

private:
  ObjectsFactory();

  class Impl;
  ScopedPtr< Impl > _d;
};

}//end namespace world

#endif //__CAESARIA_EMPIRE_OBJECTSFACTORY_H_INCLUDED__
