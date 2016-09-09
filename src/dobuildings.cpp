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
 *  @file dobuildings.cpp
 *
 *  Working with building units.
 *
 *  @author Peter Knut
 *  @author Valeria Sventova
 *  @author Jiri Krejsa
 *
 *  @date 2002, 2003, 2004
 */

#include <math.h>

#include "dodata.h"
#include "dodraw.h"
#include "dologs.h"
#include "doselection.h"
#include "dosimpletypes.h"
#include "dounits.h"
#include "dopool.h"
#include "dohost.h"


//=========================================================================
// Variables
//=========================================================================


//=========================================================================
// Methods definitions of the class TBUILDING_UNIT
//=========================================================================


/**
 *  Building constructor.
 *
 *  @param uplayer      Player ID of unit owner.
 *  @param ux           X coordinate of unit.
 *  @param uy           Y coordinate of unit.
 *  @param mi           Pointer to the unit kind.
 *  @param new_unit_id  If new_unit_id != 0, unit_id = new_unit_id, else new unique unit_id is generated according to @param global_unit
 */
TBUILDING_UNIT::TBUILDING_UNIT(int uplayer, int ux, int uy, TBUILDING_ITEM *mi, int new_unit_id, bool global_unit)
: TBASIC_UNIT(uplayer, ux, uy, mi->GetExistSegments().min, mi, new_unit_id, global_unit)
{
  prepayed = max_prepayed = 0;
  progress = 0;

  if (!burn_animation && player->race->tg_burning_id > -1) {
    ((TMAP_ITEM *)pitem)->tg_burning_id = player->race->tg_burning_id;
    burn_animation = NEW TGUI_ANIMATION(player->race->tex_table.GetTexture(player->race->tg_burning_id, 0));
    burn_animation->SetRandomFrame();
    burn_animation->Hide();
  }
}


/**
 *  Constructor. Copy values from copy_unit.
 */
TBUILDING_UNIT::TBUILDING_UNIT(TBUILDING_UNIT *copy_unit, int new_unit_id, bool global_unit)
:TBASIC_UNIT(copy_unit->GetPlayerID(), copy_unit->GetPosition().x, copy_unit->GetPosition().y, copy_unit->GetPosition().segment, (TBUILDING_ITEM *)copy_unit->GetPointerToItem(), new_unit_id, global_unit)
{
  TBUILDING_ITEM *pit = static_cast<TBUILDING_ITEM *>(copy_unit->GetPointerToItem());

  pitem = pit;
  life = copy_unit->GetLife();

  prepayed = copy_unit->GetPrepayed();
  progress = copy_unit->GetProgress();
  lieing_down = copy_unit->IsLieingDown();

  PutState(copy_unit->GetState());
  PutState(US_GHOST);

  ghost_owner = copy_unit->AcquirePointer();

  animation->SetTexItem(copy_unit->GetAnimation()->GetTexItem());
}


/**
 *  Destructor
 */
TBUILDING_UNIT::~TBUILDING_UNIT()
{
  ;
}


void TBUILDING_UNIT::RecreateBurnAnimation(void)
{
  TBUILDING_ITEM *pit = (TBUILDING_ITEM *)pitem;

  if (burn_animation) {
    delete burn_animation;
    burn_animation = NULL;
  }

  if (pit->tg_burning_id == -1)
    pit->tg_burning_id = player->race->tg_burning_id;

  if (pit->tg_burning_id > -1) {
    burn_animation = NEW TGUI_ANIMATION(player->race->tex_table.GetTexture(pit->tg_burning_id, 0));
    burn_animation->SetRandomFrame();
    burn_animation->Hide();
  }
}


/** Sets private variable progress and send messagethrough network to remote players. */
void TBUILDING_UNIT::SetProgress(T_BYTE pr)
{
  if (pr > 100) progress = 100;
  else progress = pr; 

  // only not remote (local) units send message about change of progress
  if (!player_array.IsRemote(this->GetPlayerID()) && TestState(US_IS_BEING_BUILT)) {
    TEVENT * hlp;
    
    // send info about buildings's actual progress to all not local players
    hlp = pool_events->GetFromPool();
    hlp->SetEventProps(GetPlayerID(), GetUnitID(), false, glfwGetTime(), RQ_SYNC_PROGRESS, US_NONE, -1, 0, 0, 0, 0, 0, 0, progress);
    SendNetEvent(hlp, all_players);
    pool_events->PutToPool(hlp);
  }
}



/**
 *  Clears all actions of unit.
 */
void TBUILDING_UNIT::ClearActions()
{
  TBASIC_UNIT::ClearActions();
}


/**
 *  Change unit animation according to its state.
 */
void TBUILDING_UNIT::ChangeAnimation()
{
  int tg = -1;  // texture group
  int id = 0;   // texture id in group
  int cn;       // count of textures in group

  bool changed_build = false;
  bool play = true;

  double time = -1;

  TBUILDING_ITEM *pit = static_cast<TBUILDING_ITEM *>(pitem);

  // building
  if (TestState(US_IS_BEING_BUILT)) {
    if (life > 0) {
      tg = pit->tg_build_id;
      if (tg < 0) {
        tg = pit->tg_stay_id;
        id = 0;
      }
      else {
        cn = player->race->tex_table.groups[tg].count;
        id = GetProgress() / (100  / (cn + 1));

        if (id >= cn) {
          tg = pit->tg_stay_id;
          id = 0;
        }

        changed_build = (GetProgress() > 0);
      }
    }
    else {
      tg = pit->tg_stay_id;
      id = 0;
      play = false;
    }
  }

  // dying
  else if (TestState(US_DYING)) {
    tg = pit->tg_dying_id;
  }

  // zombie
  else if (TestState(US_ZOMBIE)) {
    tg = pit->tg_zombie_id;
    time = UNI_ZOMBIE_TIME;
  }

  // staying and attacking
  else {
    tg = pit->tg_stay_id;
    cn = player->race->tex_table.groups[tg].count;

    // set texture in group according to building life
    if (cn < 2) id = 0;
    else {
      int ml = pit->GetMaxLife();
      id = int((ml - life) / (ml  / cn));
      if (id >= cn) id--;
    }
  }


  if (tg >= 0) {
    if (animation->SetTexItem(player->race->tex_table.GetTexture(tg, id))) {

#if SOUND
      // play constructing sound
      if (changed_build && visible) PlaySound(&player->race->snd_construction, 2);
#endif

    }
    if (play) animation->Play();
    else animation->Stop();

    if (time >= 0) animation->SetAnimTime(time);
  }
  else animation->SetTexItem(NULL);
}


/** 
 *  The method start attack to the unit from parameter.
 *
 *  @param unit Pointer to attacked unit.
 *  @param auto_call Value is true if function was called automaticly (watch, aim lists).
 *  @return The method return true if start was successful.
 */
bool TBUILDING_UNIT::StartAttacking(TMAP_UNIT *unit, bool auto_call)
{
  process_mutex->Lock();

  SetAutoAttack(auto_call);
  SendEvent(false, glfwGetTime(), US_START_ATTACK, -1, GetPosition().x, GetPosition().y, GetPosition().segment, 0, 0, unit->GetPlayerID(), unit->GetUnitID());

  process_mutex->Unlock();

  return true;
}

/**
 *  Delete unit from all lists. This function is called when remote player disconnect game.
 */
void TBUILDING_UNIT::Disconnect()
{
  TBUILDING_ITEM * itm = static_cast<TBUILDING_ITEM *>(pitem);
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
  SendRequestLocal(false, glfwGetTime() + state_time + TS_MIN_EVENTS_DIFF, RQ_ZOMBIE, -1, pos.x, pos.y, pos.segment);

  // send US_DELETE
  if (itm->tg_zombie_id != -1) state_time = UNI_ZOMBIE_TIME;
  else state_time = 0;
  SendRequestLocal(false, glfwGetTime() + state_time + 2 * TS_MIN_EVENTS_DIFF, RQ_DELETE, -1, pos.x, pos.y, pos.segment);
}

/**
 *  To "pevent" put event which is integrated to QUEUE according to @param new_ts (time stamp).
 *  If unit has som event in queue, check if it is endable state or not.
 *  if in queue is not endable state make necessary undo actions (returns one piece of material into source...)
 */
TEVENT* TBUILDING_UNIT::SendEvent(bool n_priority, double n_time_stamp, int n_event, int n_request_id, T_SIMPLE n_simple1, T_SIMPLE n_simple2, T_SIMPLE n_simple3, T_SIMPLE n_simple4, T_SIMPLE n_simple5, T_SIMPLE n_simple6, int n_int1,int n_int2)
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
      { // if timestamp od pevent is greater than new DYING timestamp -> remove pevent from queue and put new event
        queue_events->GetEvent(pevent);
        pevent->SetEventProps(GetPlayerID(), GetUnitID(), n_priority, n_time_stamp, n_event, last_event, n_request_id, n_simple1, n_simple2, n_simple3, n_simple4, n_simple5, n_simple6, n_int1,n_int2);
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
    else if ((old_state == US_NEXT_ATTACK) || (old_state == US_START_ATTACK))
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
    else
    { // it is possible to end state in queue -> make undo actions, put new time stamp
      queue_events->GetEvent(pevent);
      pevent->SetEventProps(GetPlayerID(), GetUnitID(), n_priority, n_time_stamp, n_event, last_event, n_request_id, n_simple1, n_simple2, n_simple3, n_simple4, n_simple5, n_simple6, n_int1,n_int2);
      queue_events->PutEvent(pevent); // put event to queue
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
     ((n_event == US_STAY) ||
     (n_event == US_DYING) || (n_event == US_ZOMBIE) || (n_event == US_DELETE) ||
     (n_event == US_ATTACKING))
     )
  {
    SendNetEvent(pevent, all_players);
  }

  return pevent;
}


/**
 *  Method is calling when new event is get from queue. Returns true if unit does something.
 *
 *  @param Pointer to processed event.
 */
void TBUILDING_UNIT::ProcessEvent(TEVENT * proc_event)
{
  unsigned int last_state = state;
  double state_time = 0;
  double new_time_stamp;
  unsigned int new_state = US_NONE;

  TBUILDING_ITEM * itm = static_cast<TBUILDING_ITEM *>(pitem);
  
  /****************** Put state of event ********************************/
       
  TMAP_UNIT::ProcessEvent(proc_event);

  if (proc_event->TestEvent(US_DELETE) || proc_event->TestEvent(RQ_DELETE)) return;
    
  /****************** Undo Actions (ONLY LOCAL UNITS) *************************/
  
  if (!player_array.IsRemote(proc_event->GetPlayerID())){ // compute only local units
    ;
  }
  
  /****************** Synchronisation and setup (ALL UNITS) *************************/

  // RQ_SYNC_PROGRESS
  if (proc_event->TestEvent(RQ_SYNC_PROGRESS))
  {
    SetProgress(proc_event->int1);
  }

  /****************** Compute new values - textures, times (ALL UNITS) ********************************/
 
  // RQ_UPGRADE_UNIT
  if (proc_event->TestEvent(RQ_UPGRADE_UNIT)) {
    TBUILDING_ITEM * new_itm = static_cast<TBUILDING_ITEM *>(player->race->buildings[proc_event->int1]);
    
    ClearActions();
    SetPointerToItem(new_itm);

    PutState(US_IS_BEING_BUILT);  //set to building that it is being built
    ChangeAnimation();
  }
  
  
  // US_DYING, RQ_DYING
  if (proc_event->TestEvent(US_DYING) || proc_event->TestEvent(RQ_DYING)) {
    
    // kill all working units
    if (!working_units.IsEmpty()) {
      TLIST<TWORKER_UNIT>::TNODE<TWORKER_UNIT> *node;
      TWORKER_UNIT *unit;

      for (node = working_units.GetFirst(); node; node = node->GetNext()) {
        unit = node->GetPitem();

        // test if unit is local (is not remote)
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
    DeleteFromMap(false);
    lieing_down = true;
  }
  
  // set animation
  if (last_state != state) ChangeAnimation();
    
  
  /****************** Compute next event (ONLY LOCAL UNITS) *******************************/
  
  if (!player_array.IsRemote(proc_event->GetPlayerID())) { // compute only not remote (local) units

    // US_STAY
    if (proc_event->TestEvent(US_STAY) && last_state == US_IS_BEING_BUILT) {
      ClearActions();
      SetView(true);
    }

    // US_DYING
    if (proc_event->TestEvent(US_DYING)) {
      state_time = 0;
    
      if (itm->tg_dying_id != -1) {
        TGUI_TEXTURE * pgui_texture;

        pgui_texture = player->race->tex_table.GetTexture(itm->tg_dying_id, 0);
        state_time = pgui_texture->frame_time * pgui_texture->frames_count;
      }
      
      player->RemoveUnitEnergyFood(itm->energy, itm->food); // change energy and food of player
      player->DecPlayerUnitsCount(); // decrease count of active players units
      itm->DecreaseActiveUnitCount();  // decrease count of units of this kind
      ClearActions();

      new_time_stamp = proc_event->GetTimeStamp() + state_time + TS_MIN_EVENTS_DIFF;
      new_state = US_ZOMBIE;

      // send event to queue
      SendEvent(false, new_time_stamp, new_state, -1, proc_event->simple1, proc_event->simple2, proc_event->simple3, proc_event->simple4);
    }

    // US_ZOMBIE
    if (proc_event->TestEvent(US_ZOMBIE)) {
      if (itm->tg_zombie_id != -1) state_time = UNI_ZOMBIE_TIME;
      else state_time = 0;

      new_time_stamp = proc_event->GetTimeStamp() + state_time + TS_MIN_EVENTS_DIFF;
      new_state = US_DELETE;
      
      // send event to queue
      SendEvent(false, new_time_stamp, new_state, -1, proc_event->simple1, proc_event->simple2, proc_event->simple3, proc_event->simple4);
    }

    // US_STAY
    if (proc_event->TestEvent(US_STAY)) {
      TMAP_UNIT * standing_target;

      standing_target = CheckNeighbourhood(last_target);
      
      if ((standing_target) && (GetPlayer()->GetLocalMap()->GetAreaVisibility(standing_target->GetPosition(), standing_target->GetUnitWidth(), standing_target->GetUnitHeight()))) {
        SetAutoAttack(true);
        
        // send event to queue
        SendEvent(false, proc_event->GetTimeStamp() + TS_MIN_EVENTS_DIFF, US_START_ATTACK, -1, pos.x, pos.y, pos.segment, 0, 0, standing_target->GetPlayerID(), standing_target->GetUnitID());
      }
    }
  }
}


/**
 *  Method is called for periodically updating of look and features.
 *
 *  @param time_shift  Time shift from the last update.
 */
bool TBUILDING_UNIT::UpdateGraphics(double time_shift)
{
  bool last_visible = visible; 

  TMAP_UNIT::UpdateGraphics(time_shift);

  if (!TestState(US_GHOST)) {
    if (((TBUILDING_ITEM *)pitem)->min_energy > player->GetPercentEnergy())
      ShowNeedEnergy();
    else ResetSignAnimation();
  }

  // ghots
  if (
    (TestState(US_GHOST) && visible) ||
    (player != myself && !visible && last_visible)
  ) return true;

  return false;
}


void TBUILDING_UNIT::TestVisibility()
{
  TBUILDING_ITEM *pit = static_cast<TBUILDING_ITEM*>(pitem);

  if (player == myself || show_all || myself_units)
    visible = true;
  else
    visible = myself->GetLocalMap()->GetAreaVisibility(pos.x, pos.y, pit->GetExistSegments().min, pit->GetExistSegments().max, GetUnitWidth(), GetUnitHeight());
}


/**
 *  Method creates ghost unit which is drawn under warfog.
 */
void TBUILDING_UNIT::CreateGhost()
{
  if (TestState(US_DELETE)) return;

  TBUILDING_UNIT *b = new TBUILDING_UNIT(this, 0, false);
  b->AddToMap(true, false);
}


/**
 *  Adds building into segments lists. Unit will be drawn.
 */
void TBUILDING_UNIT::AddToSegments()
{
  TBUILDING_ITEM *pit = static_cast<TBUILDING_ITEM *>(pitem);

  for ( int k = pit->GetExistSegments().min; k <= pit->GetExistSegments().max; k++)
    map.segment_units[k]->AddUnit(this);
}


/**
 *  Removes building from segments lists. Unit will be not drawn.
 */
void TBUILDING_UNIT::DeleteFromSegments()
{
  TBUILDING_ITEM *pit = static_cast<TBUILDING_ITEM *>(pitem);

  for (int k = pit->GetExistSegments().min; k <= pit->GetExistSegments().max; k++)
    map.segment_units[k]->DeleteUnit(this);
}


/** 
 *  Function adds building to the map and draw list.
 */
bool TBUILDING_UNIT::AddToMap(bool to_segment, bool set_view)
{
  TBUILDING_ITEM *pit = static_cast<TBUILDING_ITEM *>(pitem);
  int i, j, k;

  if (TestState(US_GHOST)) {
    // update terrain ID and in each segment of map
    for (i = 0; i < GetUnitWidth(); i++)
      for (j = 0; j < GetUnitHeight(); j++)
        for (k = pit->GetExistSegments().min; k <= pit->GetExistSegments().max; k++)
          map.segments[k].surface[pos.x + i][pos.y + j].ghost = this;
  }

  else {
    // check if another unit is in the map
    for (i = 0; i < GetUnitWidth(); i++)
      for (j = 0; j < GetUnitHeight(); j++)
        for (k = pit->GetExistSegments().min; k <= pit->GetExistSegments().max; k++)
          if (map.segments[k].surface[pos.x + i][pos.y + j].unit) return false;

    // update terrain ID and in each segment of map
    for (i = 0; i < GetUnitWidth(); i++)
      for (j = 0; j < GetUnitHeight(); j++)
        for (k = pit->GetExistSegments().min; k <= pit->GetExistSegments().max; k++)
        {
          map.segments[k].surface[pos.x + i][pos.y + j].t_id += MAP_BUILDING_COEF;
          map.segments[k].surface[pos.x + i][pos.y + j].unit = this;
        }

    // updating local maps for each player
    for (i = 0; i < player_array.GetCount(); i++)
      players[i]->UpdateLocalMap(pos, pit, GetPlayerID() == i , GetPlayerID(), true);

    if (set_view) SetView(true);
  }

  if (to_segment) AddToSegments();

  is_in_map = true;
  
  return true;
}


/**
* Function deletes building from map and draw list.
*/
void TBUILDING_UNIT::DeleteFromMap(bool from_segment)
{
  TBUILDING_ITEM *pit = static_cast<TBUILDING_ITEM *>(pitem);
  int i, j, k;

  // remove from segments lists
  if (from_segment) DeleteFromSegments();
  
  if (TestState(US_GHOST)) {
    for (i = 0; i < GetUnitWidth(); i++)
      for (j = 0; j < GetUnitHeight(); j++)
        for (k = pit->GetExistSegments().min; k <= pit->GetExistSegments().max; k++) {
          map.segments[k].surface[pos.x + i][pos.y + j].ghost = NULL;
        }
  }

  else {
    // update terrain ID and in each segment of map
    for (i = 0; i < GetUnitWidth(); i++)
      for (j = 0; j < GetUnitHeight(); j++)
        for (k = pit->GetExistSegments().min; k <= pit->GetExistSegments().max; k++) {
          map.segments[k].surface[pos.x + i][pos.y + j].t_id -= MAP_BUILDING_COEF;
          map.segments[k].surface[pos.x + i][pos.y + j].unit = NULL;
        }
    
    if (selected) selection->DeleteUnit(this);

    is_in_map = false;

    // updating local maps for each player
    for (i = 0; i < player_array.GetCount(); i++)
      players[i]->UpdateLocalMap(pos, (TBUILDING_ITEM *)pit, GetPlayerID()  == i , WLK_EMPTY_FIELD , false);

    if (has_view) SetView(false);
  }
}


/**
*  Function adds building to the material_array of its player owner and increases number of newly added buildings in 
*  every source that offers the same material as the building accepts. 
*/
void TBUILDING_UNIT::AddToPlayerArray()
{  
  //add the building to the players material_array array, according to the material, which can be mined in it
  TBUILDING_ITEM *pit = dynamic_cast<TBUILDING_ITEM *>(pitem);

  for (int i = 0; i < scheme.materials_count; i ++)
  {
    if (pit->GetAllowedMaterial(i)){
      GetPlayer()->material_array[i].AddNonDuplicitNode(this);      
      players[GetPlayerID()]->IncreaseBuildingCount(i,GetPlayerID()-1); //set to the all sources which offers the same material,as this building accepts, that new building was created, set new_building count to ++1                  
    }
  }
}

/**
*  Function deletes building from the material_array of its player owner. 
*/
void TBUILDING_UNIT::DeleteFromPlayerArray()
{  
  for (int i = 0; i < scheme.materials_count; i++)
  {
    if (static_cast<TBUILDING_ITEM *>(pitem)->GetAllowedMaterial(i))
    {
      GetPlayer()->material_array[i].RemoveNode(this);
      //GetPlayer()->sources[i].Iterator(&TSOURCE_UNIT::BuildingDestroyed,this);
      //vyhodit premennu
    }
  }
}

/**
 *  Sets state to US_DYING
 */
void TBUILDING_UNIT::Dead(bool local)
{
  if (static_cast<TBUILDING_ITEM *>(pitem)->AllowAnyMaterial() && !TestState(US_IS_BEING_BUILT))     //if building accepts some kind of material
    DeleteFromPlayerArray();   // deletes from the players material_array and from source also  
  
  int new_state;
  if (local)
    new_state = US_DYING;
  else
    new_state = RQ_DYING;

  if (TestState(US_IS_BEING_BUILT) && !GetProgress() && (life == 0)) {
    if (local)
      new_state = US_DELETE;
    else
      new_state = RQ_DELETE;

    
    player->DecPlayerUnitsCount(); // decrease count of active players units
    DeleteFromMap(false);
  }
  if (local) {
    if (group_id >= 0)
      selection->DeleteStoredUnit(group_id, this);

    SendEvent(false, glfwGetTime(), new_state, -1, pos.x, pos.y, pos.segment);
  }
  else
    SendRequestLocal(false, glfwGetTime(), new_state, -1, pos.x, pos.y, pos.segment);
}


//=========================================================================
// END
//=========================================================================
// vim:ts=2:sw=2:et:

