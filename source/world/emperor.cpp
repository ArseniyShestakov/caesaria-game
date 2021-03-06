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

#include "emperor.hpp"
#include "core/foreach.hpp"
#include "game/funds.hpp"
#include "game/gamedate.hpp"
#include "events/showinfobox.hpp"
#include "romechastenerarmy.hpp"
#include "city.hpp"
#include "core/saveadapter.hpp"
#include "core/utils.hpp"
#include "empire.hpp"
#include "game/gift.hpp"
#include "core/variant_map.hpp"

namespace world
{

namespace {
static const unsigned int legionSoldiersCount = 16;
static const int maxFavour = 100;
static const int maxFavourUpdate = 5;
static const int yearlyDecreaseFavour = 2;
static const int taxBrokenFavourDecrease = 3;
static const int taxBrokenFavourDecreaseMax = 8;
static const int normalSalaryFavourUpdate = 1;
static const int minimumFabvour4wrathGenerate = 20;
static const int maxWrathPointValue = 20;
}

struct Relation
{
  int value;
  unsigned int soldiersSent;
  unsigned int lastSoldiersSent;
  DateTime lastGiftDate;
  DateTime lastTaxDate;
  unsigned int wrathPoint;
  unsigned int tryCount;
  int chastenerFailed;
  int lastGiftValue;
  bool debtMessageSent;

  Relation()
    : value( 0 ),
      soldiersSent( 0 ), lastSoldiersSent( 0 ),
      lastGiftDate( DateTime( -351, 1, 1 ) ),
      wrathPoint(0), tryCount( 0 ),
      lastGiftValue( 0 ), debtMessageSent(false)
  {

  }

  void removeSoldier()
  {
    if( soldiersSent > 0)
      --soldiersSent;
  }

  VariantMap save() const
  {
    VariantMap ret;
    VARIANT_SAVE_ANY(ret, lastGiftDate )
    VARIANT_SAVE_ANY(ret, lastTaxDate )
    VARIANT_SAVE_ANY(ret, value)
    VARIANT_SAVE_ANY(ret, lastGiftValue )
    VARIANT_SAVE_ANY(ret, soldiersSent )
    VARIANT_SAVE_ANY(ret, lastSoldiersSent)
    VARIANT_SAVE_ANY(ret, debtMessageSent )
    VARIANT_SAVE_ANY(ret, chastenerFailed )
    VARIANT_SAVE_ANY(ret, wrathPoint)
    VARIANT_SAVE_ANY(ret, tryCount)

    return ret;
  }

  void load( const VariantMap& stream )
  {
    VARIANT_LOAD_TIME(lastGiftDate, stream )
    VARIANT_LOAD_TIME(lastTaxDate, stream )
    VARIANT_LOAD_ANY(value, stream)
    VARIANT_LOAD_ANY(lastGiftValue, stream);
    VARIANT_LOAD_ANY(lastSoldiersSent, stream);
    VARIANT_LOAD_ANY(soldiersSent, stream)
    VARIANT_LOAD_ANY(debtMessageSent, stream)
    VARIANT_LOAD_ANY(chastenerFailed, stream)
    VARIANT_LOAD_ANY(wrathPoint, stream)
    VARIANT_LOAD_ANY(tryCount, stream)
  }
};

class Emperor::Impl
{
public:
  typedef std::map< std::string, Relation > Relations;

  Relations relations;
  Empire* empire;
  std::string name;

  void resolveTroubleCities( const CityList& cities );
};

Emperor::Emperor() : __INIT_IMPL(Emperor)
{
  __D_IMPL(d,Emperor)
  d->name = "Emperor";
}

Emperor::~Emperor(){}

int Emperor::relation(const std::string& cityname)
{
  __D_IMPL(d,Emperor)
  Impl::Relations::iterator i = d->relations.find( cityname );
  return ( i == d->relations.end() ? 0 : i->second.value );
}

void Emperor::updateRelation(const std::string& cityname, int value)
{
  __D_IMPL(d,Emperor)
  int current = d->relations[ cityname ].value;
  d->relations[ cityname ].value = math::clamp<int>( current + value, 0, maxFavour );
}

void Emperor::sendGift(const Gift& gift)
{
  Relation relation;
  Impl::Relations::iterator it = _dfunc()->relations.find( gift.sender() );
  if( it != _dfunc()->relations.end() )
  {
    relation = it->second;
  }

  int monthFromLastGift = math::clamp<int>( relation.lastGiftDate.monthsTo( game::Date::current() ),
                                            0, (int)DateTime::monthsInYear );

  float timeKoeff = monthFromLastGift / (float)DateTime::monthsInYear;
  int affectMoney = relation.lastGiftValue / ( monthFromLastGift + 1 );
  float moneyKoeff = math::max<float>( gift.value() - affectMoney, 0.f ) / gift.value();
  int favourUpdate = maxFavourUpdate * timeKoeff * moneyKoeff;
  relation.lastGiftDate = game::Date::current();
  relation.lastGiftValue = gift.value();

  updateRelation( gift.sender(), favourUpdate );
}

DateTime Emperor::lastGiftDate(const std::string& cityname)
{
  Relation relation;
  Impl::Relations::iterator it = _dfunc()->relations.find( cityname );
  if( it != _dfunc()->relations.end() )
  {
    relation = it->second;
    return DateTime( -350, 1, 1 );
  }

  return relation.lastGiftDate;
}

void Emperor::timeStep(unsigned int time)
{
  if( game::Date::isYearChanged() )
  {
    __D_IMPL(d,Emperor)

    CityList empireCities = d->empire->cities();
    foreach( it, empireCities )
    {
      CityPtr cityp = *it;

      if( !cityp->isAvailable() )
        continue;

      Relation& relation = d->relations[ cityp->name() ];

      relation.soldiersSent = 0;     //clear chasteners count
      relation.value -= yearlyDecreaseFavour;
      int monthWithoutTax = relation.lastTaxDate.monthsTo( game::Date::current() );
      if( monthWithoutTax > DateTime::monthsInYear )
      {
        int decrease = math::clamp( taxBrokenFavourDecrease + monthWithoutTax / DateTime::monthsInYear * 2, 0, taxBrokenFavourDecreaseMax );
        relation.value -= decrease;
      }

      float salaryKoeff = EmpireHelper::governorSalaryKoeff( cityp );
      if( salaryKoeff > 1.f ) { relation.value -= (int)salaryKoeff * salaryKoeff; }
      else if( salaryKoeff < 1.f ) { relation.value += normalSalaryFavourUpdate; }

      int brokenEmpireTax = cityp->treasury().getIssueValue( econ::Issue::overdueEmpireTax, econ::Treasury::lastYear );
      if( brokenEmpireTax > 0 )
      {
        relation.value -= 1;

        brokenEmpireTax = cityp->treasury().getIssueValue( econ::Issue::overdueEmpireTax, econ::Treasury::twoYearAgo );
        if( brokenEmpireTax > 0 )
          relation.value -= 2;
      }
    }
  }

  if( game::Date::isMonthChanged() )
  {
    __D_IMPL(d,Emperor)

    CityList troubleCities;
    foreach( it, d->relations )
    {
      CityPtr city = d->empire->findCity( it->first );
      Relation& relation = d->relations[ it->first ];

      if( city.isValid() )
      {
        if( relation.value < minimumFabvour4wrathGenerate  )
        {
          relation.wrathPoint += math::clamp( maxWrathPointValue - relation.value, 0, maxWrathPointValue );
          if( relation.soldiersSent == 0 )
            troubleCities << city;
        }
        else
        {
          relation.wrathPoint = 0;
        }
      }
    }

    d->resolveTroubleCities( troubleCities );
  }
}

void Emperor::Impl::resolveTroubleCities( const CityList& cities )
{
  foreach( it, cities )
  {
    Relation& relation = relations[ (*it)->name() ];
    float rule2destroy = utils::eventProbability( math::clamp( relation.wrathPoint / 100.f, 0.f, 1.f ),
                                                 math::clamp<int>( relation.tryCount, 0, DateTime::monthsInYear ),
                                                 DateTime::monthsInYear );

    relation.tryCount++;

    if( rule2destroy < 0.9 )
      continue;

    relation.wrathPoint = 0;
    relation.tryCount = 0;
    relation.soldiersSent = relation.lastSoldiersSent * 2;

    unsigned int sldrNumber = std::max( legionSoldiersCount, relation.soldiersSent );

    RomeChastenerArmyPtr army = RomeChastenerArmy::create( empire );
    army->setCheckFavor( true );
    army->setBase( empire->rome() );
    army->setSoldiersNumber( sldrNumber );
    army->attack( ptr_cast<Object>( *it ) );

    if( !army->isDeleted() )
    {
      relation.lastSoldiersSent = sldrNumber;
    }
    else
    {
      relation.chastenerFailed++;
    }
  }
}

void Emperor::remSoldiers(const std::string& cityname, int value)
{
  __D_IMPL(d,Emperor)
  for( int i=0; i < value; i++ )
  {
    d->relations[ cityname ].removeSoldier();
  }
}

void Emperor::addSoldiers(const std::string& name, int value)
{
  __D_IMPL(d,Emperor)
  Relation& relation = d->relations[ name ];
  relation.soldiersSent += value;
}

std::string Emperor::name() const { return _dfunc()->name; }
void Emperor::setName(const std::string& name){ _dfunc()->name = name; }

void Emperor::cityTax(const std::string &cityname, unsigned int money)
{
  __D_IMPL(d,Emperor)
  d->relations[ cityname ].lastTaxDate = game::Date::current();
}

void Emperor::resetRelations(const StringArray& cities)
{
  __D_IMPL(d,Emperor)
  CityList empCities;
  if( !cities.empty() )
  {
    if( cities.front() == "all" )
    {
      empCities = d->empire->cities();
    }
    else
    {
      foreach( it, cities )
      {
        CityPtr pCity = d->empire->findCity( *it );
        if( pCity.isValid() )
          empCities << pCity;
      }
    }

    foreach( it, empCities )
    {
      Relation& r = d->relations[ (*it)->name() ];

      r.value = maxFavour/2;
      r.lastGiftValue = 0;
      r.lastTaxDate = game::Date::current();
      r.lastGiftDate = game::Date::current();
    }
  }

}

void Emperor::checkCities()
{
  __D_IMPL(d,Emperor)
  CityList empireCities = d->empire->cities();
  foreach( it, empireCities )
  {
    if( !(*it)->isAvailable() )
      continue;

    if( d->relations.count( (*it)->name() ) == 0 )
    {
      Relation& relation = d->relations[ (*it)->name() ];
      relation.value = maxFavour/2;
    }
  }
}

VariantMap Emperor::save() const
{
  __D_IMPL_CONST(d,Emperor)
  VariantMap ret;

  Impl::Relations r = d->relations;
  VariantMap vm_relations;
  foreach( it, r )
  {
    vm_relations[ it->first ] = it->second.save();
  }

  ret[ "relations" ] = vm_relations;
  VARIANT_SAVE_STR_D( ret, d, name )
  return ret;
}

void Emperor::load(const VariantMap& stream)
{
  __D_IMPL(d,Emperor)
  VariantMap vm_relations = stream.get( "relations" ).toMap();

  Impl::Relations& relations = d->relations;
  foreach( it, vm_relations )
  {
    Relation r;
    r.load( it->second.toMap() );
    relations[ it->first ] = r;
  }

  VARIANT_LOAD_STR_D( d, name, stream )
}

void Emperor::init(Empire &empire)
{
  __D_IMPL(d,Emperor)
  d->empire = &empire;
}

struct EmperorInfo
{
  std::string name;
  DateTime beginReign;
  VariantMap options;
};

class EmperorLine::Impl
{
public:
  typedef std::map< DateTime, EmperorInfo> ChangeInfo;

  ChangeInfo changes;
};

EmperorLine& EmperorLine::instance()
{
  static EmperorLine inst;
  return inst;
}

std::string EmperorLine::getEmperor(DateTime time)
{
  foreach( it, _d->changes )
  {
    if( it->first >= time )
      return it->second.name;
  }

  return "";
}

VariantMap EmperorLine::getInfo(std::string name) const
{
  foreach( it, _d->changes )
  {
    if( name == it->second.name )
      return it->second.options;
  }

  return VariantMap();
}

void EmperorLine::load(vfs::Path filename)
{
  _d->changes.clear();

  VariantMap opts = config::load( filename );
  foreach( it, opts )
  {
    VariantMap opts = it->second.toMap();
    EmperorInfo info;
    info.beginReign = opts.get( "date" ).toDateTime();
    info.name = opts.get( "name" ).toString();
    info.options = opts;

    _d->changes[ info.beginReign ] = info;
  }
}

EmperorLine::EmperorLine() : _d( new Impl )
{

}

}
