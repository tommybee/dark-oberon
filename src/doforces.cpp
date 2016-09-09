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
 *  @file doforces.cpp
 *
 *  Working with force units.
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
#include <stdlib.h>

#include "dodata.h"
#include "dodraw.h"
#include "dologs.h"
#include "dounits.h"
#include "doplayers.h"
#include "dosimpletypes.h"
#include "dopool.h"
#include "dohost.h"
#include "doselection.h"


//=========================================================================
// class TFORCE_UNIT
//=========================================================================

/**
 *  Constructor.
 *
 *  @param uplayer      Player ID of unit owner.
 *  @param ux           X coordinate of unit.
 *  @param uy           Y coordinate of unit.
 *  @param uz           Z coordinate of unit.
 *  @param udirection   Direction of the unit.
 *  @param mi           Pointer to the unit kind.
 *  @param new_unit_id  if != 0, unit_id is set as new_unit_id, else new unique id is genetrated according to @param global_unit
 */
TFORCE_UNIT::TFORCE_UNIT(int uplayer, int ux, int uy, int uz, int udirection, TFORCE_ITEM *mi, int new_unit_id, bool global_unit)  // unit constructor
:TBASIC_UNIT(uplayer, ux, uy, uz, mi, new_unit_id, global_unit)
{
  // from TMAP_UNIT
  life = (float)mi->GetMaxLife();

  // others
  speed = rotation_speed = 0;
  look_direction = move_direction = udirection;
  goal.segment = uz; goal.x = ux; goal.y = uy;

  move_shift = move_time = try_move_shift = try_land_shift = heal_shift = 0.0;
  check_target = check_hider = 0;

  path = NULL;
//  ppath_info = NULL;

  hider = NULL;
  held = false;

  ChangeAnimation();
}


/**
 *  Destructor
 */
TFORCE_UNIT::~TFORCE_UNIT()
{
  // !!! zatial tu nie je nic [PPP]    
  if (path)
  {
    delete path;
    path = NULL;
  }
}



/**
 *  To "pevent" put event which is integrated to QUEUE according to @param new_ts (time stamp).
 *  If unit has som event in queue, check if it is endable state or not.
 *  if in queue is not endable state make necessary undo actions (returns one piece of material into source...)
 */
TEVENT* TFORCE_UNIT::SendEvent(bool n_priority, double n_time_stamp, int n_event, int n_request_id, T_SIMPLE n_simple1, T_SIMPLE n_simple2, T_SIMPLE n_simple3, T_SIMPLE n_simple4, T_SIMPLE n_simple5, T_SIMPLE n_simple6, int n_int1,int n_int2)
{
  int old_state = US_NONE, last_event = US_NONE;

  if (pevent)
  { // if unit has some event in queue
    if (pevent->GetLastEvent())
      last_event = pevent->GetLastEvent();
    else
      last_event = pevent->GetEvent();

    old_state = pevent->GetEvent();

    if (n_event == US_DYING)
    { // new state is US_DYING => stop all actions of unit
      if (pevent->GetTimeStamp() > n_time_stamp)
      { // if timestamp od pevent is greater than new DYING timestamp -> remove pevent frm queue and put new event
        queue_events->GetEvent(pevent);
        pevent->SetEventProps(GetPlayerID(), GetUnitID(), n_priority, n_time_stamp, n_event, US_NONE, n_request_id, n_simple1, n_simple2, n_simple3, n_simple4, n_simple5, n_simple6, n_int1,n_int2);
        queue_events->PutEvent(pevent); // put event to queue
      }
      else 
      { // rewrite pevent properties (without timetsamp)
        n_time_stamp = pevent->GetTimeStamp(); // get old time stamp
        pevent->SetEventProps(GetPlayerID(), GetUnitID(), n_priority, n_time_stamp, n_event, last_event, n_request_id, n_simple1, n_simple2, n_simple3, n_simple4, n_simple5, n_simple6, n_int1,n_int2);;
      }
    }
    else if ((old_state == US_DYING) || (old_state == US_ZOMBIE) || (old_state == US_DELETE))
    { // state in queue is final and it is not possible to change it
      return NULL;
    }
    else if ((old_state == US_HEALING) || (old_state == US_TRY_TO_MOVE))   
    { // it is possible to end state in queue -> make undo actions, put new time stamp
      // US_HEALING undo
      if (old_state == US_HEALING){
        // if message is received, increase heal_shift about value of how long was event in queue
        heal_shift += (pevent->GetTimeStamp() - glfwGetTime());
      }

      queue_events->GetEvent(pevent);
      pevent->SetEventProps(GetPlayerID(), GetUnitID(), n_priority, n_time_stamp, n_event, US_NONE, n_request_id, n_simple1, n_simple2, n_simple3, n_simple4, n_simple5, n_simple6, n_int1,n_int2);
      queue_events->PutEvent(pevent); // put event to queue

    }
    else
    { // state in queue is not endable -> time stamp is not changed     
      #if DEBUG_EVENTS
      Debug(LogMsg("ChOQ: P:%d U:%d E:%s RQ:%d X:%d Y:%d Z:%d R:%d TS:%f COUNT:%d", pevent->GetPlayerID(), pevent->GetUnitID(), EventToString(pevent->GetEvent()), pevent->GetRequestID(), pevent->simple1, pevent->simple2, pevent->simple3, pevent->simple4, pevent->GetTimeStamp(), queue_events->GetQueueLength()));
      #endif

      n_time_stamp = pevent->GetTimeStamp(); // get old time stamp
      pevent->SetEventProps(GetPlayerID(), GetUnitID(), n_priority, n_time_stamp, n_event, last_event, n_request_id, n_simple1, n_simple2, n_simple3, n_simple4, n_simple5, n_simple6, n_int1,n_int2);

      #if DEBUG_EVENTS
      Debug(LogMsg("ChNQ: P:%d U:%d E:%s RQ:%d X:%d Y:%d Z:%d R:%d TS:%f COUNT:%d", pevent->GetPlayerID(), pevent->GetUnitID(), EventToString(pevent->GetEvent()), pevent->GetRequestID(), pevent->simple1, pevent->simple2, pevent->simple3, pevent->simple4, pevent->GetTimeStamp(), queue_events->GetQueueLength()));
      #endif
    }
  }
  else
  { // unit does not have event in queue -> get new from pool and put it to queue according to new_ts
    pevent = pool_events->GetFromPool(); // get new (clear) event from pool
    pevent->SetEventProps(GetPlayerID(), GetUnitID(), n_priority, n_time_stamp, n_event, US_NONE, n_request_id, n_simple1, n_simple2, n_simple3, n_simple4, n_simple5, n_simple6, n_int1,n_int2);
    queue_events->PutEvent(pevent); // put event to queue
  }
  
  
  // send pevent through network if necessary
  n_event = pevent->GetEvent();
  
  if ((!player_array.IsRemote(GetPlayerID())) && 
     ((n_event == US_STAY) || (n_event == US_MOVE) || 
     (n_event == US_ANCHORING) || (n_event == US_LANDING) || (n_event == US_UNLANDING) ||
     (n_event == US_LEFT_ROTATING) || (n_event == US_RIGHT_ROTATING) ||
     (n_event == US_MINING) || (n_event == US_UNLOADING) ||
     (n_event == US_HIDING) || (n_event == US_EJECTING) ||
     (n_event == US_REPAIRING) || (n_event == US_CONSTRUCTING) ||
     (n_event == US_DYING) || (n_event == US_ZOMBIE) || (n_event == US_DELETE) ||
     (n_event == US_ATTACKING))
     )
  {
    SendNetEvent(pevent, all_players);
  }
  
  return pevent;
}



/**
 *  Method is calling when new event is get from queue. Returns true if unit do something.
 *
 *  @param Pointer to processed event.
 */
void TFORCE_UNIT::ProcessEvent(TEVENT * proc_event)
{
  int last_move_dir = move_direction;
  unsigned int last_state = state;
  TPOSITION_3D last_ps = GetPosition();

  TFORCE_ITEM * itm = static_cast<TFORCE_ITEM *>(pitem);
    
  int new_direction = LAY_UNDEFINED;
  unsigned int new_state = US_NONE;
  TPOSITION_3D new_ps;
  double new_time_stamp;
  bool new_priority;
  int new_int = 0;
  T_SIMPLE new_simple6 = 0;

  TMAP_UNIT * new_hider;
  TMAP_UNIT * new_target;

  int i, j, test_direction;
  unsigned int hardest = 0;
  TPOSITION_3D test_ps;
  bool change_position;
  bool all_moveable;

  double state_time = 0;

  // event US_WAIT_FOR_PATH only has to come - no computation
  if (proc_event->TestEvent(US_WAIT_FOR_PATH)) {
    if ((last_state == US_LANDING) || (last_state == US_ANCHORING) || (last_state == US_UNLANDING)) return;
  }

  // test if event is event (else it is request)
  if (proc_event->GetEvent() < RQ_FIRST) PutState(proc_event->GetEvent());
  
  TEVENT* path_event = NULL;
 
  /****************** Undo Actions (ONLY LOCAL UNITS) *************************/
  
  if (!proc_event->TestLastEvent(US_NONE)) 
  { // compute only local units

    if (proc_event->TestLastEvent(US_HIDING)) 
    {
      if (held) hider->RemoveFromListOfUnits(this, LIST_HIDING);

      // send info that unit end action
      TEVENT * hlp;

      hlp = pool_events->GetFromPool();

      hlp->SetEventProps(proc_event->GetPlayerID(), proc_event->GetUnitID(), false, proc_event->GetTimeStamp(), US_EJECTING, US_NONE, -1, pos.x, pos.y, pos.segment, move_direction, 0, 0, 0);

      SendNetEvent(hlp, all_players);
      pool_events->PutToPool(hlp);
      proc_event->SetTimeStamp(proc_event->GetTimeStamp() + TS_MIN_EVENTS_DIFF);
    }



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
  // US_STAY, US_ANCHORING, 
  // US_DYING, US_ZOMBIE
  // US_HEALING
  // US_START_HIDING, US_NEXT_HIDING
  // US_NEXT_ATTACK, US_ATTACKING
  if (
    proc_event->TestEvent(US_MOVE) || proc_event->TestEvent(US_LANDING) || proc_event->TestEvent(US_UNLANDING) ||
    proc_event->TestEvent(US_NEXT_STEP) || proc_event->TestEvent(US_TRY_TO_MOVE) ||
    proc_event->TestEvent(US_RIGHT_ROTATING) || proc_event->TestEvent(US_LEFT_ROTATING) ||
    proc_event->TestEvent(US_STAY) || proc_event->TestEvent(US_ANCHORING) || 
    proc_event->TestEvent(US_DYING) || proc_event->TestEvent(US_ZOMBIE) || proc_event->TestEvent(RQ_DYING) || proc_event->TestEvent(RQ_ZOMBIE) ||
    proc_event->TestEvent(US_HEALING) ||
    proc_event->TestEvent(US_START_HIDING) || proc_event->TestLastEvent(US_NEXT_HIDING) ||
    proc_event->TestEvent(US_NEXT_ATTACK) || proc_event->TestEvent(US_ATTACKING)
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

    SetHeld(false);
    SetHider(NULL);
    
    if (player_array.IsRemote(proc_event->GetPlayerID())) { // remote units must be added to map

      SetDirections(proc_event->simple4);
      SetPosition(proc_event->simple1, proc_event->simple2, proc_event->simple3);
      AddToMap(true, true);
    }
    else { // local units receive US_NEXT_STEP
      SendEvent(false, proc_event->GetTimeStamp() + TS_MIN_EVENTS_DIFF, US_NEXT_STEP, -1, pos.x, pos.y, pos.segment, move_direction);
    }
  }

  // US_START_ATTACK
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


  // RQ_SYNCHRONIZE_LIFE
  if (proc_event->TestEvent(RQ_SYNC_LIFE))
  {
    SetLife((float)proc_event->int1);
  }

  /****************** Compute new values - textures, times (ALL UNITS) ********************************/
  
  // US_MOVE, US_LANDING, US_UNLANDING
  if (proc_event->TestEvent(US_MOVE) || proc_event->TestEvent(US_LANDING) || proc_event->TestEvent(US_UNLANDING))
  {
    if (last_ps != pos) 
    {
      // set direction (of texture)
      if ((move_direction >= LAY_SOUTH) && (move_direction <= LAY_SOUTH_EAST))
      { 
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

  // US_HIDING
  if (proc_event->TestEvent(US_HIDING)) {
    // it is possible to get message from network which is not correct and hider is not set
    if (hider){
      if (hider->AddToListOfUnits(this, LIST_HIDING)) {
        DeleteFromMap(true);
        // this is relative position to holder
        SetPosition(GetPosition().x - (hider->GetPosition().x - GetUnitWidth()), GetPosition().y - (hider->GetPosition().y - GetUnitHeight()), GetPosition().segment);
      }
    }
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
        // correctly stop unit
        ClearActions();
        SendEvent(false, proc_event->GetTimeStamp(), US_NEXT_STEP, -1, pos.x, pos.y, pos.segment, move_direction);
      }
    }
    
    // US_NEXT_ATTACK
    if (proc_event->TestEvent(US_NEXT_ATTACK))
    {
      //if target will be delete stop attacking
      if (target->TestState(US_DYING) || target->TestState(US_ZOMBIE) || target->TestState(US_DELETE) 
          || (!GetPlayer()->GetLocalMap()->GetAreaVisibility(target->GetPosition(), target->GetUnitWidth(), target->GetUnitHeight()))
          || (!target->IsInMap())
         )
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
            {
              SendEvent(false, proc_event->GetTimeStamp(), US_NEXT_STEP, -1, GetPosition().x, GetPosition().y, GetPosition().segment,
                        GetMoveDirection(), 1);
            }            
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
              else {
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
            MessageText(false, const_cast<char*>("Attack to %s failed."), target->GetPointerToItem()->name);
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
      
      // test if unit is in state TRY_TO_MOVE too long      
      if (proc_event->TestEvent(US_TRY_TO_MOVE))
      {
        try_move_shift += UNI_TRY_TO_MOVE_SHIFT;  // add trying time
     
        if (try_move_shift >= UNI_TRY_TO_MOVE_LIMIT)  //unit tries to long => punish it 
        {
          // write messages if necessary
          if (hider) MessageText(false, const_cast<char*>("Can not get to %s."), hider->GetPointerToItem()->name);
      
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
          if ((last_state == US_ANCHORING) || (last_state == US_LANDING))
            new_state = US_UNLANDING;
          else 
            new_state = US_MOVE;
          change_position = true;
      
          if (proc_event->TestEvent(US_TRY_TO_MOVE))
          { //if can not move, change rotation first
            
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
                    ComputePath(new_ps, waiting_request_id, ET_NOTNEXTPOS_LANDING, last_state,state);
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
                ComputePath(test_ps, waiting_request_id, ET_NEXTPOS_NOTMOVABLE, last_state,state);              

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

              ComputePath(test_ps, waiting_request_id, ET_UNIT_AGAINST_UNIT, last_state,state);
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

                ComputePath(new_ps,waiting_request_id,ET_NOTPATH_LAND,last_state,state);  //narozdiel od workera brane ako 6tka
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
     
      
      if (change_position) path->IncreaseASteps();    // !!! skontrolovat, ci sa to vzdy korekne nastavi
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

      // unit is staying and has some hider
      else if (hider) {
        new_state = US_NEXT_HIDING;
        new_int = hider->GetUnitID();
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

      else return;
      
      // send event to queue
      SendEvent(false, new_time_stamp, new_state, -1, new_ps.x, new_ps.y, new_ps.segment, new_direction, 0, new_simple6, new_int);
    }

    // US_DYING
    if (proc_event->TestEvent(US_DYING)) 
    {
      state_time = 0;
    
      if (itm->tg_dying_id != -1) 
      {
        TGUI_TEXTURE * pgui_texture;

        pgui_texture = player->race->tex_table.GetTexture(itm->tg_dying_id, 0);
        state_time = pgui_texture->frame_time * pgui_texture->frames_count;
      }
      
      player->RemoveUnitEnergyFood(itm->energy, itm->food); // change players energy and food
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

      SendEvent(false, new_time_stamp, last_state, -1, proc_event->simple1, proc_event->simple2, proc_event->simple3, proc_event->simple4);
    }

    // US_START_HIDING
    if (proc_event->TestEvent(US_START_HIDING)) 
    {
      check_hider = 0;      

      path_event = SendEvent(false, proc_event->GetTimeStamp(),US_WAIT_FOR_PATH,0);
      waiting_request_id = path_event->GetRequestID();                    
      ComputePath(hider->GetPosition(),waiting_request_id,ET_FORCE_UNIT_HIDING);
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
        
        ComputePath(hider->GetPosition(), waiting_request_id,ET_NEXT_HIDING);         
      }
    }

    // RQ_PATH_FINDING
    if (proc_event->TestEvent(RQ_PATH_FINDING))
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
          case ET_ATTACK_TOO_FAR_AWAY:  //goal is far away, unit has to move towards the goal at first
          {
            T_SIMPLE new_state = proc_event->simple5;

            if (path)
              SendEvent(false, proc_event->GetTimeStamp(), US_NEXT_STEP, -1, GetPosition().x, GetPosition().y, GetPosition().segment,
                        GetLookDirection(), 1);
            else
            {
              MessageText(false, const_cast<char*>("Can not get to target %s."), target->GetPointerToItem()->name);
              SendEvent(false, proc_event->GetTimeStamp(), US_END_ATTACK, -1, pos.x, pos.y, pos.segment, move_direction);
            }
            PutState(new_state);
          }
          break;
          case ET_TARGET_MOVING:
          case ET_HIDER_MOVING:
          {
            SendEvent(false, proc_event->GetTimeStamp(), US_NEXT_STEP, -1, pos.x, pos.y, pos.segment, move_direction);
          }
          break;
          case ET_NOTNEXTPOS_LANDING:
          case ET_NEXTPOS_NOTMOVABLE:
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
          case ET_UNIT_AGAINST_UNIT:
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
          case ET_FORCE_UNIT_HIDING:
          {
            new_time_stamp = proc_event->GetTimeStamp();
            new_state = US_NEXT_STEP;
            
            // send event to queue
            SendEvent(false, new_time_stamp, new_state, -1, pos.x, pos.y, pos.segment, move_direction);
          }
          break;
          case ET_NEXT_HIDING:
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
 *  Method is called for periodically updating of look, moving and features.
 *
 *  @param time_shift Time shift from the last update.
 */
bool TFORCE_UNIT::UpdateGraphics(double time_shift)
{
  TMAP_UNIT::UpdateGraphics(time_shift);

  // if unit is moving
  if (TestState(US_MOVE) || TestState(US_LANDING) || TestState(US_UNLANDING) || TestState(US_LEFT_ROTATING) || TestState(US_RIGHT_ROTATING))
    move_shift += time_shift;
    
  // if unit is still moving compute new position
  if ((TestState(US_MOVE) || TestState(US_LANDING) || TestState(US_UNLANDING)) && move_shift < move_time)
  {
    // race length
    double s = (speed * (move_shift - move_time));

    // compute new real position
    SetRealPositionX(static_cast<float>(pos.x + dir_angle[move_direction][2] * s));
    SetRealPositionY(static_cast<float>(pos.y + dir_angle[move_direction][1] * s));
  }
  else {
    SetRealPositionX(pos.x);
    SetRealPositionY(pos.y);
  }

  return false;
}


/**
 *  Method draws line to unit's destination.
 */
void TFORCE_UNIT::DrawLine()
{
  if (
    TestState(US_MOVE) || TestState(US_NEXT_STEP) ||
    TestState(US_LEFT_ROTATING) || TestState(US_RIGHT_ROTATING) ||
    TestState(US_LANDING) || TestState(US_UNLANDING) ||
    TestState(US_TRY_TO_MOVE)
  ) {
    SetMapPosition(0, 0);
    glTranslated(0, DAT_MAPEL_DIAGONAL_SIZE * pitem->GetWidth() / 2, 0);

    glDisable(GL_TEXTURE_2D);
    glColor4f(0, 1, 0, 0.5);
    glBegin(GL_LINES); 
      glVertex2d(DAT_MAPEL_STRAIGHT_SIZE / 2 * (rpos_x) - DAT_MAPEL_STRAIGHT_SIZE / 2 * (rpos_y),
                 DAT_MAPEL_DIAGONAL_SIZE / 2 * (rpos_x) + DAT_MAPEL_DIAGONAL_SIZE / 2 * (rpos_y));
      glVertex2d(DAT_MAPEL_STRAIGHT_SIZE / 2 * (goal.x) - DAT_MAPEL_STRAIGHT_SIZE / 2 * (goal.y),
                 DAT_MAPEL_DIAGONAL_SIZE / 2 * (goal.x) + DAT_MAPEL_DIAGONAL_SIZE / 2 * (goal.y));
    glEnd();
    glEnable(GL_TEXTURE_2D);
  }
}


/**
 *  Sets state to US_DYING
 */
void TFORCE_UNIT::Dead(bool local)
{
  if (hider && held) {
    hider->RemoveFromListOfUnits(this, LIST_HIDING);
  }

  if (local) {
    if (group_id >= 0)
      selection->DeleteStoredUnit(group_id, this);

    SendEvent(false, glfwGetTime(), US_DYING, -1, pos.x, pos.y, pos.segment, move_direction);
  }
  else
    SendRequestLocal(false, glfwGetTime(), RQ_DYING, -1, pos.x, pos.y, pos.segment, move_direction);
}

/**
 *  Sets newly "visible" fields and unset fields, that aren't visible any more
 *  (while unit moves).
 *
 *  @param direction    direction of the move
 */
void TFORCE_UNIT::SetViewDirection(int direction)
{
  int x_start, y_start, x_stop, y_stop;
  int x_start_r, y_start_r, x_stop_r, y_stop_r;
  TPOSITION pos_new;
  TFORCE_ITEM* kind = static_cast<TFORCE_ITEM*>(GetPointerToItem());
  TGUN *p_gun = kind->GetArmament()->GetOffensive();
  int view = kind->view;
  int seg_num_min = kind->visible_segments[pos.segment].min;
  int seg_num_max = kind->visible_segments[pos.segment].max;
  int u_width = GetUnitWidth();
  int u_height = GetUnitHeight();
  int tex_id;
  T_SIMPLE map_w = map.width + MAP_AREA_SIZE + 1;
  TLOC_MAP *local_map = GetPlayer()->GetLocalMap();
  T_SIMPLE range_min = 0;
  T_SIMPLE range_max = 0;
  int aim_seg_num_min = 0;
  int aim_seg_num_max = 0;

  if (p_gun != NULL) 
  {
    range_min = p_gun->GetRange().min;
    range_max = p_gun->GetRange().max;
    aim_seg_num_min = p_gun->GetShotableSegments().min;
    aim_seg_num_max = p_gun->GetShotableSegments().max;
  }

  switch (direction) {
  case LAY_NORTH:
    pos_new.x = pos.x ;
    pos_new.y = pos.y + 1;

    x_start = pos.x - view;
    y_start = pos.y - view;

    x_stop = pos_new.x + (u_width - 1) + view;
    y_stop = pos_new.y + (u_height - 1) + view;

    x_start_r = pos.x - range_max;
    y_start_r = pos.y - range_max;

    x_stop_r = pos_new.x + (u_width - 1) + range_max;
    y_stop_r = pos_new.y + (u_height - 1) + range_max;
    break;

  case LAY_SOUTH:
    pos_new.x = pos.x;
    pos_new.y = pos.y - 1;

    x_start = pos_new.x - view;
    y_start = pos_new.y - view;

    x_stop = pos.x + (u_width - 1) + view;
    y_stop = pos.y + (u_height - 1) + view;

    x_start_r = pos_new.x - range_max;
    y_start_r = pos_new.y - range_max;

    x_stop_r = pos.x + (u_width - 1) + range_max;
    y_stop_r = pos.y + (u_height - 1) + range_max;
    break;

  case LAY_EAST:
    pos_new.x = pos.x + 1;
    pos_new.y = pos.y;

    x_start = pos.x - view;
    y_start = pos.y - view;

    x_stop = pos_new.x + (u_width - 1) + view;
    y_stop = pos_new.y + (u_height - 1) + view;

    x_start_r = pos.x - range_max;
    y_start_r = pos.y - range_max;

    x_stop_r = pos_new.x + (u_width - 1) + range_max;
    y_stop_r = pos_new.y + (u_height - 1) + range_max;
    break;

  case LAY_WEST:
    pos_new.x = pos.x - 1;
    pos_new.y = pos.y;

    x_start = pos_new.x - view;
    y_start = pos_new.y - view;

    x_stop = pos.x + (u_width - 1) + view;
    y_stop = pos.y + (u_height - 1) + view;

    x_start_r = pos_new.x - range_max;
    y_start_r = pos_new.y - range_max;

    x_stop_r = pos.x + (u_width - 1) + range_max;
    y_stop_r = pos.y + (u_height - 1) + range_max;
    break;

  case LAY_NORTH_EAST:
    pos_new.x =pos.x + 1;
    pos_new.y = pos.y + 1;

    x_start = pos.x - view;
    y_start = pos.y - view;

    x_stop = pos_new.x + (u_width - 1) + view;
    y_stop = pos_new.y + (u_height - 1) + view;

    x_start_r = pos.x - range_max;
    y_start_r = pos.y - range_max;

    x_stop_r = pos_new.x + (u_width - 1) + range_max;
    y_stop_r = pos_new.y + (u_height - 1) + range_max;
    break;

  case LAY_SOUTH_WEST:
    pos_new.x =pos.x - 1;
    pos_new.y = pos.y - 1;

    x_start = pos_new.x - view;
    y_start = pos_new.y - view;

    x_stop = pos.x + (u_width - 1) + view;
    y_stop = pos.y + (u_height - 1) + view;

    x_start_r = pos_new.x - range_max;
    y_start_r = pos_new.y - range_max;

    x_stop_r = pos.x + (u_width - 1) + range_max;
    y_stop_r = pos.y + (u_height - 1) + range_max;
    break;

  case LAY_NORTH_WEST:
    pos_new.x =pos.x - 1;
    pos_new.y = pos.y + 1;

    x_start = pos_new.x - view;
    y_start = pos.y - view;

    x_stop = pos.x + (u_width - 1) + view;
    y_stop = pos_new.y + (u_height - 1) + view;

    x_start_r = pos_new.x - range_max;
    y_start_r = pos.y - range_max;

    x_stop_r = pos.x + (u_width - 1) + range_max;
    y_stop_r = pos_new.y + (u_height - 1) + range_max;
    break;

  case LAY_SOUTH_EAST:
    pos_new.x =pos.x + 1;
    pos_new.y = pos.y - 1;

    x_start = pos.x - view;
    y_start = pos_new.y - view;

    x_stop = pos_new.x + (u_width - 1) + view;
    y_stop = pos.y + (u_height - 1) + view;
    
    x_start_r = pos.x - range_max;
    y_start_r = pos_new.y - range_max;

    x_stop_r = pos_new.x + (u_width - 1) + range_max;
    y_stop_r = pos.y + (u_height - 1) + range_max;
    break;

  default:
    /* We do not want to do anything when we get wrong direction, so we set
     * x_start and x_stop to 0. */
    Warning ("Wrong direction");
    x_start = x_stop = x_start_r = x_stop_r = 0;
    y_start = y_stop = y_start_r = y_stop_r = 0; /* This gets rid of a warning about variables
                             possibly not initialized (gcc). */
    break;
  }

  for (register int i = x_start; i <= x_stop ; i++) {
    for (register int j = y_start; j <= y_stop; j++) {
      if (i >= 0 && i < map.width + MAP_AREA_SIZE && j >= 0 && j < map.height + MAP_AREA_SIZE) {

        tex_id = (j + 1) * map_w * 4 + (i + 1) * 4;

        if (!(IsSeenByUnit(pos, i, j, u_width, u_height, view)) && (IsSeenByUnit(pos_new, i, j, u_width, u_height, view))) { //set visibility
          for (int seg_num = seg_num_max; seg_num >= seg_num_min; seg_num--) {  //segmetny nastavovat potom inak....

            //unknown area
            if (local_map->map[seg_num][i][j].state == WLK_UNKNOWN_AREA) {
              local_map->map[seg_num][i][j].state  = 1;
              if (player == myself) {
                 map.war_fog.tex[DRW_ALL_SEGMENTS][tex_id + 0] = map.war_fog.tex[seg_num][tex_id + 0] = config.pr_warfog_color[0];
                 map.war_fog.tex[DRW_ALL_SEGMENTS][tex_id + 1] = map.war_fog.tex[seg_num][tex_id + 1] = config.pr_warfog_color[1];
                 map.war_fog.tex[DRW_ALL_SEGMENTS][tex_id + 2] = map.war_fog.tex[seg_num][tex_id + 2] = config.pr_warfog_color[2];
                 map.war_fog.tex[DRW_ALL_SEGMENTS][tex_id + 3] = map.war_fog.tex[seg_num][tex_id + 3] = 0;
               }
            }
            else {
              local_map->map[seg_num][i][j].state++;

              if (player == myself && local_map->map[seg_num][i][j].state == 1) {
                map.war_fog.tex[seg_num][tex_id + 3] = 0;
              }
            }

            if (map.IsInMap(i, j))
            {
              local_map->map[seg_num][i][j].terrain_id = map.segments[seg_num].surface[i][j].t_id;
              if (p_gun != NULL) map.segments[seg_num].surface[i][j].GetWatchersList()->AddNode(this);
    
              //field has been hidden in warfog or unknown => updating infor.
              if (local_map->map[seg_num][i][j].state == (WLK_WARFOG + 1)) {

                //there is some unit
                if (map.segments[seg_num].surface[i][j].unit != NULL) {
                    local_map->map[seg_num][i][j].player_id = map.segments[seg_num].surface[i][j].unit->GetPlayerID();
                }
              }
            } // is in map
          } // for seg_num

          // update warfog for multi segment view
          if (player == myself) {
            int vis_seg = -1;
            int wf_seg = -1;
            for (int s = 2; s >= 0 && vis_seg < 0; s--)
            {
              if (local_map->map[s][i][j].state != WLK_UNKNOWN_AREA)
              {
                if (local_map->map[s][i][j].state > 0) vis_seg = s;
                else wf_seg = s;
              }                  
            }
            if (vis_seg >= 0)
              map.war_fog.tex[DRW_ALL_SEGMENTS][tex_id + 3] = map.war_fog.tex[vis_seg][tex_id + 3];
            else
              map.war_fog.tex[DRW_ALL_SEGMENTS][tex_id + 3] = map.war_fog.tex[wf_seg][tex_id + 3];
          }

        } // if IsSeenByUnit

        else if (!(this->IsSeenByUnit(pos_new, i, j, u_width, u_height, view)) && (this->IsSeenByUnit(pos, i, j, u_width, u_height, view))) {
          //set invisibility
          for (int seg_num = seg_num_max; seg_num >= seg_num_min; seg_num--) {   //podobne,segmenty nastavovat inak...
            if (local_map->map[seg_num][i][j].state > 0)
              local_map->map[seg_num][i][j].state -= 1;
            /*else {  OFIK: Nesynchronizovane is_in_map
              Critical("!!!!!!!!!!!!!!!!!!!!!");
            }*/
            if (map.IsInMap(i, j))
              if (p_gun != NULL) map.segments[seg_num].surface[i][j].GetWatchersList()->RemoveNode(this);

            if (!local_map->map[seg_num][i][j].state) {
              if (player == myself) {
                map.war_fog.tex[seg_num][tex_id + 3] = config.pr_warfog_color[3];
              }

              local_map->map[seg_num][i][j].player_id = WLK_EMPTY_FIELD;
            }
          } // for seg_num

          // update warfog for multi segment view
          if (player == myself) {
            int vis_seg = -1;
            int wf_seg = -1;
            for (int s = 2; s >= 0 && vis_seg < 0; s--)
            {
              if (local_map->map[s][i][j].state != WLK_UNKNOWN_AREA)
              {
                if (local_map->map[s][i][j].state > 0) vis_seg = s;
                else wf_seg = s;
              }                  
            }
            if (vis_seg >= 0)
              map.war_fog.tex[DRW_ALL_SEGMENTS][tex_id + 3] = map.war_fog.tex[vis_seg][tex_id + 3];
            else
              map.war_fog.tex[DRW_ALL_SEGMENTS][tex_id + 3] = map.war_fog.tex[wf_seg][tex_id + 3];
          }

        }  // if !IsSeenByUnit

      } // if
    } // for j
  } // for i

  if (p_gun != NULL) 
  {

    for (int i = x_start_r; i <= x_stop_r; i++)
      for (int j = y_start_r; j <= y_stop_r; j++)
      {
        if (map.IsInMap(i, j))
        {
          for (int k = aim_seg_num_max; k >= aim_seg_num_min; k--)
          {
            if (!(IsAimableByUnit(pos, i, j, u_width, u_height, range_min, range_max)) && (IsAimableByUnit(pos_new, i, j, u_width, u_height, range_min, range_max)))
            {
              map.segments[k].surface[i][j].GetAimersList()->AddNode(this);
            }
            else if (!(IsAimableByUnit(pos_new, i, j, u_width, u_height, range_min, range_max)) && (IsAimableByUnit(pos, i, j, u_width, u_height, range_min, range_max)))
            {
              map.segments[k].surface[i][j].GetAimersList()->RemoveNode(this);
            }
          }
        }
      }
  }
}


/**
 *  Selects reaction of the unit with the dependence of the clicked unit/building.
 *
 *  @param unit    Unit, that was clicked on.
 */
bool TFORCE_UNIT::SelectReaction(TMAP_UNIT *unit, TUNIT_ACTION action)
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
  case UA_REPAIR:
    ok = StartMoving(unit->GetPosition(), false);
    break;

  default: break;
  }

  return ok;
}


/**
 *  Clears all actions of unit.
 */
void TFORCE_UNIT::ClearActions()
{
  TBASIC_UNIT::ClearActions();

  if (path) {
    delete path;
    path = NULL;
  }

  if (held && held_list == LIST_HIDING)
    held_where->RemoveFromListOfUnits (this, held_list);
  ReleaseCountedPointer(hider);
}

/**
 *  Delete unit from all lists. This function is called when remote computer disconnect game.
 */
void TFORCE_UNIT::Disconnect()
{
  TFORCE_ITEM * itm = static_cast<TFORCE_ITEM *>(pitem);
  double state_time;
  
  // send US_DYING
  Dead(false);

  // send US_ZOMBIE  
  state_time = 0;
  if (itm->tg_dying_id != -1) 
  {
    TGUI_TEXTURE * pgui_texture;

    pgui_texture = player->race->tex_table.GetTexture(itm->tg_dying_id, 0);
    state_time = pgui_texture->frame_time * pgui_texture->frames_count;
  }
  SendRequestLocal(false, glfwGetTime() + state_time + TS_MIN_EVENTS_DIFF, RQ_ZOMBIE, -1, pos.x, pos.y, pos.segment, move_direction);

  // send US_DELETE
  if (itm->tg_zombie_id != -1) state_time = UNI_ZOMBIE_TIME;
  else state_time = 0;

  SendRequestLocal(false, glfwGetTime() + state_time + 2 * TS_MIN_EVENTS_DIFF, RQ_DELETE, -1, pos.x, pos.y, pos.segment, move_direction);
}

/**
 *  Send event US_TRY_TO_STAY for correct sop unit (land, stay).
 */
bool TFORCE_UNIT::StartStaying()
{
  if (TestState(US_STAY)) return false;

  // send event to queue
  process_mutex->Lock();
  ClearActions();
  SendEvent(false, glfwGetTime(), US_NEXT_STEP, -1, pos.x, pos.y, pos.segment, move_direction);
  process_mutex->Unlock();

  return true;
}

/**
 *  Find out if unit can hide to another unit given as parameter.
 *
 *  @param unit       Tested hider.
 *  @param write_msg  True if messages should be written.
 *  @param auto_call  Mark if function was called automaticly (computer player) or manualny (by real player).
 *
 *  @return true, if the unit can hide into hider, false otherwise.
 */
bool TFORCE_UNIT::CanHide(TMAP_UNIT *unit, bool write_msg, bool auto_call)
{
  if (unit->GetPlayer() != myself) {
    if (write_msg) MessageText(false, const_cast<char*>("Can not hide into enemy unit."));
    return false;
  }

  if (unit->TestState(US_DYING) || unit->TestState(US_ZOMBIE) || unit->TestState(US_DELETE)) {
    if (write_msg) MessageText(false, const_cast<char*>("Can not hide into destroyed %s."), unit->GetPointerToItem()->name);
    return false;
  }

  if (unit->TestState(US_IS_BEING_BUILT)) {
    if (write_msg) MessageText(false, const_cast<char*>("Can not hide into the %s."), unit->GetPointerToItem()->name);
    return false;
  }

  if (!unit->AcceptsHidedUnits()) {
    if (write_msg) MessageText(false, const_cast<char*>("%s does not accept units."), unit->GetPointerToItem()->name);
    return false;
  }

  if (!unit->CanAcceptUnit(this)) {
    if (write_msg) MessageText(false, const_cast<char*>("Can not hide into the %s."), unit->GetPointerToItem()->name);
    return false;
  }

  if (!((TMAP_ITEM *)unit->GetPointerToItem())->hide_list.IsMember((TFORCE_ITEM *)pitem)) {
    if (write_msg) MessageText(false, const_cast<char*>("Can not hide into the %s."), unit->GetPointerToItem()->name);
    return false;
  }
    
  return true;
}


/**
 *  Starts the moving process.
 *  
 *  @param target_pos Move target.
 *  @param auto_call  Mark if function was called automaticly (by computer player) or manualy (by real player).
 *
 *  @return true, if the unit can move to target position, false otherwise.
 */
bool TFORCE_UNIT::StartMoving(TPOSITION_3D target_pos, bool auto_call)
{
  bool ok = false;

  process_mutex->Lock();
  
  if (player->pathtools->PathFinder(target_pos, this, player->GetLocalMap(),&path, &goal) && this->path)  //OK
  {
    SendEvent(false, glfwGetTime(), US_NEXT_STEP, -1, pos.x, pos.y, pos.segment, move_direction);
    ok = true;
  }
  else MessageText(false, const_cast<char*>("%s: Can not move there."), pitem->name);
  
  process_mutex->Unlock();

  return ok;
}


/**
 *  Starts the hiding process.
 *  
 *  @param unit       Hider (target of hiding).
 *  @param auto_call  Mark if function was called automaticly (by computer player) or manualy (by real player).
 *
 *  @return true, if the unit can hide into hider, false otherwise.
 */
bool TFORCE_UNIT::StartHiding(TMAP_UNIT *unit, bool auto_call)
{
  bool ok = false;

  process_mutex->Lock();

  if (CanHide(unit, true, auto_call)) 
  {
    // send event to queue
    SendEvent(false, glfwGetTime(), US_START_HIDING, -1, pos.x, pos.y, pos.segment, move_direction, 0, 0, unit->GetUnitID());
    ok = true;
  }

  process_mutex->Unlock();

  return ok;
}


/** 
 *  The method start attack to the unit from parameter.
 *
 *  @param unit Pointer to attacked unit.
 *  @param auto_call Value is true if function was called automaticly (watch, aim lists).
 *  @return The method return true if start was successful.
 */
bool TFORCE_UNIT::StartAttacking(TMAP_UNIT *unit, bool auto_call)
{
  if (unit == this) return false;

  process_mutex->Lock();

  SetAutoAttack(auto_call);
  SendEvent(false, glfwGetTime(), US_START_ATTACK, -1, GetPosition().x, GetPosition().y, GetPosition().segment, GetMoveDirection(), 
            0, unit->GetPlayerID(), unit->GetUnitID());

  process_mutex->Unlock();

  return true;
}


/**
 *  Change unit animation according to its state.
 */
void TFORCE_UNIT::ChangeAnimation()
{
  int tg, tex = 0;
  TFORCE_ITEM *pit = static_cast<TFORCE_ITEM *>(pitem);
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
 *  Move unit one field in the direction from parameter. If direction is in the same segment and unit can't move in the direction
 *  from the parameter try move in the nearest direction. If unit can't move in this nearest direction try another nearest. 
 *  The largest deviatation is ninety degrees. If unit can't move into five nearest directions try to move to another segment.
 *  If direction is into another segment and unit can't move there try move in the actual segment in one of the other direction.
 *  The unit never moves to opposite direction.
 *
 *  @return The method returns true if some direction except opposite is possible. Otherwise returns false.
 *
 *  @param direct Direction in which unit try to move.
 */
bool TFORCE_UNIT::MoveInDirection(int direct)
{
  bool result = true;
  TPOSITION_3D new_pos;
  int counter = 0;
  int hardest_TID = LAY_UNAVAILABLE_POSITION;
  int dir_changes[8] = {0,1,-1,2,-2,3,-3,4};
  
  if ((direct >= 0) && (direct < LAY_DIRECTIONS_COUNT))     //if it is valid direction to move.
  {
    if (direct < LAY_UP)      //direction in the same segment
    {
      while ((counter < 6) && ((hardest_TID = IsAdjacentPositionAvailable(direct)) == -1))    //testing through nearest directions
      {
        direct += dir_changes[counter];   //next nearest direction
        counter++;
        if (direct < 0)       //checking direction
          direct += 8;
        else if (direct > 7)
          direct -= 8;
      }
      if (counter == 6)       //nearest directions aren't available => try upper segment
      {
        direct = LAY_UP;
        if ((hardest_TID = IsAdjacentPositionAvailable(direct)) == -1)
        {
          direct = LAY_DOWN;
          if ((hardest_TID = IsAdjacentPositionAvailable(direct)) != -1)    //lower segment
          {
            result = false;     //all possible directions are unavailable
          }
        }
      }
    }
    else                      //direction to another segment
    {
      if ((hardest_TID = IsAdjacentPositionAvailable(direct)) == -1)  //another segment isn't available => try move in this segment
      {
        direct = 3;
        while ((counter < 8) && ((hardest_TID = IsAdjacentPositionAvailable(direct)) == -1))    //testing through directions in same segment
        {
          direct += dir_changes[counter];   //next nearest direction
          counter++;
          //It is not necessary check direction because direct start at 3 and dir_changes are between -3 and 4 so nevertime be higher then seven or lower then zero.
        }
        if (counter == 8)     //all directions aren't available
          result = false;
      }
    }
  }
  else
    result = false;

  if (result)                 //some direction is available
  {
    TFORCE_ITEM* kind = static_cast<TFORCE_ITEM*>(GetPointerToItem());
    int i, j;

    SetDirections(direct);
    SetViewDirection(direct);
    TPOSITION_3D ps = pos + changes[direct];

    for (i = 0; i < player_array.GetCount(); i++)
      players[i]->UpdateLocalMap(pos, ps, this);     //updating local map for each of players

    for (i = pos.x; i < pos.x + GetUnitWidth(); i++)
      for (j = pos.y; j < pos.y + GetUnitHeight(); j++)
        map.segments[pos.segment].surface[i][j].unit = NULL;   //updating global map - leaving old position
    for (i = ps.x; i < ps.x + GetUnitWidth(); i++)
      for (j = ps.y; j < ps.y + GetUnitHeight(); j++)
      {
        map.segments[ps.segment].surface[i][j].unit = this;   //updating global map - taking up new position
        map.segments[ps.segment].surface[i][j].GetAimersList()->AttackEnemy(this, false);
        map.segments[ps.segment].surface[i][j].GetWatchersList()->AttackEnemy(this, true);
      }

    SetPosition(ps);         //seting new position

    move_shift = 0.0;

    speed = kind->max_speed[pos.segment]*(1.001f - scheme.terrain_props[pos.segment][map.segments[pos.segment].surface[pos.x][pos.y].t_id].difficulty / 1000.0f);
    //actual speed in dependence to difficulty of the terrain - terrain with difficulty 500 reduce speed to half

    if (direct == LAY_EAST || direct == LAY_NORTH || direct == LAY_WEST || direct == LAY_SOUTH)
      move_time = 1 / speed;
    else if (direct == LAY_SOUTH_EAST || direct == LAY_NORTH_EAST || direct == LAY_NORTH_WEST || direct == LAY_SOUTH_WEST)
      move_time = 1.4142 / speed;
    else
      move_time = 2 / speed;

    PutState(US_MOVE);
  }

  return result;
}


/**
 *  Tests whether is adjacent position in direction from parameter available to move. 
 *
 *  @return If it is available returns difficulty of the hardest terrain otherwise returns 0.
 *
 *  @param direct Direction of tested position.
 */
unsigned int TFORCE_UNIT::IsAdjacentPositionAvailable(const int direct)
{
  TPOSITION_3D new_pos;
  TMAP_SURFACE *field;
  TFORCE_ITEM *kind = static_cast<TFORCE_ITEM*>(GetPointerToItem());
  unsigned int result = 0;
  
  if ((direct >= 0) && (direct < LAY_DIRECTIONS_COUNT))     //if it is valid direction to move.
  {
    new_pos = pos + changes[direct];                        //calculate new position

    if (kind->GetExistSegments().IsMember(new_pos.segment))     //it is in existable segment
    {
      TTERRAIN_PROPS* terrains = scheme.terrain_props[new_pos.segment];     //terrain types in the segment
      for (int i = pos.x; i < pos.x + GetUnitWidth(); i++)
        for (int j = pos.y; j < pos.y + GetUnitHeight(); j++)
        {         //testing through new position
          field = &map.segments[new_pos.segment].surface[i][j];
          if ((!map.IsInMap(i, j)) || (!kind->moveable[new_pos.segment].IsMember(field->t_id)) 
              || ((field->unit != NULL)&&(field->unit != this))) //it isn't in the map, moveable terrain and empty field (or with me)
            return 0;

          result = (terrains[field->t_id].difficulty > result) ? terrains[field->t_id].difficulty : result;
                    //difficulty of the hardest terrain
        }
    }
  }

  return result;
}


/**
 *  Tests whether is selected position available to move. 
 *
 *  @return If it is available returns true otherwise returns false.
 *
 *  @param new_pos Tested position.
 */
bool TFORCE_UNIT::IsSelectedPositionAvailable(const TPOSITION_3D new_pos)
{
  TMAP_SURFACE *field;
  TFORCE_ITEM *kind = static_cast<TFORCE_ITEM*>(GetPointerToItem());

  if (kind->GetExistSegments().IsMember(new_pos.segment))     //it is in existable segment
  {
    for (int i = new_pos.x; i < new_pos.x + GetUnitWidth(); i++)
      for (int j = new_pos.y; j < new_pos.y + GetUnitHeight(); j++)
      {         //testing through new position
        field = &map.segments[new_pos.segment].surface[i][j];
        if ((!map.IsInMap(i, j)) || (!kind->moveable[new_pos.segment].IsMember(field->t_id) && !kind->landable[new_pos.segment].IsMember(field->t_id)) 
          || ((field->unit != NULL)&&(field->unit != this))) {//it isn't in the map, moveable terrain and empty field (or with me)
          #if DEBUG_EVENTS
            Debug(LogMsg("Tested position: %d, %d, %d. Restlt:false", new_pos.x, new_pos.y, new_pos.segment));
          #endif
          return false;
        }
      }
  }
  else {
    #if DEBUG_EVENTS
      Debug(LogMsg("Tested position: %d, %d, %d. Restlt:false", new_pos.x, new_pos.y, new_pos.segment));
    #endif
    return false;
  }

  #if DEBUG_EVENTS
    Debug(LogMsg("Tested position: %d, %d, %d. Restlt:true", new_pos.x, new_pos.y, new_pos.segment));
  #endif
  
  return true;
}


 /**
 * The unit try to land. If is impossible to land at the place, try positions in surrounding of 
 * a unit view size from the position of unit. 
 *
 * @return Returns true if unit can land. To in/out parameter position returns new position.
 */

bool TFORCE_UNIT::LandUnit(TPOSITION_3D * position)
{
  T_SIMPLE view = static_cast<TBASIC_ITEM*>(pitem)->view;
  TFORCE_ITEM *kind = static_cast<TFORCE_ITEM*>(GetPointerToItem());
  T_SIMPLE v;
  TPOSITION_3D ps;
  int x,y;
 
  
  for (ps.segment = kind->GetExistSegments().min; ps.segment <= kind->GetExistSegments().max; ps.segment++) {
    for (v = 0; v < view; v++) {
      //east  
      for (ps.x = pos.x + v, y = 0; y <= v; y++)
      {
        ps.y = pos.y + y;
        if (GetPlayer()->GetLocalMap()->IsLandablePosition(kind, ps)) {
          *position = ps;
          return true;
        }
        
        ps.y = pos.y - y;
        if (GetPlayer()->GetLocalMap()->IsLandablePosition(kind, ps)) {
          *position = ps;
          return true;
        }
      }
    
      //west
      for (ps.x = pos.x - v, y = 0; y <= v; y++)
      {
        ps.y = pos.y + y;
        if (GetPlayer()->GetLocalMap()->IsLandablePosition(kind, ps)) {
          *position = ps;
          return true;
        }

        ps.y = pos.y - y;
        if (GetPlayer()->GetLocalMap()->IsLandablePosition(kind, ps)) {
          *position = ps;
          return true;
        }
      }    

      //north
      for (x = 0, ps.y = pos.y + v; x <= v; x++)
      {
        ps.x = pos.x + x;
        if (GetPlayer()->GetLocalMap()->IsLandablePosition(kind, ps)) {
          *position = ps;
          return true;
        }

        ps.x = pos.x - x;
        if (GetPlayer()->GetLocalMap()->IsLandablePosition(kind, ps)) {
          *position = ps;
          return true;
        }
      }

      // south
      for  (x = v, ps.y = pos.y - v; x >= 0; x--)
      {
        ps.x = pos.x + x;
        if (GetPlayer()->GetLocalMap()->IsLandablePosition(kind, ps)) {
          *position = ps;
          return true;
        }

        ps.x = pos.x - x;
        if (GetPlayer()->GetLocalMap()->IsLandablePosition(kind, ps)) {
          *position = ps;
          return true;
        }
      }
    }
  }

  return false;
}



/**
 * Creates the copy of source_unit's path list of course shifted by shift, which is counted as 
 * (position of source_unit - position of actual unit).Also sets the goal of new unit as the 
 * shifted goal of the source_unit.
 */
TPATH_LIST * TFORCE_UNIT::CreatePathCopy(TPATH_LIST * leader_path,int shift_x, int shift_y, int shift_z)
{  
  //position of new goal must be in the map,so test it  
  TPATH_LIST* u_path;
  
  u_path = leader_path->CreateCopy(shift_x, shift_y, shift_z);  //creates copy of the source_unit->path
  SetGoal(leader_path->GetGoalPosition()); //sets the goal

  return u_path;
}


/** 
 *  Determines direction of the rotation.
 *
 *  @return The method returns one rotation state of the unit.
 *
 *  @param direction  Goal direction of the unit.
 */
int TFORCE_UNIT::DetermineRotationDirection(int direction)
{
  if (direction > move_direction)
  {
    if ((move_direction + LAY_HOR_DIRECTIONS_COUNT/2) > direction)
      return US_RIGHT_ROTATING;
    else 
      return US_LEFT_ROTATING;
  }
  else
  {
    if ((move_direction - LAY_HOR_DIRECTIONS_COUNT/2) < direction)
      return US_LEFT_ROTATING;
    else 
      return US_RIGHT_ROTATING;
  }
}

/** 
 *  The method finds place adjacent to holder unit.
 *  The finding position has to be in neighbourghood with the position of the holder and
 *  has to be empty and available for the unit. If the method is successful sets the unit
 *  position to the finded place. If the unit has to land when isn't moving, the method 
 *  lands the unit if it is possible at the position. If it is not try to move to adjacent
 *  positions. 
 *
 *  @param holder Pointer to the holder TMAP_UNIT or descendants.
 *  
 *  @return The method returns true if the finding was successful otherwise return false.
 */
bool TFORCE_UNIT::LeaveHolderUnit(TMAP_UNIT* holder)
{
  bool success = false;   // true if position finded
  TFORCE_ITEM* fitem = static_cast<TFORCE_ITEM*>(pitem);  // pointer to unit kind
  TPOSITION_3D holder_position = holder->GetPosition();   // position of the holder
  TMAP_ITEM * holder_item = dynamic_cast<TMAP_ITEM*>(holder->GetPointerToItem());
  TPOSITION_3D free_position;
  int s;
  int min_test_segment, max_test_segment; // min and max segment where is possible to leave

  if (!holder_item) return false;

  if (holder_item->IsMoveable())
    min_test_segment = max_test_segment = holder->GetPosition().segment;
  else {
    min_test_segment = MAX(holder_item->GetExistSegments().min - 1, 0);
    max_test_segment = MIN(holder_item->GetExistSegments().max + 1, 2);
  }
  
  success = false;  

  // test if unit can go out on default position (from which it goes inside)
  if ((pos.segment >= min_test_segment) && (pos.segment <= max_test_segment)) {
    free_position.x = holder->GetPosition().x - GetUnitWidth() + pos.x;
    free_position.y = holder->GetPosition().y - GetUnitHeight() + pos.y;
    free_position.segment = pos.segment;
  
    if ((GetPlayer()->GetLocalMap()->IsMoveablePosition(fitem, free_position) || 
        GetPlayer()->GetLocalMap()->IsLandablePosition(fitem, free_position)) &&
        (map.IsPositionFree(fitem->GetWidth(), fitem->GetHeight(), free_position)))
          success = true;
  }
  
  // searching non default position
  // searching around holder unit in last segment, from which unit came, was unsuccesfull - search through all the other segments
  // make intersection of holder unit segments and tforce_unit segments
  for (s = max_test_segment; !success && (s >= min_test_segment); s--)
  {
     if (!fitem->IsExistSegment(s)) continue;
     if (fitem->IsPosAroundHolderAvailable(this, holder, s, &free_position)) 
       success = true;
  }

  // in variable free_position is stored free position
  if (success) {
    SetPosition(free_position);
    look_direction = move_direction = holder->GetCenterPosition().GetDirection(GetCenterPosition());
    AddToMap(true, true);
    ChangeAnimation();
  }

  return success;
}


/**
 *  The method tests whether unit from parameter is neighbour of the unitself.
 *
 *  @return The method returns true if the @p unit is in the neighbourhood.
 *  Otherwise the method returns false.
 *
 *  @param unit Pointer to unit which is tested to neighbourhood.
 */
bool TFORCE_UNIT::IsNeighbour(TMAP_UNIT *unit) const
{
  TMAP_ITEM *mitem = static_cast<TMAP_ITEM*>(unit->GetPointerToItem()); //pointer to item of the tested unit

  TPOSITION_3D ne = unit->GetPosition() + TPOSITION_3D(unit->GetUnitWidth(), unit->GetUnitHeight(), 0);
  TPOSITION_3D sw = unit->GetPosition() - TPOSITION_3D(1, 1, 0);
  TPOSITION_3D my_ne = GetPosition() + TPOSITION_3D(GetUnitWidth(), GetUnitHeight(), 0) - TPOSITION_3D(1, 1, 0);
  TPOSITION_3D my_sw = GetPosition();

  if (!unit) return false;
  
  //if units are neighbours in first and second coordinates
  if ((((my_ne.x == sw.x) || (my_sw.x == ne.x)) && (ne.y >= my_sw.y) && (my_ne.y >= sw.y)) || 
      (((my_ne.y == sw.y) || (my_sw.y == ne.y)) && (ne.x >= my_sw.x) && (my_ne.x >= sw.x)))
  {
    //if tested unit is moveable unit then the segment has to be same
    if (mitem->IsMoveable()){
      if (GetPosition().segment == unit->GetPosition().segment) return true;
    }
    //unit isn't moveable, it is sufficient be in the exist segment of the force unit
    else {
      if (mitem->GetExistSegments().IsMember(GetPosition().segment)) return true;
    }
  }

  return false;
}


/**
 *  Method unhides unit and place it to map.
 */
bool TFORCE_UNIT::AddToMap(bool to_segment, bool set_view)
{
  if (!TMAP_UNIT::AddToMap(to_segment, set_view)) return false;

  int i;

  // updating local maps for each player
  for (i = 0; i < player_array.GetCount(); i++)
    players[i]->UpdateLocalMap(pos, (TFORCE_ITEM *)pitem, GetPlayerID());

  if (set_view) SetView(true);

  return true;
}


/**
 *  Method hides unit from map.
 */
void TFORCE_UNIT::DeleteFromMap(bool from_segment)
{
  TMAP_UNIT::DeleteFromMap(from_segment);

  int i;

  // updating local maps for each player
  for (i = 0; i < player_array.GetCount(); i++)
    players[i]->UpdateLocalMap(pos, (TFORCE_ITEM *)pitem, WLK_EMPTY_FIELD);

  if (has_view) SetView(false);
}

/*
*  Method prepares structure for thread, which is taken from pool.
*/
void TFORCE_UNIT::ComputePath(TPOSITION_3D goal,int request_id,int event_type,T_SIMPLE e_simple1, T_SIMPLE e_simple2)
{
  TPATH_INFO *ppath_info;

  ppath_info = pool_path_info->GetFromPool();

  ppath_info->goal = goal;
  ppath_info->loc_map = player->GetLocalMap();
  ppath_info->path = NULL;   //nahrada za predchadzajuci riadok
  ppath_info->real_goal = goal;
  ppath_info->succ = false;
  ppath_info->e_simple1 =e_simple1;
  ppath_info->e_simple2 = e_simple2;
  ppath_info->event_type = event_type;
  ppath_info->request_id = request_id;  

  glfwLockMutex(delete_mutex);
  ppath_info->unit = (TFORCE_UNIT *)AcquirePointer();
  glfwUnlockMutex(delete_mutex);

  if (!ppath_info->unit) {
    pool_path_info->PutToPool(ppath_info);
    return;
  }

  threadpool_astar->AddRequest(ppath_info, &TA_STAR_ALG::ComputePath);
}


/*
* Method prepades structure for thread, which will find the nearest building to source unit.
*/
void TFORCE_UNIT::SearchForNearestBuilding(TSOURCE_UNIT *source, int request_id, int event_type,T_SIMPLE simple1,T_SIMPLE simple2)
{
  TNEAREST_INFO *pnearest_info;

  pnearest_info = pool_nearest_info->GetFromPool();
  pnearest_info->event_type = event_type;
  pnearest_info->unit = this;
  pnearest_info->nearest = NULL;
  pnearest_info->request_id = request_id;
  pnearest_info->src_unit = source;  
  pnearest_info->simple1 = simple1;
  pnearest_info->simple2 = simple2;
  
  threadpool_nearest->AddRequest(pnearest_info, &TA_STAR_ALG::SearchForNearestBuilding);
}

/** 
 *  The method test whether unit is present in the segment interval of parameter.
 *  The unit is present in the interval if at least one part of the unit is in.
 *  The borders are included into the interval. 
 *
 *  @param bottom The down border of the interval.
 *  @param top    The up border of the interval.
 *  @return The method returns true if unit is present in the interval.
 */
bool TFORCE_UNIT::ExistInSegment(int bottom, int top) const
{
  if ((GetPosition().segment >= bottom) && (GetPosition().segment <= top))
    return true;
  else
    return false;
}

/**
 *  Counts distance of unit from the area given as parameters.Uses modified Pathfinder.
 *  @param area_position    position of down left corner of area
 *  @param area_width       width of area
 *  @param area_height      height of area
 *  @param max_cnt          maximal count of steps given from open to close set
 *  @return                 returns distance of the unit from the area
*/
int TFORCE_UNIT::CountPathDistance(TPOSITION_3D area_pos, int area_width, int area_height,int max_cnt)
{   
  TPOSITION_3D goal;
  TPATH_LIST *path = NULL;
  long area_dist =-1;

  this->GetPlayer()->pathtools->PathFinder(area_pos,this,this->GetPlayer()->GetLocalMap(),&path, &goal,&area_dist,max_cnt,area_width,area_height);  

  return area_dist;
}

//=========================================================================
// END
//=========================================================================
// vim:ts=2:sw=2:et:

