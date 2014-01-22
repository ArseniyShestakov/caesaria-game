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
// Copyright 2012-2013 Gregoire Athanase, gathanase@gmail.com
// Copyright 2012-2013 Dalerank, dalerankn8@gmail.com

#ifndef _CAESARIA_CART_PUSHER_H_INCLUDE_
#define _CAESARIA_CART_PUSHER_H_INCLUDE_

#include "walker/walker.hpp"
#include "core/predefinitions.hpp"

/** This walker delivers goods */
class CartPusher : public Walker
{
public:
  static CartPusherPtr create( PlayerCityPtr city );

  void setProducerBuilding( BuildingPtr building );
  void setConsumerBuilding( BuildingPtr building );
  BuildingPtr getProducerBuilding();
  BuildingPtr getConsumerBuilding();
  GoodStock& getStock();

  Picture& getCartPicture();
  virtual void getPictureList(PicturesArray& oPics);

  void send2city( BuildingPtr building, GoodStock& carry );

  void computeWalkerDestination();
 
  virtual void timeStep(const unsigned long time);

  virtual void save(VariantMap& stream) const;
  virtual void load(const VariantMap& stream);
  virtual void die();
  virtual std::string getThinks() const;

protected:
  CartPusher( PlayerCityPtr city );

  virtual void _changeDirection();
  virtual void _reachedPathway();

private:
  class Impl;
  ScopedPtr< Impl > _d;
};

#endif //_CAESARIA_CART_PUSHER_H_INCLUDE_
