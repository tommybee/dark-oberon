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
 *  @file doworkers.cpp
 *
 *  Working with worker units.
 *
 *  @author Peter Knut
 *  @author Valeria Sventova 
 *  @author Jiri Krejsa
 *
 *  @date 2002, 2003, 2004
 */

//=========================================================================
// Included files
//=========================================================================

#include <math.h>

#include "dodata.h"
#include "dodraw.h"
#include "dologs.h"
#include "domouse.h"
#include "doplayers.h"
#include "dosimpletypes.h"
#include "dounits.h"
#include "dohost.h"
#include "doselection.h"


//=========================================================================
//  class TWORKER_UNIT
//=========================================================================

/**
 *  Method is calling when new event is get from queue. Returns true if unit do something.
 *
 *  @param Pointer to processed event.
 */
void TWORKER_UNIT::ProcessEvent(TEVENT * proc_event)
{
  int last_move_dir = move_direction;
  unsigned int last_state = state;
  TPOSITION_3D last_ps = GetPosition();

  TWORKER_ITEM * itm = static_cast<TWORKER_ITEM *>(pitem);

  int new_direction = LAY_UNDEFINED;
  unsigned int new_state = US_NONE;
  TPOSITION_3D new_ps;
  double new_time_stamp;
  bool new_priority;
  int new_int = 0;
  T_SIMPLE new_simple5 = 0, new_simple6 = 0;

  TSOURCE_UNIT * new_source;
  TMAP_UNIT * new_hider;
  TMAP_UNIT * new_target;
  TBASIC_UNIT * new_build;
  TBUILDING_UNIT * new_acceptor;
  TEVENT * path_event = NULL;
  
  int i, j, test_direction;
  unsigned int hardest = 0;
  TPOSITION_3D test_ps;
  bool change_position;
  bool all_moveable;
  bool first_mine = false;
  
  double state_time = 0;

  // event US_WAIT_FOR_PATH only has to come - no computation
  if (proc_event->TestEvent(US_WAIT_FOR_PATH)) {
    if ((last_state == US_LANDING) || (last_state == US_ANCHORING) || (last_state == US_UNLANDING)) 
      return;
  }
  
  // test if event is event (else it is request)
  if (proc_event->GetEvent() < RQ_FIRST) PutState(proc_event->GetEvent());

  /****************** Undo Actions (ONLY LOCAL UNITS) *************************/

  if (!proc_event->TestLastEvent(US_NONE)) { // compute only local units

    
    if (proc_event->TestLastEvent(US_MINING) || proc_event->TestLastEvent(US_NEXT_MINE) ||
       proc_event->TestLastEvent(US_UNLOADING) || proc_event->TestLastEvent(US_NEXT_UNLOADING) ||
       proc_event->TestLastEvent(US_HIDING) || proc_event->TestLastEvent(US_NEXT_HIDING))
    {
      // send info that unit end action
      TEVENT * hlp;

      hlp = pool_events->GetFromPool();

      if (proc_event->TestLastEvent(US_MINING) || proc_event->TestLastEvent(US_NEXT_MINE)) {
        if (held) source->RemoveFromListOfUnits(this, LIST_WORKING);

        hlp->SetEventProps(proc_event->GetPlayerID(), proc_event->GetUnitID(), false, proc_event->GetTimeStamp(), RQ_END_MINE, US_NONE, -1, pos.x, pos.y, pos.segment, move_direction, 0, 0, 0);
      }

      if (proc_event->TestLastEvent(US_UNLOADING) || proc_event->TestLastEvent(US_NEXT_UNLOADING)) {
        if (held) acceptor->RemoveFromListOfUnits(this, LIST_WORKING);

        hlp->SetEventProps(proc_event->GetPlayerID(), proc_event->GetUnitID(), false, proc_event->GetTimeStamp(), RQ_END_UNLOAD, US_NONE, -1, pos.x, pos.y, pos.segment, move_direction, 0, 0, 0);
      }

      if (proc_event->TestLastEvent(US_HIDING) || proc_event->TestLastEvent(US_NEXT_HIDING)) {
        if (held) hider->RemoveFromListOfUnits(this, LIST_HIDING);

        hlp->SetEventProps(proc_event->GetPlayerID(), proc_event->GetUnitID(), false, proc_event->GetTimeStamp(), US_EJECTING, US_NONE, -1, pos.x, pos.y, pos.segment, move_direction, 0, 0, 0);
      }

      SendNetEvent(hlp, all_players);
      pool_events->PutToPool(hlp);
      proc_event->SetTimeStamp(proc_event->GetTimeStamp() + TS_MIN_EVENTS_DIFF);
    }

    ReleaseCountedPointer(built_or_repaired_unit);
    ReleaseCountedPointer(source);
    ReleaseCountedPointer(acceptor);
    ReleaseCountedPointer(target);
    ReleaseCountedPointer(hider);

    sound_request_id = 0;
  }
  
  /****************** Synchronisation and setup (ALL UNITS) *************************/
  
  // RQ_DELETE
  if (proc_event->TestEvent(RQ_DELETE))
    PutState(US_DELETE);

  // RQ_ZOMBIE,
  if (proc_event->TestEvent(RQ_ZOMBIE))
    PutState(US_ZOMBIE);

  // RQ_DYING
  if (proc_event->TestEvent(RQ_DYING))
    PutState(US_DYING);

  // RQ_FEEDING
  if (proc_event->TestEvent(RQ_FEEDING))
    if (TestState(US_ATTACKING)) PutState(US_FEEDING);
  
  
  // US_MOVE, US_LANDING, US_UNLANDING
  // US_NEXT_STEP, US_TRY_TO_MOVE
  // US_RIGHT_ROTATING, US_LEFT_ROTATING
  // US_STAY, US_ANCHORING
  // US_DYING, US_ZOMBIE
  // US_START_REPAIR, US_CONSTRUCTING, US_REPAIRING
  // US_START_MINE, US_NEXT_MINE, US_MINING, 
  // US_START_UNLOAD, US_NEXT_UNLOADING, US_UNLOADING
  // US_START_HIDING, US_NEXT_HIDING
  // US_NEXT_ATTACK, US_ATTACKING
  // US_HEALING
  if (
    proc_event->TestEvent(US_MOVE) || proc_event->TestEvent(US_LANDING) || proc_event->TestEvent(US_UNLANDING) ||
    proc_event->TestEvent(US_NEXT_STEP) || proc_event->TestEvent(US_TRY_TO_MOVE) ||
    proc_event->TestEvent(US_RIGHT_ROTATING) || proc_event->TestEvent(US_LEFT_ROTATING) ||
    proc_event->TestEvent(US_STAY) || proc_event->TestEvent(US_ANCHORING) || 
    proc_event->TestEvent(US_DYING) || proc_event->TestEvent(US_ZOMBIE) ||
    proc_event->TestEvent(US_START_REPAIR) || proc_event->TestEvent(US_CONSTRUCTING) || proc_event->TestEvent(US_REPAIRING) || 
    proc_event->TestEvent(US_START_MINE) || proc_event->TestEvent(US_NEXT_MINE) || proc_event->TestEvent(US_MINING) || 
    proc_event->TestEvent(US_START_UNLOAD) || proc_event->TestEvent(US_NEXT_UNLOADING) || proc_event->TestEvent(US_UNLOADING) ||
    proc_event->TestEvent(US_START_HIDING) || proc_event->TestLastEvent(US_NEXT_HIDING) ||
    proc_event->TestEvent(US_NEXT_ATTACK) || proc_event->TestEvent(US_ATTACKING) ||
    proc_event->TestEvent(US_HEALING)
    )
  {
    if (!player_array.IsRemote(proc_event->GetPlayerID()) || (player_array.IsRemote(proc_event->GetPlayerID()) && ((IsSelectedPositionAvailable(TPOSITION_3D(proc_event->simple1, proc_event->simple2, proc_event->simple3)) && IsInMap()) || !IsInMap())))
    { //position from proc_event is available to move (this test should solve synchronisation problem between network players)

#if DEBUG_EVENTS
      if ((IsInMap()) && (!IsSelectedPositionAvailable(TPOSITION_3D(proc_event->simple1, proc_event->simple2, proc_event->simple3))))
        Debug(LogMsg("Bad position (%d,%d,%d) of local unit %d!", proc_event->simple1, proc_event->simple2, proc_event->simple3, proc_event->GetUnitID()));
#endif

      SetDirections(proc_event->simple4);

      // if position has changed, set new view
      if ((proc_event->simple1 != pos.x) || (proc_event->simple2 != pos.y)) {
        if (abs(proc_event->simple1 - pos.x) <= 1 && abs(proc_event->simple2 - pos.y) <= 1 && (proc_event->simple3 == pos.segment)) {
          SetViewDirection(pos.GetDirection(TPOSITION_3D(proc_event->simple1, proc_event->simple2, proc_event->simple3)));
          SetPosition(proc_event->simple1, proc_event->simple2, proc_event->simple3);
        }
        else {
          SetView(false);
          SetPosition(proc_event->simple1, proc_event->simple2, proc_event->simple3);
          SetView(true);
        }
      }
      else {
        if (proc_event->simple3 != pos.segment){
          SetView(false);
          SetPosition(proc_event->simple1, proc_event->simple2, proc_event->simple3);
          SetView(true);
        }
      }

      // if position of unit has changed
      if ((last_ps.x != pos.x) || (last_ps.y != pos.y) || (last_ps.segment != pos.segment)) {
        // update global map
        for (i = last_ps.x; i < last_ps.x + GetUnitWidth(); i++)
          for (j = last_ps.y; j < last_ps.y + GetUnitHeight(); j++)
            map.segments[last_ps.segment].surface[i][j].unit = NULL;   //updating global map - leaving old position

        for (i = pos.x; i < pos.x + GetUnitWidth(); i++)
          for (j = pos.y; j < pos.y + GetUnitHeight(); j++)
          {
            map.segments[pos.segment].surface[i][j].unit = this;   //updating global map - taking up new position
            map.segments[pos.segment].surface[i][j].GetAimersList()->AttackEnemy(this, false);
            map.segments[pos.segment].surface[i][j].GetWatchersList()->AttackEnemy(this, true);
          }

        // update local map of all local players
        for (i = 0; i < player_array.GetCount(); i++){
          if (!player_array.IsRemote(i))       
            players[i]->UpdateLocalMap(last_ps, GetPosition(), this);
        }
      }
    }
#if DEBUG_EVENTS
    else
      if (IsInMap()){
        Debug(LogMsg("Bad position (%d,%d,%d) of remote unit %d!", proc_event->simple1, proc_event->simple2, proc_event->simple3, proc_event->GetUnitID()));
        SetSynchronisedPosition(TPOSITION_3D(proc_event->simple1, proc_event->simple2, proc_event->simple3));
      }
#endif
  }

  // US_START_MINE
  if (proc_event->TestEvent(US_START_MINE) || proc_event->TestEvent(US_NEXT_MINE) || proc_event->TestEvent(US_MINING))
  {
    // translate source from id to pointer
    new_source = (TSOURCE_UNIT *)players[proc_event->simple5]->hash_table_units.GetUnitPointer(proc_event->int1);
    if (new_source != source) {
      ReleaseCountedPointer(source);
      if (new_source) source = (TSOURCE_UNIT *)new_source->AcquirePointer();
      if (source) {
        T_BYTE old_mined_material = mined_material;
        mined_material = static_cast<TSOURCE_ITEM *>(source->GetPointerToItem())->GetOfferMaterial();
        if (mined_material != old_mined_material) change_mined_material = true;
      }
    }

    if (!source) {
      ClearActions();
      SendEvent(false, proc_event->GetTimeStamp(), US_NEXT_STEP, -1, pos.x, pos.y, pos.segment, move_direction);
    }
  }

  // RQ_END_MINE
  if (proc_event->TestEvent(RQ_END_MINE)){
    if (TestState(US_MINING)) // it is possible that unit on leader didn't mine (source is empty), but RQ_END_MINE was send.
    {
      SetDirections(proc_event->simple4);
      SetPosition(proc_event->simple1, proc_event->simple2, proc_event->simple3);
    
      if (source->IsInsideMining()) 
        AddToMap(true, true);
    
      source->RemoveFromListOfUnits(this, LIST_WORKING);
      ReleaseCountedPointer(source);
    }
  }

  // US_START_UNLOAD
  if (proc_event->TestEvent(US_START_UNLOAD) || proc_event->TestEvent(US_NEXT_UNLOADING) || proc_event->TestEvent(US_UNLOADING))
  {
    // translate acceptor from id to pointer
    new_acceptor = (TBUILDING_UNIT *)players[proc_event->GetPlayerID()]->hash_table_units.GetUnitPointer(proc_event->int1);
    if (new_acceptor != acceptor) {
      ReleaseCountedPointer(acceptor);
      if (new_acceptor) acceptor = (TBUILDING_UNIT *)new_acceptor->AcquirePointer();
    }

    if (!acceptor) {
      ClearActions();
      SendEvent(false, proc_event->GetTimeStamp(), US_NEXT_STEP, -1, pos.x, pos.y, pos.segment, move_direction);
    }
  }
  
  // RQ_END_UNLOAD
  if (proc_event->TestEvent(RQ_END_UNLOAD)){
    SetDirections(proc_event->simple4);
    SetPosition(proc_event->simple1, proc_event->simple2, proc_event->simple3);
    
    AddToMap(true, true);
    acceptor->RemoveFromListOfUnits(this, LIST_WORKING);
    ReleaseCountedPointer(acceptor);
  }

  // US_START_REPAIR  
  if (proc_event->TestEvent(US_START_REPAIR) || proc_event->TestEvent(US_CONSTRUCTING) || proc_event->TestEvent(US_REPAIRING))
  {
    // translate built_or_repair_unit from id to pointer
    new_build = (TBASIC_UNIT *)players[proc_event->GetPlayerID()]->hash_table_units.GetUnitPointer(proc_event->int1);
    if (new_build != built_or_repaired_unit) {
      ReleaseCountedPointer(built_or_repaired_unit);
      if (new_build) built_or_repaired_unit = (TBASIC_UNIT *)new_build->AcquirePointer();
    }

    if (!built_or_repaired_unit) {
      ClearActions();
      SendEvent(false, proc_event->GetTimeStamp(), US_NEXT_STEP, -1, pos.x, pos.y, pos.segment, move_direction);
    }
  }

  // US_START_HIDING
  if (proc_event->TestEvent(US_START_HIDING) || proc_event->TestEvent(US_HIDING) || proc_event->TestEvent(US_NEXT_HIDING))
  {
    // translate hider from id to pointer
    new_hider = (TMAP_UNIT *)players[proc_event->GetPlayerID()]->hash_table_units.GetUnitPointer(proc_event->int1);
    if (new_hider != hider) {
      ReleaseCountedPointer(hider);
      if (new_hider) hider = (TMAP_UNIT *)new_hider->AcquirePointer();
    }

    if ((!hider) && (!player_array.IsRemote(proc_event->GetPlayerID()))) {
      ClearActions();
      SendEvent(false, proc_event->GetTimeStamp(), US_NEXT_STEP, -1, pos.x, pos.y, pos.segment, move_direction);
      return;
    }
  }

  // US_EJECTING
  if (proc_event->TestEvent(US_EJECTING))
  {
    hider->RemoveFromListOfUnits(this, LIST_HIDING);
    
    Debug ("Unseting held ???");
    SetHeld(false);
    SetHider(NULL);
    
    if (player_array.IsRemote(proc_event->GetPlayerID())) { // remote units must be added to map
      SetPosition(proc_event->simple1, proc_event->simple2, proc_event->simple3);
      
      AddToMap(true, true);
    }
    else { // local units receive US_NEXT_STEP
      SendEvent(false, proc_event->GetTimeStamp() + TS_MIN_EVENTS_DIFF, US_NEXT_STEP, -1, pos.x, pos.y, pos.segment, move_direction);
    }
  }

  //US_START_ATTACK
  if (proc_event->TestEvent(US_START_ATTACK))
  {
    check_target = 0;
    
    // translate target from id to pointer
    new_target = static_cast<TMAP_UNIT*>(players[proc_event->simple6]->hash_table_units.GetUnitPointer(proc_event->int1));
    if (new_target != GetTarget()) 
    {
      ReleaseCountedPointer(target);
      if (new_target) 
        SetTarget(static_cast<TMAP_UNIT*>(new_target->AcquirePointer()));
    }
  }

  // US_END_ATTACK
  if (proc_event->TestEvent(US_END_ATTACK))
  {
    last_target = target; // not counted pointer, but it is OK, because last_targed will be compared only to new target
    ClearActions(); 
    
    if (!player_array.IsRemote(proc_event->GetPlayerID())) {
      SendEvent(false, proc_event->GetTimeStamp() + TS_MIN_EVENTS_DIFF, US_NEXT_STEP, -1, pos.x, pos.y, pos.segment, move_direction);
      SetAutoAttack(false);
    }
  }

  // RQ_SYNC_LIFE
  if (proc_event->TestEvent(RQ_SYNC_LIFE))
  {
    SetLife((float)proc_event->int1);
  }

  // RQ_CREATE_UNIT
  if (proc_event->TestEvent(RQ_CREATE_UNIT))
  {
    TBUILDING_UNIT * new_building = NULL;
    TBUILDING_ITEM * new_itm = static_cast<TBUILDING_ITEM *>(player->race->buildings[proc_event->int1]);

    // add building to the list of buildings of the player, add a sign to all the used fields of map, that building is staying on them
    if (new_itm->GetItemType() == IT_BUILDING)
      new_building = NEW TBUILDING_UNIT(proc_event->GetPlayerID(), proc_event->simple1, proc_event->simple2, new_itm, proc_event->int2, true);
    else
      new_building = NEW TFACTORY_UNIT(proc_event->GetPlayerID(), proc_event->simple1, proc_event->simple2, new_itm, proc_event->int2, true);

    if (!new_building) return;

    new_building->AddToMap(true, false);
    new_building->ClearActions();

    new_building->PutState(US_IS_BEING_BUILT);  //set to building that it is being built
    new_building->ChangeAnimation();
  }

  /****************** Compute new values - textures, times (ALL UNITS) ********************************/
  
  // US_MOVE, US_LANDING, US_UNLANDING
  if (proc_event->TestEvent(US_MOVE) || proc_event->TestEvent(US_LANDING) || proc_event->TestEvent(US_UNLANDING)){
    if (last_ps != pos) {
      // set direction (of texture)
      if ((move_direction >= LAY_SOUTH) && (move_direction <= LAY_SOUTH_EAST)){ 

        if (proc_event->TestEvent(US_UNLANDING)){
          all_moveable = true;
          for (int i = last_ps.x; (i < last_ps.x + GetUnitWidth()); i++)
            for (int j = last_ps.y; j < last_ps.y + GetUnitHeight(); j++)
            {
              if (!itm->moveable[last_ps.segment].IsMember(map.segments[last_ps.segment].surface[i][j].t_id)) // it is NOT possible to land and to move here
              {
                all_moveable = false;
                break;
              }
            }

          if (all_moveable)
            look_direction = move_direction;
          else
            look_direction = (LAY_HOR_DIRECTIONS_COUNT/2 + move_direction) % LAY_HOR_DIRECTIONS_COUNT;
        }
        else look_direction = move_direction;
      }
    
      // find out hardest terrain
      for (i = pos.x; i < pos.x + GetUnitWidth(); i++)
        for (j = pos.y; j < pos.y + GetUnitHeight(); j++)
          hardest = (scheme.terrain_props[pos.segment][map.segments[pos.segment].surface[pos.x][pos.y].t_id].difficulty > hardest)?
                    scheme.terrain_props[pos.segment][map.segments[pos.segment].surface[pos.x][this->pos.y].t_id].difficulty : hardest;

      // set speed
      //actual speed in dependence to difficulty of the terrain - terrain with difficulty 500 reduce speed to half
      speed = itm->max_speed[pos.segment]*(1.001f - hardest / 1000.0f);

      if (proc_event->TestEvent(US_LANDING) || proc_event->TestEvent(US_UNLANDING)) speed = speed / WLK_LANDING_PENALTY; //speed correction
      
      // set move_time
      if (move_direction == LAY_EAST || move_direction == LAY_NORTH || move_direction == LAY_WEST || move_direction == LAY_SOUTH)
        move_time = (1 / speed);
      else if (move_direction == LAY_SOUTH_EAST || move_direction == LAY_NORTH_EAST || move_direction == LAY_NORTH_WEST || move_direction == LAY_SOUTH_WEST)
        move_time = (SQRT_2 / speed);
      else
        move_time = (2 / speed);
    }
    else {
      speed = itm->max_speed[pos.segment]*(1.001f - hardest / 1000.0f);
      if (proc_event->TestEvent(US_LANDING) || proc_event->TestEvent(US_UNLANDING)) speed = speed / WLK_LANDING_PENALTY; //speed correction
      move_time = (1 / speed);
      move_direction = LAY_UNDEFINED;
    }
    
    move_shift = 0;
  }

  // US_RIGHT_ROTATING, US_LEFT_ROTATING
  if (proc_event->TestEvent(US_RIGHT_ROTATING) || proc_event->TestEvent(US_LEFT_ROTATING)){

    if ((move_direction >= LAY_SOUTH) && (move_direction <= LAY_SOUTH_EAST)) 
      look_direction = move_direction;
    
    for (i = pos.x; i < (pos.x + GetUnitWidth()); i++)
      for (j = pos.y; j < (pos.y + GetUnitHeight()); j++)   //find hardest terrain
        hardest = (scheme.terrain_props[pos.segment][map.segments[pos.segment].surface[pos.x][pos.y].t_id].difficulty > hardest)?
                  scheme.terrain_props[pos.segment][map.segments[pos.segment].surface[pos.x][pos.y].t_id].difficulty : hardest;

    // set rotation speed
    rotation_speed = itm->GetMaximumRotation(pos.segment)*(1.001f - hardest / 1000.0f);

    //actual rotation speed in dependence to difficulty of the terrain - terrain with difficulty 500 reduce speed to half
    move_time = (1 / rotation_speed);
    move_shift = 0;
  }

  // US_REPAIRING, US_CONSTRUCTING
  if (proc_event->TestEvent(US_CONSTRUCTING) || proc_event->TestEvent(US_REPAIRING)){
    if ((move_direction >= LAY_SOUTH) && (move_direction <= LAY_SOUTH_EAST)) look_direction = move_direction;
  }
  
  // US_MINING
  if (proc_event->TestEvent(US_MINING))
  {
    first_mine = false;
    
    if (source->AddToListOfUnits(this, LIST_WORKING)) {
      if (source->IsInsideMining()) {
        DeleteFromMap(true);
        // this is relative position to holder
        SetPosition(GetPosition().x - (source->GetPosition().x - GetUnitWidth()), GetPosition().y - (source->GetPosition().y - GetUnitHeight()), GetPosition().segment);
      }

      first_mine = true;
    }
  }

  // US_UNLOADING
  if (proc_event->TestEvent(US_UNLOADING)) {
    if (acceptor->AddToListOfUnits(this, LIST_WORKING)){
      DeleteFromMap(true);
      // this is relative position to holder
      SetPosition(GetPosition().x - (acceptor->GetPosition().x - GetUnitWidth()), GetPosition().y - (acceptor->GetPosition().y - GetUnitHeight()), GetPosition().segment);
    }
  }

  // US_HIDING
  if (proc_event->TestEvent(US_HIDING))
  {
    // it is possible to get message from network which is not correct and hider is not set
    if (hider){
      if (hider->AddToListOfUnits(this, LIST_HIDING)) {
        DeleteFromMap(true);
        // this is relative position to holder
        SetPosition(GetPosition().x - (hider->GetPosition().x - GetUnitWidth()), GetPosition().y - (hider->GetPosition().y - GetUnitHeight()), GetPosition().segment);
      }
    }
  }

  // US_DYING, RQ_DYING
  if (proc_event->TestEvent(US_DYING) || proc_event->TestEvent(RQ_DYING)) {

    #if SOUND
      // play sound
      if (config.snd_unit_speech && visible) {
        if (!((TMAP_ITEM *)pitem)->snd_dead.IsEmpty())
          PlaySound(&((TMAP_ITEM *)pitem)->snd_dead, 1);
        else
          PlaySound(&GetPlayer()->race->snd_dead, 1);
      }
    #endif
    
    // kill all working units
    if (!working_units.IsEmpty()) {
      TLIST<TWORKER_UNIT>::TNODE<TWORKER_UNIT> *node;
      TWORKER_UNIT *unit;

      for (node = working_units.GetFirst(); node; node = node->GetNext()) {
        unit = node->GetPitem();

        // test if unit is local (not remote)
        if (!player_array.IsRemote(unit->GetPlayerID())) {
          unit->SendEvent(false, proc_event->GetTimeStamp(), US_DYING, -1, unit->GetPosition().x, unit->GetPosition().y, unit->GetPosition().segment, unit->GetMoveDirection());
        }
      }
    }

    // kill all held units
    if (!hided_units.IsEmpty()) {
      TLIST<TFORCE_UNIT>::TNODE<TFORCE_UNIT> *node;
      TFORCE_UNIT *unit;

      for (node = hided_units.GetFirst(); node; node = node->GetNext()) {
        unit = node->GetPitem();

        // test if unit is local (not remote)
        if (!player_array.IsRemote(unit->GetPlayerID())) {
          unit->SendEvent(false, proc_event->GetTimeStamp(), US_DYING, -1, unit->GetPosition().x, unit->GetPosition().y, unit->GetPosition().segment, unit->GetMoveDirection());
        }
      }
    }
  }

  // US_ZOMBIE, RQ_ZOMBIE
  if (proc_event->TestEvent(US_ZOMBIE) || proc_event->TestEvent(RQ_ZOMBIE)) {
    if (!held) {
      DeleteFromMap(false);
      lieing_down = true;
    }
  }

  // US_DELETE, RQ_DELETE
  if (proc_event->TestEvent(US_DELETE) || proc_event->TestEvent(RQ_DELETE)) {
    DeleteFromSegments();
    UnitToDelete(true);
    return;
  }


#if SOUND
  // RQ_MINE_SOUND
  if (proc_event->TestEvent(RQ_MINE_SOUND)) {

    if (!player_array.IsRemote(proc_event->GetPlayerID())) { // compute only not remote (local) units
      // if sound request is still valid
      if (proc_event->GetRequestID() == sound_request_id && (TestState(US_MINING) || TestState(US_NEXT_MINE))) {
        if (source && !(source->TestState(US_DYING) || source->TestState(US_ZOMBIE) || source->TestState(US_DELETE))) {
          PlaySound(&itm->snd_mine_material[source->GetOfferMaterial()], 2);
        } 
      }
    }
    else
      if (IsVisible() && (source))
        PlaySound(&itm->snd_mine_material[source->GetOfferMaterial()], 2);
  }
#endif

  
  // RQ_CREATE_PROJECTILE
  if (proc_event->TestEvent(RQ_CREATE_PROJECTILE))
  {
    TGUN *p_gun = itm->GetArmament()->GetOffensive();
    TPROJECTILE_ITEM* pritem = static_cast<TPROJECTILE_ITEM*>(p_gun->GetProjectileItem());

    TPOSITION_3D ipos;  //impact position
    TPOSITION_3D spos;  //start position
    float distance = 1.0;
    double impact_time;

    spos.SetPosition(proc_event->simple1, proc_event->simple2, proc_event->simple3);
    ipos.SetPosition(proc_event->simple4, proc_event->simple5, proc_event->simple6);

    if ((p_gun->GetRange().min != 1) || (p_gun->GetRange().max != 1)) {
      distance = static_cast<float>(sqrt(
      static_cast<float>(sqr(static_cast<int>(spos.x) - ipos.x)
      + sqr(static_cast<int>(spos.y) - ipos.y)
      + sqr(static_cast<int>(GetPosition().segment) - ipos.segment))
      ));
    }
    
    if (distance == 1)     //target is at the neighbourgh field in the map
      impact_time = 0;
    else
      impact_time = distance / pritem->GetSpeed();

    //allocate of the new TPROJECTILE
    shot = NEW TPROJECTILE_UNIT(GetPlayerID(), impact_time, spos, ipos, p_gun, proc_event->int2, true);

    #if DEBUG_EVENTS
      Debug(LogMsg("SET_SHOT_R: P:%d U:%d E:%s RQ:%d X:%d Y:%d R:%d I1:%d TS:%f COUNT:%d", proc_event->GetPlayerID(), proc_event->GetUnitID(), EventToString(proc_event->GetEvent()), proc_event->GetRequestID(), proc_event->simple1, proc_event->simple2, proc_event->simple4, proc_event->int2, proc_event->GetTimeStamp(), queue_events->GetQueueLength()));
    #endif

    if (!shot){
      Warning(LogMsg("Not enough memory for shot in the function FireOn().\n"));
      return;
    }

#if SOUND
    if (visible) {
      PlaySound(&itm->snd_fireoff, 2);
    }
#endif
  }
  
  // RQ_FIRE_OFF
  if (proc_event->TestEvent(RQ_FIRE_OFF))
  {
    //if shot is prepared fire off
    if ((shot != NULL) && (TestState(US_ATTACKING)))
    {
      //add unit to segment lists
      shot->AddToSegments();
      shot->Shot();

      if (player_array.IsRemote(proc_event->GetPlayerID())) {
        #if DEBUG_EVENTS
          Debug(LogMsg("NULLSHOT_R: P:%d U:%d E:%s RQ:%d X:%d Y:%d R:%d I1:%d TS:%f COUNT:%d", proc_event->GetPlayerID(), proc_event->GetUnitID(), EventToString(proc_event->GetEvent()), proc_event->GetRequestID(), proc_event->simple1, proc_event->simple2, proc_event->simple4, shot->GetUnitID(), proc_event->GetTimeStamp(), queue_events->GetQueueLength()));
        #endif

        shot = NULL; // null shot to remote players
      }
    }
  }

  // RQ_FEEDING
  if (proc_event->TestEvent(RQ_FEEDING))
  {
#if SOUND
    if ((visible) && (TestState(US_FEEDING))) {
      PlaySound(&itm->snd_fireon, 2);
    }
#endif
  }

  // set animation
  if (state != US_NEXT_STEP && ((last_state != state) || (last_move_dir != move_direction)))
    ChangeAnimation();


  /****************** Compute next event (ONLY LOCAL UNITS) *******************************/
  
  if (!player_array.IsRemote(proc_event->GetPlayerID())) { // compute only not remote (local) units
  
    // US_START_ATTACK
    if (proc_event->TestEvent(US_START_ATTACK)) {
      if (HasTarget()) 
      {
        SendEvent(false, proc_event->GetTimeStamp(), US_NEXT_ATTACK, -1, pos.x, pos.y, pos.segment, move_direction, 0, target->GetPlayerID(), target->GetUnitID());
      }
      else 
      {
        ClearActions();
        SendEvent(false, proc_event->GetTimeStamp(), US_NEXT_STEP, -1, pos.x, pos.y, pos.segment, move_direction);
      }
    }
    
    
    // US_NEXT_ATTACK
    if (proc_event->TestEvent(US_NEXT_ATTACK))
    {
      //if target will be delete stop attacking
      if (target->TestState(US_DYING) || target->TestState(US_ZOMBIE) || target->TestState(US_DELETE) || (!GetPlayer()->GetLocalMap()->GetAreaVisibility(target->GetPosition(), target->GetUnitWidth(), target->GetUnitHeight()))) 
      {
        // put event to queue
        SendEvent(false, proc_event->GetTimeStamp(), US_END_ATTACK, -1, GetPosition().x, GetPosition().y, GetPosition().segment, GetMoveDirection());
      }
      else
      {
        TATTACK_INFO attack_info = itm->GetArmament()->IsPossibleAttack(this, target);   //tests possibility of attack and determine best impact position
        signed char new_direction;
        switch (attack_info.state)
        {
          case FIG_AIF_ATTACK_OK:       // Run of attack is O.K.
          {
            //send event to start attack
            SendEvent(false, proc_event->GetTimeStamp(), US_ATTACKING, -1, GetPosition().x, GetPosition().y, GetPosition().segment, GetLookDirection(), 
                      attack_info.impact_position.x, attack_info.impact_position.y, attack_info.impact_position.segment);
          }
          break;
          case FIG_AIF_TURN_LEFT:       // before attack is necessary to turn left
          case FIG_AIF_TURN_RIGHT:      // before attack is necessary to turn right
          {
            SetGoal(GetPosition());     //because of green line from unit to its goal
            //compute new direction
            new_direction = GetMoveDirection() + ((attack_info.state == FIG_AIF_TURN_LEFT)?-1:+1);
            //check overflow and underflow of direction
            if (new_direction < 0)
              new_direction += LAY_HOR_DIRECTIONS_COUNT;
            else if (new_direction >= LAY_HOR_DIRECTIONS_COUNT)
              new_direction -= LAY_HOR_DIRECTIONS_COUNT;

            SendEvent(false, proc_event->GetTimeStamp(), ((attack_info.state == FIG_AIF_TURN_LEFT)?US_LEFT_ROTATING:US_RIGHT_ROTATING),
                    -1, GetPosition().x, GetPosition().y, GetPosition().segment, static_cast<T_SIMPLE>(new_direction));
          }
          break;
          case FIG_AIF_TO_UPPER_SEG:    // Attack is possible only in the upper segment.
          case FIG_AIF_TO_LOWER_SEG:    // Attack is possible only in the lower segment.
          {
            if ((itm) && (itm->GetArmament()->GetOffensive()->GetFlags() & FIG_GUN_SAME_SEGMENT) 
                && (!itm->GetExistSegments().IsMember(target->GetPosition().segment)))
            {   
              MessageText(false, const_cast<char*>("Target %s is in the unshotable segment."), target->GetPointerToItem()->name);

              //it is impossible shot to segment where is target -> stop all actions
              SendEvent(false, proc_event->GetTimeStamp(), US_END_ATTACK, -1, pos.x, pos.y, pos.segment, move_direction);
              break;     
            }
            if (path != NULL)
              SendEvent(false, proc_event->GetTimeStamp(), US_NEXT_STEP, -1, GetPosition().x, GetPosition().y, GetPosition().segment, GetMoveDirection(), 1);
            else
            {
              path_event = SendEvent(false, proc_event->GetTimeStamp(),US_WAIT_FOR_PATH,0); 
              waiting_request_id = path_event->GetRequestID();             
              ComputePath(target->GetPosition(),waiting_request_id,ET_ATTACK_TO_OTHER_SEG,new_state);
            }
          }
          break;
          case FIG_AIF_TOO_FAR_AWAY:    // Attacker is too far away from the target.
          { 
            if ((GetAggressivity() == AM_AGGRESSIVE) || (GetAggressivity() == AM_OFFENSIVE) || (!GetAutoAttack())) {
              if (path != NULL)
                SendEvent(false, proc_event->GetTimeStamp(), US_NEXT_STEP, -1, GetPosition().x, GetPosition().y, GetPosition().segment, GetLookDirection(), 1);
              else
              {
                path_event = SendEvent(false, proc_event->GetTimeStamp(),US_WAIT_FOR_PATH,0); 
                waiting_request_id = path_event->GetRequestID();             
                ComputePath(target->GetPosition(),waiting_request_id,ET_ATTACK_TOO_FAR_AWAY,new_state);
              }
            }
            else {
              MessageText(false, const_cast<char*>("Target %s is too far."), target->GetPointerToItem()->name);
              SendEvent(false, proc_event->GetTimeStamp(), US_END_ATTACK, -1, pos.x, pos.y, pos.segment, move_direction);
            }
          }
          break;
          case FIG_AIF_TOO_CLOSE_TO:    // Attacker is too close to the target.
          {
            MessageText(false, const_cast<char*>("Target %s is too close."), target->GetPointerToItem()->name);
            SendEvent(false, proc_event->GetTimeStamp(), US_END_ATTACK, -1, pos.x, pos.y, pos.segment, move_direction);
          }
          break;
          case FIG_AIF_UNSHOTABLE_SEG:  // Defender is in the unshotable segment.
          {
            MessageText(false, const_cast<char*>("Target %s is in the unshotable segment."), target->GetPointerToItem()->name);
            SendEvent(false, proc_event->GetTimeStamp(), US_END_ATTACK, -1, pos.x, pos.y, pos.segment, move_direction);
          }
          break;
          case FIG_AIF_ATTACT_NOT_EFFECIVE:  // Attacker's attack does not have effect.
          {
            MessageText(false, const_cast<char*>("Attack to %s does not have effect."), target->GetPointerToItem()->name);
            SendEvent(false, proc_event->GetTimeStamp(), US_END_ATTACK, -1, pos.x, pos.y, pos.segment, move_direction);
          }
          break;
          case FIG_AIF_ATTACK_FAILED:   // Flag informates that attack failed.
          default:
          {
            MessageText(false,const_cast<char*>( "Attack to %s failed."), target->GetPointerToItem()->name);
            SendEvent(false, proc_event->GetTimeStamp(), US_END_ATTACK, -1, pos.x, pos.y, pos.segment, move_direction);
          }
          break;
        }
      }
    }

    // US_ATTACKING
    if (proc_event->TestEvent(US_ATTACKING))
    {
      if (FireOn(proc_event->simple5, proc_event->simple6, proc_event->int1, proc_event->GetTimeStamp()))    //fire on
      {
        new_time_stamp = proc_event->GetTimeStamp();

        TEVENT * hlp;
        hlp = pool_events->GetFromPool();
        
        new_time_stamp += (itm->GetArmament()->GetOffensive()->GetShotTime() + TS_MIN_EVENTS_DIFF);
        SendRequest(false, new_time_stamp, RQ_FIRE_OFF, -1, GetPosition().x, GetPosition().y, GetPosition().segment, GetLookDirection());
        // send info to not local players that new projectile is creating now
        hlp->SetEventProps(GetPlayerID(), GetUnitID(), false, new_time_stamp, RQ_FIRE_OFF, US_NONE, -1, GetPosition().x, GetPosition().y, GetPosition().segment, GetLookDirection(), 0, 0, 0);
        SendNetEvent(hlp, all_players);
              
        new_time_stamp += (itm->GetArmament()->GetOffensive()->GetWaitTime() + TS_MIN_EVENTS_DIFF);
        SendRequest(false, new_time_stamp, RQ_FEEDING, -1, GetPosition().x, GetPosition().y,GetPosition().segment, GetLookDirection(), 0, target->GetPlayerID(), target->GetUnitID());
        // send info to not local players that unit is feeding
        hlp->SetEventProps(GetPlayerID(), GetUnitID(), false, new_time_stamp, RQ_FEEDING, US_NONE, -1, GetPosition().x, GetPosition().y, GetPosition().segment, GetLookDirection(), 0, target->GetPlayerID(), target->GetUnitID());
        SendNetEvent(hlp, all_players);

        pool_events->PutToPool(hlp);

        // plan new US_NEXT_ATTACK (new attack cycle)
        new_time_stamp += (itm->GetArmament()->GetOffensive()->GetFeedTime() + TS_MIN_EVENTS_DIFF);
        SendEvent(false, new_time_stamp, US_NEXT_ATTACK, -1, GetPosition().x, GetPosition().y, GetPosition().segment, GetLookDirection(), 0, target->GetPlayerID(), target->GetUnitID());
      }
      else
      {
        MessageText(false, const_cast<char*>("Attack to %s failed."), target->GetPointerToItem()->name);
        SendEvent(false, proc_event->GetTimeStamp(), US_END_ATTACK, -1, pos.x, pos.y, pos.segment, move_direction);      
      }
    }

    // RQ_FIRE_OFF
    if (proc_event->TestEvent(RQ_FIRE_OFF))
    {
      //if shot is prepared fire off
      if ((shot != NULL) && (TestState(US_ATTACKING)))
      {
        // send info to not local players
        TEVENT * hlp;
        hlp = pool_events->GetFromPool();

        // send info about changing segment
        new_time_stamp = proc_event->GetTimeStamp() + TS_MIN_EVENTS_DIFF;
        for (int i = 0; i < (shot->GetPosition().segment - shot->GetImpactPos().segment); i++){
          shot->SendRequest(false, new_time_stamp + ((i+1)*shot->GetSegmentTime()), RQ_CHANGE_SEGMENT, -1);
          
          hlp->SetEventProps(shot->GetPlayerID(), shot->GetUnitID(), false, new_time_stamp + ((i+1)*shot->GetSegmentTime()), RQ_CHANGE_SEGMENT, US_NONE, -1, 0, 0, 0, 0, 0, 0, 0);
          SendNetEvent(hlp, all_players);
        }
        
        
        //send event about impact
        shot->SendRequest(false, new_time_stamp + shot->GetImpactTime(), RQ_IMPACT, -1, shot->GetImpactPos().x, shot->GetImpactPos().y, shot->GetImpactPos().segment);

        hlp->SetEventProps(shot->GetPlayerID(), shot->GetUnitID(), false, new_time_stamp + shot->GetImpactTime(), RQ_IMPACT, US_NONE, -1, shot->GetImpactPos().x, shot->GetImpactPos().y, shot->GetImpactPos().segment, 0, 0, 0, 0);
        SendNetEvent(hlp, all_players);
        
        
        pool_events->PutToPool(hlp);

        #if DEBUG_EVENTS
          Debug(LogMsg("NULLSHOT_L: P:%d U:%d E:%s RQ:%d X:%d Y:%d R:%d I1:%d TS:%f COUNT:%d", proc_event->GetPlayerID(), proc_event->GetUnitID(), EventToString(proc_event->GetEvent()), proc_event->GetRequestID(), proc_event->simple1, proc_event->simple2, proc_event->simple4, shot->GetUnitID(), proc_event->GetTimeStamp(), queue_events->GetQueueLength()));
        #endif

        shot = NULL;
      }
    }

    // US_MOVE, US_LANDING, US_UNLANDING
    if (proc_event->TestEvent(US_MOVE) || proc_event->TestEvent(US_LANDING) || proc_event->TestEvent(US_UNLANDING))
    {
      new_ps = pos;
      if (proc_event->TestEvent(US_UNLANDING)) { 
        all_moveable = true;
        for (int i = last_ps.x; (i < last_ps.x + GetUnitWidth()); i++)
          for (int j = last_ps.y; j < last_ps.y + GetUnitHeight(); j++)
          {
            if (!itm->moveable[last_ps.segment].IsMember(map.segments[last_ps.segment].surface[i][j].t_id)) // it is NOT possible to land and to move here
            {
              all_moveable = false;
              break;
            }
          }
        if (all_moveable)
          new_direction = move_direction;
        else    
          new_direction = look_direction;
      }
      else
        new_direction = move_direction;

      new_time_stamp = proc_event->GetTimeStamp() + this->move_time;

      // send event to queue
      if (proc_event->TestEvent(US_LANDING)){
        new_state = US_ANCHORING;
        ResetTryToLandTimer();
      }
      else new_state = US_NEXT_STEP;
      
      // if position didn't changed, move_direction is LAY_UNDEFINED (because units jumped)
      if (new_direction == LAY_UNDEFINED) 
        new_direction = look_direction;
      
      // put event to queue
      SendEvent(false, new_time_stamp, US_NEXT_STEP, -1, new_ps.x, new_ps.y, new_ps.segment, new_direction);
    }
    
    
    // US_NEXT_STEP, US_TRY_TO_MOVE
    if (proc_event->TestEvent(US_NEXT_STEP) || proc_event->TestEvent(US_TRY_TO_MOVE))
    {
      // test if target is near enough
      if ((target) && (proc_event->TestEvent(US_NEXT_STEP)))
      {
        if (proc_event->simple5 == 0) 
        {
          ResetTryToMoveTimer();

          // send event to queue
          SendEvent(false, proc_event->GetTimeStamp(), US_NEXT_ATTACK, -1, pos.x, pos.y, pos.segment, move_direction, 0, target->GetPlayerID(), target->GetUnitID());
          return;
        }
        else 
        { // test if target changed position
          check_target = (check_target + 1 ) % UNI_CHECK_TARGET;

          if (!check_target) 
          {
            // if tagret is visible -> check if change position
            if (GetPlayer()->GetLocalMap()->GetAreaVisibility(target->GetPosition().x, target->GetPosition().y, target->GetPosition().segment, target->GetPosition().segment, target->GetUnitWidth(), target->GetUnitHeight())) {
              if (target->GetPosition() != path->GetRealGoalPosition())
              {
                if (path) 
                {
                  delete path;
                  path = NULL;
                }              
              
                ResetTryToMoveTimer();
                path_event = SendEvent(false, proc_event->GetTimeStamp(), US_WAIT_FOR_PATH, 0); 
                waiting_request_id = path_event->GetRequestID();

                ComputePath(target->GetPosition(),waiting_request_id,ET_TARGET_MOVING);              
                return;
              }
            }
            // target is not visible -> stop unit
            else {
              ClearActions(); //stop moving of the unit
    
              // send event to queue
              SendEvent(false, proc_event->GetTimeStamp(), US_NEXT_STEP, -1, pos.x, pos.y, pos.segment, move_direction);
              return;
            }
          }
        }
      }

      // check if hider changed position
      if ((hider) && (proc_event->TestEvent(US_NEXT_STEP)))
      {
        check_hider = (check_hider + 1) % UNI_CHECK_HIDER;
        
        if ((!check_hider) && (path) && (hider->GetPosition() != path->GetRealGoalPosition())) 
        {          
          if (path) 
          {
            delete path;
            path = NULL;
          }
          
          ResetTryToMoveTimer();
          path_event = SendEvent(false, proc_event->GetTimeStamp(), US_WAIT_FOR_PATH, 0); 
          waiting_request_id = path_event->GetRequestID();

          ComputePath(hider->GetPosition(), waiting_request_id,ET_HIDER_MOVING); 
          return;
        }
      }

      //check if built_or_repaired_unit changed position
      if ((built_or_repaired_unit) && (proc_event->TestEvent(US_NEXT_STEP)))
      {
        check_built_or_repaired_unit = (check_built_or_repaired_unit + 1) % UNI_CHECK_BUILT_OR_REPAIRED_UNIT;
        
        if ((!check_built_or_repaired_unit) && (path) && (built_or_repaired_unit->GetPosition() != path->GetRealGoalPosition())) {
          
          if (path) 
          {
            delete path;
            path = NULL;
          }
          
          ResetTryToMoveTimer();
          path_event = SendEvent(false, proc_event->GetTimeStamp(), US_WAIT_FOR_PATH, 0); 
          waiting_request_id = path_event->GetRequestID();

          ComputePath(built_or_repaired_unit->GetPosition(),waiting_request_id,ET_CONSTRUCTED_OBJECT_MOVING); 
          return;
        }
      }
      
      // test if unit is in state TRY_TO_MOVE too long      
      if (proc_event->TestEvent(US_TRY_TO_MOVE))
      {
        try_move_shift += UNI_TRY_TO_MOVE_SHIFT;  // add trying time
     
        if (try_move_shift >= UNI_TRY_TO_MOVE_LIMIT)  //unit tries to long => punish it 
        {
          if (hider) MessageText(false, const_cast<char*>("Can not get to %s."), hider->GetPointerToItem()->name);
          if (acceptor) MessageText(false, const_cast<char*>("Can not get to the %s."), acceptor->GetPointerToItem()->name);
          if (built_or_repaired_unit) MessageText(false, const_cast<char*>("Can not get to the %s."), built_or_repaired_unit->GetPointerToItem()->name);
         
          if ((source) && (!acceptor))
          { // if unit have some source, can not remove it
            if (path) 
            {
              delete path;
              path = NULL;
            }
          }
          else
            ClearActions(); //stop moving of the unit
    
          ResetTryToMoveTimer();
          // send event to queue
          SendEvent(false, proc_event->GetTimeStamp(), US_NEXT_STEP, -1, pos.x, pos.y, pos.segment, move_direction);
          return;
        }
      }
      
      if (path) 
      { //exist path
        if (!path->TestLastPathPosition())
        { // test if unit is in the end
          // unit is in end -> set state US_STAY, US_ANCHORING, US_LANDING
          if  ((last_state == US_LANDING) || (last_state == US_ANCHORING)) 
            new_state = US_ANCHORING;
          else 
            new_state = US_NEXT_STEP;

          delete path;
          path = NULL;

          ResetTryToMoveTimer();
          // send event to queue
          SendEvent(false, proc_event->GetTimeStamp(), new_state, -1, pos.x, pos.y, pos.segment, move_direction);
          return;
        }
        else 
        { // exists next step in path
          new_ps = path->GetNextPosition();    //set new position to local variable
          if (last_state == US_ANCHORING) 
            new_state = US_UNLANDING;
          else 
            new_state = US_MOVE;
          change_position = true;
      
          if (proc_event->TestEvent(US_TRY_TO_MOVE))
          { //if can not move, change roation first
            
            all_moveable = false;
            if (last_state == US_UNLANDING) {
              all_moveable = true;
              for (int i = last_ps.x; (i < last_ps.x + GetUnitWidth()); i++)
                for (int j = last_ps.y; j < last_ps.y + GetUnitHeight(); j++)
                {
                  if (!itm->moveable[last_ps.segment].IsMember(map.segments[last_ps.segment].surface[i][j].t_id)) // it is NOT possible to land and to move here
                  {
                    all_moveable = false;
                    break;
                  }
                }
            }
            
            if ((new_state != US_UNLANDING) || (!all_moveable))  { // direction of unit is changed after unlanding
              new_direction = pos.GetDirection(new_ps);
              if ((new_direction != move_direction) && (new_direction != LAY_UNDEFINED) && (new_direction != LAY_UP) && (new_direction != LAY_DOWN)){
                // unit is bad rotated -> next state will be US_XXX_ROTATING;

                new_state = DetermineRotationDirection(new_direction);
                // set new direction
                if (new_state == US_RIGHT_ROTATING)
                  new_direction = (move_direction + 1) % LAY_HOR_DIRECTIONS_COUNT;
                else
                  new_direction = (move_direction + LAY_HOR_DIRECTIONS_COUNT - 1) % LAY_HOR_DIRECTIONS_COUNT;
                
                ResetTryToMoveTimer();
                // send event to queue
                SendEvent(false, proc_event->GetTimeStamp(), new_state, -1, pos.x, pos.y, pos.segment, new_direction);
                return;
              }
            }
          }
          
          // test if next position is moveable
          if (!GetPlayer()->GetLocalMap()->IsMoveablePosition(itm, new_ps))
          { 
            // test if new position is not visible
            if (GetPlayer()->GetLocalMap()->IsAnyAreaUnknown(new_ps.x, new_ps.y, new_ps.segment, GetUnitWidth(), GetUnitHeight())){
              if (path)
              { // delete old path
                delete path;
                path = NULL;
              }

              // send event to queue
              SendEvent(false, proc_event->GetTimeStamp(), US_NEXT_STEP, -1, pos.x, pos.y, pos.segment, move_direction);
              return;
            }
            else{
              // next position is unmoveable for item
              if (new_ps == path->GetRealGoalPosition()) { // next position is last
                test_ps = new_ps;
                if (LandUnit(&new_ps))
                { // unit can land
                  if (test_ps != new_ps)
                  { // unit can land, but not at new_ps position

                    if (path)
                    { // delete old path
                      delete path;
                      path = NULL;
                    }

                    path_event = SendEvent(false, proc_event->GetTimeStamp(), US_WAIT_FOR_PATH, 0);
                    waiting_request_id = path_event->GetRequestID();           
                    ComputePath(new_ps,waiting_request_id,ET_NOTNEXTPOS_LANDING,last_state,state);
                    return;

                  }
                  else { // unit can land on next position
                    new_state = US_LANDING;
                    change_position = true;
                  }
                }
                else 
                { // unit can not land
                  //stop moving of the unit
                  if (path)
                  {
                    delete path;
                    path = NULL;
                  }
      
                  // send event to queue
                  SendEvent(false, proc_event->GetTimeStamp() + UNI_TRY_TO_MOVE_SHIFT, US_TRY_TO_MOVE, -1, pos.x, pos.y, pos.segment, move_direction);
                  return;
                }
              }
              else 
              { // next position is not last -> find new path
                test_ps = path->GetRealGoalPosition();

                if (path)
                { // delete old path
                  delete path;
                  path = NULL;
                }

                path_event = SendEvent(false, proc_event->GetTimeStamp(), US_WAIT_FOR_PATH, 0);
                waiting_request_id = path_event->GetRequestID();                         
                ComputePath(test_ps,waiting_request_id,ET_NEXTPOS_NOTMOVABLE,last_state,state);              
                return;
              }
            }
          }

          if (change_position)
          {  
            new_direction = pos.GetDirection(new_ps);
            // test if next position is not empty
            if (!GetPlayer()->GetLocalMap()->IsNextPositionEmpty(new_ps, new_direction, this))
            {
              // next position isn't empty, try to find new path
              test_ps = path->GetRealGoalPosition(); // get real goal position
        
              if (path) // delete path
              {
                delete path;
                path = NULL;
              } 
              
              path_event = SendEvent(false, proc_event->GetTimeStamp(), US_WAIT_FOR_PATH, 0);
              waiting_request_id = path_event->GetRequestID();                         

              ComputePath(test_ps,waiting_request_id,ET_UNIT_AGAINST_UNIT,last_state,state);
              return;
            }
            else
            {
              if (proc_event->TestEvent(US_NEXT_STEP))
              { //change rotation first
                
                // if state is US_UNLANDING, check which type of unlanding
                all_moveable = false;
                if (new_state == US_UNLANDING) {
                  all_moveable = true;
                  for (int i = last_ps.x; (i < last_ps.x + GetUnitWidth()); i++)
                    for (int j = last_ps.y; j < last_ps.y + GetUnitHeight(); j++)
                    {
                      if (!itm->moveable[last_ps.segment].IsMember(map.segments[last_ps.segment].surface[i][j].t_id)) // it is NOT possible to land and to move here
                      {
                        all_moveable = false;
                        break;
                      }
                    }
                }
            
                if ((new_state != US_UNLANDING) || (all_moveable)) // direction of unit is changed after unlanding
                { // direction of unit is changed after unlanding
                  if ((new_direction != move_direction) && (new_direction != LAY_UNDEFINED) && (new_direction != LAY_UP) && (new_direction != LAY_DOWN) && (move_direction != LAY_UNDEFINED) && (move_direction != LAY_UP) && (move_direction != LAY_DOWN))
                  {
                    // unit is bad rotated -> next state will be US_XXX_ROTATING;
                    new_state = DetermineRotationDirection(new_direction);
                    // set new direction
                    if (new_state == US_RIGHT_ROTATING)
                      new_direction = (move_direction + 1) % LAY_HOR_DIRECTIONS_COUNT;
                    else
                      new_direction = (move_direction + LAY_HOR_DIRECTIONS_COUNT - 1) % LAY_HOR_DIRECTIONS_COUNT;

                    ResetTryToMoveTimer();
                    // send event to queue
                    SendEvent(false, proc_event->GetTimeStamp(), new_state, -1, pos.x, pos.y, pos.segment, new_direction);
                    return;
                  }
                }
              }
            }
          }
        }        
      }
      else 
      { // path is NULL -> stop or land unit
        if (itm->TestSomeFeatures(RAC_HAVE_TO_LAND))
        { //unit has to land when stay
          if ((last_state != US_LANDING) && (last_state != US_ANCHORING))
          { // unit is not landing and not anchoring
          
            test_ps = new_ps = pos;
            new_direction = move_direction;

            if (LandUnit(&new_ps))
            { // unit can land
              if (test_ps != new_ps)
              { // unit can land, but not at new_ps position
              
                if (path)
                { // delete old path
                  delete path;
                  path = NULL;
                }

                path_event = SendEvent(false, proc_event->GetTimeStamp(), US_WAIT_FOR_PATH, 0);
                waiting_request_id = path_event->GetRequestID();       

                ComputePath(new_ps,waiting_request_id,ET_NOTPATH_LAND,last_state,state);  //brane ako 7cka
                return;
              }
              else { // unit can land on next position
                new_state = US_LANDING;
              }
            }
            else { // unit can not land
              try_land_shift += UNI_TRY_TO_MOVE_SHIFT;

              if (try_land_shift > UNI_TRY_TO_MOVE_LIMIT){
                Injure(1);
                ResetTryToLandTimer();
              }           
            
              // send event to queue
              SendEvent(false, proc_event->GetTimeStamp() + UNI_TRY_TO_MOVE_SHIFT, US_TRY_TO_MOVE, -1, pos.x, pos.y, pos.segment, move_direction);
              return;
            }
          }
          else
          { // unit is landing or anchoring
            new_state = US_ANCHORING;
            new_direction = move_direction;
            new_ps = pos;
          }

          change_position = false;
        }
        else 
        {
          new_state = US_STAY;
          change_position = false;
          new_direction = move_direction;
          new_ps = pos;
        }
      }
     
      
      if (change_position) path->IncreaseASteps(); // !!! skontrolovat, ci sa to vzdy korekne nastavi
      new_priority = change_position;
      
      if (new_state == US_TRY_TO_MOVE)
        new_time_stamp = proc_event->GetTimeStamp() + UNI_TRY_TO_MOVE_SHIFT;
      else {
        new_time_stamp = proc_event->GetTimeStamp();
        ResetTryToMoveTimer();
      }
      
      // send event to queue
      SendEvent(new_priority, new_time_stamp, new_state, -1, new_ps.x, new_ps.y, new_ps.segment, new_direction);
    }
          
    
    // US_RIGHT_ROTATING, US_LEFT_ROTATING
    if (proc_event->TestEvent(US_RIGHT_ROTATING) || proc_event->TestEvent(US_LEFT_ROTATING))
    {
      new_time_stamp = proc_event->GetTimeStamp() + this->move_time;
      new_state = US_NEXT_STEP;
      new_direction = move_direction;
      
      SendEvent(false, new_time_stamp, new_state, -1, proc_event->simple1, proc_event->simple2, proc_event->simple3, new_direction);
    }


    // US_STAY, US_ANCHORING
    if (proc_event->TestEvent(US_STAY) || proc_event->TestEvent(US_ANCHORING)) 
    {
      TMAP_UNIT * standing_target;

      standing_target = CheckNeighbourhood(last_target);

      if (path)
      {
        delete path;
        path = NULL;
      }

      if ((standing_target) && (GetPlayer()->GetLocalMap()->GetAreaVisibility(standing_target->GetPosition(), standing_target->GetUnitWidth(), standing_target->GetUnitHeight()))) {
        SetAutoAttack(true);
        new_time_stamp = proc_event->GetTimeStamp() + TS_MIN_EVENTS_DIFF;
        new_state = US_START_ATTACK;
        new_ps = pos;
        new_direction = move_direction;
        new_simple6 = standing_target->GetPlayerID();
        new_int = standing_target->GetUnitID();
      }

      // worker is staying and has some hider
      else if (hider) {
        new_state = US_NEXT_HIDING;
        new_int = hider->GetUnitID();
        new_time_stamp = proc_event->GetTimeStamp() + TS_MIN_EVENTS_DIFF;
        new_ps = pos;
        new_direction = move_direction;
      }

      // worker is staying and has some unit to built or repair
      else if (built_or_repaired_unit) {
        if (built_or_repaired_unit->TestState(US_IS_BEING_BUILT)){
          new_state = US_NEXT_CONSTRUCTING;
        }
        else {
          new_state = US_NEXT_REPAIRING;
        }
        
        new_int = built_or_repaired_unit->GetUnitID();
        new_time_stamp = proc_event->GetTimeStamp() + TS_MIN_EVENTS_DIFF;
        new_ps = pos;
        new_direction = move_direction;
      }

      else if (acceptor) {
        new_state = US_NEXT_UNLOADING;
        new_int = acceptor->GetUnitID();
        new_time_stamp = proc_event->GetTimeStamp() + TS_MIN_EVENTS_DIFF;
        new_ps = pos;
        new_direction = move_direction;
      }   

      //worker is staying and has some source to mine
      else if (source) {
        new_state = US_NEXT_MINE;
        new_int = source->GetUnitID();
        new_simple5 = source->GetPlayerID();
        new_time_stamp = proc_event->GetTimeStamp() + TS_MIN_EVENTS_DIFF;
        new_ps = pos;
        new_direction = move_direction;
      }


      // test if unit can heal itself and if it has full life
      else if (((proc_event->TestEvent(US_STAY) && itm->TestSomeFeatures(RAC_HEAL_WHEN_STAY)) 
        || (proc_event->TestEvent(US_ANCHORING) && itm->TestSomeFeatures(RAC_HEAL_WHEN_ANCHOR)))
        && life < itm->GetMaxLife())
      {
        // set values
        new_time_stamp = proc_event->GetTimeStamp() + itm->GetHealTime() - heal_shift;
        new_state = US_HEALING;
        new_ps.SetPosition(proc_event->simple1, proc_event->simple2, proc_event->simple3);
        new_direction = proc_event->simple4;
      }

      // nothing to do
      else return;

      // send event to queue
      SendEvent(false, new_time_stamp, new_state, -1, new_ps.x, new_ps.y, new_ps.segment, new_direction, new_simple5, new_simple6, new_int);
    }
  
    // US_DYING
    if (proc_event->TestEvent(US_DYING)) 
    {
      state_time = 0;
    
      if (itm->tg_dying_id != -1) {
        TGUI_TEXTURE * pgui_texture;

        pgui_texture = player->race->tex_table.GetTexture(itm->tg_dying_id, 0);
        state_time = pgui_texture->frame_time * pgui_texture->frames_count;
      }
      
      player->RemoveUnitEnergyFood(itm->energy, itm->food); // change players unit energy and food
      player->DecPlayerUnitsCount(); // decrease count of active players units
      itm->DecreaseActiveUnitCount();  // decrease count of units of this kind
      ClearActions();

      new_time_stamp = proc_event->GetTimeStamp() + state_time + TS_MIN_EVENTS_DIFF;
      new_state = US_ZOMBIE;

      // send event to queue
      SendEvent(false, new_time_stamp, new_state, -1, proc_event->simple1, proc_event->simple2, proc_event->simple3, proc_event->simple4);
    }

    // US_ZOMBIE
    if (proc_event->TestEvent(US_ZOMBIE)) 
    {
      if (itm->tg_zombie_id != -1) state_time = UNI_ZOMBIE_TIME;
      else state_time = 0;

      new_time_stamp = proc_event->GetTimeStamp() + state_time + TS_MIN_EVENTS_DIFF;
      new_state = US_DELETE;
      
      // send event to queue
      SendEvent(false, new_time_stamp, new_state, -1, proc_event->simple1, proc_event->simple2, proc_event->simple3, proc_event->simple4);
    }

    // US_HEALING
    if (proc_event->TestEvent(US_HEALING))
    {
      Heal(1);
      heal_shift = 0;

      new_time_stamp = proc_event->GetTimeStamp();

      // send event to queue
      SendEvent(false, new_time_stamp, last_state, -1, proc_event->simple1, proc_event->simple2, proc_event->simple3, proc_event->simple4);
    }

    // US_START_MINE, US_START_UNLOAD, US_START_REPAIR, US_START_HIDING
    if (
      proc_event->TestEvent(US_START_MINE) || proc_event->TestEvent(US_START_UNLOAD) ||
      proc_event->TestEvent(US_START_REPAIR) || proc_event->TestEvent(US_START_HIDING)
    ) 
    {      
      TMAP_UNIT *destination = NULL;
      int state = US_NONE;
      TEVENT * path_event;

      if (proc_event->TestEvent(US_START_MINE)) {destination = source; state = US_START_MINE;}
      else if (proc_event->TestEvent(US_START_UNLOAD)) {destination = acceptor; state = US_START_UNLOAD;}
      else if (proc_event->TestEvent(US_START_REPAIR))  {destination = built_or_repaired_unit; state=US_START_REPAIR;}
      else  { destination = hider; state = US_START_HIDING;};
      
      path_event = SendEvent(false, proc_event->GetTimeStamp(),US_WAIT_FOR_PATH,0); //send also old state
      waiting_request_id = path_event->GetRequestID();     //save request id for later use

      //create thread for path finding
      ComputePath(destination->GetPosition(),waiting_request_id,ET_WORKER_ACTIVITY);      
    }

    // US_NEXT_HIDING
    if (proc_event->TestEvent(US_NEXT_HIDING))
    {
      new_time_stamp = proc_event->GetTimeStamp();

      if (hider->TestState(US_DYING) || hider->TestState(US_ZOMBIE) || hider->TestState(US_DELETE)) 
      {
        MessageText(false, const_cast<char*>("Can not hide into destroyed %s."), hider->GetPointerToItem()->name);
        ClearActions();
        new_state = US_NEXT_STEP; // correctly land or stay (finish action)
        // send event to queue
        SendEvent(false, new_time_stamp, new_state, -1, pos.x, pos.y, pos.segment, new_direction);
      }
      // test if hider is not US_IS_BEING_BUILD
      else if (hider->TestState(US_IS_BEING_BUILT)){
        MessageText(false, const_cast<char*>("Can not hide into the %s."), hider->GetPointerToItem()->name);
        ClearActions();
        new_state = US_NEXT_STEP; // correctly land or stay (finish action)
        // send event to queue
        SendEvent(false, new_time_stamp, new_state, -1, pos.x, pos.y, pos.segment, new_direction);
      }
      else if (!hider->IsInMap()){
        MessageText(false, const_cast<char*>("%s is not accessible."), hider->GetPointerToItem()->name);
        ClearActions();
        new_state = US_NEXT_STEP; // correctly land or stay (finish action)
        // send event to queue
        SendEvent(false, new_time_stamp, new_state, -1, pos.x, pos.y, pos.segment, new_direction);
      }
      else if (IsNeighbour(hider)) 
      { //if unit is in the neighbourhood -> start hiding
        // test if hider is not full
        if (!hider->CanAcceptUnit(this)) {
          MessageText(false, const_cast<char*>("Can not hide into the %s."), hider->GetPointerToItem()->name);
          ClearActions();
        }

        // test if unit is good rotated
        else {
          new_ps.x = hider->GetPosition().x + (int)(hider->GetPointerToItem()->GetWidth() / 2);
          new_ps.y = hider->GetPosition().y + (int)(hider->GetPointerToItem()->GetHeight() / 2);
          new_ps.segment = pos.segment;

          new_direction = GetCenterPosition().TPOSITION::GetDirection(new_ps);
    
          if ((new_direction != move_direction) && (new_direction != LAY_UNDEFINED)){
            // unit is bad rotated -> next state will be US_XXX_ROTATING;

            new_state = DetermineRotationDirection(new_direction);
            // set new direction
            if (new_state == US_RIGHT_ROTATING)
              new_direction = (move_direction + 1) % LAY_HOR_DIRECTIONS_COUNT;
            else
              new_direction = (move_direction + LAY_HOR_DIRECTIONS_COUNT - 1) % LAY_HOR_DIRECTIONS_COUNT;
          }
          else {
            // unit is good ratated -> hide
            new_state = US_HIDING;
            new_int = hider->GetUnitID();
          }
        }
        // send event to queue
        SendEvent(false, new_time_stamp, new_state, -1, pos.x, pos.y, pos.segment, new_direction, 0, 0, new_int);
      }// unit is not neighbour of hider
      else 
      { 
        path_event = SendEvent(false, proc_event->GetTimeStamp(), US_WAIT_FOR_PATH, 0); 
        waiting_request_id = path_event->GetRequestID();  

        ComputePath(hider->GetPosition(),waiting_request_id,ET_NEXT_HIDING);                 
      }
    }

    // US_NEXT_REPAIRING, US_NEXT_CONSTRUCTING
    if (proc_event->TestEvent(US_NEXT_CONSTRUCTING) || proc_event->TestEvent(US_NEXT_REPAIRING))
    {
      new_time_stamp = proc_event->GetTimeStamp();
      new_direction = move_direction;
      
      // if building is destroyed
      if (built_or_repaired_unit->TestState(US_DYING) || built_or_repaired_unit->TestState(US_ZOMBIE) || built_or_repaired_unit->TestState(US_DELETE)) 
      {
        if (proc_event->TestEvent(US_NEXT_CONSTRUCTING))
          MessageText(false, const_cast<char*>("Can not build destroyed %s."), built_or_repaired_unit->GetPointerToItem()->name);
        else
          MessageText(false, const_cast<char*>("Can not repair destroyed %s."), built_or_repaired_unit->GetPointerToItem()->name);

        ClearActions();
        new_state = US_NEXT_STEP; // correctly land or stay (finish action)
      }
      // if built_or_repaired_unit is force or worker, it shoult hide into another unit
      else if (!built_or_repaired_unit->IsInMap())
      {
        MessageText(false, const_cast<char*>("%s is not accessible."), built_or_repaired_unit->GetPointerToItem()->name);
        ClearActions();
        new_state = US_NEXT_STEP; // correctly land or stay (finish action)
        // send event to queue
        SendEvent(false, new_time_stamp, new_state, -1, pos.x, pos.y, pos.segment, new_direction);
      }      
      else if (IsNeighbour(built_or_repaired_unit))  // if building is neighbour
      {
        new_ps.x = built_or_repaired_unit->GetPosition().x + (int)(built_or_repaired_unit->GetPointerToItem()->GetWidth() / 2);
        new_ps.y = built_or_repaired_unit->GetPosition().y + (int)(built_or_repaired_unit->GetPointerToItem()->GetHeight() / 2);
        new_ps.segment = pos.segment;

        new_direction = GetCenterPosition().TPOSITION::GetDirection(new_ps);

        // unit is bad rotated -> next state will be US_XXX_ROTATING
        if ((new_direction != move_direction) && (new_direction != LAY_UNDEFINED)) 
        {
          new_state = DetermineRotationDirection(new_direction);

          // set new direction
          if (new_state == US_RIGHT_ROTATING)
            new_direction = (move_direction + 1) % LAY_HOR_DIRECTIONS_COUNT;
          else
            new_direction = (move_direction + LAY_HOR_DIRECTIONS_COUNT - 1) % LAY_HOR_DIRECTIONS_COUNT;
        }
        
        else if (proc_event->TestEvent(US_NEXT_REPAIRING))   // building is repaired
        {         
          if (built_or_repaired_unit->HasMaxLife()) 
          {
            MessageText(true, const_cast<char*>("%s is repaired."), built_or_repaired_unit->GetPointerToItem()->name);

            ClearActions();
            new_state = US_NEXT_STEP;
            
            #if SOUND
              // play sound
              if (config.snd_unit_speech && TestPlayer(myself))
                PlaySound(&itm->snd_workcomplete, 1);
            #endif
          }

          // building is not repaired
          else {
            TBASIC_ITEM *bitem = static_cast<TBASIC_ITEM *>(built_or_repaired_unit->GetPointerToItem());  //building item
            int i;
            bool enough_mat = true;

            //loop through all materials
            for (i = 0; i < scheme.materials_count && enough_mat; i++) {
              enough_mat = (GetPlayer()->GetStoredMaterial(i) >= bitem->mat_per_pt[i]);
            }

            if (enough_mat) {
              new_state = US_REPAIRING;
              new_int = built_or_repaired_unit->GetUnitID();
            }
            else {
              MessageText(false, const_cast<char*>("Not enough %s for the %s."), scheme.materials[i - 1]->name, bitem->name);

              ClearActions();
              new_state = US_NEXT_STEP;
            }
          }
        }
        else if (proc_event->TestEvent(US_NEXT_CONSTRUCTING)) 
        {
          TBUILDING_UNIT *building = (TBUILDING_UNIT *)built_or_repaired_unit;

          //if prepayed is zero then the building was finished
          if (!building->GetPrepayed()) 
          {
            ClearActions();
            new_state = US_NEXT_STEP; // correctly end action to stay or land
        
            if (building->TestState(US_IS_BEING_BUILT))
            {
              building->SendEvent(false, proc_event->GetTimeStamp(), US_STAY, -1, building->GetPosition().x, building->GetPosition().y, building->GetPosition().segment);

              // adding only possitive energy and food
              if (((TBASIC_ITEM *)building->GetPointerToItem())->energy > 0)
                player->AddUnitEnergyFood(((TBASIC_ITEM *)building->GetPointerToItem())->energy, 0);
              if (((TBASIC_ITEM *)building->GetPointerToItem())->food > 0)
                player->AddUnitEnergyFood(0, ((TBASIC_ITEM *)building->GetPointerToItem())->food);

              ((TBASIC_ITEM *)(building->GetPointerToItem()))->IncreaseActiveUnitCount();  // increase count of units of this kind
              building->PutState(US_STAY);
              building->SetAggressivity(((TBASIC_ITEM *)(building->GetPointerToItem()))->GetAggressivity(), true);

              if (itm->AllowAnyMaterial())   //add unit to material_array of player and increase counter in every source
                building->AddToPlayerArray();

              if (building->IsSelected()) {
                selection->UnselectAll();
                selection->AddUnit(building, false);
              }

              building->PutState(US_IS_BEING_BUILT);
              building->SetProgress(0);

              MessageText(true, const_cast<char*>("%s is complete."), building->GetPointerToItem()->name); //report, that work is done
      
              #if SOUND
                // play sound
                if (config.snd_unit_speech && TestPlayer(myself))
                  PlaySound(&itm->snd_workcomplete, 1);
              #endif
            }
          }
          else 
          {
            new_state = US_CONSTRUCTING;
            new_int = built_or_repaired_unit->GetUnitID();
          }
        }
        // send event to queue
        SendEvent(false, new_time_stamp, new_state, -1, pos.x, pos.y, pos.segment, new_direction, 0, 0, new_int);
      }      
      else  // built_or_repair_building is not neighbour
      {
        path_event = SendEvent(false, proc_event->GetTimeStamp(),US_WAIT_FOR_PATH,0); 
        waiting_request_id = path_event->GetRequestID();             
        new_state = US_NEXT_STEP;
        ComputePath(built_or_repaired_unit->GetPosition(),waiting_request_id,ET_NEXT_CR_NOT_NEIGHBOUR,new_state);
      }
    }
   
    
    // US_REPAIRING, US_CONSTRUCTING
    if (proc_event->TestEvent(US_CONSTRUCTING) || proc_event->TestEvent(US_REPAIRING))
    {     
      if (proc_event->TestEvent(US_REPAIRING))
      {
        Repair();
        new_state = US_NEXT_REPAIRING;
      }
      else
      {
        Build();
        new_state = US_NEXT_CONSTRUCTING;
      }
      
      new_time_stamp = proc_event->GetTimeStamp() + itm->GetRepairingTime();
 
      // send event to queue
      SendEvent(false, new_time_stamp, new_state, -1, pos.x, pos.y, pos.segment, move_direction);
     
    }

    // US_NEXT_UNLOADING
    if (proc_event->TestEvent(US_NEXT_UNLOADING))
    {
      new_time_stamp = proc_event->GetTimeStamp();
      new_direction = move_direction;

      if (!material_amount)
      {
        // worker can go out from building
        if (LeaveHolderUnit(acceptor)) 
        {
          acceptor->RemoveFromListOfUnits(this, LIST_WORKING);
          ReleaseCountedPointer(acceptor);

          // send info to not local playets that unit end unloading
          TEVENT * hlp;

          hlp = pool_events->GetFromPool();
          hlp->SetEventProps(proc_event->GetPlayerID(), proc_event->GetUnitID(), false, proc_event->GetTimeStamp(), RQ_END_UNLOAD, US_NONE, -1, pos.x, pos.y, pos.segment, move_direction, 0, 0, 0);
          SendNetEvent(hlp, all_players);
          pool_events->PutToPool(hlp);

          if (source)
          {
            new_state = US_NEXT_STEP; // move to source
            path_event = SendEvent(false, proc_event->GetTimeStamp() + TS_MIN_EVENTS_DIFF,US_WAIT_FOR_PATH,0); 
            waiting_request_id = path_event->GetRequestID();             
            ComputePath(source->GetPosition(),waiting_request_id,ET_NEXT_UNLOADING,new_state); 
          }
          else
          {
            MessageText(true, const_cast<char*>("Material unloaded."));   //otherwise notify about it and stop the worker
            ClearActions();
            new_state = US_NEXT_STEP; // correctly land or stay

            #if SOUND
              // play sound
              if (config.snd_unit_speech && TestPlayer(myself))
                PlaySound(&itm->snd_ready);
            #endif
            
            SendEvent(false, proc_event->GetTimeStamp() + TS_MIN_EVENTS_DIFF, new_state, -1, pos.x, pos.y, pos.segment, move_direction, 0, 0, 0);
          }
        }
        else  // cant go out -> plan new unloading and try to do it later
        {
          new_time_stamp = proc_event->GetTimeStamp() + UNI_TRY_TO_LEAVE_SHIFT;
          new_state = US_NEXT_UNLOADING;
          new_int = acceptor->GetUnitID();
          SendEvent(false, new_time_stamp, new_state, -1, pos.x, pos.y, pos.segment, new_direction, 0, 0, new_int);
        }        
      }
      else
      {
        //if unit is in the neighbourhood
        if (IsNeighbour(acceptor) || held) 
        {          
          // acceptor is destroyed
          if (acceptor->TestState(US_DYING) || acceptor->TestState(US_ZOMBIE) || acceptor->TestState(US_DELETE))
          {
            MessageText(false, const_cast<char*>("Can not unload to destroyed %s."), acceptor->GetPointerToItem()->name);
            ReleaseCountedPointer(acceptor);

            path_event = SendEvent(false, proc_event->GetTimeStamp(),US_SEARCHING_NEAREST,0); 
            waiting_request_id = path_event->GetRequestID();              
            SearchForNearestBuilding(source,waiting_request_id,ET_ACC_BUILD_DESTROYED);              
           
          }          
          else if (acceptor->TestState(US_IS_BEING_BUILT))   // acceptor is in state US_IS_BEING_BUILT
          {
            if (held) 
            {
              if (LeaveHolderUnit(acceptor)) 
              {
                acceptor->RemoveFromListOfUnits(this, LIST_WORKING);
                ReleaseCountedPointer(acceptor);

                // send info that unit end unloading
                TEVENT * hlp;

                hlp = pool_events->GetFromPool();
                hlp->SetEventProps(proc_event->GetPlayerID(), proc_event->GetUnitID(), false, proc_event->GetTimeStamp(), RQ_END_UNLOAD, US_NONE, -1, pos.x, pos.y, pos.segment, move_direction, 0, 0, 0);
                SendNetEvent(hlp, all_players);
                pool_events->PutToPool(hlp);

                if (source)
                {
                  path_event = SendEvent(false, proc_event->GetTimeStamp(),US_SEARCHING_NEAREST,0); 
                  waiting_request_id = path_event->GetRequestID();              
                  SearchForNearestBuilding(source,waiting_request_id,ET_UNLOAD_HELD_SEARCH_ACC);              
                 
                }
                else
                {
                  ClearActions();
                  MessageText(false, const_cast<char*>("Can not unload to %s."), acceptor->GetPointerToItem()->name);
                  new_state = US_NEXT_STEP; // correctly land or stay

                  #if SOUND
                    // play sound
                    if (config.snd_unit_speech && TestPlayer(myself))
                      PlaySound(&itm->snd_ready);
                  #endif
                  SendEvent(false, new_time_stamp, new_state, -1, pos.x, pos.y, pos.segment, new_direction, 0, 0, new_int);
                }                
              }              
              else   // cant go out -> plan new unloading and try to do it later
              {
                new_time_stamp = proc_event->GetTimeStamp() + UNI_TRY_TO_LEAVE_SHIFT;
                new_state = US_NEXT_UNLOADING;
                new_int = acceptor->GetUnitID();
                SendEvent(false, new_time_stamp, new_state, -1, pos.x, pos.y, pos.segment, new_direction, 0, 0, new_int);
              }              
            }
            else 
            { // unit was not held in acceptor
              if (source)
              {
                path_event = SendEvent(false, proc_event->GetTimeStamp(),US_SEARCHING_NEAREST,0); 
                waiting_request_id = path_event->GetRequestID();              
                SearchForNearestBuilding(source,waiting_request_id,ET_UNLOAD_DEST_BUILD_SEARCH_ACC);                
              }
              else
              {
                ClearActions();
                MessageText(false, const_cast<char*>("Can not unload to %s."), acceptor->GetPointerToItem()->name);
                new_state = US_NEXT_STEP; // correctly land or stay
                #if SOUND
                  // play sound
                  if (config.snd_unit_speech && TestPlayer(myself))
                    PlaySound(&itm->snd_ready);
                #endif
                SendEvent(false, new_time_stamp, new_state, -1, pos.x, pos.y, pos.segment, new_direction, 0, 0, new_int);
              }              
            }            
          }
          // acceptor is not destroyed and not being built
          else 
          {
            new_ps.x = acceptor->GetPosition().x + (int)(acceptor->GetPointerToItem()->GetWidth() / 2);
            new_ps.y = acceptor->GetPosition().y + (int)(acceptor->GetPointerToItem()->GetHeight() / 2);
            new_ps.segment = pos.segment;
            new_direction = GetCenterPosition().TPOSITION::GetDirection(new_ps);
          
            if ((new_direction != move_direction) && (new_direction != LAY_UNDEFINED)) // && (new_direction != LAY_UP) && (new_direction != LAY_DOWN) && (move_direction != LAY_UNDEFINED) && (move_direction != LAY_UP) && (move_direction != LAY_DOWN))
            {
              // unit is bad rotated -> next state will be US_XXX_ROTATING;

              new_state = DetermineRotationDirection(new_direction);
              // set new direction
              if (new_state == US_RIGHT_ROTATING)
                new_direction = (move_direction + 1) % LAY_HOR_DIRECTIONS_COUNT;
              else
                new_direction = (move_direction + LAY_HOR_DIRECTIONS_COUNT - 1) % LAY_HOR_DIRECTIONS_COUNT;
            }
            else {
              new_state = US_UNLOADING;
              new_int = acceptor->GetUnitID();
            }
            SendEvent(false, new_time_stamp, new_state, -1, pos.x, pos.y, pos.segment, new_direction, 0, 0, new_int);
          }
        }
        else 
        { // acceptor is not neighbour
         
          path_event = SendEvent(false, proc_event->GetTimeStamp(),US_WAIT_FOR_PATH,0); 
          waiting_request_id = path_event->GetRequestID();              
          new_state = US_NEXT_STEP;
          ComputePath(acceptor->GetPosition(),waiting_request_id,ET_UNLOAD_DEST_NOT_NEIGHBOUR,new_state); 
        }        
      }      
    }
    
    // US_UNLOADING
    if(proc_event->TestEvent(US_UNLOADING))
    {      
      GetPlayer()->IncStoredMaterial(mined_material, 1);
      material_amount--;
      
      new_time_stamp = proc_event->GetTimeStamp() + itm->GetUnloadingTime(mined_material);

      // send event to queue
      SendEvent(false, new_time_stamp, US_NEXT_UNLOADING, -1, pos.x, pos.y, pos.segment, move_direction, 0, 0, acceptor->GetUnitID()); //as info send worker id, and player id
    }
    
    
#if SOUND
    // RQ_MINE_SOUND
    if (proc_event->TestEvent(RQ_MINE_SOUND)) {

      // if sound request is still valid
      if (proc_event->GetRequestID() == sound_request_id && (TestState(US_MINING) || TestState(US_NEXT_MINE))) {
        if (source && !(source->TestState(US_DYING) || source->TestState(US_ZOMBIE) || source->TestState(US_DELETE))) {

          // prepare next mining sound
          new_time_stamp = itm->GetMiningSoundTime(source->GetOfferMaterial());

          if (new_time_stamp) {
            new_time_stamp += proc_event->GetTimeStamp();

            // put request into queue and send it to other players
            sound_request_id = SendRequest(false, new_time_stamp, RQ_MINE_SOUND, sound_request_id);
        
            TEVENT * help_event;
            help_event = pool_events->GetFromPool(); // get new (clear) event from pool
            help_event->SetEventProps(GetPlayerID(), GetUnitID(), false, new_time_stamp, RQ_MINE_SOUND, US_NONE, sound_request_id, 0, 0, 0, 0, 0, 0, 0, 0);
            SendNetEvent(help_event, all_players);
            pool_events->PutToPool(help_event);
          }
        } 
      }
    }
#endif
    
    
    // US_NEXT_MINE
    if (proc_event->TestEvent(US_NEXT_MINE))   //worker will test the source and start mining if possible
    {
      new_time_stamp = proc_event->GetTimeStamp();
            
      // in case that worker changer mined material type, set amount to 0
      if (change_mined_material){
        material_amount = 0;
        change_mined_material = false;
      }
      
      //test, whaether has enough material or not ; if yes, plann way to nearest_building
      if (material_amount >= itm->GetMaxMaterialAmount(mined_material))
      {
        bool ok = true;

        if (source->IsInsideMining() && held) 
          ok = LeaveHolderUnit(source);

        // worker cant go out from source -> plan new mining and try to do it later
        if (!ok) 
        {
          new_time_stamp = proc_event->GetTimeStamp() + UNI_TRY_TO_LEAVE_SHIFT;
          new_state = US_NEXT_MINE;
          new_simple5 = source->GetPlayerID();
          new_int = source->GetUnitID();
          SendEvent(false, new_time_stamp, US_NEXT_STEP, -1, pos.x, pos.y, pos.segment, move_direction, new_simple5, 0, new_int);
        }                
        else   // worker can go out from source
        {
          source->RemoveFromListOfUnits(this, LIST_WORKING);

          TEVENT * hlp;

          // send info that unit enf mining
          hlp = pool_events->GetFromPool();
          hlp->SetEventProps(proc_event->GetPlayerID(), proc_event->GetUnitID(), false, proc_event->GetTimeStamp(), RQ_END_MINE, US_NONE, -1, pos.x, pos.y, pos.segment, 0, 0, 0, 0);
          SendNetEvent(hlp, all_players);
          pool_events->PutToPool(hlp);

          
          path_event = SendEvent(false, proc_event->GetTimeStamp() + TS_MIN_EVENTS_DIFF, US_SEARCHING_NEAREST,0); 
          waiting_request_id = path_event->GetRequestID();              

          SearchForNearestBuilding(source,waiting_request_id,ET_MINED_SEARCH_ACCEPTOR,new_simple5);
        }  
      }
      else
      {
        new_time_stamp = proc_event->GetTimeStamp();

        //if unit is in the neighbourhood start mining
        if (IsNeighbour(source) || (held && source->IsInsideMining())) 
        {
          // source is not destroyed yet, but its beeing destryed (it is somewhere in the process of destroing)
          if (source->TestState(US_DYING) || source->TestState(US_ZOMBIE) || source->TestState(US_DELETE))
          {
            MessageText(false, const_cast<char*>("Can not mine from destroyed %s."), source->GetPointerToItem()->name);
            ReleaseCountedPointer(source);
            source = FindNewSource(pos);
        
            if (source) 
            {
              source = (TSOURCE_UNIT *)source->AcquirePointer();

              //new source found, move to new source              
              path_event = SendEvent(false, proc_event->GetTimeStamp(),US_WAIT_FOR_PATH,0); 
              waiting_request_id = path_event->GetRequestID();                       
              
              ComputePath(source->GetPosition(),waiting_request_id,ET_NEXTMINE_SOURCE_DESTR);                            
            }
            else 
            {
              ClearActions(); //correctly finish all actions
              new_state = US_NEXT_STEP;

              // send event to queue
              SendEvent(false, new_time_stamp, new_state, -1, pos.x, pos.y, pos.segment, move_direction);
            }
          }

          // source is not destroyed
          else
          {
            // test if worker is good rotated
            new_ps.x = source->GetPosition().x + (int)(source->GetPointerToItem()->GetWidth() / 2);
            new_ps.y = source->GetPosition().y + (int)(source->GetPointerToItem()->GetHeight() / 2);
            new_ps.segment = pos.segment;

            new_direction = GetCenterPosition().TPOSITION::GetDirection(new_ps);
                               
            if ((new_direction != move_direction) && (new_direction != LAY_UNDEFINED) && (!(held && source->IsInsideMining())))
            {
              // unit is bad rotated -> next state will be US_XXX_ROTATING;

              new_state = DetermineRotationDirection(new_direction);
              // set new direction
              if (new_state == US_RIGHT_ROTATING)
                new_direction = (move_direction + 1) % LAY_HOR_DIRECTIONS_COUNT;
              else
                new_direction = (move_direction + LAY_HOR_DIRECTIONS_COUNT - 1) % LAY_HOR_DIRECTIONS_COUNT;

              // send event to queue
              SendEvent(false, new_time_stamp, new_state, -1, pos.x, pos.y, pos.segment, new_direction);
            }
            else 
            {
              int requested_material_amount = MIN((int)((glfwGetTime() - new_time_stamp) / itm->GetMiningTime(mined_material)) + 1, int(itm->GetMaxMaterialAmount(mined_material) - material_amount));
              
              // send request to hyperplayer
              waiting_request_id = source->SendRequest(false, new_time_stamp, RQ_CAN_MINE, 0, GetPlayerID(), 0, 0, 0, 0, 0, GetUnitID(), requested_material_amount); // sets info about requested request
            }
          }
        }
        else  // unit is not neighbour of source
        {
          new_source = FindNewSource(pos);
          new_state = US_NEXT_STEP;
        
          // if new source founded by function is the same, report false (US_NEXT_STEP)
          if (!new_source || source == new_source) 
          {
            MessageText(false, const_cast<char*>("Can not get to the %s."), source->GetPointerToItem()->name);
            ClearActions();
            // send event to queue
            SendEvent(false, new_time_stamp, new_state, -1, pos.x, pos.y, pos.segment, move_direction);
          }
          else   // found new source
          {            
            source = (TSOURCE_UNIT *)new_source->AcquirePointer();

            path_event = SendEvent(false, proc_event->GetTimeStamp(),US_WAIT_FOR_PATH,0); 
            waiting_request_id = path_event->GetRequestID();                       
            ComputePath(source->GetPosition(),waiting_request_id,ET_NEXTMINE_NOT_HEIGHBOUR,new_state);
          }          
        }
      }
    }

    // RQ_CAN_MINE
    if (proc_event->TestEvent(RQ_CAN_MINE))
    {
      // test if unit is waiting for tris request
      if ((TestState(US_NEXT_MINE)) && (waiting_request_id == proc_event->GetRequestID()))
      {
        new_time_stamp = proc_event->GetTimeStamp();
        bool ok = true;

        // test if answer is true ot false (can mine or can't mine)
        if (proc_event->int2)
        {
          new_state = US_MINING;
          new_simple5 = source->GetPlayerID();
          new_int = source->GetUnitID();
          // send event to queue
          if (ok) SendEvent(false, new_time_stamp, new_state, -1, pos.x, pos.y, pos.segment, move_direction, new_simple5, 0, new_int, proc_event->int2);
        }
        else // can not mine
        {
          if (source->IsInsideMining() && held) 
            ok = LeaveHolderUnit(source);
                    
          if (!ok) 
          {
            new_time_stamp = proc_event->GetTimeStamp() + UNI_TRY_TO_LEAVE_SHIFT;
            SendRequest(false, new_time_stamp, RQ_CAN_MINE, proc_event->GetRequestID());
          }
          else 
          {
            source->RemoveFromListOfUnits(this, LIST_WORKING);

            // send info that unit enf mining
            TEVENT * hlp;

            hlp = pool_events->GetFromPool();
            hlp->SetEventProps(proc_event->GetPlayerID(), proc_event->GetUnitID(), false, proc_event->GetTimeStamp(), RQ_END_MINE, US_NONE, -1, pos.x, pos.y, pos.segment, 0, 0, 0, 0);
            SendNetEvent(hlp, all_players);
            pool_events->PutToPool(hlp);

            new_source = FindNewSource(pos);

            //new source found
            if (new_source)
            {
              ReleaseCountedPointer(source);

              source = (TSOURCE_UNIT *)new_source->AcquirePointer();       
              
              path_event = SendEvent(false, proc_event->GetTimeStamp(),US_WAIT_FOR_PATH,0); 
              waiting_request_id = path_event->GetRequestID();                       
              
              ComputePath(source->GetPosition(),waiting_request_id,ET_CANTMINE_CANLEAVE_SOURCE,new_simple5);              
            }
            else   //source not found
            {
              if (material_amount) //test ,whaether worker has some material to exploit
              {

                path_event = SendEvent(false, proc_event->GetTimeStamp(),US_SEARCHING_NEAREST,0); 
                waiting_request_id = path_event->GetRequestID();            

                if (ok)
                  SearchForNearestBuilding(source,waiting_request_id,ET_UNLOAD_NO_SOURCE,1,new_simple5);
                else
                  SearchForNearestBuilding(source,waiting_request_id,ET_UNLOAD_NO_SOURCE,0,new_simple5);                
              }
              else
              {
                ClearActions();
                new_state = US_NEXT_STEP;
                ReleaseCountedPointer(source);
                // send event to queue
                if (ok) SendEvent(false, new_time_stamp, new_state, -1, pos.x, pos.y, pos.segment, move_direction, new_simple5, 0, new_int);
              }
              //ReleaseCountedPointer(source);
              // send event to queue
              //if (ok) SendEvent(false, new_time_stamp, new_state, -1, pos.x, pos.y, pos.segment, move_direction, new_simple5, 0, new_int);
            }
          }
        }
          
        // send event to queue
        //if (ok) SendEvent(false, new_time_stamp, new_state, -1, pos.x, pos.y, pos.segment, move_direction, new_simple5, 0, new_int);
      }
    }

    // US_MINING
    if (proc_event->TestEvent(US_MINING))
    {

#if SOUND
      if (first_mine) {
        // plan mining sound
        new_time_stamp = proc_event->GetTimeStamp() + itm->GetMiningSoundShift(source->GetOfferMaterial()) + TS_MIN_EVENTS_DIFF;

        // put request into queue
        sound_request_id = SendRequest(false, new_time_stamp, RQ_MINE_SOUND);
        
        TEVENT * help_event;
        help_event = pool_events->GetFromPool(); // get new (clear) event from pool
        help_event->SetEventProps(GetPlayerID(), GetUnitID(), false, new_time_stamp, RQ_MINE_SOUND, US_NONE, sound_request_id, 0, 0, 0, 0, 0, 0, 0, 0);
        SendNetEvent(help_event, all_players);
        pool_events->PutToPool(help_event);
      }
#endif
      
      if(mined_material != static_cast<TSOURCE_ITEM*>(source->GetPointerToItem())->GetOfferMaterial())
        material_amount = 0;  //worker starts to mine new material, old material is thrown away
          
      mined_material = static_cast<TSOURCE_ITEM *>(source->GetPointerToItem())->GetOfferMaterial();
      material_amount += proc_event->int2;
      
      double time_per_mine = itm->GetMiningTime(mined_material);
      new_time_stamp = proc_event->GetTimeStamp() + (proc_event->int2 * time_per_mine);
      
      // send event to queue
      SendEvent(false, new_time_stamp, US_NEXT_MINE, -1, pos.x, pos.y, pos.segment, move_direction, source->GetPlayerID(), 0, source->GetUnitID());
    }
   
    // RQ_PATH_FINDING
    if (proc_event->TestEvent(RQ_PATH_FINDING))
    {
      //read all the values
      int event_type = US_NONE;
      
      #if DEBUG_PATHFINDING || DEBUG_EVENTS
      Debug (LogMsg ("comming= %d , expected=%d", proc_event->GetRequestID(), this->waiting_request_id));
      #endif

      if (proc_event->GetRequestID() == this->waiting_request_id) //waiting request id
      {
        event_type = proc_event->int2;
        goal.x = proc_event->simple2;
        goal.y = proc_event->simple3;
        goal.segment = proc_event->simple4;
      
        if (path) delete path;
        path = reinterpret_cast<TPATH_LIST*>(proc_event->int1); 

        switch(event_type)
        {
          case ET_ATTACK_TO_OTHER_SEG: //attack to other segment
          {
            T_SIMPLE new_state = proc_event->simple5;

            if (path)
            {
              SendEvent(false, proc_event->GetTimeStamp(), US_NEXT_STEP,-1, GetPosition().x, GetPosition().y, GetPosition().segment,
                        GetLookDirection()); 
            }
            else
            { 
              MessageText(false, const_cast<char*>("Attack to %s failed."), target->GetPointerToItem()->name);
              SendEvent(false, proc_event->GetTimeStamp(), US_END_ATTACK, -1, pos.x, pos.y, pos.segment, move_direction);
            }
            PutState(new_state);
          }
          break;
          case ET_ATTACK_TOO_FAR_AWAY:  //unit had state US_NEXT_ATTACK before called ComputePath function and was too far away from goal(for attack)
          {            
            T_SIMPLE new_state = proc_event->simple5;            

            if (path)
              SendEvent(false, proc_event->GetTimeStamp(), US_NEXT_STEP, -1, GetPosition().x, GetPosition().y, GetPosition().segment,
                    GetLookDirection());                  //try to move nearer to the target            
            else    //can't fire off - bad position
            {
              MessageText(false, const_cast<char*>("Can not get to target %s."), target->GetPointerToItem()->name);
              SendEvent(false, proc_event->GetTimeStamp(), US_END_ATTACK, -1, pos.x, pos.y, pos.segment, move_direction);
            }
            PutState(new_state);
          }
          break;
          case ET_TARGET_MOVING:                //unit has target, which is moving
          case ET_HIDER_MOVING:                 //unit has hider, which is moving and change position
          case ET_CONSTRUCTED_OBJECT_MOVING:    //unit has build_or_repaired_unit, which is moving and change position
             SendEvent(false, proc_event->GetTimeStamp(), US_NEXT_STEP, -1, pos.x, pos.y, pos.segment, move_direction);
          break;
          case ET_NOTNEXTPOS_LANDING:           //next position in path is not moveable, but last -> landing
          case ET_NEXTPOS_NOTMOVABLE:           //next position in path is not moveable
          {
            last_state = proc_event->simple5;
            T_SIMPLE unit_state = proc_event->simple6;            

            if (unit_state == US_TRY_TO_MOVE)
              new_time_stamp = proc_event->GetTimeStamp() + UNI_TRY_TO_MOVE_SHIFT;
            else 
            {
              new_time_stamp = proc_event->GetTimeStamp();
              ResetTryToMoveTimer();
            }
            // send event to queue
            SendEvent(false, new_time_stamp, unit_state, -1, pos.x, pos.y, pos.segment, move_direction);
            PutState(last_state);          
          }
          break;          
          case ET_UNIT_AGAINST_UNIT:           //next position is not empty -> worker unit against the another unit 
          {
            last_state = proc_event->simple5;
            T_SIMPLE unit_state = proc_event->simple6;            

            if (path)   //new path found
            {
              test_ps = path->GetNextPosition();
              test_direction = GetPosition().GetDirection(test_ps);
              if (!GetPlayer()->GetLocalMap()->IsNextPositionEmpty(test_ps, test_direction, this))
              {
                // send event to queue
                SendEvent(false, proc_event->GetTimeStamp() + UNI_TRY_TO_MOVE_SHIFT, US_TRY_TO_MOVE, -1, pos.x, pos.y, pos.segment, move_direction);
                break;
              }
              // another path found, but must be tested
              if (unit_state == US_TRY_TO_MOVE)
                new_time_stamp = proc_event->GetTimeStamp() + UNI_TRY_TO_MOVE_SHIFT;
              else 
              {
                new_time_stamp = proc_event->GetTimeStamp();
                ResetTryToMoveTimer();
              }
              // send event to queue
              SendEvent(false, new_time_stamp, unit_state, -1, pos.x, pos.y, pos.segment, move_direction);
              PutState(last_state);
              break;
            }
            else 
            { //found insurmountable snag -> new state will be US_NEXT_STEP
              ResetTryToMoveTimer();

              // send event to queue
              SendEvent(false, proc_event->GetTimeStamp(), US_NEXT_STEP, -1, pos.x, pos.y, pos.segment, move_direction);
              break;
            }
          }
          break;
          case ET_NOTPATH_LAND:
          {           
            last_state = proc_event->simple5;
            T_SIMPLE unit_state = proc_event->simple6;

            if (unit_state == US_TRY_TO_MOVE)
              new_time_stamp = proc_event->GetTimeStamp() + UNI_TRY_TO_MOVE_SHIFT;
            else 
            {
              new_time_stamp = proc_event->GetTimeStamp();
              ResetTryToMoveTimer();
            }
            // send event to queue
            SendEvent(false, new_time_stamp, unit_state, -1, pos.x, pos.y, pos.segment, move_direction);
            PutState(last_state);
          }
          break;
          case ET_WORKER_ACTIVITY:  //unit is one of following states: US_START_MINE, US_START_REPAIR,US_START_HIDING,US_START_UNLOAD
          {            
            new_state = US_NEXT_STEP;

            // send event to queue
            SendEvent(false, proc_event->GetTimeStamp(), new_state, -1, pos.x, pos.y, pos.segment, move_direction);
          }
          break;
          case ET_NEXT_HIDING:      //unit has state US_NEXT_HIDING
          {
            if (!path)
            {
              MessageText(false, const_cast<char*>("Can not get to %s."), hider->GetPointerToItem()->name);
              ClearActions();      
            }
            // send event to queue
            SendEvent(false, proc_event->GetTimeStamp(), US_NEXT_STEP, -1, pos.x, pos.y, pos.segment, new_direction);
          }
          break;
          case ET_NEXT_UNLOADING:   //worker will unload material if possible
          {            
            SendEvent(false, proc_event->GetTimeStamp(), proc_event->simple5, -1, pos.x, pos.y, pos.segment, move_direction, 0, 0, new_int);
          }
          break;
          case ET_MINED_HAS_ACCEPTOR:   //worker mined maximal amount of material and has acceptable building
          {
            new_simple5 = proc_event->simple5;
            SendEvent(false, proc_event->GetTimeStamp(), US_NEXT_STEP, -1, pos.x, pos.y, pos.segment, move_direction, new_simple5, 0, new_int);
          }
          break;
          case ET_CANTMINE_CANLEAVE_SOURCE: 
          {
            new_simple5 = proc_event->simple5;
            SendEvent(false, proc_event->GetTimeStamp(), US_NEXT_STEP, -1, pos.x, pos.y, pos.segment, move_direction, new_simple5, 0, new_int); // send event to queue
          }
          break;
          case ET_UNLOAD_DEST_RUINED: //unit has material to unload; acceptable building ruined while unit walks towards it
          { 
            SendEvent(false, proc_event->GetTimeStamp(), proc_event->simple5, -1, pos.x, pos.y, pos.segment, new_direction, 0, 0, new_int);
          }
          break;
          case ET_UNLOAD_DEST_BUILD_NOT_HELD:
          {
            SendEvent(false, proc_event->GetTimeStamp(), proc_event->simple5, -1, pos.x, pos.y, pos.segment, new_direction, 0, 0, new_int);
          }
          break;
          case ET_UNLOAD_DEST_NOT_NEIGHBOUR:  //unit is in state US_NEXT_UNLOADING,acceptor is not neighbour->unit has to find its way to acceptor          
          {
            if (!path)
            {
              MessageText(false, const_cast<char*>("Can not get to the %s."), acceptor->GetPointerToItem()->name);
              ClearActions();      
            }
            SendEvent(false, proc_event->GetTimeStamp(),proc_event->simple5, -1, pos.x, pos.y, pos.segment, new_direction, 0, 0, new_int);
          }
          break;
          case ET_NEXTMINE_SOURCE_DESTR: //unit is in state US_NEXT_MINE, it is in the neighbourhood of the source, but source is in one of states US_DYING or US_ZOMBIE or US_DELETE  
          {
            new_state = US_NEXT_STEP;

            // send event to queue
            SendEvent(false, proc_event->GetTimeStamp(), new_state, -1, pos.x, pos.y, pos.segment, move_direction);
          }
          break;
          case ET_NEXT_CR_NOT_NEIGHBOUR:  //unit is in state US_NEXT_CONSTRUCTING or US_NEXT_REPAIRING, but build/repaired unit is not in the heighbourhood , so unit has to walk towards it
          {                              
            if (!path)
            {
              MessageText(false, const_cast<char*>("Can not get to the %s."), built_or_repaired_unit->GetPointerToItem()->name);
              ClearActions();      
            }                   
            SendEvent(false, proc_event->GetTimeStamp(),proc_event->simple5, -1, pos.x, pos.y, pos.segment, new_direction, 0, 0, new_int);
          }
          break;
          case ET_UNLOAD_HELD:  //unit has material to unload;it is in some held list;acceptable building was found  
          { 
            SendEvent(false, proc_event->GetTimeStamp(),proc_event->simple5, -1, pos.x, pos.y, pos.segment, new_direction, 0, 0, new_int);
          }
          break;
          case ET_NEXTMINE_NOT_HEIGHBOUR: //unit is in state US_NEXT_MINE, doesnt have max.amount of material and unit is not neighbour of source    
          { 
            SendEvent(false, proc_event->GetTimeStamp(),proc_event->simple5, -1, pos.x, pos.y, pos.segment, move_direction);
          }
          break;
          case ET_CANTMINE_LEAVE_NOSOURCE_OK:   //unit has some unmined material, new source wasnt found, needs to go to the acceptable building to unload at least what has, ok = 1.
          {           
            ReleaseCountedPointer(source);
            // send event to queue
            SendEvent(false, proc_event->GetTimeStamp(), proc_event->simple5, -1, pos.x, pos.y, pos.segment, move_direction, proc_event->simple6, 0, new_int);
          }
          break;
          case ET_CANTMINE_LEAVE_NOSOURCE_NOK:  //unit has some unmined material, new source wasnt found, needs to go to the acceptable building to unload at least what has, ok = 0.
          {
            ReleaseCountedPointer(source);
            //if ok == false , none event is sent
          }
          break;
        } 
      }
    } 

    
    if (proc_event->TestEvent(RQ_NEAREST_SEARCHING))
    {
      int event_type = US_NONE;

      #if DEBUG_PATHFINDING || DEBUG_EVENTS
      Debug (LogMsg ("comming= %d , expected=%d", proc_event->GetRequestID(), this->waiting_request_id));
      #endif
      
      if (proc_event->GetRequestID() == this->waiting_request_id) //waiting request id
      {
        event_type = proc_event->int2;
        acceptor = reinterpret_cast<TBUILDING_UNIT*>(proc_event->int1); 

        switch (event_type)
        {
          case ET_MINED_SEARCH_ACCEPTOR: //worker unmined something from mine and found/not found acceptable building
          { 
            T_SIMPLE simple5 = proc_event->simple1; 

            new_state = US_NEXT_STEP;

            if (acceptor) 
            {                   //if nearest building found, then send worker there
              acceptor = (TBUILDING_UNIT *)acceptor->AcquirePointer();

              path_event = SendEvent(false, proc_event->GetTimeStamp(),US_WAIT_FOR_PATH,0); 
              this->waiting_request_id = path_event->GetRequestID();              
              ComputePath(acceptor->GetPosition(),waiting_request_id,ET_MINED_HAS_ACCEPTOR,simple5);
            }
            else 
            {
              ClearActions();
              SendEvent(false, proc_event->GetTimeStamp(), US_NEXT_STEP, -1, pos.x, pos.y, pos.segment, move_direction, simple5, 0, new_int);
            }
          }
          break;
          case ET_ACC_BUILD_DESTROYED: //worker unmined some material, acceptable building was destroyed 
          {
            if (acceptor)                                 //if nearest building found, then send worker there
            {
              acceptor = (TBUILDING_UNIT *)acceptor->AcquirePointer();
              new_state = US_NEXT_STEP;

              path_event = SendEvent(false, proc_event->GetTimeStamp(),US_WAIT_FOR_PATH,0); 
              waiting_request_id = path_event->GetRequestID();              
              ComputePath(acceptor->GetPosition(),waiting_request_id,ET_UNLOAD_DEST_RUINED,new_state);                        
            }
            else
            {
              ClearActions();
              new_state = US_NEXT_STEP; // correctly finish all actions of unit
              SendEvent(false, proc_event->GetTimeStamp(), new_state, -1, pos.x, pos.y, pos.segment, new_direction, 0, 0, new_int);
            } 
          }
          break;
          case ET_UNLOAD_HELD_SEARCH_ACC: //unit has material to unload;it is in some held list;acceptable building must be found now
          {             
            new_state = US_NEXT_STEP;

            if (acceptor) 
            {
              //if nearest building found, then send worker there              
              acceptor = (TBUILDING_UNIT *)acceptor->AcquirePointer();              

              path_event = SendEvent(false, proc_event->GetTimeStamp(),US_WAIT_FOR_PATH,0); 
              waiting_request_id = path_event->GetRequestID();              
              ComputePath(acceptor->GetPosition(),waiting_request_id,ET_UNLOAD_HELD,new_state);
            }
            else 
            {
              MessageText(false, const_cast<char*>("Can not unload material."));
              ClearActions();
              #if SOUND
                // play sound
                if (config.snd_unit_speech && TestPlayer(myself))
                  PlaySound(&itm->snd_ready);
              #endif
              SendEvent(false, proc_event->GetTimeStamp(), new_state, -1, pos.x, pos.y, pos.segment, new_direction, 0, 0, new_int);
            }
          }
          break;
          case ET_UNLOAD_DEST_BUILD_SEARCH_ACC:
          {            
            new_state = US_NEXT_STEP;

            //if nearest building found, then send worker there
            if (acceptor) 
            { 
              acceptor = (TBUILDING_UNIT *)acceptor->AcquirePointer();
              path_event = SendEvent(false, proc_event->GetTimeStamp(),US_WAIT_FOR_PATH,0); 
              waiting_request_id = path_event->GetRequestID();              
              ComputePath(acceptor->GetPosition(),waiting_request_id,ET_UNLOAD_DEST_BUILD_NOT_HELD,new_state);
            }
            else 
            {
              ClearActions();
              MessageText(false, const_cast<char*>("Can not unload the %s."), scheme.materials[GetMaterial()]->name);
              #if SOUND
                // play sound
                if (config.snd_unit_speech && TestPlayer(myself))
                  PlaySound(&itm->snd_ready);
              #endif
              SendEvent(false, proc_event->GetTimeStamp(), new_state, -1, pos.x, pos.y, pos.segment, new_direction, 0, 0, new_int);
            } 
          }
          break;
          case ET_UNLOAD_NO_SOURCE:
          {            
            T_SIMPLE is_ok = proc_event->simple1;             

            if (acceptor)                                 //if nearest building found, then send worker there
            {                  
              acceptor = (TBUILDING_UNIT *)acceptor->AcquirePointer();
              new_state = US_NEXT_STEP; // start moving to acceptor

              path_event = SendEvent(false, proc_event->GetTimeStamp(),US_WAIT_FOR_PATH,0); 
              waiting_request_id = path_event->GetRequestID();                       

              if (is_ok == 1)
                ComputePath(acceptor->GetPosition(),waiting_request_id,ET_CANTMINE_LEAVE_NOSOURCE_OK,new_state,proc_event->simple2);              
              else
                ComputePath(acceptor->GetPosition(),waiting_request_id,ET_CANTMINE_LEAVE_NOSOURCE_NOK,new_state,proc_event->simple2);              
            }
            else
            {
              ClearActions();
              new_state = US_NEXT_STEP;
              ReleaseCountedPointer(source);
              // send event to queue
              if (is_ok == 1) 
                SendEvent(false, proc_event->GetTimeStamp(), new_state, -1, pos.x, pos.y, pos.segment, move_direction, proc_event->simple2, 0, new_int);
            }
          }
          break;
        }
      }
    }

    //group moving
    if (proc_event->TestEvent(RQ_GROUP_MOVING))
    {
      int event_type = US_NONE;
      
      #if DEBUG_PATHFINDING || DEBUG_EVENTS
      Debug (LogMsg ("comming= %d , expected=%d", proc_event->GetRequestID(), this->waiting_request_id));
      #endif

      if (proc_event->GetRequestID() == this->waiting_request_id) //waiting request id
      {
        event_type = proc_event->int2;
        goal.x = proc_event->simple2;
        goal.y = proc_event->simple3;
        goal.segment = proc_event->simple4;

        ClearActions();

        path = reinterpret_cast<TPATH_LIST*>(proc_event->int1); 
        
        switch(event_type)
        {
          case ET_GROUP_MOVING: //unit moves as a part of groups of units
          {
            T_SIMPLE succ = proc_event->simple1;

            if ((!succ) && (path)) delete path;

            // in all cases send US_NEXT_STEP
            SendEvent(false, proc_event->GetTimeStamp(), US_NEXT_STEP, -1, GetPosition().x, GetPosition().y, GetPosition().segment, GetMoveDirection()); 
          }
          break;
        }
      }
    }

  }
}


/**
 * Test if there is any source to the EAST of unit in the distance between v1 and v2-1
 */
inline TSOURCE_UNIT * TWORKER_UNIT::TestSourceEast(TPOSITION_3D position, T_SIMPLE w, T_SIMPLE h, T_SIMPLE v1, T_SIMPLE v2){
  TSOURCE_UNIT * found_source = NULL;
  int x,y;
  T_SIMPLE v;

  for (v = v1; v < v2; v++)
    for (x = position.x + w + v, y = position.y - v; y <= position.y + h + v - 1; y++)
    {
      found_source = IsSourceOnPosition(x,y);
      if (found_source) return found_source;
    }


  return NULL;
}

/**
 * Test if there is any source to the WEST of unit in the distance between v1 and v2-1
 */
inline TSOURCE_UNIT * TWORKER_UNIT::TestSourceWest(TPOSITION_3D position, T_SIMPLE w, T_SIMPLE h, T_SIMPLE v1, T_SIMPLE v2){
  TSOURCE_UNIT * found_source = NULL;
  int x,y;
  T_SIMPLE v;

  for (v = v1; v < v2; v++)
    for (x = position.x - v - 1, y = position.y - v ;  y <= position.y + h + v - 1; y++) 
    {
      found_source = IsSourceOnPosition(x,y);
      if (found_source) return found_source;
    }

  return NULL;
}


/**
 * Test if there is any source to the NORTH of unit in the distance between v1 and v2-1
 */
inline TSOURCE_UNIT * TWORKER_UNIT::TestSourceNorth(TPOSITION_3D position, T_SIMPLE w, T_SIMPLE h, T_SIMPLE v1, T_SIMPLE v2){
  TSOURCE_UNIT * found_source = NULL;
  int x,y;
  T_SIMPLE v;

  for (v = v1; v < v2; v++)
    for (x = position.x - v - 1, y = position.y + h + v; x <= position.x + w + v; x++)
    {
      found_source = IsSourceOnPosition(x,y);
      if (found_source) return found_source;
    }

  return NULL;
}


/**
 * Test if there is any source to the SOUTH of unit in the distance between v1 and v2-1
 */
inline TSOURCE_UNIT * TWORKER_UNIT::TestSourceSouth(TPOSITION_3D position, T_SIMPLE w, T_SIMPLE h, T_SIMPLE v1, T_SIMPLE v2)
{
  TSOURCE_UNIT * found_source = NULL;
  int x,y;
  T_SIMPLE v;

  for (v = v1; v < v2; v++)
    for  (x = position.x + v + w, y = position.y - v - 1 ; x >= position.x - v - 1; x--)
    {
      found_source = IsSourceOnPosition(x,y);
      if (found_source) return found_source;
    }

  return NULL;
}



/**
 *  Finds a new source, the old one has collapsed or it is unable to get to it. It is used if the source
 *  collapses while worker is mining or if there is no path to source.
 *
 *  What he does is that he tries to find the new source in the surrounding of
 *  a unit view size from the position of collapsed source. Strategy: random search
 *
 *  @return The method returns pointer to found source or NULL
 */

TSOURCE_UNIT * TWORKER_UNIT::FindNewSource(TPOSITION_3D position)
{
  T_SIMPLE width = GetUnitWidth();
  T_SIMPLE height = GetUnitHeight();
  T_SIMPLE view = static_cast<TBASIC_ITEM*>(pitem)->view;
  TSOURCE_UNIT * found_source = NULL;
  T_SIMPLE v1, v2;
  
  int diff = 3;
  
  for (v1 = 0, v2 = MIN (v1 + diff, view) ; v1 < view ; v1+=diff, v2 = MIN (v1 + diff, view))   //loop through the distance from the worker
  {
    switch(this->look_direction)
    {
    case LAY_NORTH_EAST:
    case LAY_NORTH_WEST:
    case LAY_NORTH:
      if(GetRandomInt(2)) {
        if (!found_source) found_source = TestSourceEast(position, width, height, v1, v2);
        if (!found_source) found_source = TestSourceWest(position, width, height, v1, v2);
      }
      else {
        if (!found_source) found_source = TestSourceWest(position, width, height, v1, v2);
        if (!found_source) found_source = TestSourceEast(position, width, height, v1, v2);
      }

      if(GetRandomInt(2)) {
        if (!found_source) found_source = TestSourceNorth(position, width, height, v1, v2);
        if (!found_source) found_source = TestSourceSouth(position, width, height, v1, v2);
      }
      else {
        if (!found_source) found_source = TestSourceSouth(position, width, height, v1, v2);
        if (!found_source) found_source = TestSourceNorth(position, width, height, v1, v2);
      }
    break;

    case LAY_SOUTH_WEST:
    case LAY_SOUTH_EAST:
    case LAY_SOUTH:
      if(GetRandomInt(2)) {
        if (!found_source) found_source = TestSourceWest(position, width, height, v1, v2);
        if (!found_source) found_source = TestSourceEast(position, width, height, v1, v2);
      }
      else {
        if (!found_source) found_source = TestSourceEast(position, width, height, v1, v2);
        if (!found_source) found_source = TestSourceWest(position, width, height, v1, v2);
      }

      if(GetRandomInt(2)) {
        if (!found_source) found_source = TestSourceSouth(position, width, height, v1, v2);      
        if (!found_source) found_source = TestSourceNorth(position, width, height, v1, v2);
      }
      else {
        if (!found_source) found_source = TestSourceNorth(position, width, height, v1, v2);
        if (!found_source) found_source = TestSourceSouth(position, width, height, v1, v2);      
      }
    break;
    
    case LAY_EAST:
      if(GetRandomInt(2)) {
        if (!found_source) found_source = TestSourceSouth(position, width, height, v1, v2);
        if (!found_source) found_source = TestSourceNorth(position, width, height, v1, v2);
      }
      else {
        if (!found_source) found_source = TestSourceNorth(position, width, height, v1, v2);
        if (!found_source) found_source = TestSourceSouth(position, width, height, v1, v2);
      }

      if(GetRandomInt(2)) {
        if (!found_source) found_source = TestSourceEast(position, width, height, v1, v2);
        if (!found_source) found_source = TestSourceWest(position, width, height, v1, v2);
      }
      else {
        if (!found_source) found_source = TestSourceWest(position, width, height, v1, v2);
        if (!found_source) found_source = TestSourceEast(position, width, height, v1, v2);
      }
      break;

    case LAY_WEST:
      if(GetRandomInt(2)) {
        if (!found_source) found_source = TestSourceNorth(position, width, height, v1, v2);
        if (!found_source) found_source = TestSourceSouth(position, width, height, v1, v2);
      }
      else
      {
        if (!found_source) found_source = TestSourceSouth(position, width, height, v1, v2);
        if (!found_source) found_source = TestSourceNorth(position, width, height, v1, v2);
      }

      if(GetRandomInt(2)) {
        if (!found_source) found_source = TestSourceWest(position, width, height, v1, v2);
        if (!found_source) found_source = TestSourceEast(position, width, height, v1, v2);
      }
      else {
        if (!found_source) found_source = TestSourceEast(position, width, height, v1, v2);
        if (!found_source) found_source = TestSourceWest(position, width, height, v1, v2);
      }
      break;   

    default:
      break;
    }
  }

  if (!found_source) {
    MessageText(false, const_cast<char*>("Can not find new source."));
  }

  return found_source;
}

/*
*  Function finds out, if in the current position is source which offers the same material
*  as the worker mines. If so, returns it.
*
*  @param position The position where new source is beeing found.
*/
TSOURCE_UNIT * TWORKER_UNIT::IsSourceOnPosition(int pos_x, int pos_y)
{
  int unit_view = static_cast<TBASIC_ITEM*>(pitem)->view;  
  TSOURCE_UNIT * found_source = NULL;
  TSOURCE_UNIT * source_unit = NULL;
  TMAP_UNIT * map_unit = NULL;

  if (IsSeenByUnit(pos,pos_x, pos_y,GetUnitWidth(),GetUnitHeight(),unit_view) && map.IsInMap(pos_x,pos_y)) 
  {
    map_unit = map.segments[pos.segment].surface[pos_x][pos_y].unit;
    if ((map_unit) && (map_unit->TestItemType(IT_SOURCE)))  //if it is valid source
    {
      source_unit = static_cast<TSOURCE_UNIT*>(map_unit);
      // get source materials
      int source_material = static_cast<TSOURCE_ITEM*>(source_unit->GetPointerToItem())->GetOfferMaterial();
      // zisti ci je zdroj neprazdny a ci moze tazit a ak uz nieco tazi, tak primarne to nech tazi dalej
      if ((!source_unit->IsEmpty()) && (GetMaterial() == source_material) && (source_unit != source)) 
      { 
        found_source =  source_unit;   //set finded source as new                    
        return found_source;
      }                          
    }
  }  
  return found_source;
}


/**
 *  Selects reaction of the unit with the dependence of the clicked unit/building.
 *
 *  @param unit    Unit, that was clicked on.
 */
bool TWORKER_UNIT::SelectReaction(TMAP_UNIT *unit, TUNIT_ACTION action)
{
  bool ok = false;

  if (unit->TestState(US_GHOST)) unit = unit->GetGhostOwner();

  // attack mine
  switch (action) {
  case UA_ATTACK:
    ok = StartAttacking(unit, false);
    break;

  case UA_HIDE:
    ok = StartHiding(unit, false);
    break;

  case UA_MINE:
    if (unit->TestItemType(IT_SOURCE))
      ok = StartMine(static_cast<TSOURCE_UNIT *>(unit), false);
    else
      ok = StartUnload(static_cast<TBUILDING_UNIT *>(unit), false);
    break;

  case UA_REPAIR:
    ok = StartRepair(static_cast<TBASIC_UNIT *>(unit), false);
    break;

  default: break;
  }

  return ok;
}



/**
 *  Returns unit action - what is doing now.
 */
TUNIT_ACTION TWORKER_UNIT::GetAction()
{
  if (target) return UA_ATTACK;
  else if (built_or_repaired_unit) return UA_REPAIR;
  else if (source) return UA_MINE;
  else if (
    TestState(US_LANDING) || TestState(US_UNLANDING) ||
    TestState(US_MOVE) || TestState(US_NEXT_STEP) || TestState(US_TRY_TO_MOVE) || 
    TestState(US_LEFT_ROTATING) || TestState(US_RIGHT_ROTATING) || 
    TestState(US_WAIT_FOR_PATH) 
  ) return UA_MOVE;
  else return UA_STAY;
}


void TWORKER_UNIT::ClearActions()
{
  TFORCE_UNIT::ClearActions();

  if (held && held_list == LIST_WORKING)
    held_where->RemoveFromListOfUnits (this, held_list);

  ReleaseCountedPointer(built_or_repaired_unit);
  ReleaseCountedPointer(source);
  ReleaseCountedPointer(acceptor);

  sound_request_id = 0;
}


/**
 *  Change unit animation according to its state.
 */
void TWORKER_UNIT::ChangeAnimation()
{
  int tg, tex = 0;
  TWORKER_ITEM *pit = static_cast<TWORKER_ITEM *>(pitem);
  double ratio = 0;
  double time = -1;
  bool reverse = false;
  bool loop = true;

  if (TestState(US_MOVE)) {
    tg = pit->tg_move_id;
    if (tg < 0) tg = pit->tg_stay_id;
    ratio = pit->max_speed[pos.segment] / speed;
  }
  else if (TestState(US_LEFT_ROTATING) || TestState(US_RIGHT_ROTATING)) {
    tg = pit->tg_rotate_id;
    if (tg < 0) tg = pit->tg_stay_id;
    ratio = pit->max_rotation[pos.segment] / rotation_speed;
  }
  else if (TestState(US_ATTACKING) || TestState(US_FEEDING)) {
    tg = pit->tg_attack_id;
    if (tg < 0) tg = pit->tg_stay_id;
  }
  else if (TestState(US_MINING)) {
    tg = pit->tg_mine_id;
    if (tg < 0) tg = pit->tg_stay_id;
  }
  else if (TestState(US_LANDING)) {
    tg = pit->tg_land_id;
    if (tg < 0) tg = pit->tg_stay_id;
    time = move_time;
    loop = false;
  }
  else if (TestState(US_UNLANDING)) {
    tg = pit->tg_land_id;
    if (tg < 0) tg = pit->tg_stay_id;
    time = move_time;
    reverse = true;
    loop = false;
  }
  else if (TestState(US_ANCHORING)) {
    tg = pit->tg_anchor_id;
    if (tg < 0) tg = pit->tg_stay_id;
  }
  else if (TestState(US_CONSTRUCTING) || TestState(US_REPAIRING)) {
    tg = pit->tg_repair_id;
    if (tg < 0) tg = pit->tg_stay_id;
  }
  else if (TestState(US_DYING)) {
    tg = pit->tg_dying_id;
  }
  else if (TestState(US_ZOMBIE)) {
    tg = pit->tg_zombie_id;
    time = UNI_ZOMBIE_TIME;
  }
  else tg = pit->tg_stay_id;


  if (tg >= 0) {
    if (player->race->tex_table.groups[tg].count >= 8) tex = look_direction;
    else tex = 0;

    animation->SetTexItem(player->race->tex_table.GetTexture(tg, tex));
    animation->Play();

    if (ratio) animation->SetSpeedRatio(ratio);
    if (time >= 0) animation->SetAnimTime(time);
    if (reverse) animation->SetReverse(reverse, true);
    if (!loop) animation->SetLoop(loop);
  }
  else animation->SetTexItem(NULL);
}


/**
 *  Find out, if the building given by parameter No1, can be built on position or not.
 *
 *  @param building   Type of building, which will be build.
 *  @param pos        Center position of the future building.
 *  @param build_map  2D array of boolean values. Field will be true if building can be built on it.
 *  @param write_msg  True if messages should be written.
 *  @param auto_call  Mark if function was called automaticly (computer player) or manualny (by real player).
 *
 *  @return true, if the building can be built, false otherwise.
 */
bool TWORKER_UNIT::CanBuild(TBUILDING_ITEM *building, TPOSITION pos, bool **build_map, bool write_msg, bool auto_call) 
{
  int i, j;
  bool can_build = true;

  // find out, whaether there is enough material for construction of building, if the test fails, return false  
  for (i = 0 ; i < scheme.materials_count ; i++)
  {
    if (building->materials[i] > GetPlayer()->GetStoredMaterial(i))
    {
      if (write_msg) MessageText(false, const_cast<char*>("Not enough %s for building the %s."), scheme.materials[i]->name, building->name);

      if (build_map) can_build = false;
      else return false;
    }
  }

  // check if there is enough food for building
  if (((building->ancestor) && (player->GetInFood() - player->GetOutFood() + building->food - building->ancestor->food < 0) ) || ((!building->ancestor) && (building->food < 0) && (player->GetInFood() - player->GetOutFood() + building->food < 0))) 
  {
    if (write_msg) MessageText(false, const_cast<char*>("Not enough food for the %s."), building->name);
    if (build_map) can_build = false;
    else return false; 
  }

  int x, y, seg;

  bool map_ok = true;
  bool field_ok = true;
  bool visible_field = false;

  TTERRAIN_ID tid;
  T_BYTE state;
  TMAP_UNIT *unit = NULL;

  T_SIMPLE w = building->GetWidth();
  T_SIMPLE h = building->GetHeight();

  // test, whaether building can be build exactly on this position  
  for (j = 0; j < h; j++) 
  {
    for (i = 0; i < w; i++)
    { 
      x = pos.x + i - w / 2;
      y = pos.y + j - h / 2;

      /*
        Building cannot be built on specified position if any of the following conditions passes:
        - some needed fields are not in map
        - area under the future building is unknown
        - other building or unit is already standing on any of needed fields
        - terrain_id on at least one needed fields is in not permitted range for the building
        - building has ancestor and some fields are not on ancestor
        If any ot these tests passes, building cannot be built, return false
      */

      field_ok = map.IsInMap(x, y);

      if (field_ok)
        for (seg = building->GetExistSegments().min; seg <= building->GetExistSegments().max; seg++) 
        {

          tid = map.segments[seg].surface[x][y].t_id;
          state = GetPlayer()->GetLocalMap()->map[seg][x][y].state;
          unit = map.segments[seg].surface[x][y].unit;

          field_ok = ((state != WLK_UNKNOWN_AREA) || auto_call);

          if (field_ok) 
          {
            if (state != WLK_WARFOG) visible_field = true;

        if (building->ancestor) {
              field_ok = unit && unit->GetPointerToItem() == building->ancestor 
                && !unit->TestState(US_IS_BEING_BUILT)
                && (!unit->TestItemType(IT_FACTORY) || ((TFACTORY_UNIT *)unit)->GetOrderSize() == 0)
                && building->buildable[seg].IsMember(tid - WLK_BUILDING_PLACED);
            }
            else 
            {
              if (build_map) 
                field_ok = building->buildable[seg].IsMember(tid) && (!unit || !unit->IsVisible());
              else 
                field_ok = building->buildable[seg].IsMember(tid) && !unit;
            }
          }

          if (!field_ok) break;
        }

      if (build_map) build_map[i][j] = field_ok;

      if (map_ok && !field_ok) map_ok = false;
    }
  }

  if (write_msg) 
  {
    if (!map_ok) 
      MessageText(false, const_cast<char*>("Can not build there."));
    else 
      if (!visible_field) 
        MessageText(false, const_cast<char*>("Can not build on invisible area."));
  }

  can_build = can_build && map_ok && visible_field;

  // for upgrade check  for material for reparison also
  if (can_build && building->ancestor && unit) {
    float delta_life = static_cast<TBUILDING_ITEM*>(unit->GetPointerToItem())->GetMaxLife() - unit->GetLife();

    if (delta_life > 0)
      for (i = 0; i < scheme.materials_count; i++)
        { //find out how much units of life is needed for complete repare
          if (building->materials[i] + delta_life * building->ancestor->mat_per_pt[i] > GetPlayer()->GetStoredMaterial(i)) 
          {
            if (write_msg) MessageText(false, const_cast<char*>("Not enough %s for repairing and building the %s."), scheme.materials[i]->name, building->name);
            can_build = false;
            break;
          }      
        }
  }

  // check if there are some holder units
  if ((building->ancestor) && (unit) && (!unit->GetHidedUnits().IsEmpty())) {
    if (write_msg) MessageText(false, const_cast<char*>("There are some hided units in the %s."), unit->GetPointerToItem()->name);

    if (build_map) can_build = false;
    else return false;
  }

  return can_build;
}


/**
 *  Finds out, if the unit given by parameter @p unit, can be repaired (built) or not.
 *  
 *  @param unit       Unit, which will be repaired.
 *  @param write_msg  Write message if error occured.
 *  @param auto_call  Mark if function was called automaticly (by computer player) or manualy (by real player).
 *
 *  @return           True, if the unit can be repaired, false otherwise.
 */
bool TWORKER_UNIT::CanBuildOrRepair(TBASIC_UNIT *unit, bool write_msg, bool auto_call)
{ 
  TWORKER_ITEM* it = static_cast<TWORKER_ITEM*>(pitem);
  bool ok;

  ok = (unit->GetPlayer() == myself);
  if (!ok && write_msg) MessageText(false, const_cast<char*>("Can not buid or repair enemy unit."));

  if (ok) {
    ok = !(unit->TestState(US_DYING) || unit->TestState(US_ZOMBIE) || unit->TestState(US_DELETE));
    if (!ok && write_msg) MessageText(false, const_cast<char*>("Can not build or repair destroyed %s."), unit->GetPointerToItem()->name);
  }

  //controll whaether worker can repair this type of building
  if (ok) {
    if (unit->TestState(US_IS_BEING_BUILT)) {
      ok = it->build_list.IsMember(static_cast<TBUILDING_ITEM *>(unit->GetPointerToItem()));
      if (!ok && write_msg) MessageText(false, const_cast<char*>("Can not build the %s."), unit->GetPointerToItem()->name);
    }
    else {
      ok = it->repair_list.IsMember(static_cast<TBASIC_ITEM *>(unit->GetPointerToItem())) && !unit->HasMaxLife();
      if (!ok && write_msg) MessageText(false, const_cast<char*>("Can not repair the %s."), unit->GetPointerToItem()->name);
    }
  }

  return ok;
}


/**
 *  Worker starts creating given building on given position.
 *
 *  @param building    Type of building, which will be build.
 *  @param build_here  Position of the center of the future building.
 *  @param auto_call   Mark if function was called automaticly (by computer player) or manualy (by real player).
 *
 *  @return Pointer to new unit, if the process was successful, NULL otherwise.
 */
TBUILDING_UNIT *TWORKER_UNIT::StartBuild(TBUILDING_ITEM *building, TPOSITION build_here, bool auto_call) 
{ 
  int i = 0;
  TBUILDING_UNIT * new_building = NULL;
  TPOSITION_3D building_position;
  double new_ts = glfwGetTime();

  building_position.SetPosition(build_here.x, build_here.y, building->GetExistSegments().min);

  process_mutex->Lock();
  if (CanBuild(building, build_here, NULL, true, auto_call))
  {

#if SOUND
    if (TestPlayer(myself)) {
      // play sound  
      myself->race->snd_placement.Play();
    }
#endif

    // if building has ancestor, change item type only
    if (building->ancestor)
    {
      player->RemoveUnitEnergyFood(building->ancestor->energy, building->ancestor->food);
      ((TBASIC_ITEM *)building->ancestor)->DecreaseActiveUnitCount();  // decrease count of units of this kind
      
      new_building = static_cast<TBUILDING_UNIT *>(map.segments[building->GetExistSegments().min].surface[build_here.x][build_here.y].unit);
      new_building->DeleteFromPlayerArray();   //ancestor is deleted from the list of acceptable buildings of player, who owns it
      new_building->ClearActions();
      new_building->SetView(false);
      new_building->SetPointerToItem(building);
      new_building->SetAggressivity(AM_IGNORE, true);

      // create burn animation
      new_building->RecreateBurnAnimation();

      // send info to not local playets that building is upgrading now
      TEVENT * hlp;

      hlp = pool_events->GetFromPool();
      hlp->SetEventProps(new_building->GetPlayerID(), new_building->GetUnitID(), false, new_ts, RQ_UPGRADE_UNIT, US_NONE, -1, building_position.x, building_position.y, building_position.segment, building->GetItemType(), 0, 0, building->index);
      SendNetEvent(hlp, all_players);
      pool_events->PutToPool(hlp);
    }
    else 
    {
      // add building to the list of buildings of the player, add a sign to all the used fields of map, that building is staying on them
      if (building->GetItemType() == IT_BUILDING)
        new_building = NEW TBUILDING_UNIT(GetPlayerID(),build_here.x - building->GetWidth() / 2, build_here.y - building->GetHeight() / 2, building, 0, true);
      else
        new_building = NEW TFACTORY_UNIT(GetPlayerID(),build_here.x - building->GetWidth() / 2, build_here.y - building->GetHeight() / 2, building, 0, true);

      player->IncPlayerUnitsCount(); //increase count of active units of player

      //if (!new_building) return false;
      if (!new_building) return NULL;

      new_building->SetAggressivity(AM_IGNORE, true);
      new_building->AddToMap(true, false);

      // send info to not local playets that new building is creating now
      TEVENT * hlp;

      hlp = pool_events->GetFromPool();
      hlp->SetEventProps(GetPlayerID(), GetUnitID(), false, new_ts, RQ_CREATE_UNIT, US_NONE, -1, new_building->GetPosition().x, new_building->GetPosition().y, new_building->GetPosition().segment, 0, building->GetItemType(), 0, building->index, new_building->GetUnitID());
      SendNetEvent(hlp, all_players);
      pool_events->PutToPool(hlp);
    }

    // prepay the construction by substraction of material
    for (i = 0 ; i < scheme.materials_count; i++)
      GetPlayer()->DecStoredMaterial(i, building->materials[i]);

    // upgrade with repairing ancestor unit
    if (building->ancestor) 
    {
      float delta_life = building->ancestor->GetMaxLife() - new_building->GetLife();

      for (i = 0 ; i < scheme.materials_count; i++)
        GetPlayer()->DecStoredMaterial(i, delta_life * building->ancestor->mat_per_pt[i]);
    }

    // only removing food or energy (adding negative)
    if (building->energy <= 0)
      player->AddUnitEnergyFood(building->energy, 0);
    if (building->food <= 0)
      player->AddUnitEnergyFood(0, building->food);

    // prepay materials for building
    if (building->ancestor)  
      new_building->SetPrepayed((float)(building->GetMaxLife() - new_building->GetLife()));    
    else
      new_building->SetPrepayed((float)dynamic_cast<TBUILDING_ITEM *>(new_building->GetPointerToItem())->GetMaxLife()); //we prepayed max_life of life for the created building

    new_building->PutState(US_IS_BEING_BUILT);  //set to building that it is being built
    new_building->ChangeAnimation();

    process_mutex->Unlock();
    return new_building;
  }

  process_mutex->Unlock();
  return NULL;
}


/**
 *  Adds 'life' to building, the goal is to build new building.
 *  Builds building. Function represents job of worker through the one game loop.
 * 
 */
void TWORKER_UNIT::Build()
{
  TBUILDING_UNIT *building = dynamic_cast<TBUILDING_UNIT*>(built_or_repaired_unit);     //built unit
  
  if (building)                                                           //if unit is building or descendant then add life and decrease prepayed amount of life
  {    
    building->DecPrepayed(1);       // descrease prepayed life of the unit about the possible value
    building->Heal(1);              // increase building life about added value
  }
}


/**
 *  Starts the repairing process.
 *  
 *  @param unit       Unit, which will be repaired.
 *  @param auto_call  Mark if function was called automaticly (by computer player) or manualy (by real player).
 *
 *  @return true, if the unit can be repaired, false otherwise.
 */
bool TWORKER_UNIT::StartRepair(TBASIC_UNIT *unit, bool auto_call) 
{ 
  process_mutex->Lock();

  if (CanBuildOrRepair(unit, true, auto_call))
  { 
  
    // send event to queue
    SendEvent(false, glfwGetTime(), US_START_REPAIR, -1, pos.x, pos.y, pos.segment, move_direction, 0, 0, unit->GetUnitID());
    process_mutex->Unlock();
  
    return true;
  }

  process_mutex->Unlock();
  return false;
}


/**
 *  The method adds 'life' to the repairable unit and represents job of worker
 *  through the one game loop.
 */
void TWORKER_UNIT::Repair(void)
{   
  int i;

  if (built_or_repaired_unit){
    built_or_repaired_unit->Heal(1);  //we have enough material -> add life to building
    TBASIC_ITEM *bitem = static_cast<TBASIC_ITEM *>(built_or_repaired_unit->GetPointerToItem());  //building item

    for (i = 0; i < scheme.materials_count; i++)               //reduce needed material
      GetPlayer()->DecStoredMaterial(i, bitem->mat_per_pt[i]);
  }
}


/**
 *  Find out, if the worker can mine in source given as parameter.
 *
 *  @param unit       Tested source.
 *  @param write_msg  True if messages should be written.
 *  @param auto_call  Mark if function was called automaticly (computer player) or manualny (by real player).
 *
 *  @return true, if the worker can mine in source, false otherwise.
 */
bool TWORKER_UNIT::CanMine(TSOURCE_UNIT *unit, bool write_msg, bool auto_call)
{
  if (unit->TestState(US_DYING) || unit->TestState(US_ZOMBIE) || unit->TestState(US_DELETE)) {
    if (write_msg) MessageText(false, const_cast<char*>("Can not mine in destroyed %s."), unit->GetPointerToItem()->name);
    return false;
  }

  TWORKER_ITEM *pit = static_cast<TWORKER_ITEM *>(pitem);
      
  T_BYTE off_mat = unit->GetOfferMaterial();
  
  if ((off_mat != (T_BYTE)-1) && (pit->GetAllowedMaterial(off_mat)) && !unit->IsEmpty()) {
    return true;
  }
  else {
    if (write_msg) MessageText(false, const_cast<char*>("Can not mine in the %s."), unit->GetPointerToItem()->name);
    return false;
  }
}

/**
 *  Starts the mining process.
 *  
 *  @param unit       Source, which will be mined.
 *  @param auto_call  Mark if function was called automaticly (by computer player) or manualy (by real player).
 *
 *  @return true, if the source can be mined, false otherwise.
 */
bool TWORKER_UNIT::StartMine(TSOURCE_UNIT *unit, bool auto_call)
{
  TPOSITION_3D building_pos_center;

  process_mutex->Lock();

  if (CanMine(unit, true, auto_call)) 
  {
    // send event to queue
    SendEvent(false, glfwGetTime(), US_START_MINE, -1, pos.x, pos.y, pos.segment, move_direction, unit->GetPlayerID(), 0, unit->GetUnitID());
    process_mutex->Unlock();

    return true;
  }

  process_mutex->Unlock();
  return false;
}

/**
 *  Find out, if worker can unload to the building given as parameter.
 *
 *  @param unit       Tested building.
 *  @param write_msg  True if messages should be written.
 *  @param auto_call  Mark if function was called automaticly (computer player) or manualny (by real player).
 *
 *  @return true, if the worker can unload material to the building, false otherwise.
 */
bool TWORKER_UNIT::CanUnload(TBUILDING_UNIT *unit, bool write_msg, bool auto_call)
{
  if (!material_amount) {
    if (write_msg) MessageText(false, const_cast<char*>("Nothing to unload."), pitem->name);
    return false;
  }

  if (unit->TestState(US_DYING) || unit->TestState(US_ZOMBIE) || unit->TestState(US_DELETE)) {
    if (write_msg) MessageText(false, const_cast<char*>("Can not unload into destroyed %s."), unit->GetPointerToItem()->name);
    return false;
  }

  if (unit->TestState(US_IS_BEING_BUILT)) {
    if (write_msg) MessageText(false, const_cast<char*>("Can not unload into the %s."), unit->GetPointerToItem()->name);
    return false;
  }

  if (!unit->GetAllowedMaterial(mined_material)) {
    if (write_msg) MessageText(false, const_cast<char*>("Can not unload %s into the %s."), scheme.materials[mined_material]->name, unit->GetPointerToItem()->name);
    return false;
  }
    
  return true;
}


/**
 *  Starts the unloading process.
 *  
 *  @param unit       Building where will be stored mined material.
 *  @param auto_call  Mark if function was called automaticly (by computer player) or manualy (by real player).
 *
 *  @return true, if building can accept material, false otherwise.
 */
bool TWORKER_UNIT::StartUnload(TBUILDING_UNIT *unit, bool auto_call)
{
  process_mutex->Lock();

  if (CanUnload(unit, true, auto_call))
  {
    // send event to queue
    SendEvent(false, glfwGetTime(), US_START_UNLOAD, -1, pos.x, pos.y, pos.segment, move_direction, 0, 0, unit->GetUnitID());
    process_mutex->Unlock();

    return true;
  }

  process_mutex->Unlock();
  return false;
}


/**
 *  The method gets pointer to nearest building for the worker from the source.
 *
 *  @return The method returns pointer to nearest building for worker item from
 *  the source which is available.
 *
 *  @param src  Pointer to source from which is looking for the nearest building.
 *  @param path_tool  Pointer to path tool with auxiliary data which is used 
 *  for computing nearest building.
 */
TBUILDING_UNIT* TWORKER_UNIT::GetNearestBuilding(TSOURCE_UNIT *src, TA_STAR_ALG *path_tool)
{
  //ak panacik este u svojho playera nema nastaveny nearest alebo ten je neplatny alebo k nemu neexistuje cesta
  //alebo od posledneho volania   
  int order = static_cast<TWORKER_ITEM*>(GetPointerToItem())->GetOrder(src->GetOfferMaterial());
  TNEAREST_BUILDINGS& nearest = src->GetPlayerArray()[GetPlayerID() - 1][order];  
  
  ////!!!!!!! v pripade ze sa jednotka pri delete sama vyberie z PlayerArray tak netreba 
  // testovat ci vobec existuje.

  if (!nearest.nearest_building || ///!!!! !map.IsUnitPresent(nearest.nearest_building) ||
     !GetPlayer()->material_array[mined_material].IsMember(nearest.nearest_building) ||
     nearest.nearest_building->TestState(US_DYING) || nearest.nearest_building->TestState(US_ZOMBIE) || nearest.nearest_building->TestState(US_DELETE))
    //!!!!!!!!!!!!!!mozno uplne neidentifikuje zaniknutu budovu!!!!!!!!!! pocitane pointre
  {
     nearest.nearest_building = NULL;
     nearest.path_time = WRK_MAX_PATH_TIME;
     FindNearestBuilding(GetPlayer()->material_array[mined_material], nearest, path_tool);     
  }

  if (nearest.new_buildings_count > 0) 
  {
     FindNearestBuilding(GetPlayer()->material_array[mined_material], nearest, path_tool);     
  }

  if (!nearest.nearest_building)
    MessageText(false, const_cast<char*>("No building accepts %s."), scheme.materials[mined_material]->name);

  return nearest.nearest_building;
}


/**
 *  The method finds nearest building which accepts the same material as 
 *  the source.
 *
 *  @param acceptable_building The list of buildings which accept the same
 *  material as the source offers.
 *  @param nearest Structure with information about nearest building 
 *  for the worker kind from the source.
 *  @param worker The worker which asks for the nearest acceptable building.
 *  @param path_tool  The pointer to path tool with auxiliary data which is 
 *  used for computing nearest building.
 */
void TWORKER_UNIT::FindNearestBuilding(TLIST<TBUILDING_UNIT>&acceptable_buildings, TNEAREST_BUILDINGS &nearest, TA_STAR_ALG *path_tool)
{
  TLIST<TBUILDING_UNIT>::TNODE<TBUILDING_UNIT> *p_actual = acceptable_buildings.GetFirst();  
  double actual_time = 0;          //how long does it take to the unit to walk from its position to the actual building
  TPATH_LIST *fastest_path = NULL;    //actually fastest path

  if (!p_actual)
  {
    nearest.nearest_building = NULL;
    nearest.new_buildings_count = 0;
    nearest.path_time = WRK_MAX_PATH_TIME;
    return;
  }

  if (nearest.new_buildings_count == 0)  //search through the whole list
  {
    for (; p_actual; p_actual = p_actual->GetNext())
    {
      //spocitaj, kolko budov sa bude prehladavat a posli toto info s kazdym vlaknom ako param.
      //prepni sa do stavu hladam cestu
      //spusti vlakno
      //v reakcii na prijatie vlakna bude nejaky counter, ktory bude pocitat, ktora cesta je najkratsia a kolko vlaken uz do pocitalo a ked to dojde na maximum, tak sa prepne do nejakeho ineho stavu
      //v reakcii na tento stav potom urob to, co normalne v reakcii na GetNearestBuilding (cize to bude chciet na vstupe este nejaky identifikator, aby bolo jasne, ktora cast vyslala hladanie najblizsej budovy      
      
      //try to find path to the actual building
      
      if (path_tool->PathFinder(p_actual->GetPitem()->GetPosition(), this, GetPlayer()->GetLocalMap(),&path, &goal))
      {
        //count the necessary time
        actual_time = path ? path->CountTime(this) : 0;
        
        //if the time necessary for the actual path is lower than previous minimum
        if (nearest.path_time > actual_time)
        {
          //store actual as nearest
          nearest.path_time = actual_time;
          if (fastest_path)
            delete fastest_path;
          fastest_path = path;
          path = NULL;
          nearest.nearest_building = p_actual->GetPitem();
        }
      }
    } //end for
  }
  else
  {
    for (int i = 0; (i < nearest.new_buildings_count) && p_actual; i++)
    {
      //try to find path to the actual building           
      if (path_tool->PathFinder(p_actual->GetPitem()->GetPosition(), this,GetPlayer()->GetLocalMap(),&path, &goal))
      {
        //count the necessary time
        actual_time = path ? path->CountTime(this) : 0;

        //if the time necessary for the actual path is lower than previous minimum
        if (nearest.path_time > actual_time)
        {
          //store actual as nearest
          nearest.path_time = actual_time;
          if (fastest_path)
            delete fastest_path;
          fastest_path = path;
          path = NULL;

          nearest.nearest_building = p_actual->GetPitem();
        }
      }
      //move to the next node in the list
      p_actual = p_actual->GetNext();
    }    
  }
  //save finding result
  nearest.new_buildings_count = 0;  
}


/**
 *  Method is called when unit is destroyed. Dealocate memory, play animation, etc.
 */
void TWORKER_UNIT::Dead(bool local)
{
  if (source && !acceptor && held)     // worker is mining now
    source->RemoveFromListOfUnits(this, LIST_WORKING);

  else if (acceptor && held)     // worker is unloading now
    acceptor->RemoveFromListOfUnits(this, LIST_WORKING);

  TFORCE_UNIT::Dead(local);             // call ancestors method
}


//=========================================================================
// END
//=========================================================================
// vim:ts=2:sw=2:et:

