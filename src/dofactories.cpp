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
 *  Working with factories.
 *
 *  @author Peter Knut
 *  @author Valeria Sventova
 *  @author Jiri Krejsa
 *  @Author Martin Kosalko
 *
 *  @date 2002, 2003, 2004
 */

#include <math.h>

#include "dodata.h"
#include "dodraw.h"
#include "dologs.h"
#include "dosimpletypes.h"
#include "dounits.h"
#include "doengine.h"


//=========================================================================
// Variables
//=========================================================================


//=========================================================================
// Methods definitions of the class TFACTORY_UNIT
//=========================================================================


/** 
 *  Constructor. Prepare factory attributes.
 */
TFACTORY_UNIT::TFACTORY_UNIT(int uplayer, int ux, int uy, TBUILDING_ITEM *mi, int new_unit_id, bool global_unit)
: TBUILDING_UNIT(uplayer, ux, uy, mi, new_unit_id, global_unit)
{
  order_size = producing = production_count = 0;
  production_time = 0.0;
  need_id = -1;
  paused = false;

  for (register int i = 0; i < UNI_MAX_ORDER_LENGTH; i++) 
    order[i] = NULL;
}

/**
 *  Method is beeing called when new event is got from queue.
 *
 *  @param proc_event Pointer to processed event.
 */
void TFACTORY_UNIT::ProcessEvent(TEVENT * proc_event)
{
  unsigned int last_state = state;
  double state_time = 0;
  double new_time_stamp;
  unsigned int new_state = US_NONE;

  TFACTORY_ITEM * itm = static_cast<TFACTORY_ITEM *>(pitem);

  TFORCE_ITEM * new_prod_itm;
  TFORCE_UNIT * unit;


  /****************** Put state of event ********************************/
  
  if (proc_event->TestEvent(US_DELETE) || proc_event->TestEvent(RQ_DELETE)){
    TMAP_UNIT::ProcessEvent(proc_event);
    return;
  }
  
  TMAP_UNIT::ProcessEvent(proc_event);

   
  /****************** Undo Actions (ONLY LOCAL UNITS) *************************/
  
  if (!player_array.IsRemote(proc_event->GetPlayerID())) { // compute only not remote (local) units
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
    TFACTORY_ITEM * new_itm = static_cast<TFACTORY_ITEM *>(player->race->buildings[proc_event->int1]);
    
    ClearActions();
    SetPointerToItem(new_itm);

    PutState(US_IS_BEING_BUILT);  //set to building that it is being built
    ChangeAnimation();
  }

  // RQ_CREATE_UNIT
  if (proc_event->TestEvent(RQ_CREATE_UNIT))
  {
    unit = NULL;
    new_prod_itm = static_cast<TFORCE_ITEM *>(player->race->units[proc_event->int1]);

    //create new unit
    switch(new_prod_itm->GetItemType()) {
      case IT_FORCE:   //force unit
        unit = NEW TFORCE_UNIT(GetPlayerID(), proc_event->simple1, proc_event->simple2, proc_event->simple3, proc_event->simple4, new_prod_itm, proc_event->int2, true);
        break;
      case IT_WORKER:   //worker unit
        unit = NEW TWORKER_UNIT(GetPlayerID(), proc_event->simple1, proc_event->simple2, proc_event->simple3, proc_event->simple4, new_prod_itm, proc_event->int2, true);
        break;
      default:    //unknown unit type
        Warning(LogMsg("Unknown type of the product '%p' at factory '%p'.", new_prod_itm, this));
        unit = NULL;
        break;
    }

    if (!unit) return;

    unit->AddToMap(true, true);
    unit->ChangeAnimation();
  }

  // US_DYING, RQ_DYING
  if (proc_event->TestEvent(US_DYING) || proc_event->TestEvent(RQ_DYING)) {
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

    // kill all ready units 
    if (!ready_units.IsEmpty()) {
      TLIST<TFORCE_UNIT>::TNODE<TFORCE_UNIT> *node;
      TFORCE_UNIT *unit;

      for (node = ready_units.GetFirst(); node; node = node->GetNext()) {
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
    if (proc_event->TestEvent(US_ZOMBIE)) {
      if (itm->tg_zombie_id != -1) state_time = UNI_ZOMBIE_TIME;
      else state_time = 0;

      new_time_stamp = proc_event->GetTimeStamp() + state_time + TS_MIN_EVENTS_DIFF;
      new_state = US_DELETE;
      
      // send event to queue
      SendEvent(false, new_time_stamp, new_state, -1, proc_event->simple1, proc_event->simple2, proc_event->simple3, proc_event->simple4);
    }

    // RQ_PRODUCING
    if (proc_event->TestEvent(RQ_PRODUCING)) {

      // process event only in case that factory is not dead
      if (!paused && !(TestState(US_DYING) || TestState(US_ZOMBIE) || TestState(US_DELETE))) {
        if (waiting_request_id == proc_event->GetRequestID()) { // factory waiting for right unit

          // unit is completed
          if (production_count == 0) {
            new_prod_itm = order[producing]->GetProduceableItem();

            //create new unit
            switch(new_prod_itm->GetItemType()) {
              case IT_FORCE:   //force unit
                unit = NEW TFORCE_UNIT(GetPlayerID(), 0, 0, 0, 0, new_prod_itm, 0, true);
                break;
              case IT_WORKER:   //worker unit
                unit = NEW TWORKER_UNIT(GetPlayerID(), 0, 0, 0, 0, new_prod_itm, 0, true);
                break;
              default:    //unknown unit type
                Warning(LogMsg("Unknown type of the product '%p' at factory '%p'.", new_prod_itm, this));
                unit = NULL;
                break;
            }

            if (unit) {    //unit create successful
              player->IncPlayerUnitsCount(); //increase count of active units of player

              // if new unit could be leaved
              if (unit->LeaveHolderUnit(this)) {

                // send info to not local players that new unit is creating now
                TEVENT * hlp;

                hlp = pool_events->GetFromPool();
                hlp->SetEventProps(GetPlayerID(), GetUnitID(), false, proc_event->GetTimeStamp(), RQ_CREATE_UNIT, US_NONE, -1, unit->GetPosition().x, unit->GetPosition().y, unit->GetPosition().segment, unit->GetMoveDirection(), new_prod_itm->GetItemType(), 0, new_prod_itm->index, unit->GetUnitID());
                SendNetEvent(hlp, all_players);
                pool_events->PutToPool(hlp);
                
                #if SOUND
                // play sound
                if (config.snd_unit_speech && unit->TestPlayer(myself))
                  unit->PlaySound(&new_prod_itm->snd_ready);
                #endif

                unit->SendEvent(false, proc_event->GetTimeStamp() + TS_MIN_EVENTS_DIFF, US_NEXT_STEP, -1, unit->GetPosition().x, unit->GetPosition().y, unit->GetPosition().segment, unit->GetMoveDirection());
                                
                // adding only possitive energy and food
                if (new_prod_itm->energy > 0)
                  player->AddUnitEnergyFood(new_prod_itm->energy, 0);
                if (new_prod_itm->food > 0)
                  player->AddUnitEnergyFood(0, new_prod_itm->food);
                
                // increase count of units of this kind
                new_prod_itm->IncreaseActiveUnitCount();
              }
              else // send request to try to leave
                ready_units.AddNodeToEnd(unit);
                SendRequest(false, proc_event->GetTimeStamp() + UNI_TRY_TO_LEAVE_SHIFT, RQ_TRY_TO_LEAVE, -1);
            }
            else //creating unit failed
            {
              Warning(LogMsg("Not enough memory for creating new unit."));
            }

            TakeOffUnitFromOrder();
            if (selected) {
              CreateOrderButtons();
            }
            SetProgress(0);


            if (order_size) {   //factory has some ordered unit
              //start new production
              production_time = order[producing]->GetProduceTime();
              production_count = UNI_PRODUCING_COUNT;
         
              waiting_request_id = SendRequest(false, proc_event->GetTimeStamp(), RQ_PRODUCING); // sets info about requested request
            }


          }
          else{ // unit is not completed now
            bool possible = true;

            SetProgress((T_BYTE) ((float) (UNI_PRODUCING_COUNT - production_count) * 100/UNI_PRODUCING_COUNT));

            // test if factory has enough energy for producing
            if (GetPlayer()->GetPercentEnergy() < itm->min_energy) {
              need_id = 1;
              possible = false;
            }
         
            if (possible && (production_count == UNI_PRODUCING_COUNT)) { // start creating new unit
              new_prod_itm = order[producing]->GetProduceableItem();

              // check if there is enough food for unit
              if ((new_prod_itm->food < 0) && (player->GetInFood() - player->GetOutFood() + new_prod_itm->food < 0)){
                need_id = 0;
                possible = false;
              }
              else {
                // only removing food or energy (adding negative)
                if (new_prod_itm->energy <= 0)
                  player->AddUnitEnergyFood(new_prod_itm->energy, 0);
                if (new_prod_itm->food <= 0)
                  player->AddUnitEnergyFood(0, new_prod_itm->food);
              }
            }


            //test whether is enough of all needed material
            for (register int i = 0; possible && (i < scheme.materials_count); i++)
              if (order[producing]->GetProduceableItem()->materials[i] / UNI_PRODUCING_COUNT > player->GetStoredMaterial(i)) {  //not enough of material
                need_id = 2 + i;
                possible = false;
              }
          
            
            if (possible) {     //pay unit
              need_id = -1;

              for (register int j = 0; j < scheme.materials_count; j++)
                player->DecStoredMaterial(j, order[producing]->GetProduceableItem()->materials[j] / UNI_PRODUCING_COUNT);

              production_count --;
              new_time_stamp = proc_event->GetTimeStamp() + (production_time / UNI_PRODUCING_COUNT);
            }
            else
              new_time_stamp = proc_event->GetTimeStamp() + UNI_TRY_TO_PRODUCE_SHIFT;
          
            SendRequest(false, new_time_stamp , RQ_PRODUCING, proc_event->GetRequestID());
          }
        }
      }
    }

    // RQ_TRY_TO_LEAVE
    if (proc_event->TestEvent(RQ_TRY_TO_LEAVE)) {
      // process event only in case that factory is not dead
      if (!(TestState(US_DYING) || TestState(US_ZOMBIE) || TestState(US_DELETE))) {
        bool leaved = true;

        for (unsigned int i = 0; (leaved && (i < ready_units.GetLength())); i++){
          unit = ready_units.GetFirst()->GetPitem();
          if (unit->LeaveHolderUnit(this)) {
            ready_units.TakeFirstOut(); // leave nit from list
          
            // send event to unit
            unit->SendEvent(false, proc_event->GetTimeStamp(), US_NEXT_STEP, -1, unit->GetPosition().x, unit->GetPosition().y, unit->GetPosition().segment, unit->GetMoveDirection());
            #if SOUND
            // play sound
            if (config.snd_unit_speech && unit->TestPlayer(myself)) 
              unit->PlaySound(&((TFORCE_ITEM *)unit->GetPointerToItem())->snd_ready);
            #endif
          }
          else
            leaved = false;
        }

        // if there are some units in factory, send request
        if (!ready_units.IsEmpty())
          SendRequest(false, proc_event->GetTimeStamp() + UNI_TRY_TO_LEAVE_SHIFT, RQ_TRY_TO_LEAVE, -1);
      }
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
bool TFACTORY_UNIT::UpdateGraphics(double time_shift)
{
  bool last_visible = visible; 

  TMAP_UNIT::UpdateGraphics(time_shift);

  if (!TestState(US_GHOST)) {
    if (need_id >= 0)
      ShowNeedElement(need_id);
    else if (((TBUILDING_ITEM *)pitem)->min_energy > player->GetPercentEnergy())
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


void TFACTORY_UNIT::TogglePausedProducing()
{
  paused = !paused;

  if (!paused) {
    if (order_size) {
      process_mutex->Lock();
      SendRequest(false, glfwGetTime() + (production_time / UNI_PRODUCING_COUNT), RQ_PRODUCING, waiting_request_id);
      process_mutex->Unlock();
    }
  }
  else {
    need_id = -1;
  }
}


void TFACTORY_UNIT::CancelProducing(int buttonid)
{
  process_mutex->Lock();

  // remove just produced unit
  if (buttonid == 0) {

    // return 1/2 of materials to player
    for (int j = 0; j < scheme.materials_count; j++)
      player->IncStoredMaterial(j, order[producing]->GetProduceableItem()->materials[j] * (float(UNI_PRODUCING_COUNT - production_count) / (UNI_PRODUCING_COUNT * 2)));

    // stop producing
    SetProgress(0);
    TakeOffUnitFromOrder();

    // start new production
    if (order_size) {
      production_time = order[producing]->GetProduceTime();
      production_count = UNI_PRODUCING_COUNT;
  
      waiting_request_id = SendRequest(false, glfwGetTime(), RQ_PRODUCING); // sets info about requested request
    }
    else
      waiting_request_id = -1;
  }
  // move units in order
  else {
    int i, j;

    for (i = buttonid; i < order_size - 1; i++)
    {
      j = (producing + i) % UNI_MAX_ORDER_LENGTH;
      if (j == UNI_MAX_ORDER_LENGTH - 1) 
        order[j] = order[0];
      else
        order[j] = order[j+1];
    }
    j = (producing + i) % UNI_MAX_ORDER_LENGTH;
    order[j] = NULL;

    order_size--;         //decrease order length
  }

  if (!order_size && paused)
    TogglePausedProducing();

  process_mutex->Unlock();
}


/** Tests if unit could be added to order list.
 *  If unit_item is given, tests also if this item is in prodeceable list.
 *
 *  @param  unit_item Unit item of tested unit. Can be NULL.
 *  @return @c True if successful.
 */
bool TFACTORY_UNIT::CanAddUnitToOrder(TFORCE_ITEM *unit_item)
{
  // test state
  if (TestState(US_IS_BEING_BUILT) || TestState(US_DYING) || TestState(US_ZOMBIE) || TestState(US_DELETE))
    return false;

  // test order size
  if (order_size >= UNI_MAX_ORDER_LENGTH)
    return false;

  // test produceable list
  if (unit_item && !static_cast<TFACTORY_ITEM*>(pitem)->GetProductsList().GetNodeWithItem(unit_item))
    return false;

  return true;
}


/** Adds unit to the order list if it is possible.
 *  Checks whether it is producable unit in this factory.
 *
 *  @param added  Pointer to added unit kind.
 */
bool TFACTORY_UNIT::AddUnitToOrder(TFORCE_ITEM *unit_item)
{
  if (!unit_item || !CanAddUnitToOrder(unit_item))
    return false;

  TFACTORY_ITEM *item = static_cast<TFACTORY_ITEM*>(pitem);
  TPRODUCEABLE_NODE *product_info = item->GetProductsList().GetNodeWithItem(unit_item);

  process_mutex->Lock();

  if (order_size == 0)      //array of orders is empty
  {
    //prepare factory to producing unit
    order[0] = product_info;

    order_size = 1;
    producing = 0;
    production_time = product_info->GetProduceTime();
    production_count = UNI_PRODUCING_COUNT;
    
    waiting_request_id = SendRequest(false, glfwGetTime(), RQ_PRODUCING); // sets info about requested request
  }
  else
  {
    order[(producing + order_size) % UNI_MAX_ORDER_LENGTH] = product_info;
    order_size++;
  }
    
  process_mutex->Unlock();

  return true;
}


/**
 *  Takes off producing unit from order.
 */
void TFACTORY_UNIT::TakeOffUnitFromOrder()
{
  producing = (producing + 1) % UNI_MAX_ORDER_LENGTH;   //shift begin of the queue
  production_time = 0;
  order_size--;         //decrease order length
}


//=========================================================================
// END
//=========================================================================
// vim:ts=2:sw=2:et:

