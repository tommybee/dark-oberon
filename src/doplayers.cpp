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
 *  @file doplayers.cpp
 *
 *  Working with players.
 *
 *  @author Martin Kosalko
 *  @author Jiri Krejsa
 *  @author Michal Kral
 *
 *  @date 2003, 2004, 2005
 */


//=========================================================================
// Included files
//=========================================================================

#include "cfg.h"

#include <cmath>
#include <cstdlib>
#include <string>
#include <list>

#include "doplayers.h"
#include "doengine.h"

//using std::string;
//using std::list;
using namespace std;

//=========================================================================
// Global variables
//=========================================================================

TPLAYER **players = NULL;     //!< Array of all players that could play in actual map.
TPLAYER *myself = NULL;      //!< Pointer to my instance of player structure.
TPLAYER *hyper_player = NULL;       //!< Pointer to instance of hyper player structure.
TPLAYER_ARRAY player_array; 

//=========================================================================
// class THASHTABLE_UNITS
//=========================================================================

//!< Constructor.
THASHTABLE_UNITS::THASHTABLE_UNITS(void)
{
  for (int i = 0; i < PL_HASHTABLE_UNITS_SIZE; i++)
    table[i] = NULL;
}


//!< Destructor.
THASHTABLE_UNITS::~THASHTABLE_UNITS(void)
{
  THASH_UNIT * act = NULL, * bef = NULL;

  for (int i = 0; i < PL_HASHTABLE_UNITS_SIZE; i++)
    if(table[i]){
      for (bef = table[i], act = bef->next; act != NULL; act = act->next){
         delete bef;
         bef = act;
      }
      delete bef;
      table[i] = NULL;
    }
}


/** 
 *  Add new hash unit to table (if unit is not in table).
 *
 *  @param a_unit_id unique identificator of unit.
 *  @param a_player_unit pointer to added unit.
 *
 *  @sa TPLAYER_UNIT, THASH_UNIT
 */
void THASHTABLE_UNITS::AddToHashTable(int a_unit_id, TPLAYER_UNIT * a_player_unit)
{
  int hash_index;
  THASH_UNIT * act = NULL, * bef = NULL, * new_unit;

  hash_index = HashFunction(a_unit_id);

  // finds correct place to add new THASH_UNIT
  for ( act = bef = table[hash_index]; ((act != NULL) && (act->unit_id < a_unit_id)); bef = act, act = act->next );

  // if unit with identificator exists in table, return
  if ((act) && (act->unit_id == a_unit_id)) return;

  // create new THASH_UNIT and add it to list
  new_unit = NEW THASH_UNIT(a_unit_id, a_player_unit); 

  #if DEBUG_HASHTABLE_UNITS
    Debug(LogMsg("ADDED UID1:%d UID2:%d PID:%d", a_unit_id, a_player_unit->GetUnitID(), a_player_unit->GetPlayerID()));
  #endif
  
  new_unit->next = act;
  
  // if exists any THASH_UNIT in list
  if (bef){
    if (bef == act) //unit is first in list 
      table[hash_index] = new_unit;
    else
      bef->next = new_unit;
  }
  else
    table[hash_index] = new_unit;
}

/**
 *  Removes THASH_UNIT specified in @param r_player_unit by identificator of unit.
 *  @sa THASH_UNIT
 */
void THASHTABLE_UNITS::RemoveFromHashTable(int r_unit_id)
{
  int hash_index;
  THASH_UNIT * act = NULL, * bef = NULL;

  hash_index = HashFunction(r_unit_id);
  
  // finds removed THASH_UNIT
  for (act = bef = table[hash_index]; ((act != NULL) && (act->unit_id < r_unit_id)); bef = act, act = act->next );

  // test unit
  if (act){
    if (act->unit_id == r_unit_id){ // act will be deleted
      if (act == bef){ // act id first in list
        table[hash_index] = act->next;
        delete act;
      }
      else{ // act is not first in list
        bef->next = act->next;
        delete act;
      }
    }
  }
  
  #if DEBUG_HASHTABLE_UNITS
    Debug(LogMsg("REMOVED UID:%d PID:??", r_unit_id));
  #endif
}

/**
 *  Returns pointer to unit specifird in @param g_unit_id.
 *
 *  If unit with identificator @param g_unit_id does not exists, returns NULL.
 *  @sa TPLAYER_UNIT
 */
TPLAYER_UNIT * THASHTABLE_UNITS::GetUnitPointer(int g_unit_id)
{
  int hash_index;
  THASH_UNIT * act = NULL, * bef = NULL;

  hash_index = HashFunction(g_unit_id);

  // finds THASH_UNIT
  for ( act = bef = table[hash_index]; ((act != NULL) && (act->unit_id < g_unit_id)); bef = act, act = act->next );

  // test unit
  if ((act) && (act->unit_id == g_unit_id)){
    return act->player_unit;
  }
  
  #if DEBUG_EVENTS || DEBUG_HASHTABLE_UNITS
    Warning(LogMsg("Does not exist unit with requested unit_id: U:%d", g_unit_id));
  #endif

  // in case that unit with requested id doesn't exists return NULL
  return NULL;
}

/**
 *  Returns identificator of unit specifird in @param g_player_unit.
 *
 *  If unit does not exists, returns 0.
 *  @sa TPLAYER_UNIT
 */
int THASHTABLE_UNITS::GetUnitID(TPLAYER_UNIT * g_player_unit)
{
  if (g_player_unit){
    return g_player_unit->GetUnitID();
  }
  else return 0;
}


//=========================================================================
// struct TPLAYER
//=========================================================================

/** Constructor. */
  
TPLAYER::TPLAYER()
{
  int i;

  *name = 0;
  player_id = 0;
  race = NULL;
  active = false;
  global_unit_counter = 0;
  local_unit_counter = 0;
  request_counter = 0;
  player_units_counter = 0;

  units = NULL;

  energy_in = energy_out = 0;
  food_in = food_out = 0;

  initial_x = 0;
  initial_y = 0;
  
  pathtools = NULL;

  for (i = 0; i < SCH_MAX_MATERIALS_COUNT; i++)   
    stored_material[i] = 0;  

  // global animations
  for (i = 0; i < SCH_MAX_MATERIALS_COUNT + 2; i++)
    need_animation[i] = NULL;

  build_item = NULL;
  update_info = true;
  SetPlayerType(PT_HUMAN);

  // create mutex
  if ((mutex = glfwCreateMutex ()) == NULL) {
    Critical ("Could not create player mutex");
  }
}


/** Destructor. */
TPLAYER::~TPLAYER(void)
{
  if (pathtools) delete pathtools;

  //while (units) delete units;
  while (units)
  {
    delete units;
  }

  // global animations
  for (int i = 0; i < SCH_MAX_MATERIALS_COUNT + 2; i++)
    if (need_animation[i]) delete need_animation[i];

  glfwDestroyMutex(mutex);
}


/**
 *  Add unit into player's list of map units.
 */
void TPLAYER::AddUnit(TPLAYER_UNIT *punit)
{

  glfwLockMutex(mutex);

  if (units) {
    punit->SetNext(units);
    units->SetPrev(punit);
  }

  units = punit;


  glfwUnlockMutex(mutex);
}


/**
 *  Remove unit from player's list of map units.
 */
void TPLAYER::DeleteUnit(TPLAYER_UNIT *punit)
{
  glfwLockMutex(mutex);

  if (punit == units) units = punit->GetNext();

  if (punit->GetNext()) punit->GetNext()->SetPrev(punit->GetPrev());
  if (punit->GetPrev()) punit->GetPrev()->SetNext(punit->GetNext());

  glfwUnlockMutex(mutex);
}


void TPLAYER::Disconnect(void)
{
  TPLAYER_UNIT *unit = NULL;

  glfwLockMutex(mutex);

  for (unit = units; unit; unit = unit->GetNext()) {
      unit->Disconnect();
  }

  glfwUnlockMutex(mutex);
}


void TPLAYER::UpdateGraphics(double time_shift)
{
  TPLAYER_UNIT *unit = NULL;
  TMAP_UNIT *ghost = NULL;
  int i;

  list<TMAP_UNIT *> ghost_list;
  list<TMAP_UNIT *>::const_iterator iter;

  glfwLockMutex(delete_mutex);
  glfwLockMutex(mutex);

  // units
  for (unit = units; unit; unit = unit->GetNext()) 
  {
    if (unit->UpdateGraphics(time_shift)) {
      ghost = ((TMAP_UNIT *)unit)->AcquirePointer();
      if (ghost) ghost_list.push_back(ghost);
    }
  }

  glfwUnlockMutex(mutex);
  glfwUnlockMutex(delete_mutex);

  // creating and deleting ghosts
  for (iter = ghost_list.begin(); iter != ghost_list.end(); iter++) {
    if ((*iter)->IsGhost()) (*iter)->DestroyGhost();
    else (*iter)->CreateGhost();

    (*iter)->ReleasePointer();
  }

  ghost_list.clear();

  // global animations
  if (this == myself) 
  {
    for (i = 0; i < 2 + scheme.materials_count; i ++)
      if (need_animation[i]) need_animation[i]->Update(time_shift);
  }
}


/**
 *  Updates local map for building.
 *
 *  @param position     Building position.
 *  @param pitem        Pointer to building data.
 *  @param my_building  Specifies whether building belongs to player.
 *  @param player_ID    Player identifier.
 *  @param built        Specifies whether building is built or destroyed.
 */
void TPLAYER::UpdateLocalMap(const TPOSITION_3D position, TBUILDING_ITEM* const pitem, const bool my_building, const T_SIMPLE player_ID, const bool built)
{
  signed char coeficient = 0;
  if (built)
    coeficient = MAP_BUILDING_COEF;
  else
    coeficient = -MAP_BUILDING_COEF;

  int x, y;
  for (int i = 0; i < pitem->GetWidth(); i++)
    for (int j = 0; j < pitem->GetHeight(); j++)
    {
      x = position.x + i;
      y = position.y + j;
      for (int s = pitem->GetExistSegments().min; s <= pitem->GetExistSegments().max; s++)
      {
        bool seen = local_map.IsVisibleArea(s, x, y);  //whether is actual field visible

        if (seen || my_building)
          local_map.map[s][x][y].player_id = player_ID;
        if (my_building)     //in my local map update all fields with building
        {
          local_map.map[s][x][y].terrain_id += coeficient;
        }
        else            //in enemy map update only visible fields
        {        
          if (seen)  //the field is visible for the player, not a warfog or unknown area
            local_map.map[s][x][y].terrain_id += coeficient;
        }
      } //for s
    } // for i, j
}


/** 
 *  Updates local map for new built map units.
 *
 *  @param position     Unit position.
 *  @param pitem        Pointer to unit data.
 *  @param player_id    Player identifier.
 */
void TPLAYER::UpdateLocalMap(const TPOSITION_3D position, const TFORCE_ITEM *pitem, const T_SIMPLE player_id)
{
  if (local_map.GetAreaVisibility(position, pitem->GetWidth(), pitem->GetHeight()))  //the field is visible for the player
  {
    local_map.UnitFillPosition(position, pitem->GetWidth(), player_id);
  }
  else 
  {
    local_map.UnitLeftPosition(position, pitem->GetWidth());
  }
}


/**
 *  Update local map during unit move.
 *
 *  @param old_pos    Old position of unit.
 *  @param new_pos    New position of unit.
 *  @param unit       Pointer to unit.
 */
void TPLAYER::UpdateLocalMap(const TPOSITION_3D old_pos, const TPOSITION_3D new_pos, const TFORCE_UNIT *unit)
{
  bool seen = false;

  if (unit->GetPlayer() == this)
  {
    local_map.UnitLeftPosition(old_pos, unit->GetUnitWidth());
    local_map.UnitFillPosition(new_pos, unit->GetUnitWidth(), unit->GetPlayerID());
  }
  else
  {
    seen = local_map.GetAreaVisibility(old_pos, unit->GetUnitWidth(), unit->GetUnitHeight());

    if (seen)
    {
      local_map.UnitLeftPosition(old_pos, unit->GetUnitWidth());
      seen = false;
    }

    seen = local_map.GetAreaVisibility(new_pos, unit->GetUnitWidth(), unit->GetUnitHeight());

    if (seen)
      local_map.UnitFillPosition(new_pos, unit->GetUnitWidth(), unit->GetPlayerID());
  }
}


/**
 *  The method updates local map for source. It is used during collapsing process.
 *
 *  @param position     Source position.
 *  @param pitem        Pointer to source item.
 *  @param player_ID    Player identifier.
 *  @param collapsing   Flag whether source is collapsing or renewing.
 */
void TPLAYER::UpdateLocalMap(const TPOSITION_3D position, TSOURCE_ITEM * const pitem, const T_SIMPLE player_ID, const bool collapsing)
{
  int i, j;
  signed char coeficient;
  unsigned char PID_to_set;
  bool my_source = (player_ID == GetPlayerID());

  if (collapsing)   //source is collapsing
  {
    coeficient = -MAP_BUILDING_COEF;
    PID_to_set = WLK_EMPTY_FIELD;
  }
  else    //source is renewing
  {
    coeficient = MAP_BUILDING_COEF;
    PID_to_set = player_ID;
  }

  for (i = position.x; i < position.x + pitem->GetWidth(); i++)
    for (j = position.y; j < position.y + pitem->GetHeight(); j++)
    {
      for (int s = pitem->GetExistSegments().min; s <= pitem->GetExistSegments().max; s++)
      {
        bool seen = local_map.IsVisibleArea(s, i, j);  //whether is actual field visible

        if (seen)
          local_map.map[s][i][j].player_id = PID_to_set;
        if (my_source)     //in my local map update all fields with source
        {
          local_map.map[s][i][j].terrain_id += coeficient;
        }
        else            //in enemy map update only visible fields
        {        
          if (seen)  //the field is visible for the player, not a warfog or unknown area
            local_map.map[s][i][j].terrain_id += coeficient;
        }
      } //for s
    } // for i, j
}


/**
 *  Increases the number of newly added buildings which accepts the same
 *  material, as the source can offer.
 *
 *  @param index      Type of material.
 *  @param player_id  Player id, id of the owner of the building.
 */
void TPLAYER::IncreaseBuildingCount(int mat_index,int player_id)
{
  TLIST<TSOURCE_UNIT>::TNODE<TSOURCE_UNIT> * source;  

  source = hyper_player->sources[mat_index].GetFirst();  //evidentne sa to sem neprida...
  while (source != NULL)  //cycle through the whole list of the sources,which belong to the "hyperplayer" (player with ID == 0)
  { 
    for (int i =0 ; i < this->race->workers_item_count[mat_index] ; i++)
      source->GetPitem()->GetPositionInPlayerArray(player_id)[i].new_buildings_count += 1;            
    source = source->GetNext();
  }
}


/**
 *  Add value of @param e to player's energy and @param f to player's food.
 */

void TPLAYER::AddUnitEnergyFood(int e, int f)
{
  if (e < 0) energy_out -= e;   // add unit which take energy
  else energy_in += e;          // add unit which supply energy

  if (f < 0) food_out -= f;     // add unit which take food
  else food_in += f;            // add unit which produce food

  update_info = true;
} 


/**
 *  Remove value of @param e from player's energy and @param f from player's food.
 */
void TPLAYER::RemoveUnitEnergyFood(int e, int f)
{
  if (e < 0) energy_out += e;   // remove unit which take energy
  else energy_in -= e;          // remove unit which supply energy

  if (f < 0) food_out += f;     // remove unit which take food
  else food_in -= f;            // remove unit which produce food

  update_info = true;
}

/**
 *  Reset have_order on all units to false
 */
void TPLAYER::ResetOrders()
{
  TPLAYER_UNIT * unit;
  for (unit = units; unit; unit = unit->GetNext())
    unit->ResetOrder();
}

/**
 *  Increment count of units of player.
 */
void TPLAYER::IncPlayerUnitsCount()
{
  player_units_counter++;
}

/**
 *  Decrement count of units of player.
 *  If active count is equal to 0, disconnect signal is sent.
 */
void TPLAYER::DecPlayerUnitsCount()
{
  player_units_counter--;

  //units count in 0 -> player lost war.
  if (!player_units_counter){
    active = false;

    if (this == myself){
      action_key = 0;
      gui->ShowMessageBox("You LOST!", GUI_MB_OK);
      won_lose = true;

      // send to other computers that I'm disconnecting
      /*
      for (int i = 0; i < player_array.GetCount(); i++){
        if (players[i]->active && !player_array.IsRemote(i)){
          host->SendDisconnect (i);
          players[i]->active = false;
        }
      }*/
      host->SendDisconnect(GetPlayerID());
    }
    else {
      bool exists_player = false;
      for (int i = 0; i < player_array.GetCount(); i++) {
        if ((players[i] != hyper_player) && (players[i] != myself) && (players[i]->active == true)){
          exists_player = true;
          break;
        }
      }

      if (!exists_player && !won_lose){
        action_key = 0;
        gui->ShowMessageBox("You WON!", GUI_MB_OK);
        won_lose = true;
      }
    }
  }
}


//=========================================================================
// class TLOC_MAP
//=========================================================================

TLOC_MAP::TLOC_MAP()
{
  map = NULL;
  depth  = WLK_SIZE_NOT_SET;
  width  = WLK_SIZE_NOT_SET;
  height = WLK_SIZE_NOT_SET;
  CreateLocalMap(::map.width + MAP_AREA_SIZE, ::map.height + MAP_AREA_SIZE);
}


TLOC_MAP::~TLOC_MAP()
{
  DeleteLocalMap();
}


/**
 *  Creates local map in the memory.
 *
 *  @param width  Width of local map.
 *  @param height Height of local map.
 *  @param depth  Depth of the local map (count of segments).
 */
void TLOC_MAP::CreateLocalMap(T_SIMPLE w, T_SIMPLE h, T_SIMPLE d)
{
  int k;

  //destroy map which exists before
  DeleteLocalMap();

  //allocate segments
  TLOC_MAP_FIELD ***set_map = NEW PLOC_MAP_FIELD*[d];
  if (set_map == NULL)
    return;
  
  for (int j = 0; j < d; j++)
  {
    set_map[j] = NEW PLOC_MAP_FIELD[w];
    if (set_map[j] == NULL)
    {
      for (k = j; k >= 0; k--)
        delete set_map[k];
      delete set_map;
      return;
    }
    for (int i = 0; i < w; i++)
    {
      set_map[j][i] = NEW TLOC_MAP_FIELD[h];
      if (set_map[j][i] == NULL)
      {
        //dealocating everything allocated yet
        for (k = 0; k < i; k++)
          delete set_map[j][k];
        for (k = j; j >= 0; k--)
          delete set_map[k];
        delete set_map;
        return;
      }    
    }
  }
  //set new values
  map = set_map;
  depth  = d;
  width  = w;
  height = h;

  return;
}


/**
 *  Deletes local map in the memory.
 */
void TLOC_MAP::DeleteLocalMap()
{
  if (map != NULL) 
  {
    for (int s = 0; s < depth; s++) 
      if (map[s]) 
      {
        for (int i = 0; i < width; i++)
          if (map[s][i]) 
            delete[] map[s][i];

      delete[] map[s];
      }

    delete[] map;
  }
  map = NULL;
  depth  = WLK_SIZE_NOT_SET;
  width  = WLK_SIZE_NOT_SET;
  height = WLK_SIZE_NOT_SET;
}



/**
 *  The method tests visibility of field.
 *
 *  @param segment Number of segment.
 *  @param x       First coordinate of field.
 *  @param y       Second coordinate of field.
 */
inline bool TLOC_MAP::IsVisibleArea(const int segment, const T_SIMPLE x, const T_SIMPLE y)
{
  return ((::map.IsInMap(x, y, segment)) &&(map[segment][x][y].state > WLK_WARFOG) && (map[segment][x][y].state < WLK_UNKNOWN_AREA));
}


/**
 *  The method tests visibility of field.
 *
 *  @param pos Number of segment.
 */
inline bool TLOC_MAP::IsVisibleArea(TPOSITION_3D pos)
{
  return ((::map.IsInMap(pos)) && (map[pos.segment][pos.x][pos.y].state > WLK_WARFOG) && (map[pos.segment][pos.x][pos.y].state < WLK_UNKNOWN_AREA));
}


/**
 *  Clear flags in map after unit.
 *
 *  @param pos        Position of major unit field.
 *  @param size       Unit size.
 *
 *  @note The method doesn't test map borders.
 */
void TLOC_MAP::UnitLeftPosition(TPOSITION_3D pos, int size)
{
  for (register int i = 0; i < size; i++)
    for (register int j = 0; j < size; j++)
      map[pos.segment][pos.x+i][pos.y+j].player_id = WLK_EMPTY_FIELD;
}


/**
 *  Set flags in map where is unit.
 *
 *  @param pos        Position of major unit field.
 *  @param size       Unit size.
 *  @param player_id  Player identifier.
 *
 *  @note The method doesn't test map borders.
 */
void TLOC_MAP::UnitFillPosition(TPOSITION_3D pos, int size, T_SIMPLE player_id)
{
  for (register int i = 0; i < size; i++)
    for (register int j = 0; j < size; j++)
      map[pos.segment][pos.x+i][pos.y+j].player_id = player_id;
}


/**
 *  The method tests whether position is moveable for unit type.
 *
 *  @param type   Pointer to type of unit.
 *  @param pos    Reference to tested position.
 */
bool TLOC_MAP::IsMoveablePosition(TFORCE_ITEM * const type, const TPOSITION_3D &pos)
{
  if ((type->GetExistSegments().min > pos.segment) || (type->GetExistSegments().max < pos.segment))   //unit can't exist in this segment
    return false;

  for (register int j, i = pos.x; i < (type->GetWidth() + pos.x); i++)
    for (j = pos.y; j < (type->GetHeight() + pos.y); j++)                    //test each of field where will be unit
    {
      if ((! ::map.IsInMap(pos))      //field isn't in the map
          || (! type->moveable[pos.segment].IsMember(map[pos.segment][i][j].terrain_id))) //unavailable terrain
        return false;
    }

  return true;
}


/**
 *  The method tests whether position is landable for unit type.
 *
 *  @param type   Pointer to type of unit.
 *  @param pos    Reference to tested position.
 */
bool TLOC_MAP::IsLandablePosition(TFORCE_ITEM * const type, const TPOSITION_3D &pos)
{
  bool one_landable;
  int i, j = LAY_UNDEFINED;

  one_landable = false;
  
  if (type->GetExistSegments().IsMember(pos.segment))     // it is in existable segment
  {
    for (i = pos.x; i < pos.x + type->GetWidth(); i++) {
      for (j = pos.y; j < pos.y + type->GetHeight(); j++) { //testing through new position
        
        if (::map.IsInMap(i, j)){
          
          if (map[pos.segment][i][j].player_id != WLK_EMPTY_FIELD) // field is not empty (it is occupied by another unit)
            return false;
          else 
          {
            if (type->landable[pos.segment].IsMember(map[pos.segment][i][j].terrain_id)) // it is possible land here
            {
              one_landable = true;
            }
            else if (!type->moveable[pos.segment].IsMember(map[pos.segment][i][j].terrain_id)) // it is NOT possible to land and to move here
              return false;
          }
        }
        else
          return false;
      }
    }
  }
  else 
    return false;
  
  return one_landable; //position is allowed to land
}


/**
 *  The method tests whether position is empty for next step of unit when comes in the direction.
 *
 *  @param pos    Tested position.
 *  @param dir    Direction of moving.
 *  @param unit   Unit which is moving
 */
bool TLOC_MAP::IsNextPositionEmpty(const TPOSITION_3D pos, const int dir, TFORCE_UNIT *unit)
{
  int i, j;
  int x_delta = 0, y_delta = 0;
  TPOSITION new_pos(pos);
  int size = unit->GetUnitWidth();

  if ((dir == LAY_UP) || (dir == LAY_DOWN))   //move to another segment
  {
    for (i = pos.x; i < pos.x + size; i++)    //vsechny nove obsazovane policka musime otestovat na nepritomnost jednotek
      for (j = pos.y; j < pos.y + size; j++)
        if ((! ::map.IsInMap(i,j,pos.segment)) 
          || ((map[pos.segment][i][j].player_id != WLK_EMPTY_FIELD) && (::map.segments[pos.segment].surface[i][j].unit != unit)))
          return false;
  }
  else                                        //move in same segment
  {
    //Urcime rozdily, ktere se budou pricitam nebo odcitat podle smeru pohybu od rohoveho pole nove obsazovanych policek,
    //aby se zjistilo zda nejsou obsazeny jednotkami.
    switch(dir)
    {
    case LAY_SOUTH:
      x_delta = 1;
      break;
    case LAY_SOUTH_WEST:
      x_delta = y_delta = 1;
      break;
    case LAY_WEST:
      y_delta = 1;
      break;
    case LAY_NORTH_WEST:
      x_delta = 1;
      y_delta = -1;
      new_pos.y += size - 1;
      break;
    case LAY_NORTH:
      x_delta = 1;
      new_pos.y += size - 1;
      break;
    case LAY_NORTH_EAST:
      x_delta = y_delta = -1;
      new_pos.x += size - 1;
      new_pos.y += size - 1;
      break;
    case LAY_EAST:
      y_delta = 1;
      new_pos.x += size - 1;
      break;
    case LAY_SOUTH_EAST:
      x_delta = -1;
      y_delta = 1;
      new_pos.x += size - 1;
      break;
    default:
      return false;
    }

    if ((x_delta == 0) || (y_delta == 0))                //it is a straight direction of move
    {
      for (i = 0; i < size; i++)
        if ((! ::map.IsInMap(new_pos.x + i*x_delta, new_pos.y + i*y_delta, pos.segment)) 
            || (map[pos.segment][new_pos.x + i*x_delta][new_pos.y + i*y_delta].player_id != WLK_EMPTY_FIELD))
          return false;
    }
    else                             //it is a diagonal direction of move
    {
      for (i = 0; i < size; i++)
        if ((! ::map.IsInMap(new_pos.x + i*x_delta, new_pos.y, pos.segment)) || (map[pos.segment][new_pos.x + i*x_delta][new_pos.y].player_id != WLK_EMPTY_FIELD)
            || (! ::map.IsInMap(new_pos.x, new_pos.y + i*y_delta, pos.segment)) || (map[pos.segment][new_pos.x][new_pos.y + i*y_delta].player_id != WLK_EMPTY_FIELD))
          return false;
    }
  }

  return true;
}


/**
 *  Gets the area visibility.
 *  If one or more mapels are visible, returns true else return false.
 *
 *  @param pos     Left-down corner of the area.
 *  @param width   Width of the area.
 *  @param height  Height of the area.
 */
bool TLOC_MAP::GetAreaVisibility(TPOSITION_3D pos, const T_SIMPLE width, const T_SIMPLE height)
{
  T_BYTE fstate;

  for (T_SIMPLE i = 0; i < width; i++)
    for (T_SIMPLE j = 0; j < height; j++) {
      fstate = map[pos.segment][pos.x + i][pos.y + j].state;

      if (fstate != WLK_UNKNOWN_AREA && fstate != WLK_WARFOG) return true;
    } 

  return false;
}


/**
 *  Gets the area visibility.
 *  If one or more mapels are visible, returns true else return false.
 *
 *  @param pos     Left-down corner of the area.
 *  @param width   Width of the area.
 *  @param height  Height of the area.
 */
bool TLOC_MAP::GetAreaVisibility(const T_SIMPLE pos_x, const T_SIMPLE pos_y, 
                                 const T_BYTE seg_min, const T_BYTE seg_max, 
                                 const T_SIMPLE width, const T_SIMPLE height)
{
  T_BYTE fstate;

  for (T_SIMPLE i = 0; i < width; i++)
    for (T_SIMPLE j = 0; j < height; j++)
      for (T_BYTE k = seg_min; k <= seg_max; k++) {
        fstate = map[k][pos_x + i][pos_y + j].state;

        if (fstate != WLK_UNKNOWN_AREA && fstate != WLK_WARFOG) return true;
      } 

  return false;
}


/**
 *  If all mapels are unknow, returns true else return false.
 *
 *  @param x       X position.
 *  @param y       Y position.
 *  @param seg     Segment.
 *  @param width   Width of the area.
 *  @param height  Height of the area.
 */
bool TLOC_MAP::IsAreaUnknown(const T_SIMPLE x, const T_SIMPLE y, const T_BYTE seg,
                             const T_SIMPLE width, const T_SIMPLE height)
{
  T_BYTE fstate;

  for (T_SIMPLE i = 0; i < width; i++)
    for (T_SIMPLE j = 0; j < height; j++) {
      fstate = map[seg][x + i][y + j].state;

      if (fstate != WLK_UNKNOWN_AREA) return false;
    } 

  return true;
}

/**
 *  If any of mapels are unknow, returns true else return false.
 *
 *  @param x       X position.
 *  @param y       Y position.
 *  @param seg     Segment.
 *  @param width   Width of the area.
 *  @param height  Height of the area.
 */
bool TLOC_MAP::IsAnyAreaUnknown(const T_SIMPLE x, const T_SIMPLE y, const T_BYTE seg,
                             const T_SIMPLE width, const T_SIMPLE height)
{
  T_BYTE fstate;

  for (T_SIMPLE i = 0; i < width; i++)
    for (T_SIMPLE j = 0; j < height; j++) {
      fstate = map[seg][x + i][y + j].state;

      if (fstate == WLK_UNKNOWN_AREA) return true;
    } 

  return false;
}

//=========================================================================
// struct TPLAYER_ARRAY
//=========================================================================

/**
 *  Constructor. Creates empty player array with only one player added - hyper
 *  player.
 */
TPLAYER_ARRAY::TPLAYER_ARRAY () {
  count = 0;
  start_points_count = 0;
  running_on_leader = true;
  lock = NULL;

  /* Hyper player with no race. Race will be set from menu. */
  AddPlayer ("HyperPlayer", "", false, false);
}

/**
 *  Deletes mutex.
 */
TPLAYER_ARRAY::~TPLAYER_ARRAY () {
  if (lock) delete lock;
}

/**
 *  Creates mutex needed for proper manipulation with object.
 *
 *  @throw MutexException { Error creating mutex. }
 */
void TPLAYER_ARRAY::Initialise () {
  if (!lock)
    lock = NEW TRECURSIVE_LOCK;
}

/**
 *  Removes all connected players and clears the structure.
 */
void TPLAYER_ARRAY::Clear () {
  Lock ();

  /* Remove all players except hyper player. */
  for (int i = GetCount() - 1; i > 0; i--)
    RemovePlayer (i);

  Unlock ();
}

/**
 *  Adds a player to the array. Specifying @p race_id_name race and @p computer
 *  is optional.
 *
 *  @param player_name  Name of the player.
 *  @param race_id_name Id_name of the race.
 *  @param computer     Specifies wether the player is a computer player.
 *  @param lock         Specifies if should lock the object. This is only set
 *                      to false when adding hyper player, because the mutex
 *                      is not created yet.
 *
 *  @throw MutexException { Mutex is not created. }
 */
void TPLAYER_ARRAY::AddPlayer (string player_name, string race_id_name,
                               bool computer, bool lock, bool remote,
                               in_addr *addr_p, in_port_t port)
{
  if (lock)
    Lock ();

  if (count == PL_MAX_PLAYERS) {
    if (lock)
      Unlock ();
    throw TooManyPlayersException ();
  }

  player[count].player_name = player_name;
  player[count].race_id_name = race_id_name;
  player[count].computer = computer;
  player[count].ready_to_start_game = false;

  // remote player
  if ((player[count].remote = remote)) {
    player[count].addr = *addr_p;
    player[count].port = port;
  }

  count++;

  if (lock)
    Unlock();
}

/**
 *  Adds a computer player without a race specified. Name of the player will be
 *  automaticaly generated.
 */
void TPLAYER_ARRAY::AddComputerPlayer () {
  AddPlayer ("Computer", "", true);

  ChooseRandomStartPoints ();
}

/**
 *  Adds a local player without a race specified.
 *
 *  @param player_name Name of the player.
 */
void TPLAYER_ARRAY::AddLocalPlayer (string player_name, string race_id_name,
                                    bool computer)
{
  AddPlayer (player_name, race_id_name, computer);

  ChooseRandomStartPoints ();
}

/**
 *  Adds a remote player.
 *
 *  @param player_name Name of the player.
 */
void TPLAYER_ARRAY::AddRemotePlayer (string player_name, in_addr addr,
                                     in_port_t port, bool computer)
{
  AddPlayer (player_name, "", computer, true, true, &addr, port);

  ChooseRandomStartPoints ();
}

/**
 *  Removes player from array.
 */
void TPLAYER_ARRAY::RemovePlayer (int player_index) {
  Lock ();

  for (int i = player_index + 1; i < GetCount (); i++)
    player[i - 1] = player[i];

  count--;

  ChooseRandomStartPoints ();

  Unlock ();
}

/**
 *  Gets player name at position @p player_index in the array.
 */
string TPLAYER_ARRAY::GetPlayerName (int player_index) {
  return player[player_index].player_name;
}

/**
 *  Gets player race id_name at possiton @p player_index in the array.
 */
string TPLAYER_ARRAY::GetRaceIdName (int player_index) {
  return player[player_index].race_id_name;
}

/**
 *  Sets player race id_name at position @p player_index to @p race_id_name.
 */
void TPLAYER_ARRAY::SetRaceIdName (int player_index, string race_id_name) {
  player[player_index].race_id_name = race_id_name;
}

/**
 *  Returns true, if the player is on localhost.
 */
bool TPLAYER_ARRAY::IsRemote (int player_index) {
  return player[player_index].remote;
}

/**
 *  Returns true, if the player is a computer player.
 */
bool TPLAYER_ARRAY::IsComputer (int player_index) {
  return player[player_index].computer;
}

bool TPLAYER_ARRAY::EveryPlayerHasDifferentRace () {
  Lock ();

  bool ret = true;

  for (int i = 0; i < GetCount (); i++) {
    for (int j = 0; j < GetCount (); j++) {
      if (i != j && GetRaceIdName (i) == GetRaceIdName (j))
        ret = false;
    }
  }

  Unlock ();

  return ret;
}

/**
 *  Returns address of a player with index @p player_index.
 */
in_addr TPLAYER_ARRAY::GetAddress (int player_index) {
  return player[player_index].addr;
}

/**
 *  Returns port of a player with index @p player_index.
 */
in_port_t TPLAYER_ARRAY::GetPort (int player_index) {
  return player[player_index].port;
}

void TPLAYER_ARRAY::ChooseRandomStartPoints () {
  if (!running_on_leader)
    return;

  Lock ();

  /* Array of used startpoints. Whole array will be initialised to false. */
  bool used[PL_MAX_START_POINTS] = { 0 };
  int random;

  /* Choose random start point for every player except hyper player. */
  for (int i = 1; i < GetCount () && i <= start_points_count; i++) {
    do {
      random = GetRandomInt (start_points_count);
    } while (used[random]);

    player[i].start_point = random;
    used[random] = true;
  }

  Unlock ();
}

void TPLAYER_ARRAY::SetStartPointsCount (int value) {
  start_points_count = value;

  ChooseRandomStartPoints ();
}

int TPLAYER_ARRAY::GetStartPoint (int player_index) {
  return player[player_index].start_point;
}

void TPLAYER_ARRAY::SetStartPoint (int player_index, int value) {
  player[player_index].start_point = value;
}

int TPLAYER_ARRAY::GetMyPlayerID () {
  Lock ();

  /* Find first player, that is not a remote player and is not a computer
   * player. There should be only one such player. */
  for (int i = 1; i < GetCount (); i++) {
    if (!IsRemote (i) && !IsComputer (i)) {
      Unlock ();
      return i;
    }
  }

  Unlock ();
  throw PlayerNotFoundException ();
}

int TPLAYER_ARRAY::GetPlayerID (in_addr address, in_port_t port, int min_id) {
  int ret = -1;

  Lock ();

  /* XXX: sprava prichadza z nahodneho portu, preto ten port nekontrolujeme. */

  for (int i = min_id; i < GetCount (); i++) {
    if (TNET_RESOLVER::NetworkToAscii (player[i].addr) == TNET_RESOLVER::NetworkToAscii (address)) {
      ret = i;
      break;
    }
  }

  Unlock ();

  return ret;
}

bool TPLAYER_ARRAY::AllPlayersAreLocal () {
  Lock ();

  bool any_remote = false;

  for (int i = 0; i < GetCount (); i++) {
    if (IsRemote (i))
      any_remote = true;
  }

  Unlock ();

  return !any_remote;
}

void TPLAYER_ARRAY::PlayerReady (in_addr address, in_port_t port) {
  Lock ();

  /* XXX: sprava prichadza z nahodneho portu, preto ten port nekontrolujeme. */

  for (int i = 0; i < GetCount (); i++) {
    if (IsRemote (i) &&
        TNET_RESOLVER::NetworkToAscii (player[i].addr) == TNET_RESOLVER::NetworkToAscii (address))
    {
      player[i].ready_to_start_game = true;
    }
  }

  Unlock ();
}

bool TPLAYER_ARRAY::AllRemoteReady () {
  Lock ();

  bool ret = true;

  for (int i = 0; i < GetCount (); i++) {
    if (IsRemote (i) && !player[i].ready_to_start_game) {
      ret = false;
      break;
    }
  }

  Unlock ();

  return ret;
}


//=========================================================================
//  Global functions for working with players
//=========================================================================

/**
 *  Create array of players.
 */
bool CreatePlayers()
{
  int count = player_array.GetCount();
  players = NEW TPLAYER*[count];
  if (!players) return false;

  for (int i = 0; i < count; i++) 
  {
    /*
    if (player_array.IsComputer(i))
    {
      if (!(players[i] = NEW TCOMPUTER_PLAYER))
        return false;
    } 
    else 
    {
       if (!(players[i] = NEW TPLAYER))
         return false;
    }
    */
    if (!(players[i] = NEW TPLAYER))
      return false;

    players[i]->SetPlayerID(i);
    players[i]->pathtools = NEW TA_STAR_ALG(i);
  }

  return true;
}


/**
 *  Delete array of players.
 */
void DeletePlayers()
{
  if (players) 
  {
    for (int i = 0; i < player_array.GetCount (); i++) {
      if (players[i]) delete players[i];
      players[i] = NULL;
    }
  }


  // delete array
  delete [] players;
  players = NULL;
  myself = hyper_player = NULL;
}


//=========================================================================
// END
//=========================================================================
// vim:ts=2:sw=2:et:

