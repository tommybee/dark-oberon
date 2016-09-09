/*
 * -------------
 *  Dark Oberon
 * -------------
 * 
 * An advanced strategy game.
 *
 * Copyright (C) 2002 - 2005 Valeria Sventova, Jiri Krejsa, Peter Knut,
 *                           Martin Kosalko, Marian Cerny, Michal Kral
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License (see docs/gpl.txt) as
 * published by the Free Software Foundation; either version 2 of the License,
 * or (at your option) any later version.
 */

/**
 *  @file dofight.cpp       
 *
 *  Armaments classes, fight and combat functions.
 *
 *  @author Jiri Krejsa
 *
 *  @date 2003
 */

#include "dofight.h"
#include "dounits.h"
#include <cstdlib>
#include <math.h>


//=========================================================================
// Defines
//=========================================================================

#define FIGCPP_LLX      0         //!< Index into array with differences of the positions - abs(LeftX1 - LeftX2)
#define FIGCPP_LRX      1         //!< Index into array with differences of the positions - abs(LeftX1 - RightX2)
#define FIGCPP_RLX      2         //!< Index into array with differences of the positions - abs(RightX1 - LeftX2)
#define FIGCPP_RRX      3         //!< Index into array with differences of the positions - abs(RightX1 - RightX2)
#define FIGCPP_LLY      4         //!< Index into array with differences of the positions - abs(LeftY1 - LeftY2)
#define FIGCPP_LRY      5         //!< Index into array with differences of the positions - abs(LeftY1 - RightY2)
#define FIGCPP_RLY      6         //!< Index into array with differences of the positions - abs(RightY1 - LeftY2)
#define FIGCPP_RRY      7         //!< Index into array with differences of the positions - abs(RightY1 - RightY2)

#define FIGCPP_ACCURACY_MODIFIER    0.333333f   //!< Modifier of the accuracy of the gun used in the function TARMAMENT::CalculateRealImpactPosition().


//=========================================================================
// Functions definitions

//=========================================================================
// Methods definitions of the class TGUN
//=========================================================================

TGUN::TGUN()
{
  accuracy = 0.0f;
  feed_time = 0.0;
  shot_time = 0.0;
  wait_time = 0.0;
  flags = FIG_GUN_NOTHING;
  power.min = 0;
  power.max = 0;
  range.min = 0;
  range.max = 0;
  shotableSeg.min = 0;
  shotableSeg.max = DAT_SEGMENTS_COUNT - 1;
  item = NEW TPROJECTILE_ITEM;
}

TGUN::~TGUN()
{
  if (item)
    delete item;
}


/**
 *  Sets projectile item. Old item will be deleted.
 *
 *  @param nitem  New projectile item.
 */
void TGUN::SetItem(TPROJECTILE_ITEM *nitem)
{
  if (item) delete item;
  item = nitem;
}


/**
 *  Attack position pos. The method doesn't test whether attacking position is in the range.
 *
 *  @param defender Pointer to defender.
 */
void TGUN::ExecuteAttack(TMAP_UNIT *defender) const
{
  float protection_chance = (GetRandomFloat() * 0.2f) + 0.4f;     // variable part of the defend - between 0.4 and 0.6
  float potention_chance = GetRandomFloat();      // variable part of the gun potential - between 0 and 1
  float defend_number;                            // defend number represent value of the defend
  float wound = 0;                                // wound made by the attack

  if (!defender)                    //control of validate of parameters
    return;

  TARMAMENT &defender_armament = *(static_cast<TMAP_ITEM*>(defender->GetPointerToItem())->GetArmament());     // auxiliary variable for faster access to memory

  defend_number = protection_chance * defender_armament.GetDefense()->GetProtection();

  if (defender->DoesAttackTakeEffect(flags))
  {
    wound = ((power.min + (power.max - power.min) * potention_chance) / GetProjectileItem()->GetAttackedMapels()) / defender_armament.GetDefense()->GetArmour();

    wound = wound * (1 - defend_number);

    defender->Injure(wound);
  }
}


//=========================================================================
// Methods definitions of the class TARMAMENT
//=========================================================================

TARMAMENT::TARMAMENT()
{
  this->defense = NEW TDEFENSE;
  this->offensive = NEW TGUN;

  SetWorth(0);
}


/**
 *  Tests whether is possible attack unit. Returns TATTACK_INFO with state
 *  of the attack, impact position, distance and direction of the attack.
 *
 *  @param attacker Pointer to attacked unit.
 *  @param defender Pointer to defending unit.
 *  @param only_test  Auxiliary flag which informs that method is used 
 *  only for target testing and not for target attacking. (default false)
 */
TATTACK_INFO TARMAMENT::IsPossibleAttack(const TMAP_UNIT* attacker, const TMAP_UNIT* defender, const bool only_test) const
{
  TATTACK_INFO result;
  int distances[8] = {0, };
  int x_index_min = 0, x_index_max = 0, y_index_min = 0, y_index_max = 0;
  TINTERVAL<int> dist;
  int i;
  int min_distance, max_distance;   //quadrates of minimum and maximum distances
  TPOSITION_3D help_pos;
  int segment_distance = 0;

  if ((!attacker) || (!defender) || defender->TestState(US_DYING) || defender->TestState(US_ZOMBIE) || defender->TestState(US_DELETE))           //control of validate of parameters
  {
    result.state = FIG_AIF_ATTACK_FAILED;
    return result;
  }

  //unit cannot attack
  if (offensive == NULL) {
    result.state = FIG_AIF_ATTACK_FAILED;
    return result;
  }

  if (!defender->ExistInSegment(offensive->GetShotableSegments().min, offensive->GetShotableSegments().max))
  {
    result.state = FIG_AIF_UNSHOTABLE_SEG;
    return result;
  }

  //Flags testing.

  // FIG_GUN_NOTLANDING
  if ((offensive->GetFlags() & FIG_GUN_NOTLANDING) 
      && (attacker->TestState(US_LANDING) || attacker->TestState(US_ANCHORING) || attacker->TestState(US_UNLANDING)))
  {     //attacker can attack only if not landing
    result.state = FIG_AIF_START_FIRST;
    return result;
  }

  // FIG_GUN_DAMAGE_BUILDINGS, FIG_GUN_DAMAGE_SOURCES
  if (!defender->DoesAttackTakeEffect(offensive->GetFlags())){
    result.state = FIG_AIF_ATTACT_NOT_EFFECIVE;
    return result;
  }

  //because moving units are present only in one segment and not moving units are present in exist segment all at once
  //is necessary to do different test
  const TFORCE_UNIT *fattacker = dynamic_cast<const TFORCE_UNIT*>(attacker);
  const TFORCE_UNIT *fdefender = dynamic_cast<const TFORCE_UNIT*>(defender);
  //attacker and defender are moveable units
  if ((fdefender != NULL) && (fattacker != NULL))
  {
    //attacker can attack only in the same segment but target is in the another one
    if (offensive->GetFlags() & FIG_GUN_SAME_SEGMENT)
    {
      if (attacker->GetPosition().segment > defender->GetPosition().segment)        //target is in the lower segment
      {
        result.state = FIG_AIF_TO_LOWER_SEG;
        return result;
      }
      else if (attacker->GetPosition().segment < defender->GetPosition().segment)   //target is in the upper segment
      {
        result.state = FIG_AIF_TO_UPPER_SEG;
        return result;
      }
    }
    segment_distance = static_cast<int>(attacker->GetPosition().segment) - defender->GetPosition().segment;
  }
  //only defender is moveable unit
  else if (fdefender != NULL)
  {
    if (! static_cast<TMAP_ITEM*>(attacker->GetPointerToItem())->GetExistSegments().IsMember(fdefender->GetPosition().segment))
    {
      //attacker can attack only in the same segment but target is in the another one
      if (offensive->GetFlags() & FIG_GUN_SAME_SEGMENT)
      {
        result.state = FIG_AIF_UNSHOTABLE_SEG;
        return result;
      }
      segment_distance = static_cast<int>(fdefender->GetPosition().segment) - static_cast<TMAP_ITEM*>(attacker->GetPointerToItem())->GetExistSegments().max;
      if (segment_distance > 0)
        segment_distance *= -1;
      else
        segment_distance = static_cast<int>(static_cast<TMAP_ITEM*>(attacker->GetPointerToItem())->GetExistSegments().min) - fdefender->GetPosition().segment;
    }
    else
    {
      segment_distance = 0;
    }
  }
  //only attacker is moveable unit
  else if (fattacker != NULL)
  {
    if (static_cast<TMAP_ITEM*>(defender->GetPointerToItem())->GetExistSegments().max < fattacker->GetPosition().segment)
    {
      //attacker can attack only in the same segment but target is in the another one
      if (offensive->GetFlags() & FIG_GUN_SAME_SEGMENT)
      {
        result.state = FIG_AIF_TO_LOWER_SEG;
        return result;
      }
      segment_distance = static_cast<int>(fattacker->GetPosition().segment) - static_cast<TMAP_ITEM*>(defender->GetPointerToItem())->GetExistSegments().max;
      segment_distance *= -1;
    }
    else if (static_cast<TMAP_ITEM*>(defender->GetPointerToItem())->GetExistSegments().min > fattacker->GetPosition().segment)
    {
      //attacker can attack only in the same segment but target is in the another one
      if (offensive->GetFlags() & FIG_GUN_SAME_SEGMENT)
      {
        result.state = FIG_AIF_TO_UPPER_SEG;
        return result;
      }
      segment_distance = static_cast<int>(static_cast<TMAP_ITEM*>(defender->GetPointerToItem())->GetExistSegments().min) - fattacker->GetPosition().segment;
    }
    else
      segment_distance = 0;
  }
  //both units are unmoveable units
  else
  {
    if (static_cast<TMAP_ITEM*>(defender->GetPointerToItem())->GetExistSegments().min > static_cast<TMAP_ITEM*>(attacker->GetPointerToItem())->GetExistSegments().max)
    {
      //attacker can attack only in the same segment but target is in the another one
      if (offensive->GetFlags() & FIG_GUN_SAME_SEGMENT)
      {
        result.state = FIG_AIF_UNSHOTABLE_SEG;
        return result;
      }
      segment_distance = static_cast<int>(static_cast<TMAP_ITEM*>(defender->GetPointerToItem())->GetExistSegments().min) - static_cast<TMAP_ITEM*>(attacker->GetPointerToItem())->GetExistSegments().max;
      segment_distance *= -1;
    }
    else if (static_cast<TMAP_ITEM*>(defender->GetPointerToItem())->GetExistSegments().max < static_cast<TMAP_ITEM*>(attacker->GetPointerToItem())->GetExistSegments().min)
    {
      //attacker can attack only in the same segment but target is in the another one
      if (offensive->GetFlags() & FIG_GUN_SAME_SEGMENT)
      {
        result.state = FIG_AIF_UNSHOTABLE_SEG;
        return result;
      }
      segment_distance = static_cast<int>(static_cast<TMAP_ITEM*>(attacker->GetPointerToItem())->GetExistSegments().min) - static_cast<TMAP_ITEM*>(defender->GetPointerToItem())->GetExistSegments().max;
    }
    else
      segment_distance = 0;
  }

  //Distance testing

  //calculate of the differences
  distances[FIGCPP_LLX] = abs(static_cast<int>(attacker->GetPosition().x) - defender->GetPosition().x);
  distances[FIGCPP_LRX] = abs(static_cast<int>(attacker->GetPosition().x) - (defender->GetPosition().x + defender->GetUnitWidth() - 1));
  distances[FIGCPP_RLX] = abs((static_cast<int>(attacker->GetPosition().x) + attacker->GetUnitWidth() - 1) - defender->GetPosition().x);
  distances[FIGCPP_RRX] = abs((static_cast<int>(attacker->GetPosition().x) + attacker->GetUnitWidth()) - (defender->GetPosition().x + defender->GetUnitWidth()));
  distances[FIGCPP_LLY] = abs(static_cast<int>(attacker->GetPosition().y) - defender->GetPosition().y);
  distances[FIGCPP_LRY] = abs(static_cast<int>(attacker->GetPosition().y) - (defender->GetPosition().y + defender->GetUnitHeight() - 1));
  distances[FIGCPP_RLY] = abs((static_cast<int>(attacker->GetPosition().y) + attacker->GetUnitHeight() - 1) - defender->GetPosition().y);
  distances[FIGCPP_RRY] = abs((static_cast<int>(attacker->GetPosition().y) + attacker->GetUnitHeight()) - (defender->GetPosition().y + defender->GetUnitHeight()));

  //searching of the minimum and maximum in x coordinates and corresponding indices
  dist.max = 0;
  dist.min = LAY_UNAVAILABLE_POSITION;

  for (i = 0; i < 4; i++)
  {
    if (distances[i] < dist.min)
    {
      dist.min = distances[i];
      x_index_min = i;
    }
    if (distances[i] > dist.max)
    {
      dist.max = distances[i];
      x_index_max = i;
    }
  }
  //searching of the minimum and maximum in y coordinates and corresponding indices
  dist.max = 0;
  dist.min = LAY_UNAVAILABLE_POSITION;

  for (; i < 8; i++)
  {
    if (distances[i] < dist.min)
    {
      dist.min = distances[i];
      y_index_min = i;
    }
    if (distances[i] > dist.max)
    {
      dist.max = distances[i];
      y_index_max = i;
    }
  }
  //if range of the gun is equal to one it is necessary to test apart
  if ((offensive->GetRange().min == 1) && (offensive->GetRange().max == 1))     //range is equal to one
  {
    if (
      attacker->GetPosition().x >= int(defender->GetPosition().x) - attacker->GetUnitWidth() &&
      attacker->GetPosition().x <= int(defender->GetPosition().x) + defender->GetUnitWidth() &&
      attacker->GetPosition().y >= int(defender->GetPosition().y) - attacker->GetUnitHeight() &&
      attacker->GetPosition().y <= int(defender->GetPosition().y) + defender->GetUnitHeight() &&
      (abs(segment_distance) <= 1)
    )
    {           //defender is next to the attacker
      if (!only_test)   //if is used just for testing then is needn't to compute impact position and direction
      {
        CalculateImpactPosition(attacker, defender, result, segment_distance);
        if (fattacker != NULL) 
          TestDirection(fattacker, result);
      }
      return result;
    }
    else        //defender is too far away
    {
      result.state = FIG_AIF_TOO_FAR_AWAY;
      return result;
    }
  }

  //calculate quadrates of minimum and maximum distance and of the range
  i = sqr(segment_distance);

  min_distance = sqr(distances[x_index_min]) + sqr(distances[y_index_min]) + i;
  max_distance = sqr(distances[x_index_max]) + sqr(distances[y_index_max]) + i;

  dist.min = sqr(static_cast<TMAP_ITEM*>(attacker->GetPointerToItem())->GetArmament()->GetOffensive()->GetRange().min);
  dist.max = sqr(static_cast<TMAP_ITEM*>(attacker->GetPointerToItem())->GetArmament()->GetOffensive()->GetRange().max);

  if (dist.IsMember(min_distance) || dist.IsMember(max_distance) || 
      ((min_distance < dist.min) && (max_distance > dist.max)))   //target is in the range
  {
    //if is used just for testing then is needn't to compute impact position and direction
    if (!only_test)
    {
      CalculateImpactPosition(attacker, defender, result, segment_distance);
      if (fattacker != NULL)
        TestDirection(fattacker, result);
      else
        result.state = FIG_AIF_ATTACK_OK;
    } 
    else 
      result.state = FIG_AIF_ATTACK_OK;
  }
  else if (min_distance > dist.max)                 //target is too far away
  {
    result.state = FIG_AIF_TOO_FAR_AWAY;
  }
  else if (max_distance < dist.min)                 //target is too close to attacker
  {
    result.state = FIG_AIF_TOO_CLOSE_TO;
  }
  else
  {
    result.state = FIG_AIF_ATTACK_FAILED;
  }

  return result;
}

/**
 *  Calculate best impact positon and involve impact deviation. As best impact position calculates middle
 *  of the defender. The impact deviation is computed by private method CalculateRealImpactPositon().
 *
 *  @param attacker Pointer to attacked unit.
 *  @param defender Pointer to defending unit.
 *  @param info Reference to info about attack run.
 *  @param segment_distance Segment distance of the units.
 */
inline void TARMAMENT::CalculateImpactPosition(const TMAP_UNIT* attacker, const TMAP_UNIT* defender, TATTACK_INFO &info, const int segment_distance) const
{
  TPOSITION_3D help_pos(LAY_UNAVAILABLE_POSITION, LAY_UNAVAILABLE_POSITION, LAY_UNAVAILABLE_POSITION);
  TPOSITION_3D real_impact_pos;
  float distance = 0.0;
  
  if ((!attacker) || (!defender))           //control of validate of parameters
    info.state = FIG_AIF_ATTACK_FAILED;

  // calculate of middle field of the attacker
  help_pos.x = attacker->GetPosition().x + attacker->GetUnitWidth()/2;
  help_pos.y = attacker->GetPosition().y + attacker->GetUnitHeight()/2;
  help_pos.segment = attacker->GetPosition().segment - segment_distance;

  // calculate of impact position - now it is simple middle of the defender
  info.impact_position.SetPosition(defender->GetSynchronisedPosition().x + defender->GetUnitWidth()/2, 
                                   defender->GetSynchronisedPosition().y + defender->GetUnitHeight()/2,
                                   attacker->GetPosition().segment - segment_distance);

  real_impact_pos.SetPosition(defender->GetPosition().x + defender->GetUnitWidth()/2, 
                                   defender->GetPosition().y + defender->GetUnitHeight()/2,
                                   attacker->GetPosition().segment - segment_distance);

  distance = static_cast<float>(sqrt(
    static_cast<float>Sqr(static_cast<int>(help_pos.x) - static_cast<int>(info.impact_position.x))
    + Sqr(static_cast<int>(help_pos.y) - static_cast<int>(info.impact_position.y))
    + Sqr(static_cast<int>(help_pos.segment) - static_cast<int>(info.impact_position.segment))
  ));

  CalculateRealImpactPosition(info, distance);    //calculate real impact position (includes inaccuracy)

  // filling valid data into TATTACK_INFO structure
  info.direction = help_pos.GetDirection(real_impact_pos);
}


/**
 *  Calculate real impact position. Result impact position dependences on accuracy of gun, distance between attacker and 
 *  defender and on fortuity.
 *  Coordinates are modified according to this formula: 
 *            info.impact_position.x + static_cast<T_SIMPLE>((2 * GetRandomInt(2) - 1) * GetRandomFloat() * FIGCPP_ACCURACY_MODIFIER * dist * (1 - GetOffensive()->GetAccuracy()))
 *
 *  @param info Reference to info about attack run.
 *  @param dist The distance between start and impact position.
 */
inline void TARMAMENT::CalculateRealImpactPosition(TATTACK_INFO &info, const float dist) const
{
  int x_pos, y_pos;

  //calculate real impact position
  x_pos = info.impact_position.x + int((2 * GetRandomInt(2) - 1) * GetRandomFloat() * FIGCPP_ACCURACY_MODIFIER * dist * (1 - GetOffensive()->GetAccuracy()));
  y_pos = info.impact_position.y + int((2 * GetRandomInt(2) - 1) * GetRandomFloat() * FIGCPP_ACCURACY_MODIFIER * dist * (1 - GetOffensive()->GetAccuracy()));
  
  
  info.impact_position.x = T_SIMPLE (MAX (MIN (x_pos, map.width), 0));
  info.impact_position.y = T_SIMPLE (MAX (MIN (y_pos, map.height), 0));
}


/** 
 *  The method tests equality of the attacker look direction and shot direction.
 *
 *  @parameter attacker Pointer to attacker unit which is moveable.
 *  @parameter info     Reference to attack info unit which is used as input
 *  and output parameter
 */
inline void TARMAMENT::TestDirection(const TFORCE_UNIT *attacker, TATTACK_INFO &info) const
{
  //if direction of shot is not same as look direction of the moveable unit
  if (info.direction != attacker->GetLookDirection())
  {
    //get direction which is border between left and right rotate
    short border = attacker->GetLookDirection() -  4;
    if (border < 0)
    {
      border += LAY_HOR_DIRECTIONS_COUNT;
      if ((info.direction > attacker->GetLookDirection()) && (info.direction < border))
        info.state = FIG_AIF_TURN_RIGHT;
      else
        info.state = FIG_AIF_TURN_LEFT;
    }
    else
    {
      if ((info.direction < attacker->GetLookDirection()) && (border < info.direction))
        info.state = FIG_AIF_TURN_LEFT;
      else
        info.state = FIG_AIF_TURN_RIGHT;
    }
  }
  else
    info.state = FIG_AIF_ATTACK_OK;
}

//=========================================================================
// END
//=========================================================================
// vim:ts=2:sw=2:et:

