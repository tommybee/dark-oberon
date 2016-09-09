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
 *  @file doraces.cpp
 *
 *  Races functions.
 *
 *  @author Peter Knut
 *  @author Martin Kosalko
 *  @author Michal Kral
 *
 *  @date 2003
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "cfg.h"

#ifdef WINDOWS
 #include <io.h>
#else // on UNIX
 #include <sys/types.h>
 #include <dirent.h>
#endif

#include "dofile.h"
#include "dodata.h"
#include "doengine.h"
#include "doraces.h"
#include "doschemes.h"
#include "dofight.h"
#include "domap.h"


//=========================================================================
// Global variables
//=========================================================================

/**
 *  Races.
 */
TRACE *races = NULL;


//=========================================================================
// Local variables
//=========================================================================

static TRACE *actual;
int worker_order[SCH_MAX_MATERIALS_COUNT] = {0,0,0,0};


//=========================================================================
// class TMAP_ITEM
//=========================================================================

/** 
 *  The method tested whether position from parameter is available. If it is
 *  then returns true.
 *  
 *  @return The method returns true if the position is possible available. 
 *
 *  @param t_pos  Tested position.
 */
bool TMAP_ITEM::IsPossibleAvailablePosition(const TPOSITION_3D t_pos)
{
  return IsPossibleAvailablePosition(t_pos.x, t_pos.y, t_pos.segment);
}


/** 
 *  The method tested whether position from parameter is available. It is false
 *  for map unit. True is possible only for descendants.
 *  
 *  @return The method returns true if the position is possible available. 
 *
 *  @param t_pos_x  x-coordinate of tested position.
 *  @param t_pos_y  y-coordinate of tested position.
 *  @param t_pos_seg  segment-coordinate of tested position.
 */
bool TMAP_ITEM::IsPossibleAvailablePosition(const T_SIMPLE t_pos_x, const T_SIMPLE t_pos_y, const T_SIMPLE t_pos_seg)
{
  return false;
}


/**
 *  The method counts percentage of available positions in the map.
 */
void TMAP_ITEM::CountAvailablePositions()
{
  int count = 0;

  for (int s = 0; s < DAT_SEGMENTS_COUNT; s++)
    for (int i = 0; i < (map.width - GetWidth()); i++)
      for (int j = 0; j < (map.height - GetHeight()); j++)
      {
        if (IsPossibleAvailablePosition(i, j, s))
          count++;
      }

  available_positions = static_cast<float>(count) / static_cast<float>((map.width * map.height * DAT_SEGMENTS_COUNT));
}


//=========================================================================
// class TBASIC_ITEM
//=========================================================================

/**
 *  Constructor. Zeroize all data.
 */
TBASIC_ITEM::TBASIC_ITEM()
:TMAP_ITEM()
{ 
  int i;
  
  for (i = 0; i < SCH_MAX_MATERIALS_COUNT; i++) {
    materials[i] = 0;
    mat_per_pt[i] = 0.0f;
  }

  for (i = 0; i < DAT_SEGMENTS_COUNT; i++ ) {
    visible_segments[i].min = visible_segments[i].max = 0;
  }
  exist_segments.max = exist_segments.min = 0;

  view = 0;
  count_of_active_instances = 0;

  energy = 0;
  min_energy = 0;
  food = 0;
}

/**
 * Function returns true if it is possible to create unit with this type of item according to dependencies.
 *
 *  @param next_itm Pointer to type whi will be builded (output)
 */
bool TBASIC_ITEM::IsPossibleBuildable(float * price, TBASIC_ITEM ** next_itm)
{
  TLIST<TBASIC_ITEM>::TNODE<TBASIC_ITEM> *node;
  TBASIC_ITEM *itm, *min_itm = NULL, *act_itm;
  float avg_materials, avg_aids; // averace values of materials and food&energy
  float min_materials, min_aids; // min values of materials and food&energy
  float min_price [SCH_MAX_MATERIALS_COUNT + 2]; // min values of price
  float act_price [SCH_MAX_MATERIALS_COUNT + 2]; // actual values of price
  int i; // help variable
  bool exists;
  
  
  // item is in path -> return false
  if (buildable_counted) return false;

  // in case that value wac calculated before
  if (buildable_value[0] != -1) {
    for (i = 0; i < SCH_MAX_MATERIALS_COUNT + 2; i++){
      price[i] += buildable_value[i];
    }
    return true;
  }

  buildable_counted = true;
  
  // null act_prices
  for (i = 0; i < SCH_MAX_MATERIALS_COUNT + 2; i++) min_price[i] = 0;
  min_materials = min_aids = 0;

  exists = false;
    
  if(is_builded_by_list.IsEmpty()) {
    buildable_counted = false;
    return false; // if list of item that can produce me is empty return false
  }
  else {  // recursive call of IsPossibleBuildable()
    for (node = is_builded_by_list.GetFirst(); node; node = node->GetNext()) {
      itm = node->GetPitem();
    
      // null act_prices
      for (i = 0; i < SCH_MAX_MATERIALS_COUNT + 2; i++) act_price[i] = 0;
      act_itm = this;

      if ((itm->GetCountOfActiveInstances() > 0) || (itm->IsPossibleBuildable(act_price, &act_itm))){
        // count averages of actual price
        avg_materials = avg_aids = 0;
        for (i = 0; i < SCH_MAX_MATERIALS_COUNT; i++) avg_materials += act_price[i];
        for (i = SCH_MAX_MATERIALS_COUNT; i < SCH_MAX_MATERIALS_COUNT + 2; i++) avg_aids += act_price[i];

        avg_materials /= SCH_MAX_MATERIALS_COUNT;
        avg_aids /= 2;

        // compare min and new values
        if (((avg_materials) < (min_materials)) || (node == is_builded_by_list.GetFirst())) {
          min_materials = avg_materials;
          min_aids = avg_aids;

          for (i = 0; i < SCH_MAX_MATERIALS_COUNT + 2; i++) min_price[i] = act_price[i];

          min_itm = act_itm;

          exists = true;
        }
      }
    }

    if (exists){
      // to global value add my materials, food and energy and materials of my minimal builder
      for (i = 0; i < scheme.materials_count; i++){
        price[i] += (materials[i] + min_price[i]);
        buildable_value[i] = (materials[i] + min_price[i]);
      }
      price[SCH_MAX_MATERIALS_COUNT] += (food + min_price[SCH_MAX_MATERIALS_COUNT]);
      buildable_value[SCH_MAX_MATERIALS_COUNT] = food + min_price[SCH_MAX_MATERIALS_COUNT];
      price[SCH_MAX_MATERIALS_COUNT + 1] += (energy + min_price[SCH_MAX_MATERIALS_COUNT + 1]);
      buildable_value[SCH_MAX_MATERIALS_COUNT + 1] = energy + min_price[SCH_MAX_MATERIALS_COUNT + 1];
      
      *next_itm = min_itm;

      buildable_counted = false;
      return true;
    }
  }
  
  buildable_counted = false;
  return false;
}

//=========================================================================
// class TFORCE_ITEM
//=========================================================================

/**
 *  Constructor. Zeroize all data.
 */
TFORCE_ITEM::TFORCE_ITEM()
:TBASIC_ITEM()
{
  for (int i = 0; i < DAT_SEGMENTS_COUNT; max_speed[i++] = 0.0) {
    max_speed[i] = 0.0f;
    moveable[i].min = moveable[i].max = 0;
    landable[i].min = landable[i].max = 0;
    max_rotation[i] = 0.0f;
  }

  land_segment = 0;
  features = RAC_NO_FEATURES;
  heal_time = 0.0;

  can_move = true;
  item_type = IT_FORCE;

  tg_move_id = tg_rotate_id = tg_anchor_id = tg_land_id = -1;
}


/** 
 *  Tests whether position is available for this kind of the unit.
 *  
 *  @param pos_x First coordinate of tested position
 *  @param pos_y Second coordinate of tested position
 *  @param seg   Segment coordinate of tested position
 */
bool TFORCE_ITEM::IsPositionAvailable(int pos_x, int pos_y, int seg)
{
  int i,j;
  TMAP_SURFACE *field = NULL;

  for (i = pos_x; i < pos_x + GetWidth(); i++)
  {
    for (j = pos_y; j < pos_y + GetHeight(); j++)
    {
      if (!::map.IsInMap(i,j)) return false;   //test, whether the position is in map
      field = &(::map.segments[seg].surface[i][j]);
      if (field->unit) return false; //test, whether there's any unit standing at the position
      
      if (!moveable[seg].IsMember(field->t_id) && !landable[seg].IsMember(field->t_id)) return false; //test whether unit can move or land there
    }
  }

  return true;
}


/** 
 *  Tests whether position around holder unit is available for unit.
 *  
 *  @param holder Holder unit
 */
bool TFORCE_ITEM::IsPosAroundHolderAvailable(TFORCE_UNIT *unit, TMAP_UNIT *holder_unit, T_BYTE seg, TPOSITION_3D *free_position)
{
  T_SIMPLE x, y;
  TPOSITION_3D holder_pos = holder_unit->GetPosition(); 

  for (y = holder_pos.y - GetHeight(); y <= holder_pos.y + holder_unit->GetUnitHeight(); y++)
    for (x = holder_pos.x - GetWidth(); x <= holder_pos.x + holder_unit->GetUnitWidth(); x++){
      free_position->SetPosition(x, y, seg);
      
      if ((unit->GetPlayer()->GetLocalMap()->IsMoveablePosition(this, *free_position) || 
          unit->GetPlayer()->GetLocalMap()->IsLandablePosition(this, *free_position)) &&
          (map.IsPositionFree(width, height, *free_position)))
            return true;
    }

  return false;
}


/**
 *  The method tests whether a position is landable, moveable or both. It doesn't test whether
 *  the position is in the map or is empty.
 *  
 *  @param cx First coordinate of tested position
 *  @param cy Second coordinate of tested position
 *  @param cs Segment coordinate of tested position
 *
 *  @return The method returns RAC_LANDABLE_POSITION if the position is only landable,
 *  or RAC_MOVEABLE_POSITION if the position is only moveable or RAC_BOTHABLE_POSITION 
 *  if is landable and moveable otherwise return RAC_UNABLE_POSITION.
 */
unsigned char TFORCE_ITEM::TestPositionFeatures(T_SIMPLE tx, T_SIMPLE ty, T_SIMPLE ts)
{
  int i, j;
  TTERRAIN_ID act_field_tid;
  unsigned char result = RAC_BOTHABLE_POSITION;

  for (i = tx; i < tx + GetWidth(); i++)
    for (j = ty; j < ty + GetHeight(); j++)
    {
      //test how is field available
      act_field_tid = ::map.segments[ts].surface[i][j].t_id;
      if (! moveable[ts].IsMember(act_field_tid))   //position isn't moveable
        result &= (!RAC_MOVEABLE_POSITION);
      else if (! landable[ts].IsMember(act_field_tid))  //position isn't landable
        result &= (!RAC_LANDABLE_POSITION);
      else  //position isn't landable nor landable
        return RAC_UNABLE_POSITION;
      //test whether position is still landable or moveable
      if (result == RAC_UNABLE_POSITION)
        return RAC_UNABLE_POSITION;
    }

  return result;
}


/** 
 *  The method tested whether position from parameter is available. If it is
 *  then returns true.
 *  
 *  @return The method returns true if the position is possible available. 
 *
 *  @param t_pos_x  x-coordinate of tested position.
 *  @param t_pos_y  y-coordinate of tested position.
 *  @param t_pos_seg  segment-coordinate of tested position.
 */
bool TFORCE_ITEM::IsPossibleAvailablePosition(const T_SIMPLE t_pos_x, const T_SIMPLE t_pos_y, const T_SIMPLE t_pos_seg)
{
  return IsPositionAvailable(t_pos_x, t_pos_y, t_pos_seg);
}


//=========================================================================
// class TSOURCE_ITEM
//=========================================================================

/** 
 *  Tests whether position is available for this kind of the source.
 *  
 *  @param pos_x First coordinate of tested position
 *  @param pos_y Second coordinate of tested position
 *  @param seg   Segment coordinate of tested position
 */
bool TSOURCE_ITEM::IsPositionAvailable(int pos_x, int pos_y)
{
  int x, y, seg;
  int i, j;

  TTERRAIN_ID tid;
  TMAP_UNIT *u = NULL;

  T_SIMPLE w = GetWidth();
  T_SIMPLE h = GetHeight();

  // test, whaether building can be build exactly on this position  
  for (j = 0; j < h; j++) {
    for (i = 0; i < w; i++) { 
      x = pos_x + i;
      y = pos_y + j;

      /*
        Building cannot be built on specified position if any of the following conditions passes:
        - some needed fields are not in map
        - other building or unit is already standing on any of needed fields
        - terrain_id on at least one needed fields is in not permitted range for the building
        If any ot these tests passes, building cannot be built, return false
      */

      if (map.IsInMap(x, y)) {
        for (seg = GetExistSegments().min; seg <= GetExistSegments().max; seg++) 
        {
          tid = map.segments[seg].surface[x][y].t_id;
          u = map.segments[seg].surface[x][y].unit;

          if (!(buildable[seg].IsMember(tid) && !u)) return false;
        }
      }
      else 
        return false;
    }   
  }

  return true;
}


/** 
 *  The method tested whether position from parameter is available. If it is
 *  then returns true.
 *  
 *  @return The method returns true if the position is possible available. 
 *
 *  @param t_pos_x  x-coordinate of tested position.
 *  @param t_pos_y  y-coordinate of tested position.
 *  @param t_pos_seg  segment-coordinate of tested position.
 */
bool TSOURCE_ITEM::IsPossibleAvailablePosition(const T_SIMPLE t_pos_x, const T_SIMPLE t_pos_y, const T_SIMPLE t_pos_seg)
{
  return ((t_pos_seg == GetExistSegments().min) ? IsPositionAvailable(t_pos_x, t_pos_y) : false);
}


//=========================================================================
// class TBUILDABLE
//=========================================================================


/** Constructor. If initialize values from parameters aren't correct sets values to NULL and zero.
 *
 *  @param item Pointer to item that is produceable.
 *  @param time Necessary time to produce the unit.
 *  @param node Next node in the list.
 */
inline TPRODUCEABLE_NODE::TPRODUCEABLE_NODE(TFORCE_ITEM *item, double time, TPRODUCEABLE_NODE *node)
{
  if ((time > 0.0) && (::IsValidForceItem(item))) 
  { 
    pitem = item;
    produce_time = time;
  }
  else
  {
    pitem = NULL;
    time = 0.0;
  }
  prev = node;
  if (node) node->SetNextNode(this);
  next = NULL;
}


/** Sets pointer to produceable item but only if it is pointer to valid TFORCE_ITEM or its descendants. 
 *  Otherwise doesn't change old value. Returns true if new value is valid otherwise returns false.
 *
 *  @param item New pointer to produceable item.
 */
inline bool TPRODUCEABLE_NODE::SetProduceableItem(TFORCE_ITEM* item)
{
  if (::IsValidForceItem(item))
  {
    pitem = item;
    return true;
  }
  else
    return false;
}


//=========================================================================
// class TBUILDING_ITEM
//=========================================================================

/**
 *  Constructor. Zeroizes all data.
 */
TBUILDING_ITEM::TBUILDING_ITEM()
:TBASIC_ITEM()
{
  int i;    

  for (i = 0; i < DAT_SEGMENTS_COUNT; i++) {
    buildable[i].min = buildable[i].max = 0;
  }

  item_type = IT_BUILDING;
  for (i = 0; i <  SCH_MAX_MATERIALS_COUNT; i++) allowed_materials[i] = false;
  allow_any_material = false;

  tg_stay_id = -1;
  ancestor = NULL;
}


/** 
 *  Tests whether position is available for this kind of the building.
 *  
 *  @param pos_x First coordinate of tested position
 *  @param pos_y Second coordinate of tested position
 *  @param test_ancestor If building has ancestor, it can be built only on it.
 */
bool TBUILDING_ITEM::IsPositionAvailable(int pos_x, int pos_y, bool test_ancestor)
{
  int x, y, seg;
  int i, j;

  TTERRAIN_ID tid;
  TMAP_UNIT *u = NULL;

  T_SIMPLE w = GetWidth();
  T_SIMPLE h = GetHeight();

  // test, whaether building can be build exactly on this position  
  for (j = 0; j < h; j++) {
    for (i = 0; i < w; i++) { 
      x = pos_x + i;
      y = pos_y + j;

      /*
        Building cannot be built on specified position if any of the following conditions passes:
        - some needed fields are not in map
        - other building or unit is already standing on any of needed fields
        - terrain_id on at least one needed fields is in not permitted range for the building
        If any ot these tests passes, building cannot be built, return false
      */


      if (map.IsInMap(x, y)) {
        for (seg = GetExistSegments().min; seg <= GetExistSegments().max; seg++) 
        {
          if (ancestor && test_ancestor){
            u = map.segments[seg].surface[x][y].unit;
            if ((!u) || (u && (u->GetPointerToItem() != ancestor)))
              return false;
          }
          else {
            tid = map.segments[seg].surface[x][y].t_id;
            u = map.segments[seg].surface[x][y].unit;

            if (!(buildable[seg].IsMember(tid) && !u)) return false;
          }
        }
      }
      else 
        return false;
    }   
  }

  return true;
}

/**
 * Function returns true if it is possible to create unit with this type of item according to dependencies.
 *
 *  @param  next_itm Optimal kind of unit which will be builded (output).
 */
bool TBUILDING_ITEM::IsPossibleBuildable(float * price, TBASIC_ITEM ** next_itm)
{
  TLIST<TBASIC_ITEM>::TNODE<TBASIC_ITEM> *node;
  TBASIC_ITEM *itm, *min_itm = NULL, *act_itm, *anc_itm = NULL;
  float avg_materials, avg_aids; // averace values of materials and food&energy
  float min_materials, min_aids; // min values of materials and food&energy
  float min_price [SCH_MAX_MATERIALS_COUNT + 2]; // min values of price
  float act_price [SCH_MAX_MATERIALS_COUNT + 2]; // actual values of price
  int i; // help variable
  bool exists;
  bool need_ancestor;
  
  // building is in path -> return false
  if (buildable_counted) return false;

  // in case that value wac calculated before
  if (buildable_value[0] != -1) {
      for (i = 0; i < SCH_MAX_MATERIALS_COUNT + 2; i++){
        price[i] += buildable_value[i];
      }
      return true;
    }

  buildable_counted = true;
  
  // null act_prices
  for (i = 0; i < SCH_MAX_MATERIALS_COUNT + 2; i++) min_price[i] = 0;
  min_materials = min_aids = 0;

  exists = false;
    
  if(is_builded_by_list.IsEmpty()) {
    buildable_counted = false;  
    return false; // if list of item that can produce me is empty return false
  }
  else {  // recursive call of IsPossibleBuildable()
    
    need_ancestor = false;
    if (ancestor) {
      if (ancestor->GetCountOfActiveInstances() > 0) 
        need_ancestor = false;
      else {
        if (!ancestor->IsPossibleBuildable(price, &anc_itm)) {
          buildable_counted = false;  
          return false; // if building has ancestor, but ancestor does not have any instances, return false
        }
        else
          need_ancestor = true;
      }
    }
    
    for (node = is_builded_by_list.GetFirst(); node; node = node->GetNext()) {
      itm = node->GetPitem();
  
      // null act_prices
      for (i = 0; i < SCH_MAX_MATERIALS_COUNT + 2; i++) act_price[i] = 0;
      act_itm = this;
      
      if ((itm->GetCountOfActiveInstances() > 0) || (itm->IsPossibleBuildable(act_price, &act_itm))){
        // count averages of actual price
        avg_materials = avg_aids = 0;
        for (i = 0; i < SCH_MAX_MATERIALS_COUNT; i++) avg_materials += act_price[i];
        for (i = SCH_MAX_MATERIALS_COUNT; i < SCH_MAX_MATERIALS_COUNT + 2; i++) avg_aids += act_price[i];

        avg_materials /= SCH_MAX_MATERIALS_COUNT;
        avg_aids /= 2;

        // compare min and new values
        if (((avg_materials) < (min_materials)) || (node == is_builded_by_list.GetFirst())) {
          min_materials = avg_materials;
          min_aids = avg_aids;

          for (i = 0; i < SCH_MAX_MATERIALS_COUNT + 2; i++) min_price[i] = act_price[i];

          min_itm = act_itm;

          exists = true;
        }
      }
    }

    if (exists){
      // to global value add my materials, food and energy and materials of my minimal builder
      for (i = 0; i < scheme.materials_count; i++){
        price[i] += (materials[i] + min_price[i]);
        buildable_value[i] = (materials[i] + min_price[i]);
      }
      price[SCH_MAX_MATERIALS_COUNT] += (food + min_price[SCH_MAX_MATERIALS_COUNT]);
      buildable_value[SCH_MAX_MATERIALS_COUNT] = food + min_price[SCH_MAX_MATERIALS_COUNT];
      price[SCH_MAX_MATERIALS_COUNT + 1] += (energy + min_price[SCH_MAX_MATERIALS_COUNT + 1]);
      buildable_value[SCH_MAX_MATERIALS_COUNT + 1] = energy + min_price[SCH_MAX_MATERIALS_COUNT + 1];

      
      if (need_ancestor)
        *next_itm = anc_itm;
      else
        *next_itm = min_itm;

      buildable_counted = false;
      return true;
    }
  }
  
  buildable_counted = false;
  return false;
}


/** 
 *  The method tested whether position from parameter is available. If it is
 *  then returns true.
 *  
 *  @return The method returns true if the position is possible available. 
 *
 *  @param t_pos_x  x-coordinate of tested position.
 *  @param t_pos_y  y-coordinate of tested position.
 *  @param t_pos_seg  segment-coordinate of tested position.
 */
bool TBUILDING_ITEM::IsPossibleAvailablePosition(const T_SIMPLE t_pos_x, const T_SIMPLE t_pos_y, const T_SIMPLE t_pos_seg)
{
  return ((t_pos_seg == GetExistSegments().min) ? IsPositionAvailable(t_pos_x, t_pos_y, false) : false);
}


//=========================================================================
// class TWORKER_ITEM
//=========================================================================

/**
 *  Destructor. Dealocates dynamicly allocated lists
 */
TWORKER_ITEM::~TWORKER_ITEM()
{
  build_list.DestroyList();
  repair_list.DestroyList();
};


//=========================================================================
// class TBUILDING_ITEM
//=========================================================================

/**
 *  Destructor.
 */
TBUILDING_ITEM::~TBUILDING_ITEM()
{
  ;
}

//=========================================================================
// class TFACTORY_ITEM
//=========================================================================

/**
 *  Destructor.
 */
TFACTORY_ITEM::~TFACTORY_ITEM()
{
  ;
}

//=========================================================================
// struct TRACE
//=========================================================================

/**
 * Destructor. Frees all allocated memory.
 */
TRACE::~TRACE()
{
  int i;

  if (buildings) {
    for (i = 0; i < buildings_count; i++) if (buildings[i]) delete buildings[i];
    delete[] buildings;
  }

  if (units) {
    for (i = 0; i < units_count; i++) if (units[i]) delete units[i];
    delete[] units;
  }

  if (sources) {
    for (i = 0; i < sources_count; i++) if (sources[i]) delete sources[i];
    delete[] sources;
  }
}


/**
 *  Function returns TBASIC_ITEM which is necessary to build target.
 *
 *  @param target Item that is target of building
 *  @return Item that is necessary for building target item.
 */ 
TBASIC_ITEM * TRACE::FindProduct(TBASIC_ITEM * target)
{
  float * price;
  int i, j;
  bool buildable;
  TBASIC_ITEM * next_itm = NULL;
  
  // create price
  price = NEW float[SCH_MAX_MATERIALS_COUNT + 2];

  // null prices
  for (i = 0; i < SCH_MAX_MATERIALS_COUNT + 2; i++) price[i] = 0;

  // null race flags and values
  for (i = 0; i < buildings_count; i++) {
    buildings[i]->buildable_counted = false;
    for (j = 0; j < SCH_MAX_MATERIALS_COUNT + 2; j++)
      buildings[i]->buildable_value[j] = -1;
  }
  for (i = 0; i < units_count; i++) {
    units[i]->buildable_counted = false;
    for (j = 0; j < SCH_MAX_MATERIALS_COUNT + 2; j++)
      units[i]->buildable_value[j] = -1;
  }

  buildable = target->IsPossibleBuildable(price, &next_itm);
  
  //Debug(LogMsg("Buildable:%s  G:%.2f, W:%.2f, Energy:%.2f, Food:%.2f, Next:%s", buildable ? "true" : "false", price[0], price[1], price[4], price[5], (next_itm != NULL) ? next_itm->name : "no"));

  delete [] price;

  return NULL;
}

/**
 *  Function returns list of workers which can mine material given as parameter.
 *
 *  @param material Identificator of material that will be mining.
 *  @return list of workers that can mine material.
 */
TLIST<TWORKER_ITEM> * TRACE::FindWorker(T_BYTE material)
{
  return &scheme.materials[material]->mine_list;
}

/**
 *  Function returns list of units which can build unit given as parameter.
 *
 *  @param unit Pointer to item that will be builded.
 *  @return list of items that can build unit.
 */
TLIST<TBASIC_ITEM> * TRACE::FindBuilders(TBASIC_ITEM * unit)
{
  return &unit->is_builded_by_list;
}

/**
 *  Function returns list of units which can repair unit given as parameter.
 *
 *  @param unit Pointer to item that will be repaired.
 *  @return list of items that can repair unit.
 */
TLIST<TBASIC_ITEM> * TRACE::FindRepairers(TBASIC_ITEM * unit)
{
  return &unit->is_repaired_by_list;
}

/**
 *  Function returns array and count of buildings according to input conditions.
 *
 *  @param mat Input-output array of item IDs. Max count of IDs is RAC_MAX_CONDITION_RETURNED_UNITS.
 *  @param mat Array of needed allowed material. (0 - don't need, 1-9 degree of need, 10 = RAC_MAX_CONDITION_VALUE - necessary)
 *  @param shot_seg Array of segments where building need to shot. (0 - don't need, 1-9 degree of need, 10 = RAC_MAX_CONDITION_VALUE - necessary)
 *  @param need_food How much is needed food. (0 - don't need, 1-9 degree of need, 10 = RAC_MAX_CONDITION_VALUE - necessary)
 *  @param need_energy How much is needed energy. (0 - don't need, 1-9 degree of need, 10 = RAC_MAX_CONDITION_VALUE - necessary)
 *
 *  @param start_pos Left down corner of area wher building will be placed.
 *  @param end_pos Right upper corner of area wher building will be placed.
 */
int TRACE::FindBuilding(int * result, int * mat, int * shot_seg, int need_food, int need_energy, TPOSITION start_pos, TPOSITION end_pos)
{
  int i, j, k;
  int pos_x, pos_y;
  TBUILDING_ITEM * act_building;
  bool cond_pos, cond_mat, cond_shot, cond_energy, cond_food;
  int value;
  int result_count = 0;
  int result_values[RAC_MAX_CONDITION_RETURNED_UNITS];

  // clear result array
  for (i = 0; i < RAC_MAX_CONDITION_RETURNED_UNITS; i++) {
    result [i] = -1;
    result_values [i] = -1;
  }
  
  // cycle through all building types
  for (i = 0; i < buildings_count; i++) {
    cond_mat = cond_shot = cond_food = cond_energy = true;
    cond_pos = false; // init conditions for every building
    value = 0;
    
    act_building = buildings[i];

    // test if it is possible to build building in selected area
    for (pos_x = start_pos.x; (!cond_pos) && (pos_x <= end_pos.x - act_building->GetWidth()); pos_x++){
      for (pos_y = start_pos.y; (!cond_pos) && (pos_y <= end_pos.y - act_building->GetHeight()); pos_y++){
        if (act_building->IsPositionAvailable(pos_x, pos_y, true))
          cond_pos = true;
      }
    }


    if (cond_pos){ // if it is possible to build building
      // cycle through materials
      for (j = 0; (cond_mat) && (j < scheme.materials_count); j++) {
        if (act_building->GetAllowedMaterial(j))
          value += mat[j];  // if building accept material, increment its value
        else {
          if (mat[j] == RAC_MAX_CONDITION_VALUE) // if building does not accept material and material is necessary -> go to next building
            cond_mat = false;
        }
      }

      if (cond_mat){ // if building accept all necessary material
        // cycle through all segments
        for (j = 0; (cond_shot) && (j < DAT_SEGMENTS_COUNT); j++){
          // test if unit is offensive and building can shot to segment
          if ((act_building->GetArmament()->GetOffensive()) && (act_building->GetArmament()->GetOffensive()->GetShotableSegments().IsMember(j)))
              value =+ shot_seg[j]; // increment building value
          else{ // building is defensive or building can not shot to segment
            if (shot_seg[j] == RAC_MAX_CONDITION_VALUE) // test if shot_seg is necessary -> if is go to next building  
              cond_shot = false;  
          }
        }

        if (cond_shot) { // if building can shot to all necessary segments
          // test food
          if (act_building->food > 0)
            value += need_food; // building supply food -> increment its value
          else {
            if (need_food == RAC_MAX_CONDITION_VALUE) // test if food is necessary -> if is, go to next building
              cond_food = false;
          }

          if (cond_food) { // if building supply food (if necessary)
            // test energy
            if (act_building->energy > 0)
              value += need_energy; // building supply energy -> increment its value
            else {
              if (need_energy == RAC_MAX_CONDITION_VALUE) // test if energy is necessary -> if is, go to next building
                cond_energy = false;
            }


            // all conditions are valid, put building to result
            if (cond_energy) {
              // find index in array for added building
              for (j = 0; (j < RAC_MAX_CONDITION_RETURNED_UNITS) && (result_values [j] > value); j++);

              if (j < RAC_MAX_CONDITION_RETURNED_UNITS) { // add it
                result_count++;
            
                // move building in result array
                for (k = RAC_MAX_CONDITION_RETURNED_UNITS - 1; k > j; k--) {
                  result_values [k] = result_values [k - 1];
                  result [k] = result [k - 1];
                }
            
                result [j] = act_building->index;
                result_values [j] = value;
              }
            }
          }
          else // food is necesary, but building does not supply it -> go to next building
            continue;
        }
        else // building can not shot to necessary segment -> go to next building
          continue;
      }
      else // building does not accept necessary material -> go to next building
        continue;
    }
    else // it is not possible to buid building on selected area -> go to next building
      continue;
  }

  return result_count;
}

/**
 *  Function returns array and count of buildings according to input conditions. Max count of IDs is RAC_MAX_CONDITION_RETURNED_UNITS.
 *
 *  @param mat input-output array of item ids.
 *  @param mat Array of needed material. (0 - don't need, 1-9 degree of need, 10 = RAC_MAX_CONDITION_VALUE - necessary)
 *  @param shot_seg Array of segments where building need to shot. (0 - don't need, 1-9 degree of need, 10 = RAC_MAX_CONDITION_VALUE - necessary)
 *  @param need_food How much is needed food. (0 - don't need, 1-9 degree of need, 10 = RAC_MAX_CONDITION_VALUE - necessary)
 *  @param need_energy How much is needed energy. (0 - don't need, 1-9 degree of need, 10 = RAC_MAX_CONDITION_VALUE - necessary)
 *
 *  @param start_pos Not used now. (only for compatibility with Findbuilding function)
 *  @param end_pos Not used now. (only for compatibility with Findbuilding function)
 */
int TRACE::FindUnit(int * result, int * mat, int * shot_seg, int need_food, int need_energy, TPOSITION start_pos, TPOSITION end_pos)
{
  int i, j, k;
  TFORCE_ITEM * act_unit;
  bool cond_mat, cond_shot, cond_food, cond_energy;
  int value;
  int result_count = 0;
  int result_values[RAC_MAX_CONDITION_RETURNED_UNITS];

  // clear result array
  for (i = 0; i < RAC_MAX_CONDITION_RETURNED_UNITS; i++) {
    result [i] = -1;
    result_values [i] = -1;
  }
  
  // cycle through all unit types
  for (i = 0; i < units_count; i++) {
    cond_mat = cond_shot = cond_food = cond_energy = true; // init conditions for every unit
    value = 0;
    
    act_unit = units[i];


    // cycle through materials
    for (j = 0; (cond_mat) && (j < scheme.materials_count); j++) {
      if ((act_unit->GetItemType() == IT_WORKER) && (((TWORKER_ITEM *)act_unit)->GetAllowedMaterial(j)))
        value += mat[j];  // if unit is worker and can mine material, increment its value
      else {
        if (mat[j] == RAC_MAX_CONDITION_VALUE) // if unit is not worker or unit can not mine material and material is necessary -> go to next unit
          cond_mat = false;
      }
    }

    if (cond_mat){ // if unit mine all necessary material
      // cycle through all segments
      for (j = 0; (cond_shot) && (j < DAT_SEGMENTS_COUNT); j++){
        // test if unit is offensive and unit can shot to segment
        if ((act_unit->GetArmament()->GetOffensive()) && (act_unit->GetArmament()->GetOffensive()->GetShotableSegments().IsMember(j)))
            value =+ shot_seg[j]; // increment unit value
        else{ // unit is defensive or unit can not shot to segment
          if (shot_seg[j] == RAC_MAX_CONDITION_VALUE) // test if shot_seg is necessary -> if is go to next unit  
            cond_shot = false;  
        }
      }

      
      if (cond_shot) {
        // test food
        if (act_unit->food > 0)
          value += need_food; // unit supply food -> increment its value
        else {
          if (need_food == RAC_MAX_CONDITION_VALUE) // test if food is necessary -> if is, go to next unit
            cond_food = false;
        }

        if (cond_food) { // if unit supply food (if necessary)
          // test energy
          if (act_unit->energy > 0)
            value += need_energy; // unit supply energy -> increment its value
          else {
            if (need_energy == RAC_MAX_CONDITION_VALUE) // test if energy is necessary -> if is, go to next unit
              cond_energy = false;
          }


          // all conditions are valid, put building to result
          if (cond_energy) {
            // find index in array for added unit
            for (j = 0; (j < RAC_MAX_CONDITION_RETURNED_UNITS) && (result_values [j] > value); j++);

            if (j < RAC_MAX_CONDITION_RETURNED_UNITS) { // add it
              result_count++;
        
              // move building in result array
              for (k = RAC_MAX_CONDITION_RETURNED_UNITS - 1; k > j; k--) {
                result_values [k] = result_values [k - 1];
                result [k] = result [k - 1];
              }
        
              result [j] = act_unit->index;
              result_values [j] = value;
            }
          }
        }
        else // food is necesary, but unit does not supply it -> go to next unit
          continue;
      }
      else // unit can not shot to necessary segment -> go to next unit
        continue;
    }
    else // unit can not mine necessary material -> go to next unit
      continue;
  }

  return result_count;
}




//=========================================================================
// Create structures
//=========================================================================

/**
 *  Create table of units and fill it with default values.
 *  @sa TFORCE_ITEM
 */
bool CreateUnitsTable(void)
{
  int i;

  if (!(actual->units = NEW TFORCE_ITEM*[actual->units_count]))
  {
    RacCriticalTable(actual, "units");
    return false;
  }

  for(i = 0; i < actual->units_count; i++) actual->units[i] = NULL;

  return true;
}


/**
 *  Create table of buildings and fill it with default values
 *  @sa TBUILDING_ITEM
 */
bool CreateBuildingsTable(void)
{
  int i;

  if (!(actual->buildings = NEW TBUILDING_ITEM*[actual->buildings_count])) {
    RacCriticalTable(actual, "buildings");
    return false;
  }
  for(i = 0; i < actual->buildings_count; i++) actual->buildings[i] = NULL;

  return true;
}

/**
 *  Create table of sources and fill it with default values.
 *  @sa TFORCE_ITEM
 */

bool CreateSourcesTable(void)
{
  int i;

  if (!(actual->sources = NEW TSOURCE_ITEM*[actual->sources_count]))
  {
    RacCriticalTable(actual, "sources");
    return false;
  }

  for(i = 0; i < actual->sources_count; i++) actual->sources[i] = NULL;

  return true;
}


//=========================================================================
// Loading units
//=========================================================================

/**
 *  Load unit from file (one unit with specified ID).
 *  Parameter first=0 means that reading only dependencies (build_list, 
 *  allowed_materials and repair_list)
 *  @sa TCONF_FILE
 */
bool LoadRacUnit(TCONF_FILE *cf, int id, bool first)
{
  int ival, tid, i;
  float fval;
  bool is_offensive, ok = true, ok_in, ok_all;
  TITEM_TYPE itval;
  char strid[16]; //buffer for id
  TFILE_LINE strval; //buffer for name
  TFILE_LINE pom;
  T_SIMPLE sval;
  T_BYTE bval;
  TWORKER_ITEM * worker = NULL;

  strval[0] = '\0';

  sprintf(pom, "Unit %d", id);
  sprintf(strid, "Unit %d", id);
  ok = cf->SelectSection(pom, true);


  if(first){
    if (ok) ok = cf->ReadStr(strval, const_cast<char*>("item_type"), const_cast<char*>("f"), true); // mandatory input

    if (ok){
      // allocate memory according to item type
      itval = (TITEM_TYPE)strval[0];
      if (itval == IT_FORCE) 
        actual->units[id] = NEW TFORCE_ITEM;
      else 
        actual->units[id] = NEW TWORKER_ITEM();

      if (!actual->units[id]) {
        Critical(LogMsg("Can not allocate memory for '%s' (Unit %d) structures", actual->name, id));
        return false;
      }

      actual->units[id]->index = id;
      
      // read id      
      cf->ReadStr(strval, const_cast<char*>("id"), strid, true);
      actual->units[id]->text_id = NEW char[strlen(strval) + 1];
      actual->units[id]->text_id = strcpy(actual->units[id]->text_id, strval);
      
      // read name
      cf->ReadStr(strval, const_cast<char*>("name"), actual->units[id]->text_id, true);
      actual->units[id]->name = NEW char[strlen(strval) + 1];
      actual->units[id]->name = strcpy(actual->units[id]->name, strval);
    
      // allocate memory for Armament structure
      actual->units[id]->SetArmament(NEW TARMAMENT);
      if (!(actual->units[id]->GetArmament())){
        Critical(LogMsg("Can not allocate memory for '%s' (Unit %d) armament table", actual->name, id));
        return false;
      }
         
      // read size of unit (item must have square shape)
      cf->ReadSimpleRange(&sval, const_cast<char*>("size"), 1, RAC_MAX_UNIT_SIZE, 1);
      actual->units[id]->SetWidth(sval);
      actual->units[id]->SetHeight(sval);

      // read materials
      for (i = 0; i < scheme.materials_count; i++) {
        cf->ReadFloatGE((&fval), const_cast<char*>("materials"), 0, 0);
        actual->units[id]->materials[i] = fval;
      }

      // read maximal life of unit
      cf->ReadIntGE(&ival, const_cast<char*>("max_life"), 1, 1);
      actual->units[id]->SetMaxLife(ival);

      // read max_speed
      for (i = 0; i < DAT_SEGMENTS_COUNT; i++) {
        cf->ReadFloatGE(&fval, const_cast<char*>("max_speed"), 0.01f, 0.01f);
        actual->units[id]->max_speed[i] = fval;
      }

      // read max rotation speed (in conf. file it is in degrees/second, in structures in radians/second
      for (i = 0; i < DAT_SEGMENTS_COUNT; i++) {
        cf->ReadFloatGE(&fval, const_cast<char*>("max_rotation_speed"), 1, 1);
        actual->units[id]->SetMaximumRotation((float) ToRadian(fval), i);
      }

      // read max number of units which can this unit hide to itself
      cf->ReadByteGE(&bval, const_cast<char*>("max_hided_units"), 0, 0);  
      actual->units[id]->SetMaxHidedUnits(bval);

      // read view
      cf->ReadSimpleRange(&actual->units[id]->view, const_cast<char*>("view"), 1, MAP_MAX_SIZE, 1);
      
      // read energy
      cf->ReadInt(&actual->units[id]->energy, const_cast<char*>("energy"), 0);
      
      // read food
      cf->ReadInt(&actual->units[id]->food, const_cast<char*>("food"), 0);
      
      // read move terrain ids
      for (i = 0; i < DAT_SEGMENTS_COUNT; i++){
        cf->ReadIntGE(&tid, const_cast<char*>("move_terrain_id"), 0, 0);
        actual->units[id]->moveable[i].min = GetTerrainType(tid, TO_HIGH, i);
        cf->ReadIntGE(&tid, const_cast<char*>("move_terrain_id"), actual->units[id]->moveable[i].min, actual->units[id]->moveable[i].min);
        actual->units[id]->moveable[i].max = GetTerrainType(tid, TO_LOW, i);
      }
      
      // read stay terrain ids
      for (i = 0; i < DAT_SEGMENTS_COUNT; i++){
        cf->ReadIntGE(&tid, const_cast<char*>("land_terrain_id"), 0, 0);
        actual->units[id]->landable[i].min = GetTerrainType(tid, TO_HIGH, i);
        cf->ReadIntGE(&tid, const_cast<char*>("land_terrain_id"), actual->units[id]->landable[i].min, actual->units[id]->landable[i].min);
        actual->units[id]->landable[i].max = GetTerrainType(tid, TO_LOW, i);
      }
      
      // read selection height
      cf->ReadByteGE(&actual->units[id]->selection_height, const_cast<char*>("selection_height"), 0, 40);
      cf->ReadFloat(&actual->units[id]->burning_x, const_cast<char*>("burning_position"), 0);
      cf->ReadFloat(&actual->units[id]->burning_y, const_cast<char*>("burning_position"), 20);
    
      // read exist, visible, build and land segments
      cf->ReadByteRange(&actual->units[id]->GetExistSegments().min, const_cast<char*>("min_exist_segment_id"), 0, DAT_SEGMENTS_COUNT - 1, 0);
      cf->ReadByteRange(&actual->units[id]->GetExistSegments().max, const_cast<char*>("max_exist_segment_id"), actual->units[id]->GetExistSegments().min, DAT_SEGMENTS_COUNT-1, actual->units[id]->GetExistSegments().min);
      
      for (i = 0; i < DAT_SEGMENTS_COUNT; i++){
        cf->ReadByteRange(&actual->units[id]->visible_segments[i].min, const_cast<char*>("min_max_visible_segment_id"), 0, DAT_SEGMENTS_COUNT - 1, i);
        cf->ReadByteRange(&actual->units[id]->visible_segments[i].max, const_cast<char*>("min_max_visible_segment_id"), actual->units[id]->visible_segments[i].min, DAT_SEGMENTS_COUNT-1, actual->units[id]->visible_segments[i].min);
      }
      
      cf->ReadByteRange(&actual->units[id]->land_segment, const_cast<char*>("land_segment_id"), 0, DAT_SEGMENTS_COUNT - 1, 0);

      // read defence armour
      cf->ReadIntGE(&ival, const_cast<char*>("defence_armour"), 1, 1);
      actual->units[id]->GetArmament()->GetDefense()->SetArmour(ival);
      
      // read defence protection
      cf->ReadFloatRange(&fval, const_cast<char*>("defence_protection"), 0, 1, 0);
      actual->units[id]->GetArmament()->GetDefense()->SetProtection(fval);
      
      cf->ReadBool(&is_offensive, const_cast<char*>("is_offensive"), false);

      if (is_offensive){
        // read offensive agressivity (aggressive, offensive, guarded, ignore)
        cf->ReadStr(strval, const_cast<char*>("offensive_aggressivity"), const_cast<char*>("ignore"), true);
          if (!(strcmp(strval, "ignore")))
            actual->units[id]->SetAggressivity(AM_IGNORE);
          else if (!(strcmp(strval, "guarded")))
            actual->units[id]->SetAggressivity(AM_GUARDED);
          else if (!(strcmp(strval, "offensive")))
            actual->units[id]->SetAggressivity(AM_OFFENSIVE);
          else if (!(strcmp(strval, "aggressive")))
            actual->units[id]->SetAggressivity(AM_AGGRESSIVE);
          else {
            Warning(LogMsg("Incorrect value '%s' of item 'offensive_aggressivity' was set to default value 'ignore'", strval));
            actual->units[id]->SetAggressivity(AM_IGNORE);
          }

        // read accuracy of gun (value must be from 0 to 1)  
        cf->ReadFloatRange(&fval, const_cast<char*>("offensive_accuracy"), 0, 1, 0);
        actual->units[id]->GetArmament()->GetOffensive()->SetAccuracy(fval);
        
        // read range of gun in mapels
        cf->ReadIntGE(&ival, const_cast<char*>("offensive_range"), 1, 1);
        actual->units[id]->GetArmament()->GetOffensive()->SetRange_min(ival);
        cf->ReadIntGE(&ival, const_cast<char*>("offensive_range"), actual->units[id]->GetArmament()->GetOffensive()->GetRange().min, actual->units[id]->GetArmament()->GetOffensive()->GetRange().min);
        actual->units[id]->GetArmament()->GetOffensive()->SetRange_max(ival);
        
        // read guns shot time (time need for one fire on in seconds)
        cf->ReadFloatGE(&fval, const_cast<char*>("offensive_shot_time"), 0, 0);
        actual->units[id]->GetArmament()->GetOffensive()->SetShotTime(fval);

        // read guns wait time (time between shot and feed in seconds)
        cf->ReadFloatGE(&fval, const_cast<char*>("offensive_wait_time"), 0, 0);
        actual->units[id]->GetArmament()->GetOffensive()->SetWaitTime(fval);
        
        // read guns feed time (time needed for feeding in seconds)
        cf->ReadFloatGE(&fval, const_cast<char*>("offensive_feed_time"), 0, 0);
        actual->units[id]->GetArmament()->GetOffensive()->SetFeedTime(fval);

        // read range of segments where gun can fire
        cf->ReadByteRange(&sval, const_cast<char*>("offensive_shotable_seg_min_max"), 0, DAT_SEGMENTS_COUNT - 1, 0);
        actual->units[id]->GetArmament()->GetOffensive()->SetBottomShotableLimit(sval);
        cf->ReadByteRange(&sval, const_cast<char*>("offensive_shotable_seg_min_max"), actual->units[id]->GetArmament()->GetOffensive()->GetShotableSegments().min, DAT_SEGMENTS_COUNT - 1, actual->units[id]->GetArmament()->GetOffensive()->GetShotableSegments().min);
        actual->units[id]->GetArmament()->GetOffensive()->SetUpperShotableLimit(sval);

        // read min and max guns power
        cf->ReadIntGE(&ival, const_cast<char*>("gun_power_min_max"),0 , 0);
        actual->units[id]->GetArmament()->GetOffensive()->SetPowerMin(ival);
        cf->ReadIntGE(&ival, const_cast<char*>("gun_power_min_max"), actual->units[id]->GetArmament()->GetOffensive()->GetPower().min, actual->units[id]->GetArmament()->GetOffensive()->GetPower().min);
        actual->units[id]->GetArmament()->GetOffensive()->SetPowerMax(ival);

        // read projectile settings
        {
          TPROJECTILE_ITEM *itm = NEW TPROJECTILE_ITEM;

          // set projectile id
          sprintf(strval, "%s-projectile", actual->units[id]->text_id);
          itm->text_id = NEW char[strlen(strval) + 1];
          strcpy(itm->text_id, strval);
          
          // read guns mine radius in mapels
          cf->ReadSimpleGE(&sval, const_cast<char*>("offensive_scope"), 0, 0);
          itm->SetScope(sval);

          // read guns projectile speed in mapels per second
          cf->ReadFloatGE(&fval, const_cast<char*>("gun_shot_speed"), 0.01f, 0.01f);
          itm->SetSpeed(fval);

          cf->ReadTextureGroup(&ival, const_cast<char*>("tg_projectile_id"), &actual->tex_table, false, pom);
          itm->tg_stay_id = ival;

          actual->units[id]->GetArmament()->GetOffensive()->SetItem(itm);
        }

        // read gun flags
        ok_all = false;

        do{
          if (ok) ok = cf->ReadStr(strval, const_cast<char*>("offensive_flags"), const_cast<char*>(""), false);
          if ((ok) && (*strval != 0)){
            ok_in = false;

            if (!(strcmp(strval, "gun_same_segment"))){
              actual->units[id]->GetArmament()->GetOffensive()->AddFlag(FIG_GUN_SAME_SEGMENT);
              ok_all = ok_in = true;
            }
            if (!(strcmp(strval, "gun_damage_buildings"))){
              actual->units[id]->GetArmament()->GetOffensive()->AddFlag(FIG_GUN_DAMAGE_BUILDINGS);
              ok_all = ok_in = true;
            }
            if (!(strcmp(strval, "gun_damage_sources"))){
              actual->units[id]->GetArmament()->GetOffensive()->AddFlag(FIG_GUN_DAMAGE_SOURCES);
              ok_all = ok_in = true;
            }
            if (!(strcmp(strval, "gun_position_attack"))){
              actual->units[id]->GetArmament()->GetOffensive()->AddFlag(FIG_GUN_POSITION_ATTACK);
              ok_all = ok_in = true;
            }
            if (!(strcmp(strval, "gun_notlanding"))){
              actual->units[id]->GetArmament()->GetOffensive()->AddFlag(FIG_GUN_NOTLANDING);
              ok_all = ok_in = true;
            }
            if (!(strcmp(strval, "none"))){
              actual->units[id]->GetArmament()->GetOffensive()->SetFlags(FIG_GUN_NOTHING);
              ok_all = ok_in = true;
            }
          
            if (!ok_in) Warning(LogMsg("Bad parameter '%s' in item 'offensive_flags'", strval));
          }
        } while ((ok) && (*strval != 0));
        
        if (!ok_all) {
          cf->ReadStr(strval, const_cast<char*>("offensive_flags"), const_cast<char*>("none"), true); //only for log with line number
          actual->units[id]->GetArmament()->GetOffensive()->SetFlags(FIG_GUN_NOTHING);
        }
        
        ok = true;
      }
      else {
        actual->units[id]->GetArmament()->SetOffensive(NULL);
      }
      
      // read heal time
      cf->ReadFloatGE(&fval, const_cast<char*>("heal_time"), 0, 0);
      actual->units[id]->SetHealTime(fval);

      // read features
      ok_all = false;

      do{
        if (ok) ok = cf->ReadStr(strval, const_cast<char*>("features"), const_cast<char*>(""), false);
        if ((ok) && (*strval != 0)){
          ok_in = false;

          if (!(strcmp(strval, "have_to_land"))){
            actual->units[id]->AddFeatures(RAC_HAVE_TO_LAND);
            ok_all = ok_in = true;
          }
          if (!(strcmp(strval, "heal_when_stay"))){
            actual->units[id]->AddFeatures(RAC_HEAL_WHEN_STAY);
            ok_all = ok_in = true;
          }
          if (!(strcmp(strval, "heal_when_anchor"))){
            actual->units[id]->AddFeatures(RAC_HEAL_WHEN_ANCHOR);
            ok_all = ok_in = true;
          }
          if (!(strcmp(strval, "none"))){
            actual->units[id]->SetFeatures(RAC_NO_FEATURES);
            ok_all = ok_in = true;
          }
          if (!ok_in)Warning(LogMsg("Bad parameter '%s' in item 'features'", strval));
        }
      } while ((ok) && (*strval != 0));

      if (!ok_all) {
        cf->ReadStr(strval, const_cast<char*>("features"), const_cast<char*>("none"), true); //only for log with line number
        actual->units[id]->SetFeatures(RAC_NO_FEATURES);
      }
      ok = true;
      
      // worker has some special variables
      if (actual->units[id]->GetItemType() == IT_WORKER){
        worker = (TWORKER_ITEM *)actual->units[id];
      
        // read max amount of material which can worker carry
        for (i = 0; i < scheme.materials_count; i++){
          cf->ReadIntGE(&ival, const_cast<char*>("max_amount"), 0, 0);
          worker->SetMaxMaterialAmount(i, ival);
        }

        // read mining time (how long worker mining one "portion" of material
        for (i = 0; i < scheme.materials_count; i++){
          cf->ReadFloatGE(&fval, const_cast<char*>("mining_time"), 0, 1);
          worker->SetMiningTime(i, fval);          
          worker->SetUnloadingTime(i, (fval/10)); // uloading is 10 times quiclier than loading time
        }
      
        // read time necessary for one action point (repairing, buildig of one life point...)
        cf->ReadFloatGE(&fval, const_cast<char*>("repairing_time"), 0, 0);
        worker->SetRepairingTime(fval);

#if SOUND

        sprintf(pom, "'Units' / 'Unit %d'", id);

        // read shift (to texture) of mining sound
        for (i = 0; i < scheme.materials_count; i++){
          cf->ReadFloatGE(&fval, const_cast<char*>("mining_sound_shift"), 0, 0);
          worker->SetMiningSoundShift(i, fval);
        }

        // read time how often shoul by mining sound played
        for (i = 0; ok && i < scheme.materials_count; i++){
          cf->ReadFloatGE(&fval, const_cast<char*>("mining_sound_time"), 0, 1);
          worker->SetMiningSoundTime(i, fval);
        }
        
        // read sound of worker when finish work
        if (ok) ok = cf->ReadSound(&worker->snd_workcomplete, const_cast<char*>("snd_workcomplete"), &actual->snd_table, pom);

        // read sound of mining for every material
        for (i = 0; ok && i < scheme.materials_count; i++){
          sprintf(strval, "snd_mine_material%d", i);
          if (ok) ok = cf->ReadSound(&worker->snd_mine_material[i], strval, &actual->snd_table, pom);
        }
#endif

      } // end worker
    }

    if (actual->units[id]->GetItemType() == IT_WORKER){ // worker has some special variables
      sprintf(pom, "'Units'/'Unit %d'", id);
      // read textures (tg_mine_id, tg_repair_id)
      if (ok) ok = cf->ReadTextureGroup(&worker->tg_mine_id, const_cast<char*>("tg_mine_id"), &actual->tex_table, false, pom);
      if (ok) ok = cf->ReadTextureGroup(&worker->tg_repair_id, const_cast<char*>("tg_repair_id"), &actual->tex_table, false, pom);
    }

    sprintf(pom, "'Units'/'Unit %d'", id); //actual section
    // read textures (tg_picture_id, tg_stay_id, tg_move_id, tg_attack_id, tg_zombie_id)
    if (ok) ok = cf->ReadTextureGroup(&actual->units[id]->tg_picture_id, const_cast<char*>("tg_picture_id"), &actual->tex_table, true, pom);
    if (ok) ok = cf->ReadTextureGroup(&actual->units[id]->tg_stay_id, const_cast<char*>("tg_stay_id"), &actual->tex_table, true, pom);
    if (ok) ok = cf->ReadTextureGroup(&actual->units[id]->tg_anchor_id, const_cast<char*>("tg_anchor_id"), &actual->tex_table, false, pom);
    if (ok) ok = cf->ReadTextureGroup(&actual->units[id]->tg_move_id, const_cast<char*>("tg_move_id"), &actual->tex_table, false, pom);
    if (ok) ok = cf->ReadTextureGroup(&actual->units[id]->tg_land_id, const_cast<char*>("tg_land_id"), &actual->tex_table, false, pom);
    if (ok) ok = cf->ReadTextureGroup(&actual->units[id]->tg_rotate_id, const_cast<char*>("tg_rotate_id"), &actual->tex_table, false, pom);
    if (ok) ok = cf->ReadTextureGroup(&actual->units[id]->tg_attack_id, const_cast<char*>("tg_attack_id"), &actual->tex_table, false, pom);
    if (ok) ok = cf->ReadTextureGroup(&actual->units[id]->tg_dying_id, const_cast<char*>("tg_dying_id"), &actual->tex_table, false, pom);
    if (ok) ok = cf->ReadTextureGroup(&actual->units[id]->tg_zombie_id, const_cast<char*>("tg_zombie_id"), &actual->tex_table, false, pom);
    if (ok) ok = cf->ReadTextureGroup(&actual->units[id]->tg_burning_id, const_cast<char*>("tg_burning_id"), &actual->tex_table, false, pom);

#if SOUND

    sprintf(pom, "'Units' / 'Unit %d'", id); // put actual section to variable
    // read selected and command sound
    if (ok) ok = cf->ReadSound(&actual->units[id]->snd_selected, const_cast<char*>("snd_selected"), &actual->snd_table, pom);
    if (ok) ok = cf->ReadSound(&actual->units[id]->snd_command, const_cast<char*>("snd_command"), &actual->snd_table, pom);
    if (ok) ok = cf->ReadSound(&actual->units[id]->snd_ready, const_cast<char*>("snd_ready"), &actual->snd_table, pom);
    if (ok) ok = cf->ReadSound(&actual->units[id]->snd_dead, const_cast<char*>("snd_dead"), &actual->snd_table, pom);
    if (ok) ok = cf->ReadSound(&actual->units[id]->snd_burning, const_cast<char*>("snd_burning"), &actual->snd_table, pom);
    
    if (is_offensive){
      if (ok) ok = cf->ReadSound(&actual->units[id]->snd_fireon, const_cast<char*>("snd_fireon"), &actual->snd_table, pom);
      if (ok) ok = cf->ReadSound(&actual->units[id]->snd_fireoff, const_cast<char*>("snd_fireoff"), &actual->snd_table, pom);
      
      TPROJECTILE_ITEM * itm = actual->units[id]->GetArmament()->GetOffensive()->GetProjectileItem();
      if (ok) ok = cf->ReadSound(&itm->snd_hit, const_cast<char*>("snd_hit"), &actual->snd_table, pom);
    }
#endif


  }
  else{ //reading only dependencies
    if (ok && actual->units[id]->GetItemType() == IT_WORKER){ // worker has some special variables
      worker = (TWORKER_ITEM *)actual->units[id];

      // read allowed materials
      if (ok) {
        do{
          if (ok) ok = cf->ReadStr(strval, const_cast<char*>("allowed_materials"), const_cast<char*>(""), false);
          if ((ok) && (*strval != 0)){
            i = GetMaterialPrgID(strval);

            if (i < 0) Warning(LogMsg("Bad material id '%s' in item 'allowed_materials'", strval));
            else {
              worker->AddAllowedMaterial(i);
              worker->SetOrder(i, worker_order[i]++);
              scheme.materials[i]->mine_list.AddNode(worker);
            }
            
            if (i >= 0 && i < SCH_MAX_MATERIALS_COUNT) actual->workers_item_count[i]++;

          }
        } while ((ok) && (*strval != 0));
        ok = true;
      }

      // read list of buildings that can worker build
      if (ok){
        do{
          if (ok) ok = cf->ReadStr(strval, const_cast<char*>("can_build"), const_cast<char*>(""), false);
          if ((ok) && (*strval != 0)){
            i = GetItemPrgID(strval, (TMAP_ITEM **) actual->buildings, actual->buildings_count);
            if (i != -1){
              worker->build_list.AddNodeToEnd(actual->buildings[i]);
              worker->repair_list.AddNodeToEnd(actual->buildings[i]);
              actual->buildings[i]->is_builded_by_list.AddNodeToEnd(worker);
              actual->buildings[i]->is_repaired_by_list.AddNodeToEnd(worker);
            }
            else Warning(LogMsg("Bad building id '%s' in item 'can_build'", strval));
          }
        } while ((ok) && (*strval != 0));
        ok = true;
      }

      // read list of units that can worker repair (buildable buildings are included
      if (ok){
        do{
          if (ok) ok = cf->ReadStr(strval, const_cast<char*>("can_repair"), const_cast<char*>(""), false);
          if ((ok) && (*strval != 0)){
            i = GetItemPrgID(strval, (TMAP_ITEM **) actual->buildings, actual->buildings_count);
            if (i != -1){
              worker->repair_list.AddNonDuplicitNode(actual->buildings[i]);
              actual->buildings[i]->is_repaired_by_list.AddNonDuplicitNode(worker);
            }
            else{
              i = GetItemPrgID(strval, (TMAP_ITEM **) actual->units, actual->units_count); // basic_item wasn't in building table
              if (i != -1){
                worker->repair_list.AddNonDuplicitNode(actual->units[i]);
                actual->units[i]->is_repaired_by_list.AddNonDuplicitNode(worker);
              }
              else Warning(LogMsg("Bad unit/building id '%s' in item 'can_repair'", strval));
            }
        
          }
        } while ((ok) && (*strval != 0));
      
        ok = true;
      }
    }

    if (ok){
      do{
        if (ok) ok = cf->ReadStr(strval, const_cast<char*>("can_hide"), const_cast<char*>(""), false);
        if ((ok) && (*strval != 0)){
          i = GetItemPrgID(strval, (TMAP_ITEM **) actual->units, actual->units_count);
          if (i != -1){
            actual->units[id]->hide_list.AddNodeToEnd(actual->units[i]);
          }
          else Warning(LogMsg("Bad force/worker unit id '%s' in item 'can_hide'", strval));
        }
      } while ((ok) && (*strval != 0));
      ok = true;
    }


  }

  cf->UnselectSection();

  return ok;
}

/**
 *  Make units table and load all units from file.
 *  @sa TCONF_FILE
 */
bool LoadRacUnits(TCONF_FILE *cf)
{
  bool ok = true;
  int i;

  for  (i = 0; i < SCH_MAX_MATERIALS_COUNT; i++)
    worker_order[i] = 0; // zeroize for every race

  ok = cf->SelectSection(const_cast<char*>("Units"), true);

  if (ok) ok = cf->ReadIntGE(&actual->units_count, const_cast<char*>("count"), 1, 1);

  if (ok) ok = CreateUnitsTable();

  for (i = 0; ok && i < actual->units_count; i++) 
    ok = LoadRacUnit(cf, i, true);

  cf->UnselectSection();
  return ok;
}


//=========================================================================
// Loading buildings
//=========================================================================

/** 
 *  Loading one product (any succesor of TFORCE_ITEM) of factory from file (one product with specified ID).
 *  @sa TCONF_FILE
 *  @sa TFORCE_ITEM
 */

bool LoadRacFactoryProduct(TCONF_FILE *cf, int id, TFACTORY_ITEM * factory)
{
  bool ok = true;
  TFILE_LINE pom;
  char strval[1024]; //buffer for product
  float fval;
  int i;

  sprintf(pom, "Product %d", id);
  ok = cf->SelectSection(pom, true);
  
  // reading from conf file
  if (ok) ok = cf->ReadStr(strval, const_cast<char*>("product"), const_cast<char*>(""), true);
  if (ok) ok = cf->ReadFloatGE(&fval, const_cast<char*>("product_time"), 0, 0);

  // finding red product in units table
  i = GetItemPrgID(strval, (TMAP_ITEM **) actual->units, actual->units_count);
  if (i != -1){
    ok = factory->GetProductsList().AddProduceableItem(actual->units[i], fval);
    actual->units[i]->is_builded_by_list.AddNodeToEnd(factory);
  }
  else Warning(LogMsg("Bad unit id '%s' in item 'product'", strval));

  
  cf->UnselectSection();  

  return ok;
}


/**
 *  Load building from file (one building with specified ID).
 *  @sa TCONF_FILE
 */
bool LoadRacBuilding(TCONF_FILE *cf, int id, bool first)
{
  int ival, tid, i;
  float fval;
  TITEM_TYPE itval;
  bool is_offensive, ok = true, ok_in, ok_all;
  char strid[16], strval[1024]; //buffer for id and name
  TFILE_LINE pom;
  T_SIMPLE sval;
  T_BYTE bval;
  TFACTORY_ITEM *factory;

  strval[0] = '\0';

  sprintf(pom, "Building %d", id);
  sprintf(strid, "Building %d", id);
  ok = cf->SelectSection(pom, true);

  if (first) {
    if (ok) ok = cf->ReadStr(strval, const_cast<char*>("item_type"), const_cast<char*>("b"), true); // mandatory input

    if (ok){
      // allocate memory according to item type
      itval = (TITEM_TYPE)strval[0];
      if (itval == IT_FACTORY) 
        actual->buildings[id] = NEW TFACTORY_ITEM;
      else 
        actual->buildings[id] = NEW TBUILDING_ITEM;

      if (!actual->buildings[id]) {
        Critical(LogMsg("Can not allocate memory for '%s' (Buildin %d) structures", actual->name, id));
        return false;
      }
    
      actual->buildings[id]->index = id;

      // read id        
      cf->ReadStr(strval, const_cast<char*>("id"), strid, true);
      actual->buildings[id]->text_id = NEW char[strlen(strval) + 1];
      actual->buildings[id]->text_id = strcpy(actual->buildings[id]->text_id, strval);

      // read name
      cf->ReadStr(strval, const_cast<char*>("name"), strid, true);
      actual->buildings[id]->name = NEW char[strlen(strval) + 1];
      actual->buildings[id]->name = strcpy(actual->buildings[id]->name, strval);

      // allocate memory for Armament structure
      actual->buildings[id]->SetArmament(NEW TARMAMENT);
      if (!(actual->buildings[id]->GetArmament())){
        Critical(LogMsg("Can not allocate memory for '%s' (Building %d) armament table", actual->name, id));
        return false;
      }

      // read size of building
      cf->ReadSimpleRange(&sval, const_cast<char*>("width"), 1, RAC_MAX_UNIT_SIZE, 1);
      actual->buildings[id]->SetWidth(sval);
      
      cf->ReadSimpleRange(&sval, const_cast<char*>("height"), 1, RAC_MAX_UNIT_SIZE, 1);
      actual->buildings[id]->SetHeight(sval);

      // read maximal life of unit
      cf->ReadIntGE(&ival, const_cast<char*>("max_life"), 1, 1);
      actual->buildings[id]->SetMaxLife(ival);

      // read materials
      for (i = 0; i < scheme.materials_count; i++) {
        cf->ReadFloatGE((&fval), const_cast<char*>("materials"), 0, 0);
        actual->buildings[id]->materials[i] = fval;
        actual->buildings[id]->mat_per_pt[i] = (float)actual->buildings[id]->materials[i] / actual->buildings[id]->GetMaxLife(); // max life is ALWAYS greater than 0 (function before)
      }

      // read max number of units which can this building hide to itself
      cf->ReadByteGE(&bval, const_cast<char*>("max_hided_units"), 0, 0);  
      actual->buildings[id]->SetMaxHidedUnits(bval);

      // read view
      cf->ReadSimpleRange(&actual->buildings[id]->view, const_cast<char*>("view"), 1, MAP_MAX_SIZE, 1);
      // read energy
      cf->ReadInt(&actual->buildings[id]->energy, const_cast<char*>("energy"), 0);
      // read min_energy
      cf->ReadIntRange(&actual->buildings[id]->min_energy, const_cast<char*>("min_energy"), 0, 100, 0);
      // read food
      cf->ReadInt(&actual->buildings[id]->food, const_cast<char*>("food"), 0);

      // read selection height
      cf->ReadByteGE(&actual->buildings[id]->selection_height, const_cast<char*>("selection_height"), 0, 40);
      cf->ReadFloat(&actual->buildings[id]->burning_x, const_cast<char*>("burning_position"), 0);
      cf->ReadFloat(&actual->buildings[id]->burning_y, const_cast<char*>("burning_position"), 20);
    
      // read exist, visible, build and land segments
      cf->ReadByteRange(&actual->buildings[id]->GetExistSegments().min, const_cast<char*>("exist_segment_id"), 0, DAT_SEGMENTS_COUNT-1, 0);
      actual->buildings[id]->GetExistSegments().max = actual->buildings[id]->GetExistSegments().min;
      
      // read vivible segments
      for (i = 0; i < DAT_SEGMENTS_COUNT; i++){
        if (i == actual->buildings[id]->GetExistSegments().max) {
          cf->ReadByteRange(&actual->buildings[id]->visible_segments[i].min, const_cast<char*>("min_max_visible_segment_id"), 0, DAT_SEGMENTS_COUNT-1, i);
          cf->ReadByteRange(&actual->buildings[id]->visible_segments[i].max, const_cast<char*>("min_max_visible_segment_id"), actual->buildings[id]->visible_segments[i].min, DAT_SEGMENTS_COUNT-1, actual->buildings[id]->visible_segments[i].min);
        }
        else
          actual->buildings[id]->visible_segments[i].min = actual->buildings[id]->visible_segments[i].max = 0;
      }

      // read build terrain ids
      for (i = 0; i < DAT_SEGMENTS_COUNT; i++){
        if (i == actual->buildings[id]->GetExistSegments().max) {
          cf->ReadIntGE(&tid, const_cast<char*>("build_terrain_id"), 0, 0);
          actual->buildings[id]->buildable[i].min = GetTerrainType(tid, TO_HIGH, i);
          cf->ReadIntGE(&tid, const_cast<char*>("build_terrain_id"), actual->buildings[id]->buildable[i].min, actual->buildings[id]->buildable[i].min);
          actual->buildings[id]->buildable[i].max = GetTerrainType(tid, TO_LOW, i);
        }
        else
          actual->buildings[id]->buildable[i].min = actual->buildings[id]->buildable[i].max = 0;
      }

      // read defence armour
      cf->ReadIntGE(&ival, const_cast<char*>("defence_armour"), 1, 1);
      actual->buildings[id]->GetArmament()->GetDefense()->SetArmour(ival);
      
      // read defence protection
      cf->ReadFloatRange(&fval, const_cast<char*>("defence_protection"), 0, 1, 0);
      actual->buildings[id]->GetArmament()->GetDefense()->SetProtection(fval);
      
      cf->ReadBool(&is_offensive, const_cast<char*>("is_offensive"), false);

      if (is_offensive){
        // read offensive agressivity (aggressive, offensive, guarded, ignore)
        cf->ReadStr(strval, const_cast<char*>("offensive_aggressivity"), const_cast<char*>("ignore"), true);
          if (!(strcmp(strval, "ignore")))
            actual->buildings[id]->SetAggressivity(AM_IGNORE);
          else if (!(strcmp(strval, "guarded")))
            actual->buildings[id]->SetAggressivity(AM_GUARDED);
          else if (!(strcmp(strval, "offensive")))
            actual->buildings[id]->SetAggressivity(AM_OFFENSIVE);
          else if (!(strcmp(strval, "AM_AGGRESSIVE")))
            actual->buildings[id]->SetAggressivity(AM_AGGRESSIVE);
          else {
            Warning(LogMsg("Incorrect value '%s' of item 'offensive_aggressivity' was set to default value 'ignore'", strval));
            actual->buildings[id]->SetAggressivity(AM_IGNORE);
          }

        // read accuracy of gun (value must be from 0 to 1)  
        cf->ReadFloatRange(&fval, const_cast<char*>("offensive_accuracy"), 0, 1, 0);
        actual->buildings[id]->GetArmament()->GetOffensive()->SetAccuracy(fval);
        
        // read range of gun in mapels
        cf->ReadIntGE(&ival, const_cast<char*>("offensive_range"), 0, 0);
        actual->buildings[id]->GetArmament()->GetOffensive()->SetRange_min(ival);
        cf->ReadIntGE(&ival, const_cast<char*>("offensive_range"), actual->buildings[id]->GetArmament()->GetOffensive()->GetRange().min, actual->buildings[id]->GetArmament()->GetOffensive()->GetRange().min);
        actual->buildings[id]->GetArmament()->GetOffensive()->SetRange_max(ival);
        
        // read guns shot time (time need for one fire on in seconds)
        cf->ReadFloatGE(&fval, const_cast<char*>("offensive_shot_time"), 0, 0);
        actual->buildings[id]->GetArmament()->GetOffensive()->SetShotTime(fval);

        // read guns wait time (time between shot and feed in seconds)
        cf->ReadFloatGE(&fval, const_cast<char*>("offensive_wait_time"), 0, 0);
        actual->buildings[id]->GetArmament()->GetOffensive()->SetWaitTime(fval);
        
        // read guns feed time (time needed for feeding in seconds)
        cf->ReadFloatGE(&fval, const_cast<char*>("offensive_feed_time"), 0, 0);
        actual->buildings[id]->GetArmament()->GetOffensive()->SetFeedTime(fval);

        // read range of segments where gun can fire
        cf->ReadByteRange(&sval, const_cast<char*>("offensive_shotable_seg_min_max"), 0, DAT_SEGMENTS_COUNT - 1, 0);
        actual->buildings[id]->GetArmament()->GetOffensive()->SetBottomShotableLimit(sval);
        ok = cf->ReadByteRange(&sval, const_cast<char*>("offensive_shotable_seg_min_max"), actual->buildings[id]->GetArmament()->GetOffensive()->GetShotableSegments().min, DAT_SEGMENTS_COUNT - 1, actual->buildings[id]->GetArmament()->GetOffensive()->GetShotableSegments().min);
        actual->buildings[id]->GetArmament()->GetOffensive()->SetUpperShotableLimit(sval);

        // read min and max guns power
        cf->ReadIntGE(&ival, const_cast<char*>("gun_power_min_max"),0 , 0);
        actual->buildings[id]->GetArmament()->GetOffensive()->SetPowerMin(ival);
        cf->ReadIntGE(&ival, const_cast<char*>("gun_power_min_max"), actual->buildings[id]->GetArmament()->GetOffensive()->GetPower().min, actual->buildings[id]->GetArmament()->GetOffensive()->GetPower().min);
        actual->buildings[id]->GetArmament()->GetOffensive()->SetPowerMax(ival);

        sprintf(pom, "'Units'/'Unit %d'", id); //actual section

        // read projectile settings
        {
          TPROJECTILE_ITEM *itm = NEW TPROJECTILE_ITEM;

          // set projectile id
          sprintf(strval, "%s-projectile", actual->buildings[id]->text_id);
          itm->text_id = NEW char[strlen(strval) + 1];
          strcpy(itm->text_id, strval);
        
          // read guns mine radius in mapels
          cf->ReadSimpleGE(&sval, const_cast<char*>("offensive_scope"), 0, 0);
          itm->SetScope(sval);

          // read guns projectile speed in mapels per second
          cf->ReadFloatGE(&fval, const_cast<char*>("gun_shot_speed"), 0.01f, 0.01f);
          itm->SetSpeed(fval);
          
          cf->ReadTextureGroup(&ival, const_cast<char*>("tg_projectile_id"), &actual->tex_table, false, pom);
          itm->tg_stay_id = ival;

          actual->buildings[id]->GetArmament()->GetOffensive()->SetItem(itm);
        }

        // read gun flags
        ok_all = false;

        do{
          if (ok) ok = cf->ReadStr(strval, const_cast<char*>("offensive_flags"), const_cast<char*>(""), false);
          if ((ok) && (*strval != 0)){
            ok_in = false;

            if (!(strcmp(strval, "gun_same_segment"))){
              actual->buildings[id]->GetArmament()->GetOffensive()->AddFlag(FIG_GUN_SAME_SEGMENT);
              ok_all = ok_in = true;
            }
            if (!(strcmp(strval, "gun_damage_buildings"))){
              actual->buildings[id]->GetArmament()->GetOffensive()->AddFlag(FIG_GUN_DAMAGE_BUILDINGS);
              ok_all = ok_in = true;
            }
            if (!(strcmp(strval, "gun_damage_sources"))){
              actual->buildings[id]->GetArmament()->GetOffensive()->AddFlag(FIG_GUN_DAMAGE_SOURCES);
              ok_all = ok_in = true;
            }
            if (!(strcmp(strval, "gun_position_attack"))){
              actual->buildings[id]->GetArmament()->GetOffensive()->AddFlag(FIG_GUN_POSITION_ATTACK);
              ok_all = ok_in = true;
            }
            if (!(strcmp(strval, "gun_notlanding"))){
              actual->buildings[id]->GetArmament()->GetOffensive()->AddFlag(FIG_GUN_NOTLANDING);
              ok_all = ok_in = true;
            }
            if (!(strcmp(strval, "none"))){
              actual->buildings[id]->GetArmament()->GetOffensive()->SetFlags(FIG_GUN_NOTHING);
              ok_all = ok_in = true;
            }
          
            if (!ok_in) Warning(LogMsg("Bad parameter '%s' in item 'offensive_flags'", strval));
          }
        } while ((ok) && (*strval != 0));
        
        if (!ok_all) {
          cf->ReadStr(strval, const_cast<char*>("offensive_flags"), const_cast<char*>("none"), true); //only for log with line number
          actual->buildings[id]->GetArmament()->GetOffensive()->SetFlags(FIG_GUN_NOTHING);
        }

        ok = true;
      }
      else {
        actual->buildings[id]->GetArmament()->SetOffensive(NULL);
      }
    }

    sprintf(pom, "'Buildings'/'Building %d'", id); // actual section
    // read textures (tg_picture_id, tg_stay_id, tg_build_id, tg_attack_id, tg_zombie_id)
    if (ok) ok = cf->ReadTextureGroup(&actual->buildings[id]->tg_picture_id, const_cast<char*>("tg_picture_id"), &actual->tex_table, true, pom);
    if (ok) ok = cf->ReadTextureGroup(&actual->buildings[id]->tg_stay_id, const_cast<char*>("tg_stay_id"), &actual->tex_table, true, pom);
    if (ok) ok = cf->ReadTextureGroup(&actual->buildings[id]->tg_build_id, const_cast<char*>("tg_build_id"), &actual->tex_table, false, pom);
    if (ok) ok = cf->ReadTextureGroup(&actual->buildings[id]->tg_dying_id, const_cast<char*>("tg_dying_id"), &actual->tex_table, false, pom);
    if (ok) ok = cf->ReadTextureGroup(&actual->buildings[id]->tg_zombie_id, const_cast<char*>("tg_zombie_id"), &actual->tex_table, false, pom);
    if (ok) ok = cf->ReadTextureGroup(&actual->buildings[id]->tg_burning_id, const_cast<char*>("tg_burning_id"), &actual->tex_table, false, pom);


#if SOUND

    sprintf(pom, "'Buildings' / 'Building %d'", id);
    if (ok) ok = cf->ReadSound(&actual->buildings[id]->snd_selected, const_cast<char*>("snd_selected"), &actual->snd_table, pom);
    if (ok) ok = cf->ReadSound(&actual->buildings[id]->snd_dead, const_cast<char*>("snd_explosion"), &actual->snd_table, pom);
    if (ok) ok = cf->ReadSound(&actual->buildings[id]->snd_burning, const_cast<char*>("snd_burning"), &actual->snd_table, pom);

    if (is_offensive) {        
      if (ok) ok = cf->ReadSound(&actual->buildings[id]->snd_fireon, const_cast<char*>("snd_fireon"), &actual->snd_table, pom);
      if (ok) ok = cf->ReadSound(&actual->buildings[id]->snd_fireoff, const_cast<char*>("snd_fireoff"), &actual->snd_table, pom);

      TPROJECTILE_ITEM * itm = actual->buildings[id]->GetArmament()->GetOffensive()->GetProjectileItem();
      if (ok) ok = cf->ReadSound(&itm->snd_hit, const_cast<char*>("snd_hit"), &actual->snd_table, pom);
    }

#endif
  }
  else { // reading dependencies only
    if (ok) {
      do{
        if (ok) ok = cf->ReadStr(strval, const_cast<char*>("allowed_materials"), const_cast<char*>(""), false);
        if ((ok) && (*strval != 0)){
          i = GetMaterialPrgID(strval);

          if (i < 0) Warning(LogMsg("Bad material id '%s' in item 'allowed_materials'", strval));
          else actual->buildings[id]->AddAllowedMaterial(i);
          
        }
      } while ((ok) && (*strval != 0));
      ok = true;
    }
    
    if (ok && actual->buildings[id]->GetItemType() == IT_FACTORY){ // Factory has some special variables
      factory = (TFACTORY_ITEM *)actual->buildings[id];
      
      ok = cf->SelectSection(const_cast<char*>("Products"), true);

      if (ok) ok = cf->ReadIntGE(&ival, const_cast<char*>("count"), 1, 1);
      for (i = 0; ok && i < ival; i++) 
        ok = LoadRacFactoryProduct(cf, i, factory);

      cf->UnselectSection();
    }

    if (ok){
      ok = cf->ReadStr(strval, const_cast<char*>("ancestor"), const_cast<char*>(""), true);
      if (!(strcmp(strval, "none")))
        actual->buildings[id]->ancestor = NULL;
      else {
        i = GetItemPrgID(strval, (TMAP_ITEM **) actual->buildings, actual->buildings_count);
        if (i != -1){
          if (i == id){
            Warning(LogMsg("Building '%s' can not be 'ancestor' to to itself.", strval));
            actual->buildings[id]->ancestor = NULL;
          }
          else actual->buildings[id]->ancestor = actual->buildings[i];
        }
        else {
          Warning(LogMsg("Bad building id '%s' in item 'ancestor'", strval));
          actual->buildings[id]->ancestor = NULL;
        }
      }
    }
    
    // read units hideable by building
    if (ok){
      do{
        if (ok) ok = cf->ReadStr(strval, const_cast<char*>("can_hide"), const_cast<char*>(""), false);
        if ((ok) && (*strval != 0)){
          i = GetItemPrgID(strval, (TMAP_ITEM **) actual->units, actual->units_count);
          if (i != -1){
            actual->buildings[id]->hide_list.AddNodeToEnd(actual->units[i]);
          }
          else Warning(LogMsg("Bad force/worker unit id '%s' in item 'can_hide'", strval));
        }
      } while ((ok) && (*strval != 0));
      ok = true;
    }
  }
  
  cf->UnselectSection();

  return ok;
}

/**
 *  Make buildings table and load all buildings from file.
 *  @sa TCONF_FILE
 */
bool LoadRacBuildings(TCONF_FILE *cf)
{
  bool ok = true;
  
  ok = cf->SelectSection(const_cast<char*>("Buildings"), true);

  if (ok) ok = cf->ReadIntGE(&actual->buildings_count, const_cast<char*>("count"), 1, 1);
  if (ok) ok = CreateBuildingsTable();

  for (int i = 0; ok && i < actual->buildings_count; i++) 
    ok = LoadRacBuilding(cf, i, true);

  cf->UnselectSection();
  return ok;
}


//=========================================================================
// Loading sources
//=========================================================================

/**
 *  Load source from file (one source with specified ID).
 *  @sa TCONF_FILE
 */
bool LoadRacSource(TCONF_FILE *cf, int id, bool first)
{
  int ival, i, tid;
  float fval;
  bool bval, ok = true;
  char strid[16], strval[1024]; //buffer for id and name
  TFILE_LINE pom;
  T_SIMPLE sval;
  T_BYTE btval;

  strval[0] = '\0';
  sprintf(pom, "Source %d", id);
  sprintf(strid, "Source %d", id);
  ok = cf->SelectSection(pom, true);
  
  if (first){

    if (ok){
      // allocate memory
      actual->sources[id] = NEW TSOURCE_ITEM;

      if (!actual->sources[id]) {
        Critical(LogMsg("Can not allocate memory for '%s' (Source %d) structures", actual->name, id));
        return false;
      }
    
      actual->sources[id]->index = id;

      // read id        
      cf->ReadStr(strval, const_cast<char*>("id"), strid, true);
      actual->sources[id]->text_id = NEW char[strlen(strval) + 1];
      actual->sources[id]->text_id = strcpy(actual->sources[id]->text_id, strval);

      // read name
      cf->ReadStr(strval, const_cast<char*>("name"), strid, true);
      actual->sources[id]->name = NEW char[strlen(strval) + 1];
      actual->sources[id]->name = strcpy(actual->sources[id]->name, strval);

      // allocate memory for Armament structure
      actual->sources[id]->SetArmament(NEW TARMAMENT);
      if (!(actual->sources[id]->GetArmament())){
        Critical(LogMsg("Can not allocate memory for '%s' (Source %d) armament table", actual->name, id));
        return false;
      }

      // read size of building
      cf->ReadSimpleRange(&sval, const_cast<char*>("width"), 1, RAC_MAX_UNIT_SIZE, 1);
      actual->sources[id]->SetWidth(sval);
      
      cf->ReadSimpleRange(&sval, const_cast<char*>("height"), 1, RAC_MAX_UNIT_SIZE, 1);
      actual->sources[id]->SetHeight(sval);

      // read maximal life of unit
      cf->ReadIntGE(&ival, const_cast<char*>("max_life"), 1, 1);
      actual->sources[id]->SetMaxLife(ival);
    
      // read capacity
      cf->ReadIntGE(&ival, const_cast<char*>("capacity"), 0, 0);
      actual->sources[id]->SetCapacity(ival);
      
      // read selection height
      cf->ReadByteGE(&actual->sources[id]->selection_height, const_cast<char*>("selection_height"), 0, 40);
      cf->ReadFloat(&actual->sources[id]->burning_x, const_cast<char*>("burning_position"), 0);
      cf->ReadFloat(&actual->sources[id]->burning_y, const_cast<char*>("burning_position"), 20);
      
      // read max number of units which can this building hide to itself
      cf->ReadByteGE(&btval, const_cast<char*>("max_hided_units"), 0, 0);  
      actual->sources[id]->SetMaxHidedUnits(btval);
      
      // read exist segments
      cf->ReadByteRange(&actual->sources[id]->GetExistSegments().min, const_cast<char*>("exist_segment_id"), 0, DAT_SEGMENTS_COUNT-1, 0);
      actual->sources[id]->GetExistSegments().max = actual->sources[id]->GetExistSegments().min;

      // read build terrain ids
      for (i = 0; i < DAT_SEGMENTS_COUNT; i++){
        if (i == actual->sources[id]->GetExistSegments().max) {
          cf->ReadIntGE(&tid, const_cast<char*>("build_terrain_id"), 0, 0);
          actual->sources[id]->buildable[i].min = GetTerrainType(tid, TO_HIGH, i);
          cf->ReadIntGE(&tid, const_cast<char*>("build_terrain_id"), actual->sources[id]->buildable[i].min, actual->sources[id]->buildable[i].min);
          actual->sources[id]->buildable[i].max = GetTerrainType(tid, TO_LOW, i);
        }
        else
          actual->sources[id]->buildable[i].min = actual->sources[id]->buildable[i].max = 0;
      }

      actual->sources[id]->GetArmament()->SetOffensive(NULL);
      
      // read defence armour
      cf->ReadIntGE(&ival, const_cast<char*>("defence_armour"), 1, 1);
      actual->sources[id]->GetArmament()->GetDefense()->SetArmour(ival);
      
      // read defence protection
      cf->ReadFloat(&fval, const_cast<char*>("defence_protection"), 0);
      actual->sources[id]->GetArmament()->GetDefense()->SetProtection(fval);

      // read renewable
      cf->ReadBool(&bval, const_cast<char*>("renewable"), false);
      actual->sources[id]->SetRenewability(bval);
      
      // read time of regeneration
      cf->ReadFloatGE(&fval, const_cast<char*>("time_of_regeneration"), 0, 1);
      actual->sources[id]->SetRegenerationTime(fval);

      // read time of first regeneration
      cf->ReadFloatGE(&fval, const_cast<char*>("time_of_first_regeneration"), 0, 1);
      actual->sources[id]->SetFirstRegenerationTime(fval);

      // read if it is possible to mine inside the source
      cf->ReadBool(&bval, const_cast<char*>("inside_mining"), false);
      actual->sources[id]->SetInsideMining(bval);;

      // read is source is hided when is empty
      cf->ReadBool(&bval, const_cast<char*>("hideable"), false);
      actual->sources[id]->SetHideable(bval);
    }

    sprintf(pom, "'Sources'/'Source %d'", id); // actual section
    // read textures (tg_picture_id, tg_stay_id, tg_attack_id)
    if (ok) ok = cf->ReadTextureGroup(&actual->sources[id]->tg_picture_id, const_cast<char*>("tg_picture_id"), &actual->tex_table, true, pom);
    if (ok) ok = cf->ReadTextureGroup(&actual->sources[id]->tg_stay_id, const_cast<char*>("tg_stay_id"), &actual->tex_table, true, pom);
    if (ok) ok = cf->ReadTextureGroup(&actual->sources[id]->tg_burning_id, const_cast<char*>("tg_burning_id"), &actual->tex_table, false, pom);
    if (ok) ok = cf->ReadTextureGroup(&actual->sources[id]->tg_dying_id, const_cast<char*>("tg_dying_id"), &actual->tex_table, false, pom);
    if (ok) ok = cf->ReadTextureGroup(&actual->sources[id]->tg_zombie_id, const_cast<char*>("tg_zombie_id"), &actual->tex_table, false, pom);

#if SOUND
    
    sprintf(pom, "'Sources' / 'Source %d'", id);  
    if (ok) ok = cf->ReadSound(&actual->sources[id]->snd_selected, const_cast<char*>("snd_selected"), &actual->snd_table, pom);
    if (ok) ok = cf->ReadSound(&actual->sources[id]->snd_dead, const_cast<char*>("snd_explosion"), &actual->snd_table, pom);
    if (ok) ok = cf->ReadSound(&actual->sources[id]->snd_burning, const_cast<char*>("snd_burning"), &actual->snd_table, pom);

#endif
    

  }
  else{
    if (ok) ok = cf->ReadStr(strval, const_cast<char*>("offer_material"), const_cast<char*>(""), true);
    if (ok){
      i = GetMaterialPrgID(strval);
      if (i < 0) Warning(LogMsg("Bad material id '%s' in item 'offer_material'", strval));
      else actual->sources[id]->SetOfferMaterial(i);
    }

    // read units hideable by source
    if (ok){
      do{
        if (ok) ok = cf->ReadStr(strval, const_cast<char*>("can_hide"), const_cast<char*>(""), false);
        if ((ok) && (*strval != 0)){
          i = GetItemPrgID(strval, (TMAP_ITEM **) actual->units, actual->units_count);
          if (i != -1){
            actual->sources[id]->hide_list.AddNodeToEnd(actual->units[i]);
          }
          else Warning(LogMsg("Bad force/worker unit id '%s' in item 'can_hide'", strval));
        }
      } while ((ok) && (*strval != 0));
      ok = true;
    }
  }
   
  cf->UnselectSection();

  return ok;
}

/**
 *  Make sources table and load all sources from file.
 *  @sa TCONF_FILE
 */
bool LoadRacSources(TCONF_FILE *cf, bool hyper_player)
{
  bool ok = true;

  if (hyper_player) {
    ok = cf->SelectSection(const_cast<char*>("Sources"), true);

    if (ok) ok = cf->ReadIntGE(&actual->sources_count, const_cast<char*>("count"), 0, 0);

    if (actual->sources_count) {
      if (ok) ok = CreateSourcesTable();

      for (int i = 0; ok && i < actual->sources_count; i++) ok = LoadRacSource(cf, i, true);
    }

    cf->UnselectSection();
  }
  return ok;
}


//=========================================================================
// Dependencies
//=========================================================================

/**
 *  Load all dependencies from file.
 *  @sa TCONF_FILE
 */
bool LoadRacDependencies(TCONF_FILE *cf, bool hyper_player)
{
  bool ok = true;
  int i;
  
  /* 
   * it is necesary to read "repair_list" and "build_list" 
   * after loading all buildings and units, because of dependencies
  */
 
  ok = cf->SelectSection(const_cast<char*>("Units"), true);
  for (i = 0; ok && i < actual->units_count; i++) 
    ok = LoadRacUnit(cf, i, false);
  cf->UnselectSection();

  if (ok) ok = cf->SelectSection(const_cast<char*>("Buildings"), true);
  for (i = 0; ok && i < actual->buildings_count; i++) 
    ok = LoadRacBuilding(cf, i, false);
  cf->UnselectSection();

  if (hyper_player) {
    if (ok) ok = cf->SelectSection(const_cast<char*>("Sources"), true);
    for (i = 0; ok && i < actual->sources_count; i++) 
      ok = LoadRacSource(cf, i, false);
    cf->UnselectSection();
  }

  return ok;
}


//=========================================================================
// Races
//=========================================================================

/**
 *  Remove first race from list of races.
 */
void DeleteRace(void)
{
  if (!races) return;

  TRACE *b = races;
  races = races->next;

  delete b;                               // delete race structure
}


/**
 *  Delete list of races by calling function Deleterace to all members of list.
 */
void DeleteRaces(void)
{
  while (races) DeleteRace();
  races = NULL;
  scheme.race = NULL;
}

/**
 *  Create one instance of TRACE and fill it with default values.
 */
bool CreateRace(void)
{
  if (!(actual = NEW TRACE)) {
    Critical("Can not allocate memory for race structure");
    return false;
  }

  *actual->name = 0;
  *actual->id_name = 0;
  *actual->author = 0;

  actual->buildings = NULL;
  actual->units = NULL;
    
  actual->buildings_count = 0;
  actual->units_count = 0;
  
  actual->next = races;
  races = actual;  
  
  for (int i=0; i< SCH_MAX_MATERIALS_COUNT ; i++)
   actual->workers_item_count[i] = 0;

  return true;
}

/**
 *  Load race data and textures from file specified in parameter file_name to
 *  actual.
 */
bool LoadRace(char *file_name, bool hyper_player)
{
  TFILE_NAME racname;
  TCONF_FILE *cf;
  bool ok = true;

  Info (LogMsg ("Loading race '%s'", file_name));

  if ((hyper_player) && (races)) DeleteRaces();

  // create new race
  if (!CreateRace()) {
    Error ("Error creating race.");
    return false;
  }
  
  if (hyper_player) scheme.race = actual;
  
  // set race id_name
  strcpy(actual->id_name, file_name);

  // load texture table
  if (hyper_player) sprintf(racname, "%s%s.dat", SCH_PATH, file_name);
  else sprintf(racname, "%s%s/%s.dat", RAC_PATH, file_name, file_name);

  if (!actual->tex_table.Load(racname, config.tex_mag_filter, config.tex_min_filter))
    goto error;

#if SOUND
  // load sound table
  if (!actual->snd_table.Load(racname))
    goto error;
#endif

  // load race data
  if (hyper_player) sprintf(racname, "%s%s.rac", SCH_PATH, file_name);
  else sprintf(racname, "%s%s/%s.rac", RAC_PATH, file_name, file_name);

  Info(LogMsg("Loading race data from '%s'", racname));

  if (!(cf = OpenConfFile(racname)))
    goto error;

  if (ok) {
    cf->ReadStr(actual->name, const_cast<char*>("name"), file_name, true);
    cf->ReadStr(actual->author, const_cast<char*>("author"), const_cast<char*>(""), true);
  }

  if (ok) ok = cf->ReadTextureGroup(&actual->tg_burning_id, const_cast<char*>("tg_burning_id"), &actual->tex_table, false, const_cast<char*>("primary"));

  if (ok && !hyper_player) {
    if (ok) ok = cf->ReadTextureGroup(&actual->tg_food_id, const_cast<char*>("tg_food_id"), &actual->tex_table, true, const_cast<char*>("primary"));
    if (ok) ok = cf->ReadTextureGroup(&actual->tg_energy_id, const_cast<char*>("tg_energy_id"), &actual->tex_table, true, const_cast<char*>("primary"));
  }

#if SOUND
  if (ok) ok = cf->ReadSound(&actual->snd_error, const_cast<char*>("snd_error"), &actual->snd_table, const_cast<char*>("primary"));
  if (ok) ok = cf->ReadSound(&actual->snd_placement, const_cast<char*>("snd_placement"), &actual->snd_table, const_cast<char*>("primary"));
  if (ok) ok = cf->ReadSound(&actual->snd_construction, const_cast<char*>("snd_construction"), &actual->snd_table, const_cast<char*>("primary"));
  if (ok) ok = cf->ReadSound(&actual->snd_burning, const_cast<char*>("snd_burning"), &actual->snd_table, const_cast<char*>("primary"));
  if (ok) ok = cf->ReadSound(&actual->snd_dead, const_cast<char*>("snd_dead"), &actual->snd_table, const_cast<char*>("primary"));
  if (ok) ok = cf->ReadSound(&actual->snd_explosion, const_cast<char*>("snd_explosion"), &actual->snd_table, const_cast<char*>("primary"));
  if (ok) ok = cf->ReadSound(&actual->snd_building_selected, const_cast<char*>("snd_building_selected"), &actual->snd_table, const_cast<char*>("primary"));
#endif

  if (ok) ok = LoadRacUnits(cf);
  if (ok) ok = LoadRacBuildings(cf);
  if (ok) ok = LoadRacSources(cf, hyper_player);
  if (ok) ok = LoadRacDependencies(cf, hyper_player);

  CloseConfFile(cf);

  if (!ok)
    goto error;

  return true;

error:
  Error(LogMsg("Error loading race '%s'", file_name));
  DeleteRace();

  return false;
}

/**
 *  Load races from all files in RACE directory (files with .rac extension).
 */
bool LoadRaces(void)
{
  int i;

  Info("Loading races");

#ifdef WINDOWS  // on WINDOWS systems
  _finddata_t file;         // file in directory 
  long file_handler;        // handler to first find file in directory
  bool next_file = true;

  if ((file_handler = _findfirst((std::string(RAC_PATH) + "*").c_str(), &file)) == -1L) {  //none file or directories exists
    Critical(LogMsg("%s%s%s", "Can not find any race directories in '", RAC_PATH, "' folder"));
    return false;
  }

  _findclose(file_handler);

#else  // on UNIX systems
  DIR *dir;

  if (!(dir = opendir (RAC_PATH))) {
    Critical(LogMsg ("%s%s%s", "Error opening directory '", RAC_PATH, "'"));
    return false;
  }
  
  closedir(dir);
#endif

  for (i = 1; i < player_array.GetCount(); i++)
    if (!LoadRace((char *)player_array.GetRaceIdName(i).c_str(), false))
      return false;

  return true;
}


/**
 *  Tests whether it is valid pointer to force item of any race.
 *  
 *  @param item Tested pointer.
 */
bool IsValidForceItem(TFORCE_ITEM* item)
{
  TRACE *actual_race = races;
  int i;

  for ( ; actual_race; actual_race = actual_race->next)   //in sequence test the races
    for (i = 0; i < actual_race->units_count; i++)        //test the force items
      if (actual_race->units[i] == item)           //test pointer
        return true;    //success => valid force item

  return false;
}


/**
 *  Returns index of item with user id "usr_id".
 *  If no instance of item has usr_id, returns -1 as error.
 *  @sa TCONF_FILE
 */
int GetItemPrgID(char * usr_id, TMAP_ITEM **table, int count)
{
  int i;  

  for (i = 0; i < count; i++)
    if (!(strcmp(usr_id, table[i]->text_id))) return i;

    return -1;
};

//=========================================================================
// END
//=========================================================================
// vim:ts=2:sw=2:et:

