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
 *  @file dounits.cpp
 *
 *  Working with units.
 *
 *  @author Peter Knut
 *  @author Valeria Sventova
 *  @author Jiri Krejsa
 *
 *  @date 2002, 2003, 2004
 */

#include <cstdlib>
#include <math.h>
#include <stdarg.h>

#include "dodata.h"
#include "dodraw.h"
#include "dologs.h"
#include "dounits.h"
#include "doselection.h"
#include "dosimpletypes.h"
#include "domouse.h"
#include "dopool.h"
#include "donet.h"
#include "dohost.h"
#include "doengine.h"


//=========================================================================
// Variables
//=========================================================================

/**
 *  Table of angles with their sin() and cos().
 */
const double dir_angle[LAY_DIRECTIONS_COUNT][3] = {
// angle       sin        cos
  {PI * 1.5,   -1.0,      0.0},
  {PI * 1.25,  -SQRT_22,  -SQRT_22},
  {PI,         0.0,       -1.0},
  {PI * 0.75,  SQRT_22,   -SQRT_22},
  {PI * 0.5,   1.0,       0.0},
  {PI * 0.25,  SQRT_22,   SQRT_22},
  {0,          0.0,       1.0},
  {PI * 1.75,  -SQRT_22,  SQRT_22},
  {0,          0.0,       0.0},
  {0,          0.0,       0.0}
};

extern TMAP map;


//=========================================================================
// Global Methods
//=========================================================================

#if DEBUG

const char *EventToString(const int event)
{
  switch (event) {
  case US_NONE:               return "US_NONE";
  case US_STAY:               return "US_STAY";
  case US_ANCHORING:          return "US_ANCHORING";
  case US_LANDING:            return "US_LANDING";
  case US_UNLANDING:          return "US_UNLANDING";
  case US_MOVE:               return "US_MOVE";
  case US_NEXT_STEP:          return "US_NEXT_STEP";
  case US_TRY_TO_MOVE:        return "US_TRY_TO_MOVE";
  case US_ATTACKING:          return "US_ATTACKING";
  case US_NEXT_ATTACK:        return "US_NEXT_ATTACK";
  case US_MINING:             return "US_MINING";
  case US_UNLOADING:          return "US_UNLOADING";
  case US_CONSTRUCTING:       return "US_CONSTRUCTING";
  case US_NEXT_CONSTRUCTING:  return "US_NEXT_CONSTRUCTING";
  case US_REPAIRING:          return "US_REPAIRING";
  case US_NEXT_REPAIRING:     return "US_NEXT_REPAIRING";
  case US_REGENERATING:       return "US_REGENERATING";
  case US_IS_BEING_BUILT:     return "US_IS_BEING_BUILT";
  case US_LEFT_ROTATING:      return "US_LEFT_ROTATING";
  case US_RIGHT_ROTATING:     return "US_RIGHT_ROTATING";
  case US_GHOST:              return "US_GHOST";
  case US_DYING:              return "US_DYING";
  case US_ZOMBIE:             return "US_ZOMBIE";
  case US_DELETE:             return "US_DELETE";
  case US_NEXT_MINE:          return "US_NEXT_MINE";
  case US_NEXT_UNLOADING:     return "US_NEXT_UNLOADING";
  case US_HEALING:            return "US_HEALING";
  case US_NEXT_HIDING:        return "US_NEXT_HIDING";
  case US_HIDING:             return "US_HIDING";
  case US_START_MINE:         return "US_START_MINE";
  case US_START_REPAIR:       return "US_START_REPAIR";
  case US_START_UNLOAD:       return "US_START_UNLOAD";
  case US_START_HIDING:       return "US_START_HIDING";
  case US_WAIT_FOR_PATH:      return "US_WAIT_FOR_PATH";
  case US_START_ATTACK:       return "US_START_ATTACK";
  case US_SEARCHING_NEAREST:  return "US_SEARCHING_NEAREST";
  case US_EJECTING:           return "US_EJECTING";
  case US_END_ATTACK:         return "US_END_ATTACK";
  case US_FEEDING:            return "US_FEEDING";

  case RQ_CAN_MINE:           return "RQ_CAN_MINE";
  case RQ_MINE_SOUND:         return "RQ_MINE_SOUND";
  case RQ_PRODUCING:          return "RQ_PRODUCING";
  case RQ_TRY_TO_LEAVE:       return "RQ_TRY_TO_LEAVE";
  case RQ_CHANGE_SEGMENT:     return "RQ_CHANGE_SEGMENT";
  case RQ_FIRE_OFF:           return "RQ_FIRE_OFF";
  case RQ_PATH_FINDING:       return "RQ_PATH_FINDING";
  case RQ_IMPACT:             return "RQ_IMPACT";
  case RQ_NEAREST_SEARCHING:  return "RQ_NEAREST_SEARCHING";
  case RQ_GROUP_MOVING:       return "RQ_GROUP_MOVING";
  case RQ_SYNC_LIFE:          return "RQ_SYNC_LIFE";
  case RQ_SYNC_MAT_AMOUNT:    return "RQ_SYNC_MAT_AMOUNT";
  case RQ_SYNC_PROGRESS:      return "RQ_SYNC_PROGRESS";
  case RQ_END_MINE:           return "RQ_END_MINE";
  case RQ_END_UNLOAD:         return "RQ_END_UNLOAD";
  case RQ_CREATE_UNIT:        return "RQ_CREATE_UNIT";
  case RQ_UPGRADE_UNIT:       return "RQ_UPGRADE_UNIT";
  case RQ_CREATE_PROJECTILE:  return "RQ_CREATE_PROJECTILE";
  case RQ_DYING:              return "RQ_DYING";
  case RQ_ZOMBIE:             return "RQ_ZOMBIE";
  case RQ_DELETE:             return "RQ_DELETE";
  case RQ_FEEDING:            return "RQ_FEEDING";

  default:                    return "ENDEFINED";
  }
}

#endif


//=========================================================================
// Methods definitions of the class TDRAW_UNIT.
//=========================================================================


/**
 *  Method draws unit.
 */
void TDRAW_UNIT::Draw(T_BYTE style)
{
  if (!visible || !in_active_area) return;

  SetMapPosition(rpos_x, rpos_y);
  SetUnitColor(style);

  animation->Draw();
}


/**
 *  Method is called for periodically updating of look and features.
 *
 *  @param time_shift  Time shift from the last update.
 */
bool TDRAW_UNIT::UpdateGraphics(double time_shift)
{
  TestVisibility();

  in_active_area = map.active_area.IsInArea(pos.x, pos.y, pitem->GetWidth(), pitem->GetHeight());

  if (animation) animation->Update(time_shift);

  return false;
}


/**
 *  The method correctly kill the unit.
 *  Method is called when unit is destroy. Dealocate memory, play animation, etc.
 */
void TDRAW_UNIT::Dead(bool local)
{
  UnitToDelete(false);
}


/**
 *  Adds unit into segments lists. Unit will be drawn.
 */
void TDRAW_UNIT::AddToSegments()
{
  map.segment_units[pos.segment]->AddUnit(this);
}


/**
 *  Removes unit from segments lists. Unit will be not drawn.
 */
void TDRAW_UNIT::DeleteFromSegments()
{
  map.segment_units[pos.segment]->DeleteUnit(this);
}


bool TDRAW_UNIT::IsCloserThan(TDRAW_UNIT *unit)
{
  if (IsLieingDown()) return false;

  else if (unit->IsLieingDown()) return true;

  else if (unit->IsFlyingUp()) return false;

  else if (IsFlyingUp()) return true;

  else if ((rpos_x < unit->rpos_x + unit->GetUnitWidth())
    && (rpos_y < unit->rpos_y + unit->GetUnitHeight()))
  {
    if (rpos_x + GetUnitWidth() <= unit->rpos_x || rpos_y + GetUnitHeight() <= unit->rpos_y) return true;
    else return false;
  }

  else if ((unit->rpos_x < rpos_x + GetUnitWidth())
    && (unit->rpos_y < rpos_y + GetUnitHeight()))
  return false;

  else if (unit->rpos_x > rpos_x) {
    double pos1 = rpos_x + GetUnitWidth() + rpos_y;
    double pos2 = unit->rpos_x + unit->rpos_y + unit->GetUnitHeight();

    return (pos1 < pos2);
  }

  else {
    double pos1 = rpos_x + rpos_y + GetUnitHeight();
    double pos2 = unit->rpos_x + unit->GetUnitWidth() + unit->rpos_y;

    return (pos1 < pos2);
  }
}


void TDRAW_UNIT::TestVisibility()
{
  //visible = show_all || myself->GetLocalMap()->GetAreaVisibility(pos, GetUnitWidth(), GetUnitHeight());
}


/** 
 *  Set position of unit to position of param if it is in the map.
 *
 *  @param new_pos  New position of the unit.
 */
void TDRAW_UNIT::SetPosition(const TPOSITION_3D new_pos) 
{
  if (map.IsInMap(new_pos))        //new position is in map
  {
    if ((pos.segment != new_pos.segment) && (pos.segment != LAY_UNAVAILABLE_POSITION))     //unit leave old valid segment
    {
      map.segment_units[pos.segment]->DeleteUnit(this);
      map.segment_units[new_pos.segment]->AddUnit(this);
    }
    pos = new_pos;
    sync_pos = new_pos;
  }
}

/** 
 *  Set position of unit to position of parameters if it is in the map.
 *
 *  @param nx x-coordinate of the new position.
 *  @param ny y-coordinate of the new position.
 *  @param ns segment-coordinate of the new position.
 */
void TDRAW_UNIT::SetPosition(const T_SIMPLE nx, const T_SIMPLE ny, const T_SIMPLE ns)
{
  if (map.IsInMap(nx, ny, ns))        //new position is in map
  {
    if ((pos.segment != ns) && (pos.segment != LAY_UNAVAILABLE_POSITION))     //unit leave old valid segment
    {
      map.segment_units[pos.segment]->DeleteUnit(this);
      map.segment_units[ns]->AddUnit(this);
    }
    pos.SetPosition(nx, ny, ns);
    sync_pos.SetPosition(nx, ny, ns);
  }
}


TPOSITION_3D TDRAW_UNIT::GetCenterPosition()
{
  TPOSITION_3D pom;

  pom.x = pos.x + pitem->GetWidth() / 2;
  pom.y = pos.y + pitem->GetHeight() / 2;
  pom.segment = pos.segment;

  return pom;
}


TPOSITION_3D TDRAW_UNIT::TranslateToCentralize(const TPOSITION_3D position)
{
  TPOSITION_3D pom;

  int i;

  i = position.x - pitem->GetWidth() / 2;
  if (i < 0) i = 0;
  if (i >= map.width) i = map.width - 1;
  pom.x = i;

  i = position.y - pitem->GetWidth() / 2;
  if (i < 0) i = 0;
  if (i >= map.height) i = map.height - 1;
  pom.y = i;

  pom.segment = position.segment;

  return pom;
}


/**
 *  Constructor.
 *  Values are only zeroized.
 */
TDRAW_UNIT::TDRAW_UNIT()
{ 
  pitem = NULL;
  pos.SetPosition(LAY_UNAVAILABLE_POSITION, LAY_UNAVAILABLE_POSITION, LAY_UNAVAILABLE_POSITION);
  sync_pos.SetPosition(LAY_UNAVAILABLE_POSITION, LAY_UNAVAILABLE_POSITION, LAY_UNAVAILABLE_POSITION);
  rpos_x = rpos_y = 0.0f;
  visible = false;
  in_active_area = true;
  animation = NULL;
  lieing_down = flying_up = false;
#if SOUND
  snd_played = NULL;
#endif

  for (int i = 0; i <= DAT_SEGMENTS_COUNT; i++) seg_next[i] = seg_prev[i] = NULL;
}


/**
 *  Constructor.
 *  The constructor sets values given by parameters, other values are zeroized.
 */
TDRAW_UNIT::TDRAW_UNIT(int p_x, int p_y, int p_z, TDRAW_ITEM* set_item, TTEX_TABLE *tex_table, bool random_tex)
{
  pitem = set_item;
  pos.SetPosition(p_x, p_y, p_z);
  sync_pos.SetPosition(p_x, p_y, p_z);
  rpos_x = static_cast<float>(p_x);
  rpos_y = static_cast<float>(p_y);
  visible = false;
  in_active_area = true;
  lieing_down = flying_up = false;
#if SOUND
  snd_played = NULL;
#endif

  for (int i = 0; i <= DAT_SEGMENTS_COUNT; i++) seg_next[i] = seg_prev[i] = NULL;    
  
  if (pitem && tex_table)
    animation = NEW TGUI_ANIMATION(tex_table->GetTexture(pitem->tg_stay_id, random_tex ? DAT_TEX_RANDOM : 0));
  else animation = NULL;
}


/**
 *  Destructor.
 *  The destructor takes off the projectile from segments lists.
 */
TDRAW_UNIT::~TDRAW_UNIT()
{
  if (animation) delete animation;
}


//=========================================================================
// Methods definitions of the class TPLAYER_UNIT.
//=========================================================================

/**
 *  Constructor.
 *  The constructor sets values given by parameters, other values are zeroized.
 *  If @param new_unit_id != 0, this value is set, else new unit_id is generated acording to @param global_unit.
 */
TPLAYER_UNIT::TPLAYER_UNIT(int set_player, int p_x, int p_y, int p_z, TDRAW_ITEM* set_item, int new_unit_id, bool global_unit)
:TDRAW_UNIT(p_x, p_y, p_z, set_item, &(players[set_player])->race->tex_table)
{
  player = players[set_player];
  PutState(US_NONE);

  prev = NULL;
  next = NULL;

  sound_request_id = waiting_request_id = 0;
  pevent = NULL;
  last_event_time_stamp = 0;
  
  if (new_unit_id != 0) {
    unit_id = new_unit_id;
    if (unit_id > 0)
      player->SetGlobalUnitCounter(unit_id);
    else
      player->SetLocalUnitCounter(unit_id);
  }
  else{
    if (global_unit)
      unit_id = player->IncrementGlobalUnitCounter();
    else
      unit_id = player->DecrementLocalUnitCounter();
  }

  player->hash_table_units.AddToHashTable(unit_id, this);
  player->AddUnit(this);
  have_order = false;
}


/**
 *  Constructor.
 *  Values are only zeroized.
 */
TPLAYER_UNIT::TPLAYER_UNIT()
{ 
  player = NULL;
  PutState(US_NONE);

  prev = NULL;
  next = NULL;
  
  unit_id = 0;  // this is necessary for creaing units in segments

  sound_request_id = waiting_request_id = 0;
  pevent = NULL;
  last_event_time_stamp = 0;

  have_order = false;
}


/**
 *  Destructor.
 *  The destructor takes event from queue and unit from units hash table.
 */
TPLAYER_UNIT::~TPLAYER_UNIT()
{
  if (pevent)
  {
    queue_events->GetEvent(pevent);  //delete event from queue
    delete pevent;
    pevent = NULL;
  }
  
  if (player) {
    player->hash_table_units.RemoveFromHashTable(this->unit_id); //remove hash unit
    player->DeleteUnit(this);
  }
}

/**
 *  Send request message to owner.
 *
 *  @notes If n_request_id != 0 then requestid in event is set to n_requestid else new requestid is generated
 */
int TPLAYER_UNIT::SendRequest(bool n_priority, double n_time_stamp, int n_event, int n_request_id, T_SIMPLE n_simple1, T_SIMPLE n_simple2, T_SIMPLE n_simple3, T_SIMPLE n_simple4, T_SIMPLE n_simple5, T_SIMPLE n_simple6, int n_int1,int n_int2)
{
  TEVENT * hlp;
  int ret = 0;

  hlp = pool_events->GetFromPool(); // get new (clear) event from pool
  hlp->SetEventProps(GetPlayerID(), GetUnitID(), n_priority, n_time_stamp, n_event, US_NONE, n_request_id, n_simple1, n_simple2, n_simple3, n_simple4, n_simple5, n_simple6, n_int1, n_int2);

  ret = hlp->GetRequestID();

  // if owner is remote, send request to his local computer
  if (player_array.IsRemote(GetPlayerID())){
    
    SendNetEvent(hlp, GetPlayerID());

    pool_events->PutToPool(hlp);
  }
  else
    queue_events->PutEvent(hlp); // put event to queue

  return ret;
}

/**
 *  Send request message to owner to local computer.
 *
 *  @notes If n_request_id != 0 then requestid in event is set to n_requestid else new requestid is generated
 */
TEVENT * TPLAYER_UNIT::SendRequestLocal(bool n_priority, double n_time_stamp, int n_event, int n_request_id, T_SIMPLE n_simple1, T_SIMPLE n_simple2, T_SIMPLE n_simple3, T_SIMPLE n_simple4, T_SIMPLE n_simple5, T_SIMPLE n_simple6, int n_int1,int n_int2)
{
  TEVENT * hlp;

  hlp = pool_events->GetFromPool(); // get new (clear) event from pool
  hlp->SetEventProps(GetPlayerID(), GetUnitID(), n_priority, n_time_stamp, n_event, US_NONE, n_request_id, n_simple1, n_simple2, n_simple3, n_simple4, n_simple5, n_simple6, n_int1, n_int2);
  queue_events->PutEvent(hlp); // put event to queue

  return hlp;
}


/**
 *  Send event message to owner.
 *
 *  @param event Event which will be sent.
 *  @param player_id identificator of player. (ALL_PLAYERS is to all)
 */

void TPLAYER_UNIT::SendNetEvent(TEVENT * event, int player_id)
{
  int size; // size of data block
  char data[128]; // data block
  
  TNET_MESSAGE *msg = pool_net_messages->GetFromPool();
  msg->Init_send(net_protocol_event, 0);
  size = event->LinearizeEvent(data); //linearize event to data block
  
  msg->Pack(data, size);

  //send message
  host->SendMessage(msg, player_id);

  #if DEBUG_EVENTS
    Debug(LogMsg("TO N: P:%d U:%d E:%s RQ:%d X:%d Y:%d Z:%d R:%d TS:%f COUNT:%d", event->GetPlayerID(), event->GetUnitID(), EventToString(event->GetEvent()), event->GetRequestID(), event->simple1, event->simple2, event->simple3, event->simple4, event->GetTimeStamp(), queue_events->GetQueueLength()));
  #endif
}


/**
 *  Return owner's player ID.
 *
 *  @notes Every draw unit have an owner. If the unit is one of resources or a special unit such as pigs, wolves or sheep
 *         then owner is implicit owner number zero.
 */

T_BYTE TPLAYER_UNIT::GetPlayerID() const 
{
  return player->GetPlayerID();
}


void TPLAYER_UNIT::TestVisibility()
{
  if (player == myself || show_all) visible = true;
  else visible = myself->GetLocalMap()->GetAreaVisibility(pos, GetUnitWidth(), GetUnitHeight());
}


//=========================================================================
// Methods definitions of class TBASIC_UNIT.
//=========================================================================

/**
 *  Constructor.
 *
 *  @param uplayer      Player ID of unit owner.
 *  @param ux           X coordinate of unit.
 *  @param uy           Y coordinate of unit.
 *  @param uz           Z coordinate of unit.
 *  @param mi           Pointer to the unit kind.
 *  @param new_unit_id  if new_unit_id != 0, unit_id = new_unit_id, else new unique unit_id is generated according to @param global_unit
 */
TBASIC_UNIT::TBASIC_UNIT(int uplayer, int ux, int uy, int uz, TBASIC_ITEM *mi, int new_unit_id, bool global_unit)
:TMAP_UNIT(uplayer, ux, uy, uz, mi, new_unit_id, global_unit)
{
  has_view = false;
}


/**
 *  Destructor
 */
TBASIC_UNIT::~TBASIC_UNIT()
{
  ;
}


/**
 *  Signify all the mapels in local map that are seen by the unit - that means, that their
 *  distance is <= view of the unit, uses IsSeenByUnit
 *
 *  @param  set   If true, view area will be shown (set) else will be hidden (unset).
 */
void TBASIC_UNIT::SetView(bool set)  //sets view to the unit
{
  TBASIC_ITEM* kind = static_cast<TBASIC_ITEM*>(pitem);
  TGUN *p_gun = kind->GetArmament()->GetOffensive();
  int view = kind->view;
  int seg_num_min = kind->visible_segments[pos.segment].min;
  int seg_num_max = kind->visible_segments[pos.segment].max;
  int u_width = GetUnitWidth();
  int u_height = GetUnitHeight();
  TLOC_MAP *local_map = GetPlayer()->GetLocalMap();
  T_SIMPLE map_w = map.width + MAP_AREA_SIZE + 1;
  int x_start = pos.x - view;  //starting point in the local map
  int y_start = pos.y - view; 
  int x_stop = pos.x + u_width + view;
  int y_stop = pos.y + u_height + view;
  int tex_id;
  T_SIMPLE range_min = 0;
  T_SIMPLE range_max = 0;
  int aim_seg_num_min = 0;
  int aim_seg_num_max = 0;

  for (int i = x_start; i < x_stop ; i++)
    for (int j = y_start; j < y_stop ; j++)
    {
      if ((i >= 0 && i < map.width + MAP_AREA_SIZE && j >= 0 && j < map.height + MAP_AREA_SIZE))
      {
        if (IsSeenByUnit(pos, i, j, u_width, u_height, view)) {
          tex_id = (j + 1) * map_w * 4 + (i + 1) * 4;

          //if the position is in the map and if field is not so far from the actual field, that unit stands on (Pythagora's theorem used here)
          for (int k = seg_num_max; k >= seg_num_min; k--)
          {

            // area will be shown
            if (set) 
            {
              if (local_map->map[k][i][j].state == WLK_UNKNOWN_AREA) {
                local_map->map[k][i][j].state = 1;       //set visibility
                if (player == myself) {
                  map.war_fog.tex[DRW_ALL_SEGMENTS][tex_id + 0] = map.war_fog.tex[k][tex_id + 0] = config.pr_warfog_color[0];
                  map.war_fog.tex[DRW_ALL_SEGMENTS][tex_id + 1] = map.war_fog.tex[k][tex_id + 1] = config.pr_warfog_color[1];
                  map.war_fog.tex[DRW_ALL_SEGMENTS][tex_id + 2] = map.war_fog.tex[k][tex_id + 2] = config.pr_warfog_color[2];
                  map.war_fog.tex[DRW_ALL_SEGMENTS][tex_id + 3] = map.war_fog.tex[k][tex_id + 3] = 0;
                }
              }
              else 
              {
                local_map->map[k][i][j].state++;          //set visibility
                if (player == myself && local_map->map[k][i][j].state == 1) {
                  map.war_fog.tex[k][tex_id + 3] = 0;
                }
              }

              if (map.IsInMap(i, j)) 
              {
                if (p_gun != NULL) map.segments[k].surface[i][j].GetWatchersList()->AddNode(this);
                local_map->map[k][i][j].terrain_id = map.segments[k].surface[i][j].t_id;

                if (map.segments[k].surface[i][j].unit)   //if there's any unit on this field
                {
                  int pl_id = map.segments[k].surface[i][j].unit->GetPlayerID();

                  if (pl_id == -1)    /////!!! toto je divne lebo GetPlayerID vracia T_BYTE [PPP]
                    local_map->map[k][i][j].player_id = 254;
                  else
                    local_map->map[k][i][j].player_id = pl_id;
                }
              }
            } // if set

            // area will be hidden
            else 
            {
              if (map.IsInMap(i, j))
                if (p_gun != NULL) map.segments[k].surface[i][j].GetWatchersList()->RemoveNode(this);

              if (local_map->map[k][i][j].state > 0) 
                local_map->map[k][i][j].state -= 1;
              /*  OFIK: Nesynchronizovane is_in_map
              else {
                Critical("!!!!!!!!!!!!!!!!!!!!!");
              }
              */

              if (!local_map->map[k][i][j].state) {
                if (player == myself) {
                  map.war_fog.tex[k][tex_id + 3] = config.pr_warfog_color[3];
                }

                local_map->map[k][i][j].player_id = WLK_EMPTY_FIELD;
              }
            }
          } // for k

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
      } 
    } // for i, j

  if (p_gun != NULL) 
  {
    range_min = p_gun->GetRange().min;
    range_max = p_gun->GetRange().max;
    aim_seg_num_min = p_gun->GetShotableSegments().min;
    aim_seg_num_max = p_gun->GetShotableSegments().max;

    x_start = pos.x - range_max;  //starting point in the local map
    y_start = pos.y - range_max; 
    x_stop = pos.x + u_width + range_max;
    y_stop = pos.y + u_height + range_max;

    for (int i = x_start; i < x_stop ; i++)
      for (int j = y_start; j < y_stop ; j++)
      {
        if (map.IsInMap(i, j) && IsAimableByUnit(pos, i, j, u_width, u_height, range_min, range_max))
        {
          for (int k = aim_seg_num_max; k >= aim_seg_num_min; k--)
          {
            if (set)
              map.segments[k].surface[i][j].GetAimersList()->AddNode(this);
            else
              map.segments[k].surface[i][j].GetAimersList()->RemoveNode(this);
          }
        }
      }
  }
    

  has_view = set;
}


/**
 *  The methods tests whether distance between field with coordinates [x_test][y_test] and some part of unit is less or equal than unit's view.
 *  For comparing distance is using the Pythagoras theorem.
 *
 *  @param pos      unit position
 *  @param x_test   first coordinate of tested field
 *  @param y_test   second coordinate of tested field
 *  @param u_width  width of the unit
 *  @param u_height height of the unit
 *  @param view     view of the unit
 */
bool TBASIC_UNIT::IsSeenByUnit(TPOSITION pos, int x_test, int y_test, int u_width, int u_height, int view)
{
  int x_stop = pos.x + u_width;
  int y_stop = pos.y + u_height;  

  for (register int i = pos.x; i < x_stop; i++)     //test to each part of unit
    for (register int j= pos.y; j<y_stop; j++)
    {
      if ((x_test - i) * (x_test - i) + (y_test - j) * (y_test - j) <= view*view)   //some part can see the field
        return true;
    }
  return false;       //field is too far away
}


void TBASIC_UNIT::ShowNeedFood()
{
  sign_animation = player->need_animation[0];
}


void TBASIC_UNIT::ShowNeedEnergy()
{
  sign_animation = player->need_animation[1];
}


void TBASIC_UNIT::ShowNeedMaterial(T_BYTE mat)
{
  sign_animation = player->need_animation[2 + mat];
}


void TBASIC_UNIT::ShowNeedElement(T_BYTE id)
{
  sign_animation = player->need_animation[id];
}


/**
 *  The methods tests whether distance between field with coordinates [x_test][y_test] 
 *  and some part of unit is in the range of the unit.
 *  For comparing distance is using the Pythagoras theorem.
 *
 *  @param pos        unit position
 *  @param x_test     first coordinate of tested field
 *  @param y_test     second coordinate of tested field
 *  @param u_width    width of the unit
 *  @param u_height   height of the unit
 *  @param range_min  minimal range of the unit
 *  @param range_max  maximal range of the unit
 */
bool TBASIC_UNIT::IsAimableByUnit(TPOSITION unit_pos, int x_test, int y_test, int u_width, int u_height, int range_min, int range_max)
{
  int x_stop = unit_pos.x + u_width;
  int y_stop = unit_pos.y + u_height;

  int square_range_min = range_min*range_min;
  int square_range_max = range_max*range_max;
  int dist = 0;

  for (int i = unit_pos.x; i < x_stop; i++)     //test to each part of unit
    for (int j = unit_pos.y; j < y_stop; j++)
    {
      dist = (x_test - i) * (x_test - i) + (y_test - j) * (y_test - j);
      if ((dist >= square_range_min) && (dist <= square_range_max))   //some part can aim the field
        return true;
      else if ((square_range_min == 1) && (abs(x_test - i) <= 1) && (abs(y_test - j) <= 1))
        return true;
    }
  return false;       //field is too far away
}

/**
 *  Method checks if position tx, ty is in distance radius_min to radius_max from unit.
 *
 *  @param tx  Tested position (x).
 *  @param ty  Tested position (y).
 *  @param radius_min Min distance from unit.
 *  @param radius_max Max distance from unit.
 *
 *  @return true if position tx,ty is in distance from radus_min to radius_max.
 */
bool TBASIC_UNIT::IsGoodDistance(int tx, int ty, int radius_min, int radius_max)
{
  int stop_x = pos.x + this->GetPointerToItem()->GetWidth();
  int stop_y = pos.y + this->GetPointerToItem()->GetHeight();

  for (register int i = pos.x; i < stop_x; i++)     //test to each part of unit
    for (register int j = pos.y; j < stop_y; j++)
    {
      if (((tx - i) * (tx - i) + (ty - j) * (ty - j) <= radius_max*radius_max) &&
          ((tx - i) * (tx - i) + (ty - j) * (ty - j) >= radius_min*radius_min))
        return true;
    }
  return false;       //field is too far away or too close;
}

/**
 *  The method checks neighbourhood of the unit to start attack to enemy.
 *
 *  @param previous The previous target which attacking has failed.
 *
 *  @return The method returns pointer to enemy unit if it is in the units
 *  neighbourhood some enemy. The size of the neighbourhood is dependable to
 *  units.
 */
TMAP_UNIT* TBASIC_UNIT::CheckNeighbourhood(TMAP_UNIT *previous)
{
  short radius_min = 0, radius_max = 0;
  T_SIMPLE bottom = 0, top = 0;

  TARMAMENT *armam = static_cast<TMAP_ITEM*>(GetPointerToItem())->GetArmament();
  if (armam->GetOffensive() == NULL)
    return NULL;
  else if (dynamic_cast<TBASIC_ITEM*>(GetPointerToItem())->view < armam->GetOffensive()->GetRange().min)
    return NULL;
  else if (GetAggressivity() == AM_AGGRESSIVE) 
  {
    TBASIC_ITEM *bitem = dynamic_cast<TBASIC_ITEM*>(GetPointerToItem());
    radius_max = bitem->view;
    radius_min = armam->GetOffensive()->GetRange().min;
    bottom = bitem->visible_segments[GetPosition().segment].min;
    top = bitem->visible_segments[GetPosition().segment].max;
  } 
  else if ((GetAggressivity() == AM_OFFENSIVE) || (GetAggressivity() == AM_GUARDED)) 
  {
    radius_max = armam->GetOffensive()->GetRange().max;
    radius_min = armam->GetOffensive()->GetRange().min;
    bottom = armam->GetOffensive()->GetShotableSegments().min;
    top = armam->GetOffensive()->GetShotableSegments().max;
  }

  for (int r = 0; r < radius_max; r++) 
  {
    short tx, ty;
    for (int i = 0; i < (GetUnitWidth() + 2*r); i++)
    {
      tx = GetPosition().x - r + i;
      ty = GetPosition().y - r;
      if (map.IsInMap(tx, ty) && (IsGoodDistance(tx, ty, radius_min, radius_max)))
      {
        for (int s = top; s >= bottom; s--)
        {
          TMAP_UNIT *unit = map.segments[s].surface[tx][ty].unit;
          if ((unit != NULL) && (unit != previous) && (unit->GetPlayer() != GetPlayer()) && (unit->GetPlayerID() != 0))
          {
            TATTACK_INFO attack_info = armam->IsPossibleAttack(this, unit);   //tests possibility of attack
            if (attack_info.state == FIG_AIF_ATTACK_OK || attack_info.state == FIG_AIF_TOO_FAR_AWAY)
              return unit;
          }
        }
      }
      tx = GetPosition().x + GetUnitWidth() - 1 + r - i;
      ty = GetPosition().y + GetUnitHeight() - 1 + r;
      if (map.IsInMap(tx, ty) && (IsGoodDistance(tx, ty, radius_min, radius_max)))
      {
        for (int s = top; s >= bottom; s--)
        {
          TMAP_UNIT *unit = map.segments[s].surface[tx][ty].unit;
          if ((unit != NULL) && (unit != previous) && (unit->GetPlayer() != GetPlayer()) && (unit->GetPlayerID() != 0))
          {
            TATTACK_INFO attack_info = armam->IsPossibleAttack(this, unit);   //tests possibility of attack
            if (attack_info.state == FIG_AIF_ATTACK_OK || attack_info.state == FIG_AIF_TOO_FAR_AWAY)
              return unit;
          }
        }
      }
    }

    for (int j = 0; j < (GetUnitHeight() + 2*(r-1)); j++)
    {
      tx = GetPosition().x - r;
      ty = GetPosition().y - r + j + 1;
      if (map.IsInMap(tx, ty) && (IsGoodDistance(tx, ty, radius_min, radius_max)))
      {
        for (int s = top; s >= bottom; s--)
        {
          TMAP_UNIT *unit = map.segments[s].surface[tx][ty].unit;
          if ((unit != NULL) && (unit != previous) && (unit->GetPlayer() != GetPlayer()) && (unit->GetPlayerID() != 0))
          {
            TATTACK_INFO attack_info = armam->IsPossibleAttack(this, unit);   //tests possibility of attack
            if (attack_info.state == FIG_AIF_ATTACK_OK || attack_info.state == FIG_AIF_TOO_FAR_AWAY)
              return unit;
          }
        }
      }
      tx = GetPosition().x + GetUnitWidth() - 1 + r;
      ty = GetPosition().y + GetUnitHeight() - 2 + r - j;
      if (map.IsInMap(tx, ty) && (IsGoodDistance(tx, ty, radius_min, radius_max)))
      {
        for (int s = top; s >= bottom; s--)
        {
          TMAP_UNIT *unit = map.segments[s].surface[tx][ty].unit;
          if ((unit != NULL) && (unit != previous) && (unit->GetPlayer() != GetPlayer()) && (unit->GetPlayerID() != 0))
          {
            TATTACK_INFO attack_info = armam->IsPossibleAttack(this, unit);   //tests possibility of attack
            if (attack_info.state == FIG_AIF_ATTACK_OK || attack_info.state == FIG_AIF_TOO_FAR_AWAY)
              return unit;
          }
        }
      }
    }
  }

  return NULL;
}


//=========================================================================
// Methods definitions of class TPROJECTILE_UNIT.
//=========================================================================


/** 
 *  Constructor.
 *  The projectile is "born" as invisible.
 *  @param playerID Player ID
 *  @param time Time lefted to the impact.
 *  @param spos  Start position.
 *  @param ipos  Impact position.
 *  @param g  Pointer to gun which shot off the projectile.
 */
TPROJECTILE_UNIT::TPROJECTILE_UNIT(int playerID, double time, TPOSITION_3D spos, TPOSITION_3D ipos, TGUN* g, int new_unit_id, bool global_unit)
: TPLAYER_UNIT(playerID, spos.x, spos.y, spos.segment, g->GetProjectileItem(), new_unit_id, global_unit), impact_time(time), impact_pos(ipos), move_shift(0), start_pos(spos)
{
  gun = g;
  is_shotted = false;
  flying_up = true;

  rpos_x = static_cast<float>(spos.x) + 0.5f - static_cast<float>(g->GetProjectileItem()->GetWidth())/2.0f;
  rpos_y = static_cast<float>(spos.y) + 0.5f - static_cast<float>(g->GetProjectileItem()->GetHeight())/2.0f;
  
  CalculateAngles();
  SetSegmentTime(GetImpactTime()/(GetPosition().segment - GetImpactPos().segment + 1));

  if (animation && player->race->tex_table.groups[pitem->tg_stay_id].count >= 8)
    animation->SetTexItem(player->race->tex_table.GetTexture(pitem->tg_stay_id, direction));
}


/** 
 *  Calculates sinus and cosinus of the moving angle. 
 */
double TPROJECTILE_UNIT::CalculateAngles()
{
  double angle, mangle;

  angle = GetPosition().GetAngle(GetImpactPos());
  direction = static_cast<TPOSITION>(GetPosition()).GetDirection(angle);

  mangle = angle + PI * 0.25;     //calculate moving angle

  //calculate sinus and cosinus of the moving
  msin = static_cast<float>(sin(angle));
  mcos = static_cast<float>(cos(angle));

  return angle;
}

/**
 *  Method is called for periodically updating of look and features.
 *
 *  @param time_shift  Time shift from the last update.
 */
bool TPROJECTILE_UNIT::UpdateGraphics(double time_shift)
{
  TPROJECTILE_ITEM *pitem = static_cast<TPROJECTILE_ITEM*>(GetPointerToItem());

  TPLAYER_UNIT::UpdateGraphics(time_shift);

  if (IsShotted() && move_shift < impact_time)   //projectile doesn't impact yet
  {
    move_shift += time_shift;
    if (move_shift > impact_time) move_shift = impact_time;

    double s = pitem->GetSpeed() * move_shift;

    // compute new real position
    SetRealPositionX(static_cast<float>(start_pos.x + mcos * s));
    SetRealPositionY(static_cast<float>(start_pos.y + msin * s));

    //change integer position if was changed
    if (abs(static_cast<int>(rpos_x - pos.x)) >= 1)
      pos.x = static_cast<T_SIMPLE>(rpos_x);
    if (abs(static_cast<int>(rpos_y - pos.y)) >= 1)
      pos.y = static_cast<T_SIMPLE>(rpos_y);

    seg_time -= time_shift;

    if (seg_time <= 0 && pos.segment != impact_pos.segment)    //projectile moves into next segment
    {
      pos.segment += (pos.segment < impact_pos.segment)? 1 : -1;
      if (pos.segment == impact_pos.segment)    //impact segment
        seg_time = impact_time - move_shift;
      else
        seg_time = impact_time / (abs(start_pos.segment - impact_pos.segment) + 1);
    }
  }

  return false;
}

/**
 *  Deletes unit from all places.
 *  This is called on remote computer when player disconnect game.
 */
void TPROJECTILE_UNIT::Disconnect()
{
  DeleteFromSegments();
  UnitToDelete(false);
}


/** 
 *  Called when projectile is impacting.
 *  The method spread explosion around impact position.
 */
void TPROJECTILE_UNIT::Impact()
{
  TPROJECTILE_ITEM *pitem = static_cast<TPROJECTILE_ITEM*>(GetPointerToItem());
  int border = Sqr(pitem->GetScope());
  int r = static_cast<int>(impact_pos.x) - pitem->GetScope();
  int s = static_cast<int>(impact_pos.y) - pitem->GetScope();

  for (int i = -pitem->GetScope(); i <= pitem->GetScope(); i++, r++)   //look over the surrounding of the impact position
  {
    for (int j = -pitem->GetScope(); j <= pitem->GetScope(); j++, s++)
    {
      if (::map.IsInMap(r, s) && ((Sqr(i) + Sqr(j)) <= border))
      {
        TMAP_UNIT* affected = ::map.segments[impact_pos.segment].surface[r][s].unit;
        //field is in the radius of the explosion and there is my unit
        if ((affected != NULL) && (!player_array.IsRemote(affected->GetPlayerID())))
          if (affected->AcquirePointer())
          {
            gun->ExecuteAttack(affected);    //try to hit present unit
            affected->ReleasePointer();
          }
      }
    }
    s = static_cast<int>(impact_pos.y) - pitem->GetScope();
  }

#if SOUND
      if (visible) {
        PlaySound(&static_cast<TPROJECTILE_ITEM *>(pitem)->snd_hit, 2);
      }
#endif
}


/**
 *  Method is calling when new event is get from queue. Returns true if unit does something.
 *
 *  @param Pointer to processed event.
 */
void TPROJECTILE_UNIT::ProcessEvent(TEVENT *proc_event)
{
  /****************** Put state of event ********************************/
 
  /****************** Synchronisation and setup *************************/
  
  /****************** Compute new values (tupe vykonanie spravy) ********************************/

  // RQ_CHANGE_SEGMENT
  if (proc_event->TestEvent(RQ_CHANGE_SEGMENT))
  {
    if (GetStartPos().segment > GetImpactPos().segment) 
    {
      if (GetPosition().segment > 0) 
        SetPosition(GetPosition() - TPOSITION_3D(0,0,1));
    } 
    else
    {
      if (GetPosition().segment < 2) 
        SetPosition(GetPosition() + TPOSITION_3D(0,0,1));
    }
  }


  // RQ_IMPACT
  if (proc_event->TestEvent(RQ_IMPACT))
  {
    Impact();
    DeleteFromSegments();
    UnitToDelete(true);
    return;
  }
  
  /****************** Compute next event (rozhodovanie a planovanie) *******************************/
  
  if (!player_array.IsRemote(proc_event->GetPlayerID())) { // compute only not remote (local) units
    ;
  }
}


//=========================================================================
// END
//=========================================================================
// vim:ts=2:sw=2:et:

