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
 *  @file dowalk.cpp
 *
 *  Path-finding, step walking and A* agorithm methods
 *
 *  @author Valeria Sventova
 *  @author Jiri Krejsa
 *
 *  @date 2003, 2004
 */


//=========================================================================
// Included files
//=========================================================================

#ifdef WINDOWS
# include <math.h>
#else // on UNIX
# include <stdlib.h>
#endif

#include "dodata.h"
#include "dowalk.h"
#include "dosimpletypes.h"
#include "doselection.h"

//=========================================================================
// Macros
//=========================================================================

//#define PRINT_LOCAL_MAP_DURING_DEBUGGING
#ifdef PRINT_LOCAL_MAP_DURING_DEBUGGING
  #define FILENAME_LOC_MAP "localni.txt"
#endif


//=========================================================================
// Global variables
//=========================================================================

TTHREAD_POOL<TPATH_INFO, TPATH_INFO, TA_STAR_ALG> *threadpool_astar = NULL;
TTHREAD_POOL<TNEAREST_INFO, TNEAREST_INFO, TA_STAR_ALG> *threadpool_nearest = NULL;


//=========================================================================
// TA_STAR_ALG
//=========================================================================

/**
 *  Class constructor.
 *
 *  @param owner Player's ID.
 */
TA_STAR_ALG::TA_STAR_ALG(char owner)
{
  open_node_num = close_node_num = 0;
  closest_goal_field.pos.x =closest_goal_field.pos.y = closest_goal_field.pos.segment = 0;

  if (!(star_map = NEW TA_STAR_MAP(map.height, map.width)))  //allocation of map for A* algorithm
    return;
  if (!(open_set = NEW TSET_FIELD[DAT_SEGMENTS_COUNT*map.width*map.height + 1])) 
  {
    delete star_map;
    star_map = NULL;  
    return;
  }
  if (!(close_set = NEW  TSET_FIELD[DAT_SEGMENTS_COUNT*map.width*map.height + 1]))
  {
    delete star_map;
    star_map = NULL;
    delete []open_set;
    open_set = NULL;
    return;
  }


  playerID = owner;
  for (register int i = 0; i < DAT_SEGMENTS_COUNT; i++)
    unknown[i] = MAP_MAX_TERRAIN_DIFF;
}


/**
 *  Simple constructor for using by thread pool.
 */
TA_STAR_ALG::TA_STAR_ALG()
{
  open_node_num = close_node_num = 0;
  closest_goal_field.pos.x =closest_goal_field.pos.y = closest_goal_field.pos.segment = 0;

  if (!(star_map = NEW TA_STAR_MAP(map.height, map.width)))  //allocation of map for A* algorithm
    return;
  if (!(open_set = NEW TSET_FIELD[DAT_SEGMENTS_COUNT*map.width*map.height + 1])) 
  {
    delete star_map;
    star_map = NULL;
    return;
  }
  if (!(close_set = NEW  TSET_FIELD[DAT_SEGMENTS_COUNT*map.width*map.height + 1]))
  {
    delete star_map;
    star_map = NULL;
    delete []open_set;
    open_set = NULL;
    return;
  }


  playerID = 0;
  for (register int i = 0; i < DAT_SEGMENTS_COUNT; i++)
    unknown[i] = MAP_MAX_TERRAIN_DIFF;
}

/**
 *  Class destructor.
 */
TA_STAR_ALG::~TA_STAR_ALG()
{
  if (star_map)                           //dealocation of map for A* algorithm
    delete star_map;

  if (open_set)
    delete []open_set;

  if (close_set)
    delete []close_set;

  star_map = NULL;
  open_set = NULL;
  close_set = NULL;
}


/**
 *  Inserts new node into the heap OPEN set.
 *
 *  @param field Value of added field.
 */
void TA_STAR_ALG::InsertToOpenSet(TSET_FIELD field)
{
  unsigned int j=0, p=0;
  bool go_on = false;
  TSET_FIELD *p_actual = NULL;
  int x = field.pos.x, y= field.pos.y, z = field.pos.segment;
  double value = field.value;


  if (!open_node_num) //adding the very firt node to the OPEN set
  {
    open_node_num++;
    open_set[1] = field;
    star_map->fields[z][x][y].p_heap_fld = &(open_set[1]);     
    star_map->fields[z][x][y].set_id = WLK_OPEN_SET;
    return; 
  }  

  if (star_map->fields[z][x][y].set_id != WLK_OPEN_SET)    //if the node is not in the heap yet ...
  {      
        open_node_num++;  //new node was added
        j=open_node_num;
INSERT: go_on = j > 1;
        while (go_on)
        {
          p = j / 2;
          if (value < open_set[p].value)
          {
            open_set[j] = open_set[p];
            star_map->fields[open_set[p].pos.segment][open_set[p].pos.x][open_set[p].pos.y].p_heap_fld = &(open_set[j]);
            j=p;
            go_on = j > 1;
          }
          else 
            go_on = false;  //heap is reconstructed
        }
        open_set[j] = field;

        star_map->fields[z][x][y].p_heap_fld = &(open_set[j]);      
        star_map->fields[z][x][y].set_id = WLK_OPEN_SET;        
        return;
  }
  //if the node is in the heap, change its value and find a new position for it....
  p_actual = star_map->fields[z][x][y].p_heap_fld;

  j= p_actual - open_set;      //usefull array starts at position 1

  if (open_set[j].start_dist > field.start_dist) //if mapafield is already in map but the value of his distance from start has changed to smaller one 
  {  
    if (j > 1 )   
      if (value < open_set[j/2].value)  
        goto INSERT;
    open_set[j] = field; //replace it with the new value
    return;
  }
 
}


/**
 *  Extract the minimal node from the OPEN set. 
 *
 *  @note Minimal node will be storaged in zeroth member of heap array.
 *
 *  @return Pointer to minimal member.
 */
TSET_FIELD *TA_STAR_ALG::ExtractMinOpenSet()
{
  TSET_FIELD X;  
  unsigned int j=0, n=0;
  bool go_on =false;

  if (!open_node_num)
    return NULL;  

  X= open_set[open_node_num];  
  open_node_num--;
  open_set[0] = open_set[1];
  star_map->fields[open_set[0].pos.segment][open_set[0].pos.x][open_set[0].pos.y].p_heap_fld = NULL;
  if (!open_node_num)
    return &(open_set[0]);

  j= 1;//open_node_num;
  go_on = 2 <= open_node_num;
  while (go_on)
  {
    n=2*j;
    if (n < open_node_num)
    {
      if (open_set[n+1].value < open_set[n].value) 
        n++;
    }
    if (X.value > open_set[n].value)
    {
      open_set[j] = open_set[n];
      star_map->fields[open_set[n].pos.segment][open_set[n].pos.x][open_set[n].pos.y].p_heap_fld = &(open_set[j]);      
      j=n;
      go_on = 2*j <= open_node_num;
    }
    else
      go_on = false;
  }
  open_set[j] =X;
  star_map->fields[X.pos.segment][X.pos.x][X.pos.y].p_heap_fld = &(open_set[j]);  
  
  return &(open_set[0]);
}


/**
 *  Inserts node to CLOSE set.
 *
 *  @param open_min Pointer to minimum of OPEN set (storage in zeroth member of heap array).
 */
void TA_STAR_ALG::InsertToCloseSet(TSET_FIELD * open_min)
{
  int x = open_min->pos.x, y=open_min->pos.y, z=open_min->pos.segment;  

  close_node_num++;  
  close_set[close_node_num] = *open_min;

  star_map->fields[z][x][y].set_id = WLK_CLOSE_SET;
  star_map->fields[z][x][y].p_heap_fld = &(close_set[close_node_num]);

  return;
}

/**
 *  Tries to find path from start to goal for unit usage A* algorithm. 
 *  If unit has old path list will be destroyed on succes and replace with new one.
 *
 *  @param goal    Goal of path.
 *  @param unit    Pointer to moving unit.
 *  @param loc_map Local map of the owner of the unit.
 *  @param path    Output parameter, function returns a path in it.
 *  @param real_goal     Output parameter, function returns a closest accesible field to the goal.
 *  @param area_res     Output parameter, function returns distance from the area in it, if max_steps_cnt given
 *  @param area_width   Width of area.
 *  @param area_height   Height of area.
 *  @param max_steps_cnt Upper boundary of fields which are taken from open and given to close set.
 *
 *  @return true on success, otherwise false.
 */
bool TA_STAR_ALG::PathFinder(TPOSITION_3D goal, TFORCE_UNIT *unit,TLOC_MAP *loc_map,TPATH_LIST **path,TPOSITION_3D *real_goal,long *area_res,int max_steps_cnt,int area_width, int area_height)
{
  TSET_FIELD *now;
  bool cont = true;
  TSET_FIELD neighbours[WLK_NEIGHBOURS_COUNT];
  TFORCE_ITEM *item = dynamic_cast<TFORCE_ITEM*>(unit->GetPointerToItem());
  double easiest[DAT_SEGMENTS_COUNT] = {MAP_MAX_TERRAIN_DIFF + 1, MAP_MAX_TERRAIN_DIFF + 1, MAP_MAX_TERRAIN_DIFF + 1};
  double hardest[DAT_SEGMENTS_COUNT] = {MAP_MAX_TERRAIN_DIFF + 1, MAP_MAX_TERRAIN_DIFF + 1, MAP_MAX_TERRAIN_DIFF + 1};
  int i;  
  int act_goal_dist;   //square of the distance between goal and field, which was as last added to the CLOSE set
  int min_goal_dist  =0;
  TPOSITION_3D r_pos = goal; //set goal to help variable
  bool goal_set = false; 
  int num_of_steps =0;

  if (max_steps_cnt)
    (*area_res) = -1;    //constraint JIRI
  

  if ((!item) || (!unit))        //finding path for nonmoveable unit
  {    
    return false;
  }

  playerID = unit->GetPlayerID();

  (*real_goal).SetPosition(unit->GetPosition().x, unit->GetPosition().y, unit->GetPosition().segment); //for case that unit cant find its path to goal  
  
  this->star_map->PrepareMap(::map.width, ::map.height);

  this->star_map->ResetStarMap();       //reset star map for use
  
  TPOSITION_3D pos  = unit->GetPosition();
  if (pos.x != LAY_UNAVAILABLE_POSITION && this->star_map->fields[pos.segment][pos.x][pos.y].is_goal)    //start position is same as goal position
  {
    return false;
  }

  if (!max_steps_cnt)
    this->star_map->CreateGoalSet(goal, item->GetWidth());
  else
    this->star_map->CreateGoalSetForArea(goal,unit,area_width,area_height); //for JIRI

  this->star_map->MarksUnitPosition(unit);

  //find easiest and hardest possible moving dificulty of terrain id in each of available segment
  //sets value used for unknown areas in each of available segment
  for (i = 0; i < DAT_SEGMENTS_COUNT; i++)    //each segment
  {
    if (! item->GetExistSegments().IsMember(i))       //segment isn't available
      this->unknown[i] = MAP_MAX_TERRAIN_DIFF + 1;
    else      //segment is available, go over available terrain IDs in segment
    {
      for (register int j = item->moveable[i].min; j <= item->moveable[i].max; j++)
      {
        if (easiest[i] > scheme.terrain_props[i][j].difficulty)   // terrain ID is easier, new minimum
          easiest[i] = scheme.terrain_props[i][j].difficulty;
        if (hardest[i] < scheme.terrain_props[i][j].difficulty)   // terrain ID is harder, new maximum
          hardest[i] = scheme.terrain_props[i][j].difficulty;
      }
      this->unknown[i] = MIN(MAX(easiest[i], map.segments[i].average_surface_difficulty), hardest[i]);
    }
  }

  open_node_num = 0;
  close_node_num = 0;  

  neighbours[0].pos = unit->GetPosition();
  neighbours[0].landed = unit->IsLanding();
  neighbours[0].value = neighbours[0].start_dist = 0;   

  this->InsertToOpenSet(neighbours[0]);    //start position  is added to OPEN set


  closest_goal_field = neighbours[0];  //at the very beginning, the closest available field is the start position

  min_goal_dist =  this->CountDistance(goal, closest_goal_field.pos);  


  while (cont)
  {
    if (!(now  = this->ExtractMinOpenSet()))    //putting out minimal member of OPEN set ,which is organized as a heap
      break;    //path doesn't exist

    this->InsertToCloseSet(now);
    num_of_steps++;  //count how many fields were taken from open to close set

    if (max_steps_cnt && num_of_steps > max_steps_cnt)  //constraint for JIRI
    { 
      *area_res = -1;
      return false;
    }

    //stop searching if we are at he field from the goal set
    if (this->star_map->fields[now->pos.segment][now->pos.x][now->pos.y].is_goal)
    {
      goal_set = true;
      goal = now->pos;
      cont = false;
      break;
    }

    act_goal_dist =  this->CountDistance(goal, now->pos); 

    if (act_goal_dist < min_goal_dist)  //if the field, which was lastly added to the CLOSE set, is closer to the goal, remember it for later possible use
    {
      closest_goal_field = *(now);  // change the field, that is closest to the goal
      min_goal_dist = act_goal_dist;
      //count distance of the newly remembered field
    }
    else if (act_goal_dist == min_goal_dist)
    {
      //figure up the real distance from the start, remember the field, which is closer to the start
      if (now->start_dist < closest_goal_field.start_dist)  //compare distances  of both fields in respect of the start field
      {
        closest_goal_field = *(now);  // change the field, that is closest to the goal
        min_goal_dist = act_goal_dist;
      }
    }

    this->GetAdjacent(neighbours, now, item, loc_map->map, goal,easiest);   //neighbours of actual minimum

    for (int i = 0; i < WLK_NEIGHBOURS_COUNT; i++)    //add neighbours to OPEN set
    {
      TPOSITION_3D pos = neighbours[i].pos;

      if ((pos.x != LAY_UNAVAILABLE_POSITION) && ((!neighbours[i].landed) || (star_map->fields[pos.segment][pos.x][pos.y].is_goal)))
        this->InsertToOpenSet(neighbours[i]);
    }
  }

  if (*path)  //if the way exists from the previous pathfinding it is destroyed
    (*path)->~TPATH_LIST(); 

  
#ifdef PRINT_LOCAL_MAP_DURING_DEBUGGING
  fclose(file);
  if (file != NULL) 
  {
    int j;
    file = fopen("Start distance.txt", "w");
    for (j = star_map->GetHeight() - 1; j >= 0; j--)
    {
      for (i = 0; i < star_map->GetWidth() ; i++)
        fprintf(file, "[%3d,%3d] %3.6f;", i, j, (star_map->fields[1][i][j].p_heap_fld?star_map->fields[1][i][j].p_heap_fld->start_dist:0));
      fprintf(file, "\n");
    }
    fclose(file);
    file = fopen("Heuristic value.txt", "w");
    for (j = star_map->GetHeight() - 1; j >= 0; j--)
    {
      for (i = 0; i < star_map->GetWidth() ; i++)
        fprintf(file, "[%3d,%3d] %3.6f;", i, j, (star_map->fields[1][i][j].p_heap_fld?star_map->fields[1][i][j].p_heap_fld->value:0));
      fprintf(file, "\n");
    }
    fclose(file);
    file = fopen("Set ID.txt", "w");
    for (j = star_map->GetHeight() - 1; j >= 0; j--)
    {
      for (i = 0; i < star_map->GetWidth() ; i++)
        fprintf(file, "%d;", star_map->fields[1][i][j].set_id);
      fprintf(file, "\n");
    }
    fclose(file);
    file = fopen("Goal.txt", "w");
    for (j = star_map->GetHeight() - 1; j >= 0; j--)
    {
      for (i = 0; i < star_map->GetWidth(); i++)
        fprintf(file, "%d;", star_map->fields[1][i][j].is_goal);
      fprintf(file, "\n");
    }
    fclose(file);
    file = fopen("Unit.txt", "w");
    for (j = star_map->GetHeight() - 1; j >= 0; j--)
    {
      for (i = 0; i < star_map->GetWidth() ; i++)
        fprintf(file, "%3d;", (star_map->fields[1][i][j].is_i_am?9:loc_map->map[1][i][j].player_id));
      fprintf(file, "\n");
    }
    fclose(file);
    file = fopen("TerrainID.txt", "w");
    for (j = star_map->GetHeight() - 1; j >= 0; j--)
    {
      for (i = 0; i < star_map->GetWidth() ; i++)
        fprintf(file, "%3d;", loc_map->map[1][i][j].terrain_id);
      fprintf(file, "\n");
    }
    fclose(file);
  }
#endif
  

    
  if (cont)   // path is from the start to the closest_goal_field, whole path to the goal doesn't exist     
  {
    if (closest_goal_field.pos == unit->GetPosition())
    {
      *path = NULL;
      *real_goal = closest_goal_field.pos;  
      if (max_steps_cnt)   //if there is constraint
        *area_res = static_cast<long>(closest_goal_field.start_dist);
      return true;
    }
    else
    {
     *path = this->CreatePathList(closest_goal_field.pos, unit->GetPosition()); //new path will be created
     if (*path == NULL)
     { 
       if (max_steps_cnt)   //if there is constraint
        *area_res = -1;
       return false;
     }
     *real_goal = closest_goal_field.pos;
    }
  }
  else
  {
    TPOSITION_3D pos = unit->GetPosition();    
    if (pos.x != LAY_UNAVAILABLE_POSITION && star_map->fields[pos.segment][pos.x][pos.y].is_goal)    
    {
      *path = NULL;
      *real_goal = goal;   
      if (max_steps_cnt)   //if there is constraint
        *area_res = static_cast<long>(closest_goal_field.start_dist);
      return true;
    }
    else
    {
      *path = this->CreatePathList(goal, unit->GetPosition()); //new path will be created
      if (*path == NULL)
      {      
        if (max_steps_cnt)   //if there is constraint
          *area_res = -1;
        return false;
      }
      *real_goal = goal;
    }
  }

  (*path)->SetRealGoalPosition(r_pos);  
   if (max_steps_cnt)   //if there is constraint
        *area_res = static_cast<long>(closest_goal_field.start_dist);
  return true;
}


inline int TA_STAR_ALG::CountDistance(TPOSITION_3D pos1,TPOSITION_3D pos2)
{
  return (sqr(pos1.x - pos2.x) + sqr(pos1.y - pos2.y) + sqr(pos1.segment - pos2.segment)) ;
}


//=========================================================================
// Local macros - and inline functions

/** Returns difficulty of terrain in local map at position [x,y,z].
 *  @param x  First coordinate.
 *  @param y  Second coordinate.
 *  @param z  Third coordinate, number of segment.
 *  @note Only for using in function GetAdjacent(). */
inline unsigned int TA_STAR_ALG::FieldDifficultyAux (int x, int y, int z, TLOC_MAP_FIELD ***loc_map) {
  if (loc_map[z][x][y].terrain_id == WLK_UNKNOWN_AREA)
    return (unsigned int)unknown[z];

  return scheme.terrain_props[z][loc_map[z][x][y].terrain_id].difficulty;
}


/** Field is testing whether is occupied by own unit.
 *  @param x  First coordinate.
 *  @param y  Second coordinate.
 *  @param z  Third coordinate, number of segment.
 *  @param p  Player ID.
 *  @note Only for using in function GetAdjacent(). */
inline bool TA_STAR_ALG::IsOccupiedByOwn (int x, int y, int z, TLOC_MAP_FIELD ***loc_map) {
  return loc_map[z][x][y].player_id == playerID;
}

/** Field is testing whether is occupied by unit for which path is being found.
 *  @param x  First coordinate.
 *  @param y  Second coordinate.
 *  @param z  Third coordinate, number of segment.
 *  @note Only for using in function GetAdjacent(). */
inline bool TA_STAR_ALG::IsOccupiedByMe (int x, int y, int z) {
  return star_map->fields[z][x][y].is_i_am;
}

/** Returns difficulty of terrain in dependance to local map at position [x,y,z] and occuping by own unit
 *  @param x  First coordinate.
 *  @param y  Second coordinate.
 *  @param z  Third coordinate, number of segment.
 *  @note Only for using in function GetAdjacent(). */
#define FIELD_DIFFICULTY(x,y,z)    (\
              (IsOccupiedByOwn((x),(y),(z),loc_map) && (!IsOccupiedByMe((x), (y), (z)))) ? \
                (\
                 (FieldDifficultyAux((x),(y),(z),loc_map) <= 100) ? (10000) : \
                 (FieldDifficultyAux((x),(y),(z),loc_map)) * (FieldDifficultyAux((x),(y),(z),loc_map)) \
                ) :\
                (FieldDifficultyAux((x),(y),(z),loc_map)))
            
/** Field is testing whether is unavailable for type of unit.
 *  @param x  First coordinate.
 *  @param y  Second coordinate.
 *  @param z  Third coordinate, number of segment.
 *  @note Only for using in function GetAdjacent(). */
inline bool TA_STAR_ALG::IsUnavailableField (int x, int y, int z, TFORCE_ITEM *type, TLOC_MAP_FIELD ***loc_map) {
  return !type->moveable[z].IsMember(loc_map[z][x][y].terrain_id)
         && loc_map[z][x][y].terrain_id != LAY_UNAVAILABLE_POSITION;
}


/** Field is testing whether is occupied by enemy.
 *  @param x  First coordinate.
 *  @param y  Second coordinate.
 *  @param z  Third coordinate, number of segment.
 *  @param p  Player ID.
 *  @note Only for using in function GetAdjacent(). */
#define IS_OCCUPIED_BY_ENEMY(x,y,z,p)   (\
            (loc_map[(z)][(x)][(y)].player_id != WLK_EMPTY_FIELD) \
            && (loc_map[(z)][(x)][(y)].player_id != (p)))


/** Control whether the area is available. Result put into @p a. Difficulty of area is put into @p m.
 *  @param i  Working variable.
 *  @param j  Working variable.
 *  @param x  First coordinate.
 *  @param y  Second coordinate.
 *  @param z  Third coordinate, number of segment.
 *  @param a  Boolean value informing whether area necessary for unit is available. 
 *  @param m  Dificulty of whole area is counted as the dificulty of the most difficult field in this area.
 *  @note Only for using in function GetAdjacent(). */
#define IS_ALLOWED_AREA(i,j,x,y,z,a,m)    {\
          for ((i) = (x); (a) && ((i) < type->GetWidth() + (x)); (i)++)\
            for ((j) = (y); (a) && ((j) < type->GetHeight() + (y)); (j)++)\
            {\
              if (((i) >= map.width) || ((j) >= map.height) || IsUnavailableField((i),(j),(z),type,loc_map) || IS_OCCUPIED_BY_ENEMY((i),(j),(z),playerID))\
                 (a) = false;\
              else if (loc_map[(z)][(x)][(y)].state == WLK_UNKNOWN_AREA)\
              {\
                if (unknown[(z)] > (m))\
                  (m) = unknown[(z)];\
              }\
              else if (FIELD_DIFFICULTY((i),(j),(z)) > (m))\
                (m) = FIELD_DIFFICULTY((i),(j),(z));\
            }\
          }

/** Returns heuristic estimation of distance from start to goal.
 *  @param x  First coordinate of actual position.
 *  @param y  Second coordinate of actual position.
 *  @param z  Third coordinate of actual position, vertical cordinate, number of segment.
 *  @param g    Coordinates of goal in type TPOSITION_3D.
 *  @note Only for using in function GetAdjacent().
 *  @sa TPOSITION_3D */
#define HEURISTIC_DIST(x,y,z,g)        ((MAX(abs((g).x - (x)), abs((g).y - (y))) + 2*(abs((g).segment - (z))) - 1)\
                                              *easiest[(z)]/type->max_speed[(z)])
                                              //*unknown[(z)]/type->max_speed[(z)])

/** Field is testing whether is unlandable for type of unit.
 *  @param x  First coordinate.
 *  @param y  Second coordinate.
 *  @param z  Third coordinate, number of segment.
 *  @note Only for using in function GetAdjacent(). */
#define IS_UNLANDABLE_FIELD(x,y,z)      (\
            (! type->landable[(z)].IsMember(loc_map[(z)][(x)][(y)].terrain_id))\
            && (loc_map[(z)][(x)][(y)].terrain_id != LAY_UNAVAILABLE_POSITION))


/** Control whether area is landable. Result put into @p a. Difficulty of area is put into @p m.
 *  @param i  Working variable.
 *  @param j  Working variable.
 *  @param x  First coordinate.
 *  @param y  Second coordinate.
 *  @param z  Third coordinate, number of segment.
 *  @param a  Boolean value informing whether area necessary for unit is landable.
 *  @param m  Most difficulty terrain of fields in area it is difficulty of all area.
 *  @note Only for using in function GetAdjacent(). */
#define IS_LANDABLE_AREA(i,j,x,y,z,a,m)    { \
            (a) = true; \
            (m) = easiest[(z)]; \
            bool one_landable = false; \
            for ((i) = (x); (a) && ((i) < type->GetWidth() + (x)); (i)++) \
              for ((j) = (y); (a) && ((j) < type->GetHeight() + (y)); (j)++) \
              { \
                if (map.IsInMap((i), (j)) && (!IS_OCCUPIED_BY_ENEMY((i),(j),(z),playerID))) \
                { \
                  if (!IS_UNLANDABLE_FIELD((i),(j),(z)) || (loc_map[(z)][(x)][(y)].state == WLK_UNKNOWN_AREA)) \
                    one_landable = true; \
                  else if (IsUnavailableField((i),(j),(z),type,loc_map)) \
                    (a) = false; \
                  if ((a)) \
                  { \
                    if (loc_map[(z)][(x)][(y)].state == WLK_UNKNOWN_AREA) \
                    { \
                      if (unknown[(z)] > (m)) \
                        (m) = unknown[(z)]; \
                    } \
                    else if (FIELD_DIFFICULTY((i),(j),(z)) > (m)) \
                      (m) = FIELD_DIFFICULTY((i),(j),(z)); \
                  } \
                } \
                else \
                { \
                  (a) = false; \
                } \
              } \
          };

inline double TA_STAR_ALG::GetDistance (TDIRECTION direction, double max_diff, float max_speed)
{
  switch (direction) {
  case DIAGONAL:
    return DIAGONAL_DIST (max_diff / 1000, max_speed);

  case STRAIGHT:
    return STRAIGHT_DIST (max_diff / 1000, max_speed);

  case VERTICAL:
    return VERTICAL_DIST (max_diff / 1000, max_speed);
  }

  // Should never happen.
  return 0;
}

void TA_STAR_ALG::GetAdjacentOneDirection(int x, int y, int z, TDIRECTION direction, TSET_FIELD &neighbour, TSET_FIELD *center, TFORCE_ITEM *type, TLOC_MAP_FIELD ***loc_map, TPOSITION_3D goal, double* easiest)
{
  bool is_in_map;

  // Check if the position is valid. For VERTICAL direction x and y are
  // expected to be valid; for STRAIGHT and DIAGONAL z should be valid.
  if (direction == VERTICAL)
    is_in_map = z >= 0 && z < DAT_SEGMENTS_COUNT &&
                z >= type->GetExistSegments().min && z <= type->GetExistSegments().max;
  else
    is_in_map = x >= 0 && x < map.width && y >= 0 && y < map.height;

  // Position is unavailaible if it isn't in map or is in CLOSE set.
  if (!is_in_map || star_map->fields[z][x][y].set_id == WLK_CLOSE_SET) {
    neighbour.pos.x = LAY_UNAVAILABLE_POSITION;
    return;
  }

  bool allowed_area = true;
  bool is_landing_yet = center->landed;
  double max_diff = easiest[z];
  int i, j;

  // are at the position given by (x,y,z) is in map and isn't in CLOSE set
  IS_ALLOWED_AREA(i, j, x, y, z, allowed_area, max_diff);
  if (allowed_area)
  {     //all fields are available
    neighbour.pos.SetPosition(x, y, z);
    neighbour.landed = false;
    neighbour.start_dist = center->start_dist + GetDistance (direction, max_diff, type->max_speed[z]);
    neighbour.value = neighbour.start_dist + HEURISTIC_DIST(x, y, z, goal);
  }
  else if (is_landing_yet)    //some field is unavailable and unit is landing yet
  {
    neighbour.pos.x = LAY_UNAVAILABLE_POSITION;
    allowed_area = true;
  }
  else      //some field is unavailable and unit isn not landing yet
  {
    IS_LANDABLE_AREA(i, j, x, y, z, allowed_area, max_diff);
    if (allowed_area)
    {     //all fields are landable
      neighbour.pos.SetPosition(x, y, z);
      neighbour.landed = true;
      neighbour.start_dist = center->start_dist + GetDistance (direction, max_diff, type->max_speed[z]) * WLK_LANDING_PENALTY;
      neighbour.value = neighbour.start_dist + HEURISTIC_DIST(x, y, z, goal);
    }
    else      //some field is unlandable
    {
      neighbour.pos.x = LAY_UNAVAILABLE_POSITION;
      allowed_area = true;
    }
  }
}

/**
 *  Fill array of TSET_FIELD with adjacent area.
 *
 *  @param neighbours Pointer to beginning of output array.
 *  @param center     Pointer to field that adjacent area is scaning to.
 *  @param type       Pointer to type of unit that is moved.
 *  @param loc_map    Pointer to local map owned by owner of moved unit.
 *  @param goal       Goal of move.
 *  @param easiest    Easiest terrain for every segment.
 *
 *  @note If some neighbour field is unavailable fills x coordinate with value
 *        #LAY_UNAVALABLE_POSITION.
 */
void TA_STAR_ALG::GetAdjacent(TSET_FIELD *neighbours, TSET_FIELD *center, TFORCE_ITEM *type, TLOC_MAP_FIELD ***loc_map, TPOSITION_3D goal, double* easiest)
{
  int x = center->pos.x;
  int y = center->pos.y;
  int z = center->pos.segment;

  // south, west, north, east
  GetAdjacentOneDirection(x    , y - 1, z, STRAIGHT, neighbours[1], center, type, loc_map, goal, easiest);
  GetAdjacentOneDirection(x - 1, y    , z, STRAIGHT, neighbours[3], center, type, loc_map, goal, easiest);
  GetAdjacentOneDirection(x    , y + 1, z, STRAIGHT, neighbours[5], center, type, loc_map, goal, easiest);
  GetAdjacentOneDirection(x + 1, y    , z, STRAIGHT, neighbours[7], center, type, loc_map, goal, easiest);

  // south east, south west, north west, north east
  GetAdjacentOneDirection(x + 1, y - 1, z, DIAGONAL, neighbours[0], center, type, loc_map, goal, easiest);
  GetAdjacentOneDirection(x - 1, y - 1, z, DIAGONAL, neighbours[2], center, type, loc_map, goal, easiest);
  GetAdjacentOneDirection(x - 1, y + 1, z, DIAGONAL, neighbours[4], center, type, loc_map, goal, easiest);
  GetAdjacentOneDirection(x + 1, y + 1, z, DIAGONAL, neighbours[6], center, type, loc_map, goal, easiest);

  // up, down
  GetAdjacentOneDirection(x, y, z + 1, VERTICAL, neighbours[8], center, type, loc_map, goal, easiest);
  GetAdjacentOneDirection(x, y, z - 1, VERTICAL, neighbours[9], center, type, loc_map, goal, easiest);
}

#undef FIELD_DIFFICULTY
#undef IS_OCCUPIED_BY_ENEMY
#undef IS_ALLOWED_AREA
#undef HEURISTIC_DIST
#undef IS_UNLANDABLE_FIELD
#undef IS_LANDABLE_AREA
// undef local macros
//=========================================================================


/**
 *  Create path list (includes steps for move) from CLOSE set created in PathFinder().
 *
 *  @param goal  Goal of move.
 *  @param start Start of move (unit stay there).
 *
 *  @return Pointer to path list on succes, otherwise @c NULL.
 */
TPATH_LIST * TA_STAR_ALG::CreatePathList(TPOSITION_3D goal, TPOSITION_3D start)
{
  TPATH_LIST *first;
  bool cont = true;
  int a_x, a_y, a_z;
  double min_value;
  TSET_FIELD *min = NULL;

  first = NEW TPATH_LIST();
  first->AddToPath(goal);  

  
  while (cont)
  {
    min = NULL;
    min_value = -1;     //preparing to new cycle
    //south east
    a_x = first->GetFirstFieldX() + 1;
    a_y = first->GetFirstFieldY() - 1;
    a_z = first->GetFirstFieldZ();

    if (start.Compare(a_x, a_y, a_z))    //start has been founded
      break;
    else if ((a_x < map.width) && (a_y >= 0) && (star_map->fields[a_z][a_x][a_y].set_id == WLK_CLOSE_SET))
    {
      min = star_map->fields[a_z][a_x][a_y].p_heap_fld;
      min_value = min->start_dist;
    }

    //south
    a_x--;
    if (start.Compare(a_x, a_y, a_z))    //start has been founded
      break;
    else if ((a_y >= 0) && (star_map->fields[a_z][a_x][a_y].set_id == WLK_CLOSE_SET)
             && ((star_map->fields[a_z][a_x][a_y].p_heap_fld->start_dist < min_value) || (min_value < 0)))
    {   //new minimum neighbour
      min = star_map->fields[a_z][a_x][a_y].p_heap_fld;
      min_value = min->start_dist;
    }

    //south west
    a_x--;
    if (start.Compare(a_x, a_y, a_z))    //start has been founded
      break;
    else if ((a_x >= 0) && (a_y >= 0) && (star_map->fields[a_z][a_x][a_y].set_id == WLK_CLOSE_SET)
             && ((star_map->fields[a_z][a_x][a_y].p_heap_fld->start_dist < min_value) || (min_value < 0)))
    {   //new minimum neighour
      min = star_map->fields[a_z][a_x][a_y].p_heap_fld;
      min_value = min->start_dist;
    }

    //west
    a_y++;
    if (start.Compare(a_x, a_y, a_z))    //start has been founded
      break;
    else if ((a_x >= 0) && (star_map->fields[a_z][a_x][a_y].set_id == WLK_CLOSE_SET)
             && ((star_map->fields[a_z][a_x][a_y].p_heap_fld->start_dist < min_value) || (min_value < 0)))
    {   //new minimum neighour
      min = star_map->fields[a_z][a_x][a_y].p_heap_fld;
      min_value = min->start_dist;
    }

    //north west
    a_y++;
    if (start.Compare(a_x, a_y, a_z))    //start has been founded
      break;
    else if ((a_x >= 0) && (a_y < map.height) && (star_map->fields[a_z][a_x][a_y].set_id == WLK_CLOSE_SET)
             && ((star_map->fields[a_z][a_x][a_y].p_heap_fld->start_dist < min_value) || (min_value < 0)))
    {   //new minimum neighour
      min = star_map->fields[a_z][a_x][a_y].p_heap_fld;
      min_value = min->start_dist;
    }

    //north
    a_x++;
    if (start.Compare(a_x, a_y, a_z))    //start has been founded
      break;
    else if ((a_y < map.height) && (star_map->fields[a_z][a_x][a_y].set_id == WLK_CLOSE_SET)
             && ((star_map->fields[a_z][a_x][a_y].p_heap_fld->start_dist < min_value) || (min_value < 0)))
    {   //new minimum neighour
      min = star_map->fields[a_z][a_x][a_y].p_heap_fld;
      min_value = min->start_dist;
    }

    //north east
    a_x++;
    if (start.Compare(a_x, a_y, a_z))    //start has been founded
      break;
    else if ((a_x < map.width) && (a_y < map.height) && (star_map->fields[a_z][a_x][a_y].set_id == WLK_CLOSE_SET)
             && ((star_map->fields[a_z][a_x][a_y].p_heap_fld->start_dist < min_value) || (min_value < 0)))
    {   //new minimum neighour
      min = star_map->fields[a_z][a_x][a_y].p_heap_fld;
      min_value = min->start_dist;
    }

    //east
    a_y--;
    if (start.Compare(a_x, a_y, a_z))    //start has been founded
      break;
    else if ((a_x < map.width) && (star_map->fields[a_z][a_x][a_y].set_id == WLK_CLOSE_SET)
             && ((star_map->fields[a_z][a_x][a_y].p_heap_fld->start_dist < min_value) || (min_value < 0)))
    {   //new minimum neighour
      min = star_map->fields[a_z][a_x][a_y].p_heap_fld;
      min_value = min->start_dist;
    }

    //up
    a_x--;
    a_z++;
    if (start.Compare(a_x, a_y, a_z))     //start has been founded
      break;
    else if ((a_z < DAT_SEGMENTS_COUNT) && (star_map->fields[a_z][a_x][a_y].set_id == WLK_CLOSE_SET)
             && ((star_map->fields[a_z][a_x][a_y].p_heap_fld->start_dist < min_value) || (min_value < 0)))
    {   //new minimum neighour
      min = star_map->fields[a_z][a_x][a_y].p_heap_fld;
      min_value = min->start_dist;
    }

    //down
    a_z -= 2;
    if (start.Compare(a_x, a_y, a_z))     //start has been founded
      break;
    else if ((a_z >= 0) && (star_map->fields[a_z][a_x][a_y].set_id == WLK_CLOSE_SET)
             && ((star_map->fields[a_z][a_x][a_y].p_heap_fld->start_dist < min_value) || (min_value < 0)))
    {   //new minimum neighour
      min = star_map->fields[a_z][a_x][a_y].p_heap_fld;
      min_value = min->start_dist;
    }

    //adding previous field in the path
    if (min != NULL)
    {
      first->AddToPath(min->pos);
      star_map->fields[min->pos.segment][min->pos.x][min->pos.y].set_id = WLK_PATH_SET;
    }
    else
      cont = false;
  }

  if (cont)   //start was founded
  {
    return first;
  }
  else        //this situation never comes on in correct running
    return NULL;
}



TSEL_NODE * TA_STAR_ALG::GetGroup(TSEL_NODE **unit_list)
{
  TSEL_NODE *group = NULL;
  TSEL_NODE *u = NULL;
  TSEL_NODE *act = NULL;
  int shift_x, shift_y, shift_z;
  bool pointer_moved;

  if (! *unit_list)
    return NULL;
  
  group =  *unit_list;  //vezmi prveho  
  *unit_list = (*unit_list)->next;
  group->next = NULL;
  group->prev = NULL;  

  if (*unit_list)
    (*unit_list)->prev = NULL;
  else
    return group;   //jediny prvok, ktory vratime v group

  act = group;

    
  u = *unit_list;

  //XXX: unused
  //int iRes =0;
  while(u)
  {
    TFORCE_UNIT *unit = u->unit;   //spocitaj vzdialenost od prveho z group
    shift_x = unit->GetPosition().x - group->unit->GetPosition().x;    
    shift_y = unit->GetPosition().y - group->unit->GetPosition().y;  
    shift_z = unit->GetPosition().segment - group->unit->GetPosition().segment;    
    
    pointer_moved = false;

    //iRes = strcmp(u->selected_unit->unit->GetPointerToItem()->name,group->selected_unit->unit->GetPointerToItem()->name);  //do skupiny idu jednotky rovnakeho typu
    //if (!iRes)
    if (u->unit->GetPointerToItem()->index == group->unit->GetPointerToItem()->index)
    {
      if ((abs(shift_x) <= UPP_DIST_BOUNDARY) && (abs(shift_y) <= UPP_DIST_BOUNDARY) && (abs(shift_z) <= 0))
      {   //ak je jednotka blizko prveho,a je to jednotka rovnakeho typu,tak ju vezmi
        TSEL_NODE *del = u;   //vyberame u;
        if (del == *unit_list)  //pohyname zaciatkom zoznamu      
          *unit_list = u->next;      
      
        pointer_moved = true;
        //vypust zo zoznamu jednotiek
        u = u->next;
        if (u)
          u->prev  = del->prev;
        if (del->prev)
          del->prev->next = u;      

        del->next = NULL;
        del->prev = NULL;

        //zarad do group zoznamu na koniec

        act->next = del;
        del->prev = act;
        act = act->next;
      }
    }
    if ((u) && (!pointer_moved))
      u = u->next;
    //prveho v
  }
  return group;
}


TPATH_INFO* TA_STAR_ALG::DevideToGroups(TPATH_INFO* path_info)
{
  TSEL_NODE * unit_list = path_info->unit_list;
  TSEL_NODE * group = NULL;
  TPATH_INFO * group_info = NULL;   
  
  while (1)
  {
    group = GetGroup(&unit_list);   //groupa sa samozrejme vyjme zo zoznamu unit_list

    if (!group)
      return NULL; 

    group_info = pool_path_info->GetFromPool();
    if (!group_info)
    {
      pool_path_info->PutToPool(path_info);
      return NULL;         //???nema sa vyrobit nieco sofistikovanejsie???
    }
    group_info->goal.x = path_info->goal.x;  
    group_info->goal.y = path_info->goal.y;
    group_info->loc_map  = path_info->loc_map;      //???? nema si vziat vlastnu local mapu ??? nieco ako player->GetLoacalMap(); ??
    group_info->request_id  = path_info->request_id;
    group_info->event_type  = path_info->event_type;
    group_info->unit_list = group;            

    threadpool_astar->AddRequest(group_info, &TA_STAR_ALG::MoveGroup);
  }

  //vrat path_info spat do bazenika
  pool_path_info->PutToPool(path_info);
  return NULL;
}


/*
  Fcia najde pre danu skupinu leadra a kazdej jednotke v skupine nastavi cestu, potom kazdej jednotke posle request, ze jej cesta bola najdena a vo fcii ProcessEvent sa bude na to musiet nejak zareagovat
*/
TPATH_INFO* TA_STAR_ALG::MoveGroup(TPATH_INFO* group_info)
{
  TFORCE_UNIT *fu;         // actual movable unit from units list   
  TFORCE_UNIT *leader; //leader of the whole group of movable unitss  
  TMAP_UNIT *u;         // actual unit from units list
  TPOSITION_3D dest, real_dest;
  TSEL_NODE * actual =  group_info->unit_list;
  int shift_x, shift_y, shift_z;  
  double time_stamp;

  leader = NULL;  
  
  group_info->succ = false;
  group_info->path = NULL;

  TPOSITION_3D goal;
  TPATH_LIST *u_path = NULL;

  while (actual)      //loop cez celu skupinu
  {
    u = actual->unit;    //dana jednotka

    if (view_segment == DRW_ALL_SEGMENTS)     
      dest.SetPosition(group_info->goal.x, group_info->goal.y, u->GetPosition().segment);          
    else    
      dest.SetPosition(group_info->goal.x, group_info->goal.y, view_segment);
    
    fu = dynamic_cast<TFORCE_UNIT *>(u);
    playerID = fu->GetPlayerID();

    if (fu)           
    {
      //group_info->succ = fu->GetPlayer()->pathtools/*this*/->PathFinder(dest, fu, fu->GetPlayer()->GetLocalMap(), &(group_info->path), &(group_info->real_goal));
      group_info->succ = /*fu->GetPlayer()->pathtools*/this->PathFinder(dest, fu, fu->GetPlayer()->GetLocalMap(), &(group_info->path), &(group_info->real_goal));
      //succesfull path finding      
      if (group_info->path)
      {
        leader = fu; //set the unit as the leader one        
        //posli jednotke event, ze sa moze pohnut, pribal do neho waiting request id                  
        break;
      }
    }    
    actual = actual->next; 
  }

  time_stamp = glfwGetTime();
  actual = group_info->unit_list;
  while (actual)
  {
    u  = actual->unit;   
    fu = dynamic_cast<TFORCE_UNIT *>(u);    

    if (leader && fu && fu != leader)
    {
      if (group_info->path) //if the path for the leader was found, all the units, which are closer than UPP_DIST_BOUNDARY form the leader will use the same path,but shifted.                    
      {
        //spocitaj posun jednotky od leadra
        shift_x = u->GetPosition().x - leader->GetPosition().x;    
        shift_y = u->GetPosition().y - leader->GetPosition().y;  
        shift_z = u->GetPosition().segment - leader->GetPosition().segment;

        u_path = fu->CreatePathCopy(group_info->path, shift_x, shift_y, shift_z);

        if (group_info->real_goal.x + shift_x < 0)
          real_dest.x = 0;
        else if  (group_info->real_goal.x + shift_x >= ::map.width)
          real_dest.x = ::map.width - 1;
        else
          real_dest.x = group_info->real_goal.x + shift_x;

        if (group_info->real_goal.y + shift_y < 0)
          real_dest.y = 0;
        else if  (group_info->real_goal.y + shift_y >= ::map.height)
          real_dest.y = ::map.height - 1;
        else
          real_dest.y = group_info->real_goal.y + shift_y;

        if (group_info->real_goal.segment + shift_z < 0)
          real_dest.segment = 0;
        else if  (group_info->real_goal.segment + shift_z >= DAT_SEGMENTS_COUNT)
          real_dest.segment = DAT_SEGMENTS_COUNT - 1;
        else
          real_dest.segment = group_info->real_goal.segment + shift_z;
        
      }
      //posli jednotke request, ze sa ma pohnut s danym request Id,ci uz cestu nasla alebo nie.
      u->SendRequest(false, time_stamp,RQ_GROUP_MOVING,group_info->request_id,group_info->succ,real_dest.x,real_dest.y,real_dest.segment,
                      0,0,reinterpret_cast<int>(u_path),group_info->event_type);
    }    
    else
    {
      if (!leader || !fu)   //leader sa nenasiel, tak sa vsetkym jednotkam aspon ma poslat request, ze maju prejst do US_STAY
      {
        u->SendRequest(false, time_stamp,RQ_GROUP_MOVING,group_info->request_id,group_info->succ,group_info->real_goal.x,group_info->real_goal.y,group_info->real_goal.segment,
                      0,0,reinterpret_cast<int>(group_info->path),group_info->event_type);
      }
    }
    actual = actual->next;
  }

  if(leader)
    leader->SendRequest(false, time_stamp,RQ_GROUP_MOVING,group_info->request_id,group_info->succ,group_info->real_goal.x,group_info->real_goal.y,group_info->real_goal.segment,
                      0,0,reinterpret_cast<int>(group_info->path),group_info->event_type);

  group_info->path = NULL;

  for (actual =  group_info->unit_list; actual; actual = actual->next) {
    actual->unit->ReleasePointer();
  }

  pool_path_info->PutToPool(group_info);
  return NULL;  
}

/**
 *  The method computes new path accoring to parameter. This method is used
 *  for computing path by thread pool.
 *
 *  @param path_info  The pointer to instruction for path finding.
 *  @return The method returns NULL all the time because responses queue isn't
 *  used for path finding.
 */
TPATH_INFO* TA_STAR_ALG::ComputePath(TPATH_INFO* path_info)
{
  if (path_info)
  {
    path_info->succ = /*path_info->unit->GetPlayer()->pathtools*/this->PathFinder(path_info->goal,path_info->unit,path_info->loc_map,&(path_info->path),&(path_info->real_goal));
    TPATH_LIST *p_pathlist = path_info->path;
    path_info->path = NULL;

    //last state posli ako dalsi parameter
#if DEBUG_PATHFINDING
    Debug (LogMsg ("SendPathEvent ID = %d", path_info->request_id));
#endif

    path_info->unit->SendRequest(false, glfwGetTime(), RQ_PATH_FINDING, path_info->request_id,path_info->succ,
                                 path_info->real_goal.x, path_info->real_goal.y,path_info->real_goal.segment,path_info->e_simple1,
                                 path_info->e_simple2,reinterpret_cast<int>(p_pathlist),path_info->event_type);                 

    path_info->unit->ReleasePointer();
    pool_path_info->PutToPool(path_info);
    path_info=NULL;
  }
  return NULL;
}

/**
 *  The method computes new path according to parameter. This method is used
 *  for finding nearest building and computing path to it.
 *
 *  @param pnearest_info  Pointer of TNEAREST_INFO structure
 *  @return The method returns NULL all the time because responses queue isn't
 *  used for path finding.
 */
TNEAREST_INFO* TA_STAR_ALG::SearchForNearestBuilding(TNEAREST_INFO* pnearest_info)
{
  if (pnearest_info)
  {
    pnearest_info->nearest = static_cast<TWORKER_UNIT*>(pnearest_info->unit)->GetNearestBuilding(pnearest_info->src_unit, this);      
  
    pnearest_info->unit->SendRequest(false, glfwGetTime(),RQ_NEAREST_SEARCHING,pnearest_info->request_id,pnearest_info->simple1,
                                   pnearest_info->simple2,0,0,0,0, 
                                   reinterpret_cast<int>(pnearest_info->nearest),pnearest_info->event_type);

    pool_nearest_info->PutToPool(pnearest_info);
    pnearest_info = NULL;
  }
  
  return NULL;
}


//=========================================================================
// TSET_FIELD
//=========================================================================

/**
 *  Operator =.
 *
 *  @param model  R-value.
 */
inline TSET_FIELD& TSET_FIELD::operator= (const TSET_FIELD& model)
{
  pos = model.pos;
  landed = model.landed;
  value = model.value;
  start_dist = model.start_dist;
  return *this;
}


//=========================================================================
// TPATH_NODE
//=========================================================================


/**
 *  Class constructor.
 *
 *  @param first_old  Pointer to first member of the list.
 *  @param adding     Added field in the path.
 */
TPATH_NODE::TPATH_NODE(TPATH_NODE *first_old, TPOSITION_3D adding)
{
  next = first_old;
  if (first_old)
    first_old->prev = this;
  for (register int i = 0; i < WLK_NODES_NUM - 1; path_pos[i++].x = LAY_UNAVAILABLE_POSITION);
  first = WLK_NODES_NUM - 1;
  path_pos[first] = adding;
  prev = NULL;
}


/**
 *  Class constructor. Used to add a goal of a path in TA_STAR_ALG::PathFinder().
 *
 *  @param goal  Goal of path.
 *
 *  @sa TA_STAR_ALG
 */
TPATH_NODE::TPATH_NODE(TPOSITION_3D goal)
{
  next = prev = NULL;
  first = WLK_NODES_NUM - 1;
  register int i =0;
  for (i = 0; i < first; path_pos[i++].x = LAY_UNAVAILABLE_POSITION);
  path_pos[i] = goal;
}


/**
 *  Constructor creates copy of the origin from parameters. Each valid field
 *  position shifts about values from parameters.
 */
TPATH_NODE::TPATH_NODE(TPATH_NODE &origin, int sx, int sy, int sz)
{
  register int i;  

  //copying empty fields in the array of path fields
  for (i = 0; (origin.path_pos[i].x == LAY_UNAVAILABLE_POSITION) && (i < WLK_NODES_NUM); i++)
  {
    path_pos[i] = origin.path_pos[i];
  }

  //copy rest fields in the array of the path fields and shift its
  while (i < WLK_NODES_NUM)
  {
    //copy field
    path_pos[i] = origin.path_pos[i];
    //shift coordinates     
    if (static_cast<int>(path_pos[i].x) + sx < 0) 
      path_pos[i].x = 0;
    else if (static_cast<int>(path_pos[i].x) + sx >= ::map.width) 
      path_pos[i].x = ::map.width - 1;
    else 
      path_pos[i].x += sx;    

    if (static_cast<int>(path_pos[i].y) + sy < 0) 
      path_pos[i].y = 0;
    else if (static_cast<int>(path_pos[i].y) + sy >= ::map.height) 
      path_pos[i].y = ::map.height - 1;
    else 
      path_pos[i].y += sy;

    if (static_cast<int>(path_pos[i].segment ) + sz < 0) 
      path_pos[i].segment = 0;
    else if (static_cast<int>(path_pos[i].segment ) + sz >= DAT_SEGMENTS_COUNT) 
      path_pos[i].segment = DAT_SEGMENTS_COUNT - 1;
    else 
      path_pos[i].segment += sz;

    i++;
  }
  first = origin.first;
  prev = next = NULL;
}


//=========================================================================
// TPATH_LIST
//=========================================================================

/**
 *  Add field into path list.
 *
 *  @param adding Added field.
 *
 *  @return Pointer to TPATH_LIST.
 */
inline TPATH_LIST *TPATH_LIST::AddToPath(TPOSITION_3D adding)
{
  if (steps % WLK_NODES_NUM == 0)    //first member of the list is full or list is empty, add new
  {
    f_node = NEW TPATH_NODE(f_node, adding);
    steps++;
    return this;
  }

  steps++;
  f_node->first--;
  f_node->path_pos[f_node->first] = adding;

  return this;
}


/**
 *  Get next position in the path list.
 */
TPOSITION_3D TPATH_LIST::GetNextPosition()
{
  int abs_pos;

  if (a_step == -1)     //it is first step
  {
    a_node = f_node;
    return f_node->path_pos[f_node->first];
  }

  if (f_node && (a_step + 1 > steps)) {   /* OFIK */
    Debug(LogMsg("OFIK before - first:%i, steps:%i, a_step:%i", f_node->first, steps, a_step));

    a_step = steps - 2;
    int node_id = (f_node->first + a_step) / WLK_NODES_NUM;
    for (a_node = f_node; node_id; a_node = a_node->next)
      node_id--;

    Debug(LogMsg("OFIK after  - first:%i, steps:%i, a_step:%i", f_node->first, steps, a_step));
  }

  if (f_node && !a_node) {   /* OFIK */
    Debug(LogMsg("OFIK before - first:%i, steps:%i, a_step:%i", f_node->first, steps, a_step));

    int node_id = (f_node->first + a_step) / WLK_NODES_NUM;
    for (a_node = f_node; node_id; a_node = a_node->next)
      node_id--;

    Debug(LogMsg("OFIK after  - first:%i, steps:%i, a_step:%i", f_node->first, steps, a_step));
  }

  abs_pos = (f_node->first + a_step + 1) % WLK_NODES_NUM;   //where it is in actual node

  if (!abs_pos)               //it is in next nod
  {
    return a_node->next->path_pos[abs_pos];
  }
  else                        //it is in same node
    return a_node->path_pos[abs_pos];
}


/**
 *  Get previous position in the path list.
 */
TPOSITION_3D TPATH_LIST::GetPrevPosition()
{
  int abs_pos;
  TPOSITION_3D aux;

  if ((a_step == -1) || !a_step)     //it is first step or before first step
  {
    aux.SetPosition(LAY_UNAVAILABLE_POSITION,LAY_UNAVAILABLE_POSITION,LAY_UNAVAILABLE_POSITION);
    return f_node->path_pos[f_node->first];
  }
  
  abs_pos = (f_node->first + a_step - 1) % WLK_NODES_NUM;   //where it is in actual node

  if (abs_pos == WLK_NODES_NUM - 1)        //it is in previous node
    return a_node->prev->path_pos[abs_pos];
  else                        //it is in same node
    return a_node->path_pos[abs_pos];
}


TPOSITION_3D TPATH_LIST::GetPostitionInPath(int steps_count)
{
   return f_node->path_pos[WLK_NODES_NUM - steps_count];
}



/**
 *  If it is last step returns false.
 */
bool TPATH_LIST::TestLastPathPosition(void)
{
  if (a_step + 1 >= steps) {   //it was last step
    if (f_node && a_step + 1 > steps) {   /* OFIK: >= namiesto ==. Naviac update a_nodu. */
      Debug(LogMsg("OFIK before - first:%i, steps:%i, a_step:%i", f_node->first, steps, a_step));

      a_step = steps - 1;
      int node_id = (f_node->first + a_step) / WLK_NODES_NUM;
      for (a_node = f_node; node_id; a_node = a_node->next);

      Debug(LogMsg("OFIK after  - first:%i, steps:%i, a_step:%i", f_node->first, steps, a_step));
    }

    return false;
  }
  else
    return true;
}


/**
 *  The method get goal of the path.
 */
TPOSITION_3D TPATH_LIST::GetGoalPosition()
{
  TPATH_NODE *aux = f_node;

 // if (aux)

  while (aux->next)
      aux = aux->next;

  return aux->path_pos[WLK_NODES_NUM - 1];
}


/**
 *  The path list create copy of the itself with shift. Fields in the path moves
 *  about shift.
 *  Returns pointer to copy if it is successful. Otherwise return false.
 */
TPATH_LIST* TPATH_LIST::CreateCopy(int shift_x, int shift_y, int shift_z)
{
  TPATH_NODE *copy_first, *aux_node, *copy_aux_node, *copy_aux_node_prev;

  copy_first = NEW TPATH_NODE(*f_node, shift_x, shift_y, shift_z);   //create copy of the first node

  if (copy_first == NULL)      //test existence of copy
    return NULL;

  copy_aux_node = copy_first;
  copy_aux_node_prev = copy_first;

  for (aux_node = f_node->next; aux_node; aux_node = aux_node->next)    //create copies of the nodes
  {
    copy_aux_node->next = NEW TPATH_NODE(*aux_node, shift_x, shift_y, shift_z);
    if (copy_aux_node->next == NULL)    //copying failed
    {
      for (copy_aux_node = copy_first; copy_aux_node; copy_aux_node = copy_first)   //destroy partially result
      {
        copy_first = copy_first->next;
        delete copy_aux_node;
      }
      return NULL;            //stop copying
    }
    copy_aux_node = copy_aux_node->next;
    copy_aux_node->prev = copy_aux_node_prev;
    copy_aux_node_prev = copy_aux_node;
  }
  //copying of the nodes in the list successful

  TPATH_LIST *copy = NEW TPATH_LIST(copy_first, steps);

  if (copy == NULL)           //copying failed
    for (copy_aux_node = copy_first; copy_aux_node; copy_aux_node = copy_first)   //destroy partially result
    {
      copy_first = copy_first->next;
      delete copy_aux_node;
    }

    // shift real_goal_position  
    if (real_goal_position.x + shift_x < 0)
      copy->real_goal_position.x = 0;
    else if  (real_goal_position.x + shift_x >= ::map.width)
      copy->real_goal_position.x = ::map.width - 1;
    else
      copy->real_goal_position.x = real_goal_position.x + shift_x;

    if (real_goal_position.y + shift_y < 0)
      copy->real_goal_position.y = 0;
    else if  (real_goal_position.y + shift_y >= ::map.height)
      copy->real_goal_position.y = ::map.height - 1;
    else
      copy->real_goal_position.y = real_goal_position.y + shift_y;

    if (real_goal_position.segment + shift_z < 0)
      copy->real_goal_position.segment = 0;
    else if  (real_goal_position.segment + shift_z >= DAT_SEGMENTS_COUNT)
      copy->real_goal_position.segment = DAT_SEGMENTS_COUNT - 1;
    else
      copy->real_goal_position.segment = real_goal_position.segment + shift_z;

  return copy;
}


/**
 *  Constructor. Sets the first node and count of steps from parameters. Sets
 *  actual field to begin of the list.
 *
 *  @note Doesn't control correctness of the parameters.
 */
TPATH_LIST::TPATH_LIST(TPATH_NODE *first, int count)
{
  f_node = first;
  a_node = f_node;
  a_step = -1;
  steps = count;
}


//! Destructor.
TPATH_LIST::~TPATH_LIST()
{
  for (a_node = f_node; a_node; a_node = f_node) 
  {
    f_node = a_node->next; 
    delete a_node;
  }
}



/** 
 *  Get x position of first field in the first node.
 */
inline T_SIMPLE TPATH_LIST::GetFirstFieldX() const
{
  return f_node->path_pos[f_node->first].x;
}


/** 
 *  Get Y position of first field in the first node.
 */
inline T_SIMPLE TPATH_LIST::GetFirstFieldY() const
{
  return f_node->path_pos[f_node->first].y;
}


/** 
 *  Get segment position of first field in the first node.
 */
inline T_SIMPLE TPATH_LIST::GetFirstFieldZ() const
{
  return f_node->path_pos[f_node->first].segment;
}


double TPATH_LIST::CountTime(TFORCE_UNIT *unit)
{
  double spent_time =0.0f;
  TPOSITION_3D pos;
  TPOSITION_3D pos_next;
  float speed = 0.0f;
  unsigned int hardest = 0;
  

  TWORKER_UNIT * worker = static_cast<TWORKER_UNIT*>(unit);
  
  pos = GetNextPosition();  
  

  while(pos != worker->GetGoal()  /*tu sa to bude pytat niekoho ineho nez panacika,kedze cesta nebude ukladana k nemu ale niekam inde */)  //if we arent in the goal position already
  {
    worker->path->IncreaseASteps();
    pos_next = GetNextPosition(); 

    //? obtiaznost hardest sa pocita podla sucasnej pozicie,alebo podla pozicie v dalsom kroku ? teda podla pos alebo pos_next ?
    for (int i=pos_next.x ; i<pos_next.x + worker->GetUnitWidth(); i++)
       for (int j= pos_next.y  ; j< pos_next.y + worker->GetUnitHeight();j++)
       {
          hardest = (scheme.terrain_props[pos_next.segment][map.segments[pos_next.segment].surface[pos_next.x][pos_next.y].t_id].difficulty >hardest)?
             scheme.terrain_props[pos_next.segment][map.segments[pos_next.segment].surface[pos_next.x][pos_next.y].t_id].difficulty:hardest;
       }
        
    speed = static_cast<TFORCE_ITEM*>(unit->GetPointerToItem())->max_speed[pos.segment]*(1.001f - hardest / 1000.0f);
    
    if ((pos_next.x == pos.x) || (pos_next.y == pos.y ) || (pos_next.segment == pos.segment))
    {  //direction is east, nort,south or west (also in segment meaning - up and down)
         spent_time += 1/speed;
    }
    else if ((abs(pos_next.x - pos.x) == 1) && (abs(pos_next.y - pos.y) == 1))   //segment nezahrnat ???
    {  //direction southeast,northeast,northwest,southwest
        spent_time += 1.4142/speed;
    }
    else
      spent_time += 2/speed;
    pos = pos_next;
  }
  return spent_time;  
}


/**
 *  The method increases actuall step. If during step is changed TPATH_NODE 
 *  then change pointer to a_node too.
 */
void TPATH_LIST::IncreaseASteps()
{
  a_step++;

  if ((f_node != NULL) && (((f_node->first + a_step) % WLK_NODES_NUM) == 0))
  {
    a_node = a_node->next;
  }
}


/**
 *  The method decreases actuall step. If during step is changed TPATH_NODE 
 *  then change pointer to a_node too.
 */
void TPATH_LIST::DecreaseASteps()
{
  if ((f_node != NULL) && (((f_node->first + a_step) % WLK_NODES_NUM) == 0))
  {
    a_node = a_node->prev;
  }

  a_step--;
}

//=========================================================================
// TPATH_INFO
//=========================================================================

TPATH_INFO::TPATH_INFO()
{ 
  unit = NULL; 
  loc_map = NULL;
  path = NULL;   
  goal.x = goal.y = goal.segment = real_goal.x = real_goal.y = real_goal.segment = 0; 
  succ = false;    
  request_id = 0;  
  e_simple1=e_simple2 = 0;
  event_type = ET_NONE;
}

void TPATH_INFO::Clear(bool all)
{
  unit = NULL; 
  loc_map = NULL;
  path = NULL;   
  goal.x = goal.y = goal.segment = real_goal.x = real_goal.y = real_goal.segment = 0; 
  succ = false;   
  request_id = 0;
  e_simple1= e_simple2 = 0; 
  event_type = ET_NONE;

  if (all)
    SetNext(NULL);
}

//=========================================================================
// TSEARCH_INFO
//=========================================================================

TNEAREST_INFO::TNEAREST_INFO()
{
  unit = NULL;
  src_unit = NULL;  
  nearest = NULL;
  request_id = 0;  
  event_type = ET_NONE;
  simple1 =0;
  simple2 =0;
  
}

void TNEAREST_INFO::Clear(bool all)
{
  unit = NULL;
  src_unit = NULL;  
  nearest = NULL;
  request_id = 0;  
  event_type = ET_NONE;
  simple1 = 0;
  simple2 = 0;
  
  if (all)
    SetNext(NULL);
}

//=========================================================================
// TA_STAR_MAP
//=========================================================================

/** 
 *  The method allocates fields in the map and sets dimensions sizes.
 *
 *  @param w  The size of the second dimension of the map.
 *  @param h  The size of the first dimension of the map.
 *  @param d  The size of the third dimension of the map. Default value is 
 *  DAT_SEGMENTS_COUNT.
 *  @return The method returns false if allocation was unsuccessful otherwise
 *  returns true.
 */
bool TA_STAR_MAP::CreateNewMap(T_SIMPLE w, T_SIMPLE h, T_SIMPLE d)
{
  TA_STAR_MAP_FIELD ***set_map = NULL;
  int k;

  //destroy map which exists before
  DestroyMap();

  //allocate segments
  set_map = NEW PPTA_STAR_MAP_FIELD[d];
  if (set_map == NULL)
    return false;
  
  for (int j = 0; j < d; j++)
  {
    set_map[j] = NEW PTA_STAR_MAP_FIELD[w];
    if (set_map[j] == NULL)
    {
      for (k = j; k >= 0; k--)
        delete set_map[k];
      delete set_map;
      return false;
    }
    for (int i = 0; i < w; i++)
    {
      set_map[j][i] = NEW TA_STAR_MAP_FIELD[h];
      if (set_map[j][i] == NULL)
      {
        //dealocating everything allocated yet
        for (k = 0; k < i; k++)
          delete set_map[j][k];
        for (k = j; j >= 0; k--)
          delete set_map[k];
        delete set_map;
        return false;
      }    
    }
  }
  //set new values
  fields = set_map;
  depth  = d;
  width  = w;
  height = h;

  return true;
}

/** 
 *  The method deallocates fields in the map and sets dimensions sizes.
 */
void TA_STAR_MAP::DestroyMap()
{
  if (fields) {
    for (int j = 0; j < depth; j++) {

      if (fields[j]) {
        for (int i = 0; i < width; i++)
        {
          if (fields[j][i])
            delete []fields[j][i];
        }
        delete []fields[j];
      }

    }

    delete []fields;
  }

  fields = NULL;

  depth  = WLK_SIZE_NOT_SET;
  width  = WLK_SIZE_NOT_SET;
  height = WLK_SIZE_NOT_SET;
}


/**
 *  The method prepares map with the dimension sizes from the parameters.
 *  It means that allocates new map if the old map (if exists) has another 
 *  dimensions sizes. If the old map is same then the method do nothing.
 *
 *  @param w  The size of the second dimension of the map.
 *  @param h  The size of the first dimension of the map.
 *  @param d  The size of the third dimension of the map. Default value is 
 *  DAT_SEGMENTS_COUNT.
 *  @return The method returns false if asked map wasn't prepared. Otherwise
 *  returns true.
 */
bool TA_STAR_MAP::PrepareMap(T_SIMPLE w, T_SIMPLE h, T_SIMPLE d)
{
  //if exists some map yet and its sizes of the dimensions are same then job is done
  if ((fields != NULL) && (depth == d) && (width == w) && (height == h))
      return true;

  //try to create new map with asked size
  if (CreateNewMap(w, h, d))
    return true;
  else
    return false;
}


/** 
 *  Constructor which allocates fields in the map. The map will be prepared if
 *  the allocation is successful.
 *
 *  @param w  The size of the second dimension of the map.
 *  @param h  The size of the first dimension of the map.
 *  @param d  The size of the third dimension of the map. Default value is 
 *  DAT_SEGMENTS_COUNT.
 */
TA_STAR_MAP::TA_STAR_MAP(T_SIMPLE w, T_SIMPLE h, T_SIMPLE d)
{
  fields = NULL; 
  height = width = depth = WLK_SIZE_NOT_SET;

  CreateNewMap(w, h, d);
}


/** 
 *  Destructor deletes map if exists.
 */
TA_STAR_MAP::~TA_STAR_MAP()
{
  DestroyMap();
}


/**
 *  Reset auxilliary map for A* algorithm.
 */
void TA_STAR_MAP::ResetStarMap()
{
  register int i, j, k;

  for (k = 0; k < depth; k++)
    for (i = 0; i < width; i++)
      for (j = 0; j < height; j++)
      {
        fields[k][i][j].set_id = WLK_NO_SET;
        fields[k][i][j].p_heap_fld = NULL;
        fields[k][i][j].is_goal = false;
        fields[k][i][j].is_i_am = false;
      }
}


/**
 *  The method creates Set of goal positions.
 *  
 *  @param goal The goal of the path which is finding.
 *  @param welt The size of the welt around unit which is staying at the goal
 *  position.
 */
void TA_STAR_MAP::CreateGoalSet(TPOSITION_3D goal, T_SIMPLE welt)
{
  TMAP_UNIT *map_unit = NULL;

  map_unit = map.segments[goal.segment].surface[goal.x][goal.y].unit;
  if (map_unit)
  {
    TPOSITION_3D u_position = map_unit->GetPosition();

    for (int s = static_cast<TMAP_ITEM*>(map_unit->GetPointerToItem())->GetExistSegments().min; s <= static_cast<TMAP_ITEM*>(map_unit->GetPointerToItem())->GetExistSegments().max; s++)
      for (int i = u_position.x - welt; i <= u_position.x + map_unit->GetUnitWidth() ; i++)
        for (int j = u_position.y - welt; j <= u_position.y + map_unit->GetUnitHeight() ; j++)      
          if (map.IsInMap(i,j,s))
            fields[s][i][j].is_goal = true;
  } 
  else
    fields[goal.segment][goal.x][goal.y].is_goal = true;
}


void TA_STAR_MAP::CreateGoalSetForArea(TPOSITION_3D pos,TFORCE_UNIT *unit,int area_width,int area_height)
{

  //mark as goal set just the area positio
  if (!unit)
    return;

  for (int i = pos.x - unit->GetPointerToItem()->GetWidth() +1 ; i <= pos.x + area_width -1; i++)
    for (int j= pos.y - unit->GetPointerToItem()->GetHeight() +1 ; j <= pos.y + area_height -1 ;j++)
    {
          if (map.IsInMap(i,j,pos.segment))
            fields[pos.segment][i][j].is_goal = true;
    }
}

/**
 *  The method marks fields under unit position.
 *
 *  @param unit
 */
void TA_STAR_MAP::MarksUnitPosition(TFORCE_UNIT *unit)
{
  for (int i = unit->GetPosition().x; i < unit->GetPosition().x +unit->GetUnitWidth(); i++)
    for (int j = unit->GetPosition().y; j < unit->GetPosition().y + unit->GetUnitHeight(); j++)      
      if (map.IsInMap(i,j,unit->GetPosition().segment))
        fields[unit->GetPosition().segment][i][j].is_i_am = true;
}


//=========================================================================
// END
//=========================================================================
// vim:ts=2:sw=2:et:

