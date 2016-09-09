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
 *  @file dosources.cpp
 *
 *  Working with source units.
 *
 *  @author Peter Knut
 *  @author Valeria Sventova 
 *
 *  @date 2002, 2003
 */

//=========================================================================
// Included files
//=========================================================================

#include <math.h>

#include "dodata.h"
#include "dodraw.h"
#include "dologs.h"
#include "dounits.h"
#include "doplayers.h"
#include "dosimpletypes.h"
#include "doselection.h"
#include "dopool.h"
#include "dohost.h"


//=========================================================================
// class TSOURCE_UNIT
//=========================================================================

/**
 *  Constructor.
 */
TSOURCE_UNIT::TSOURCE_UNIT(int uplayer, int uid, int ux, int uy, TSOURCE_ITEM *pit, int new_unit_id, bool global_unit)
:TMAP_UNIT(uplayer, ux, uy, pit->GetExistSegments().min, pit, new_unit_id, global_unit)
{
  int i, j;
  life = 0;

  j = static_cast<TSOURCE_ITEM *>(this->pitem)->GetOfferMaterial(); //find out ,which material will be offered by new source

  GetPlayer()->sources[j].AddNode(this);    //add new source to the list of sources...

  // Create array for all players except hyper player.
  my_player_array = NEW PTNEAREST_BUILDINGS[player_array.GetCount()-1];

  for (i = 0; i < player_array.GetCount()-1; i++) //players except hyper player
  {    
    ///Debug (LogMsg ("i = %d", i));  !!!! hlasky pre odhalenie bugu [PPP]
    ///Debug (LogMsg ("j = %d", j));

    int index =players[i+1]->race->workers_item_count[j];

    ///Debug (LogMsg ("index = %d", index));
  
    if (index) 
      my_player_array[i] = NEW TNEAREST_BUILDINGS[index];
    else
      my_player_array[i] = NULL;

    index--;    
  }
  renew_count = 0.0;
  material_balance = 0;

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
TSOURCE_UNIT::TSOURCE_UNIT(TSOURCE_UNIT *copy_unit, int new_unit_id, bool global_unit)
:TMAP_UNIT(copy_unit->GetPlayerID(), copy_unit->GetPosition().x, copy_unit->GetPosition().y, copy_unit->GetPosition().segment, (TSOURCE_ITEM *)copy_unit->GetPointerToItem(), new_unit_id, global_unit)
{
  TSOURCE_ITEM *pit = static_cast<TSOURCE_ITEM *>(copy_unit->GetPointerToItem());

  pitem = pit;
  life = copy_unit->GetLife();

  material_balance = copy_unit->GetMaterialBalance();
  lieing_down = copy_unit->IsLieingDown();
  my_player_array = NULL;
  renew_count = 0;

  PutState(copy_unit->GetState());
  PutState(US_GHOST);

  ghost_owner = copy_unit->AcquirePointer();

  animation->SetTexItem(copy_unit->GetAnimation()->GetTexItem());
}


/**
 *  Destruktor.
 */
TSOURCE_UNIT::~TSOURCE_UNIT()
{
  TSOURCE_ITEM *pit = static_cast<TSOURCE_ITEM *>(pitem);

  if (TestState(US_GHOST)) return;

  if (my_player_array) 
  {
    for (int i = 0 ; i < player_array.GetCount() - 1; i++) 
      if (my_player_array[i]) 
      {
        delete[] my_player_array[i];
      }
    delete[] my_player_array;
    
    my_player_array = NULL;
  }

  //delete source from the source list
  player->sources[pit->GetOfferMaterial()].RemoveNode(this);
}


void TSOURCE_UNIT::TestVisibility()
{
  TSOURCE_ITEM *p_item = dynamic_cast<TSOURCE_ITEM *>(pitem);

  if (player == myself || show_all || myself_units)
    visible = true;
  else
    visible = myself->GetLocalMap()->GetAreaVisibility(pos.x, pos.y, p_item->GetExistSegments().min, p_item->GetExistSegments().max, GetUnitWidth(), GetUnitHeight());
}


/** 
 *  The method sets the source material balance to value from parameter.
 *  If new_value is greater than value max_material_amount, max is set.
 *  if new_value is smaller than 0, 0 is set.
 *  @param  The new value of the material balance. 
 */

bool TSOURCE_UNIT::SetMaterialBalance(int new_value)
{
  TEVENT * hlp;
  TSOURCE_ITEM *itm = static_cast<TSOURCE_ITEM *>(pitem);

  new_value = MAX(0, new_value); // set 0 if new_value is smaller than 0
  new_value = MIN(itm->GetCapacity(), new_value); // set max capacity if new_value is greater than max capacity

  if (new_value == material_balance) return true;

  if (IsEmpty() && itm->IsHideable()) {
    if (!AddToMap(false, false)) return false;
    lieing_down = false;
  }
    
  material_balance = new_value;

  // only not remote (local) units send message about change of material amount
  if (!player_array.IsRemote(GetPlayerID())) {
    // send info about material amount in source to not local players
    hlp = pool_events->GetFromPool();
    hlp->SetEventProps(GetPlayerID(), GetUnitID(), false, glfwGetTime(), RQ_SYNC_MAT_AMOUNT, US_NONE, -1, 0, 0, 0, 0, 0, 0, material_balance);
    SendNetEvent(hlp, all_players);
    pool_events->PutToPool(hlp);
  }


  if (IsEmpty() && itm->IsHideable()) {
    DeleteFromMap(false);
    lieing_down = true;
  }

  ChangeAnimation();

  return true;
}


/**
 *  Fills flags into player_array when building from param is destroyed.
 *
 *  @param param  Pointer to destroyed building.
 */

/*void TSOURCE_UNIT::BuildingDestroyed(void *param)
{
  TBUILDING_UNIT* building = static_cast<TBUILDING_UNIT*>(param);

  TNEAREST_BUILDINGS *nearest = GetPositionInPlayerArray(building->GetPlayerID()-1);

  for(int i = 0; i < building->GetPlayer()->race->workers_item_count[static_cast<TSOURCE_ITEM*>(pitem)->GetOfferMaterial()]; i++)
    nearest[i].was_destroyed = true;
}
*/


/**
 *  The method adds worker into the list of workers in the source according to which_list param.
 *  The method sets workers state to US_MINING (for mining workers).
 *
 *  @param  worker Added worker
 *  @param which_list  Constant, which says, which list to add to. 
 *
 *  Only two different lists supported for now.
 */
bool TSOURCE_UNIT::AddToListOfUnits(TFORCE_UNIT *unit, T_BYTE which_list)
{
  if (TMAP_UNIT::AddToListOfUnits(unit, which_list)) {
    ChangeAnimation();
    return true;
  }
  else return false;
}


/**
 *  The method removes worker from the list of workers  in the source according to which_list param. 
 *  It unset worker's state US_MINING(for workers mining inside the source)
 *
 *  @param  worker Removed worker
 *  @param which_list  Constant, which says, which list to add to. 
 *
 *  Only two different lists supported for now.
 */
void TSOURCE_UNIT::RemoveFromListOfUnits(TFORCE_UNIT *unit, T_BYTE which_list)
{
  TMAP_UNIT::RemoveFromListOfUnits(unit, which_list); 
  ChangeAnimation();
}


/**
 *  Change unit animation according to its state.
 */
void TSOURCE_UNIT::ChangeAnimation()
{
  int tg;       // texture group
  int id = 0;   // texture id in group

  double time = -1;

  TSOURCE_ITEM *pit = static_cast<TSOURCE_ITEM *>(pitem);

  // dying
  if (TestState(US_DYING)) {
    tg = pit->tg_dying_id;
  }

  // zombie
  else if (TestState(US_ZOMBIE)) {
    tg = pit->tg_zombie_id;
    time = UNI_ZOMBIE_TIME;
  }

  // staying
  else {
    tg = pit->tg_stay_id;

    // Sources should have odd count of textures. The last texture is used for
    // empty source. The previous textures are divided into groups. Every group
    // has a pair of textures where the second one is used for source where a
    // worker is actualy working on. Group is chosen according to actual
    // material balance of the source.

    int count = player->race->tex_table.groups[tg].count;  // count of textures
    int groups = (count - 1) / 2;   // how much groups we have

    if (IsEmpty())
      id = count - 1;               // last texture for empty source
    else {
      int capacity = pit->GetCapacity();
      id = (capacity - this->material_balance) * groups / capacity * 2;

      if (groups >= 1 && GetWorkingUnits().GetFirst())
        id++;
    }
  }

  if (tg >= 0) {
    animation->SetTexItem(player->race->tex_table.GetTexture(tg, id));
    animation->Play();

    if (time >= 0) animation->SetAnimTime(time);
  }
  else animation->SetTexItem(NULL);
}


/**
 *  Method creates ghost unit which is drawn under warfog.
 */
void TSOURCE_UNIT::CreateGhost()
{
  if (TestState(US_DELETE)) return;

  TSOURCE_UNIT *b = new TSOURCE_UNIT(this, 0, false);
  b->AddToMap(true, false);
}


/**
 *  Adds source into segments lists. Unit will be drawn.
 */
void TSOURCE_UNIT::AddToSegments()
{
  TSOURCE_ITEM *pit = static_cast<TSOURCE_ITEM *>(pitem);

  for ( int k = pit->GetExistSegments().min; k <= pit->GetExistSegments().max; k++)
    map.segment_units[k]->AddUnit(this);
}


/**
 *  Removes source from segments lists. Unit will be not drawn.
 */
void TSOURCE_UNIT::DeleteFromSegments()
{
  TBUILDING_ITEM *pit = static_cast<TBUILDING_ITEM *>(pitem);

  for (int k = pit->GetExistSegments().min; k <= pit->GetExistSegments().max; k++)
    map.segment_units[k]->DeleteUnit(this);
}


/** 
 *  Function adds source to the map and draw list.
 */
bool TSOURCE_UNIT::AddToMap(bool to_segment, bool set_view)
{
  TSOURCE_ITEM *pit = static_cast<TSOURCE_ITEM *>(pitem);
  int i, j, k;

  if (TestState(US_GHOST)) {
    // update terrain ID and in each segment of map
    for (i = 0; i < GetUnitWidth(); i++)
      for (j = 0; j < GetUnitHeight(); j++)
        for (k = pit->GetExistSegments().min; k <= pit->GetExistSegments().max; k++)
          map.segments[k].surface[pos.x + i][pos.y + j].ghost = this;
  }

  else {
    // check place under unit
    for (i = 0; i < GetUnitWidth(); i++)
      for (j = 0; j < GetUnitHeight(); j++)
        for (k = pit->GetExistSegments().min; k <= pit->GetExistSegments().max; k++)
          if (map.segments[k].surface[pos.x + i][pos.y + j].unit) 
            return false;

    // update terrain ID and in each segment of map
    for (i = 0; i < GetUnitWidth(); i++)
      for (j = 0; j < GetUnitHeight(); j++)
        for (k = pit->GetExistSegments().min; k <= pit->GetExistSegments().max; k++)
        {
          map.segments[k].surface[pos.x + i][pos.y + j].t_id += MAP_BUILDING_COEF;
          map.segments[k].surface[pos.x + i][pos.y + j].unit = this;
          map.segments[k].surface[pos.x + i][pos.y + j].GetAimersList()->AttackEnemy(this, false);
          map.segments[k].surface[pos.x + i][pos.y + j].GetWatchersList()->AttackEnemy(this, true);
        }

    // updating local maps for each player
    for (i = 0; i < player_array.GetCount(); i++)
      players[i]->UpdateLocalMap(pos, pit, GetPlayerID(), false);
  }

  if (to_segment) AddToSegments();

  is_in_map = true;

  return true;
}


/**
* Function deletes building from map and draw list.
*/
void TSOURCE_UNIT::DeleteFromMap(bool from_segment)
{
  TSOURCE_ITEM *pit = static_cast<TSOURCE_ITEM *>(pitem);
  int i, j, k;

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

    // updating local maps for each player
    for (i = 0; i < player_array.GetCount(); i++)
      players[i]->UpdateLocalMap(pos, pit,GetPlayerID() ,true); 
      
      //dateLocalMap(const TPOSITION_3D position, TSOURCE_ITEM * const pitem, const T_SIMPLE player_ID, const bool collapsing)
      
    if (selected) selection->DeleteUnit(this);

    is_in_map = false;
  }
}

/**
 *  Delete unit from all lists. This function is called when remote player disconnect game.
 */
void TSOURCE_UNIT::Disconnect()
{
  TSOURCE_ITEM * itm = static_cast<TSOURCE_ITEM *>(pitem);
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
TEVENT* TSOURCE_UNIT::SendEvent(bool n_priority, double n_time_stamp, int n_event, int n_request_id, T_SIMPLE n_simple1, T_SIMPLE n_simple2, T_SIMPLE n_simple3, T_SIMPLE n_simple4, T_SIMPLE n_simple5, T_SIMPLE n_simple6, int n_int1,int n_int2)
{
  int old_state = US_NONE, last_event = US_NONE;

  if (pevent){ // if unit has some event in queue
    
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
    else if (old_state == US_REGENERATING)
    { // it is possible to end state in queue -> make undo actions, put new time stamp
      if (n_event == US_REGENERATING){ // new event will have the same ts as last
        n_time_stamp = pevent->GetTimeStamp();
      }

      queue_events->GetEvent(pevent);
      pevent->SetEventProps(GetPlayerID(), GetUnitID(), n_priority, n_time_stamp, n_event, US_NONE, n_request_id, n_simple1, n_simple2, n_simple3, n_simple4, n_simple5, n_simple6, n_int1,n_int2);
      queue_events->PutEvent(pevent); // put event to queue
    }
    else
    { // state in queue is not endable -> time stamp is not changed 
      n_time_stamp = pevent->GetTimeStamp(); // get old time stamp
      pevent->SetEventProps(GetPlayerID(), GetUnitID(), n_priority, n_time_stamp, n_event, last_event, n_request_id, n_simple1, n_simple2, n_simple3, n_simple4, n_simple5, n_simple6, n_int1,n_int2);
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
     (n_event == US_DYING) || (n_event == US_ZOMBIE) || (n_event == US_DELETE))
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
void TSOURCE_UNIT::ProcessEvent(TEVENT * proc_event)
{ 
  int mine; // >=1 - worker can unmine material from the source, 0 - otherwise

  unsigned int last_state = state;
  
  double new_time_stamp;
  unsigned int new_state = US_NONE;

  TSOURCE_ITEM * itm = static_cast<TSOURCE_ITEM *>(pitem);

  double state_time = 0;

  /****************** Put state of event ********************************/
  
  // test if event is event (else it is request)
  if (proc_event->GetEvent() < RQ_FIRST) PutState(proc_event->GetEvent());

  /****************** Undo Actions (ONLY LOCAL UNITS) *************************/
  
  if (!player_array.IsRemote(proc_event->GetPlayerID())) { // compute only not remote (local) units
    ;
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

  
  // RQ_SYNC_LIFE
  if (proc_event->TestEvent(RQ_SYNC_LIFE))
  {
    SetLife((float)proc_event->int1);
  }

  // RQ_SYNC_MAT_AMOUNT
  if (proc_event->TestEvent(RQ_SYNC_MAT_AMOUNT))
  {
    SetMaterialBalance(proc_event->int1);
  }

  /****************** Compute new values - textures, times (ALL UNITS) ********************************/

  // US_DELETE, RQ_DELETE
  if (proc_event->TestEvent(US_DELETE) || proc_event->TestEvent(RQ_DELETE)) {
    DeleteFromSegments();
    UnitToDelete(true);
    return;
  }

  // US_DYING, RQ_DYING
  if (proc_event->TestEvent(US_DYING) || proc_event->TestEvent(RQ_DYING)) {
    #if SOUND    
    // play sound
    if (visible) {
      if (!((TMAP_ITEM *)pitem)->snd_dead.IsEmpty())
        PlaySound(&((TMAP_ITEM *)pitem)->snd_dead, 1);
      else
        PlaySound(&player->race->snd_explosion, 1);
    }
    #endif

    // kill all working units inside of source
    if (IsInsideMining() && !working_units.IsEmpty()) {
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
    DeleteFromMap(false);
    lieing_down = true;
  }

  
  // set animation
  if (last_state != state) ChangeAnimation();
  
   
  /****************** Compute next event (ONLY LOCAL UNITS) *******************************/

  // Compute next event only for local units.
  if (player_array.IsRemote(proc_event->GetPlayerID()))
    return;

  // US_STAY
  if (proc_event->TestEvent(US_STAY))
  {
    // test the source if it is renewable and needs to be regenerated
    if (itm->IsRenewable() && (material_balance < itm->GetCapacity())) {
      new_time_stamp = proc_event->GetTimeStamp() + itm->GetRegenerationTime();
      SendEvent(false, new_time_stamp, US_REGENERATING, -1, 0); 
    }
  }
     
  // RQ_CAN_MINE
  else if (proc_event->TestEvent(RQ_CAN_MINE))
  {
    //worker wants to mine, lets see if source has any material to offer 
    if (!IsEmpty())
    {
      mine = MIN (proc_event->int2, this->GetMaterialBalance());
      SetMaterialBalance(GetMaterialBalance() - mine);

      // test is source is renewable
      if (itm->IsRenewable())
      {//plan reweving
        if (material_balance < itm->GetCapacity())  //material_balance of source is lower than capacity
        {
          new_time_stamp = proc_event->GetTimeStamp() + itm->GetRegenerationTime();
          //send event to queue
          SendEvent(false, new_time_stamp, US_REGENERATING, -1, 0); 
        }
      }

    }
    else    
    {
      mine = 0;    
    }

    //send event to worker that can/cant mine    
    new_time_stamp = proc_event->GetTimeStamp(); // get old time stamp
    // put request into queue
    TPLAYER_UNIT * worker = players[proc_event->simple1]->hash_table_units.GetUnitPointer(proc_event->int1);
    
    if (worker)
      worker->SendRequest(false, new_time_stamp, RQ_CAN_MINE, proc_event->GetRequestID(), 0, 0, 0, 0, 0, 0, 0, mine);
  }    

  //US_REGENERATING 
  else if (proc_event->TestEvent(US_REGENERATING)) {
    T_SIMPLE first_regenerating;
    
    //plann next regenerating if possible
    if (material_balance + 1 < itm->GetCapacity()) { // material_balance is used because increment of amount is after this line
      
      if ((material_balance == 0) && (proc_event->simple1 == 0)){
        new_time_stamp = proc_event->GetTimeStamp() + itm->GetFirstRegenerationTime();
        first_regenerating = 1;
      }
      else {
        new_time_stamp = proc_event->GetTimeStamp() + itm->GetRegenerationTime();
        first_regenerating = 0;
        IncMaterialBalance();
      }
      //send event to queue
      SendEvent(false, new_time_stamp, US_REGENERATING, -1, first_regenerating); 
    }
    else {
      IncMaterialBalance();
      
      new_time_stamp = proc_event->GetTimeStamp();
      //send event to queue
      SendEvent(false, new_time_stamp, US_STAY, -1); 
    }
  }

  // US_DYING
  else if (proc_event->TestEvent(US_DYING)) {
    state_time = 0;
  
    if (itm->tg_dying_id != -1) {
      TGUI_TEXTURE * pgui_texture;

      pgui_texture = player->race->tex_table.GetTexture(itm->tg_dying_id, 0);
      state_time = pgui_texture->frame_time * pgui_texture->frames_count;
    }
    
    ClearActions();

    new_time_stamp = proc_event->GetTimeStamp() + state_time + TS_MIN_EVENTS_DIFF;
    new_state = US_ZOMBIE;

    // send event to queue
    SendEvent(false, new_time_stamp, new_state, -1, proc_event->simple1, proc_event->simple2, proc_event->simple3, proc_event->simple4);
  }

  // US_ZOMBIE
  else if (proc_event->TestEvent(US_ZOMBIE)) {
    if (itm->tg_zombie_id != -1) state_time = UNI_ZOMBIE_TIME;
    else state_time = 0;

    new_time_stamp = proc_event->GetTimeStamp() + state_time + TS_MIN_EVENTS_DIFF;
    new_state = US_DELETE;
    
    // send event to queue
    SendEvent(false, new_time_stamp, new_state, -1, proc_event->simple1, proc_event->simple2, proc_event->simple3, proc_event->simple4);
  }
}


/**
 *  Method is called for periodically updating of look and features.
 *  Main filling of Update function is regeneration of sources 
 *  @param time_shift  Time shift from the last update.
 */
bool TSOURCE_UNIT::UpdateGraphics(double time_shift)
{
  bool last_visible = visible;

  TMAP_UNIT::UpdateGraphics(time_shift);

  // ghots
  if (
    (TestState(US_GHOST) && visible) ||
    (player != myself && !visible && last_visible)
  ) return true;

  return false;
}


//=========================================================================
// Definition of global functions.
//=========================================================================



//=========================================================================
// END
//=========================================================================
// vim:ts=2:sw=2:et:

