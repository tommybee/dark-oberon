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
 *  @file dowalk.h
 *
 *  Declarations needed for path-finding, step walking and A* agorithm.
 *
 *  @author Valeria Sventova
 *  @author Jiri Krejsa
 *
 *  @date 2003, 2004
 */

#ifndef __dowalk_h__
#define __dowalk_h__

//========================================================================
// Forward declarations
//========================================================================

struct TLOC_MAP_FIELD;
struct TSET_FIELD;
struct TA_LOC_MAP_FIELD;
class TPATH_LIST;
struct TPATH_NODE;
class TA_STAR_ALG;
class TSEL_NODE;
struct TNODE_OF_UNITS_LIST;


//========================================================================
// Definitions
//========================================================================

#define WLK_NODES_NUM         256             //!< Number of fields in heap_nodes array.
#define WLK_UNKNOWN_AREA      255             //!< Value sets as state in the local map when field is in the unknown area.
#define WLK_WARFOG            0               //!< Value sets as state in the local map when field is in the warfog.
#define WLK_MAX_MAP_SIZE      (62500 * DAT_SEGMENTS_COUNT + 1)            //!< Size of help array.
#define WLK_EMPTY_FIELD       255             //!< Field without any player.
#define WLK_NO_GUARDED        0               //!< Signifies field, which isn't guarded in loc_field array. Isn't used now!

#define WLK_OPEN_SET          1               //!< Value in the auxiliary map in A* algorithm - field is in the OPEN set.
#define WLK_CLOSE_SET         2               //!< Value in the auxiliary map in A* algorithm - field is in the CLOSE set.
#define WLK_PATH_SET          3               //!< Value in the auxiliary map in A* algorithm - field is on the final path.
#define WLK_NO_SET            0               //!< Value in the auxiliary map in A* algorithm - field isn't in the set.

//! Sign to the local map that some building was put down on the ground.
#define WLK_BUILDING_PLACED   100

#define WLK_NEIGHBOURS_COUNT  10              //!< Count of neighbours.

#define WLK_LANDING_PENALTY   3               //!< Recourse difficulty of the field or landing time -  used as multiple constant.

#define UPP_DIST_BOUNDARY     5               //!< Number of the fields, that can be unit distant from the leader unit, so that it is in the same group
#define WLK_SIZE_NOT_SET      255             //!< Size of the dimension if the dimension doesn't exist.


//========================================================================
// Included files
//========================================================================

#include "cfg.h"
#include "doalloc.h"
#include "domap.h"
#include "dothreadpool.h"


//========================================================================
// Froward declaration
//========================================================================

class TPATH_INFO;
class TNEAREST_INFO;

//=========================================================================
// Macros
//=========================================================================

/** @def STRAIGHT_DIST(_a_,_s_)
 *  @param _a_ difficulty of field.
 *  @param _s_ speed of unit in segment
 *  Modifier of difficulty in straight distance (south, west, east or north neighbour).
 *  Difficulty is same.*/
/** @def DIAGONAL_DIST(_a_,_s_)
 *  @param _a_ difficulty of field.
 *  @param _s_ speed of unit in segment
 *  Modifier of difficulty in diagonal distance.
 *  Difficulty is diff*sqrt(2)*/
/** @def VERTICAL_DIST(_a_,_s_)
 *  @param _a_ difficulty of field.
 *  @param _s_ speed of unit in segment
 *  Modifier of difficulty in vertical distance (upper or lower segment).
 *  Difficulty is doubled.*/
#define STRAIGHT_DIST(_a_,_s_)      ((_a_) / (_s_))
#define DIAGONAL_DIST(_a_,_s_)      (((_a_) * SQRT_2) / (_s_))
#define VERTICAL_DIST(_a_,_s_)      (2*(_a_) / (_s_))


//========================================================================
// Typedefs
//========================================================================

/**
 *  One field in player's local map.
 *  Contains information about state of this field like warfog or terrain id.
 */
struct TLOC_MAP_FIELD {

  T_SIMPLE state;         //!< Visibility and warfog state. [#WLK_WARFOG, #WLK_UNKNOWN_AREA]
  T_SIMPLE terrain_id;    //!< Terrain identifier of field.
  T_SIMPLE player_id;     //!< Inforamtion which units stands on this field.
  T_SIMPLE guard;         //!< Info wheather my unit is seen by enemy unit.

  /** Structure constructor. Sets default values. */
  TLOC_MAP_FIELD()
  {    
    state       = WLK_UNKNOWN_AREA;   
    player_id   = WLK_EMPTY_FIELD;    
    guard       = WLK_NO_GUARDED;
    terrain_id  = WLK_UNKNOWN_AREA;
  };
};


typedef TLOC_MAP_FIELD *PLOC_MAP_FIELD;


/**
 *  Member of sets OPEN and CLOSE in A* algorithm.
 *
 *  @sa TA_STAR_ALG
 */
struct TSET_FIELD {
  TPOSITION_3D pos;     //!< Coordinates of field.
  bool landed;          //!< If unit lands at position pos,landed is set to true.
  double value;       //!< Value counted by A* algorithm (sume of the distance form the start, terrain_id and the distance from the goal).
  double start_dist;    //!< Real distance from start.

  TSET_FIELD& operator= (const TSET_FIELD& model);  //!< Assigning operator.

  //! Constructor.
  TSET_FIELD()
    { landed = false; value = start_dist = 0;};
};


/**
 *  Field in local map of A* algorithm. Contains information needed for
 *  algorithm.
 *
 *  @sa TA_STAR_ALG
 */
struct TA_STAR_MAP_FIELD {
  char set_id;              //!< Sign of the set, whether map fields belongs to during the A* algorithm.
  TSET_FIELD *p_heap_fld;   //!< Pointer to the heap, where the field was inserted during the A* algorithm.
  bool is_goal;             //!< Sign whether field is in the set of goal field or not
  bool is_i_am;             //!< Sign whether field is under unit for which path is being found.

  //! Constructor.
  TA_STAR_MAP_FIELD()
    { set_id = 0; p_heap_fld = NULL; is_goal = false; is_i_am = false;};
};

//! Alias name for pointer to TA_STAR_MAP_FIELD for simple usage.
typedef TA_STAR_MAP_FIELD *PTA_STAR_MAP_FIELD;
//! Alias name for pointer to PTA_STAR_MAP_FIELD for simple usage.
typedef PTA_STAR_MAP_FIELD* PPTA_STAR_MAP_FIELD;

/**
 *  The map which encapsulate fields and dimension sizes of the auxiliary map
 *  used by A* algorithm.
 *
 *  @sa TA_STAR_ALG, TA_STAR_MAP_FIELD
 */
class TA_STAR_MAP {
public:
  /** @return The method returns map height.*/
  T_SIMPLE GetHeight() const
    { return height;}

  /** @return The method returns map width.*/
  T_SIMPLE GetWidth() const
    { return width;}

  /** @return The method returns map depth.*/
  T_SIMPLE GetDepth() const
    { return depth;}

private:
  /** Constructor which allocates fields in the map. */
  TA_STAR_MAP(T_SIMPLE width, T_SIMPLE height, T_SIMPLE depth = DAT_SEGMENTS_COUNT);
  /** Constructor which doesn't allocate fields in the map.*/
  TA_STAR_MAP()
    { fields = NULL; height = width = depth = WLK_SIZE_NOT_SET;};
  /** Destructor deletes map if exists.*/
  ~TA_STAR_MAP();

  /** The method allocates fields in the map and sets dimensions sizes.*/
  bool CreateNewMap(T_SIMPLE width, T_SIMPLE height, T_SIMPLE depth = DAT_SEGMENTS_COUNT);
  /** The method deallocates fields in the map and sets dimensions sizes.*/
  void DestroyMap();

  /** The method prepares map with the dimension sizes from the parameters.*/
  bool PrepareMap(T_SIMPLE width, T_SIMPLE height, T_SIMPLE depth = DAT_SEGMENTS_COUNT);

  /** Reset star map for next use.*/
  void ResetStarMap();

  /** Creates the set of goal positions. */
  void CreateGoalSet(TPOSITION_3D goal, T_SIMPLE welt);

  /** Creates the set of goal positions for an area*/
  void CreateGoalSetForArea(TPOSITION_3D pos,TFORCE_UNIT *unit,int area_width,int area_height);

  /** The method marks fields under unit position.*/
  void MarksUnitPosition(TFORCE_UNIT *unit);

private:
  TA_STAR_MAP_FIELD ***fields;  //!< Three dimension array of the star map fields. It is map.
  T_SIMPLE height;              //!< The size of the second dimension.
  T_SIMPLE width;               //!< The size of the first dimension.
  T_SIMPLE depth;               //!< The size of the third dimension.
  friend class TA_STAR_ALG;
};


/**
 *  Class envelopes list with walking path and methods for working with it.
 *
 *  @sa TUNIT
 */
class TPATH_LIST {
  TPATH_NODE *f_node;   //!< First node in the list.
  TPATH_NODE *a_node;   //!< Actual node in the list.
  int steps;            //!< Count of steps.
  int a_step;           //!< Number of actual step.
  TPOSITION_3D real_goal_position;  //!< Real goal position of path. (first parameter of PathFinder function)
public:
  //! Constructor.
  TPATH_LIST()
    {steps = 0; a_step = -1; f_node = a_node = NULL;};
  TPATH_LIST(TPATH_NODE *first, int count);     //!< Constructor values from the parameters.

  //! Destructor.
  ~TPATH_LIST();

  TPATH_LIST *AddToPath(TPOSITION_3D adding);
  TPOSITION_3D GetNextPosition();
  TPOSITION_3D GetPrevPosition();
  TPOSITION_3D GetPostitionInPath(int steps_count);           //! returns position i steps ago

  T_SIMPLE GetFirstFieldX() const;              //! Get x position of first field in the first node.
  T_SIMPLE GetFirstFieldY() const;              //! Get y position of first field in the first node.
  T_SIMPLE GetFirstFieldZ() const;              //! Get z position of first field in the first node.

  bool TestLastPathPosition(void);              //!< Tests if a_stem is last step.
  TPOSITION_3D GetGoalPosition();                       //!<Get goal of the path.
  TPATH_LIST* CreateCopy(int shift_x, int shift_y, int shift_z);          //!<The path list create copy of the itself with shift.
#ifdef __GNUC__
	double CountTime(TFORCE_UNIT *unit);      //!<Counts the time, which unit spends on the exact way.
#else
  	double TPATH_LIST::CountTime(TFORCE_UNIT *unit);      //!<Counts the time, which unit spends on the exact way.
#endif
  
  void DecreaseSteps(int st_count) { steps -= st_count;};    //! Decreases variable steps, used when building is build, unit cant go to the goal, but just near the newly built building.
  void IncreaseASteps();                        //!< Increases actuall step
  void DecreaseASteps();                        //!< Decrease actuall step
  void SetASteps(int value) {a_step = value;};
  void SetSteps(int value) { steps = value;};
  void SetANode(TPATH_NODE * n) { a_node = n;};
  TPOSITION_3D GetRealGoalPosition() { return real_goal_position;};
  void SetRealGoalPosition(TPOSITION_3D new_pos) { real_goal_position = new_pos;};
private:
#ifdef NEW_GLFW3
	mtx_t mutex;
#else
	GLFWmutex mutex;                      //!< Units mutex.
#endif
  
};


/**
 *  Node in path list, where is storaged result path finded by A* algorithm.
 * 
 *  @sa TPATH_LIST
 */
struct TPATH_NODE {
  TPATH_NODE *next;   //!< Pointer to next node in the list.
  TPATH_NODE *prev;   //!< Pointer to previous node in the list.
  int first;          //!< Placement number of first valid fields in this node.

  //! Coordinates in the map, each step of the path is storaged in this array.
  TPOSITION_3D path_pos[WLK_NODES_NUM];
#ifdef __GNUC__
	TPATH_NODE(TPATH_NODE *first, TPOSITION_3D adding);  //!< Adding constructor.
  	TPATH_NODE(TPOSITION_3D goal);    //!< Constructor.
#else
  	TPATH_NODE::TPATH_NODE(TPATH_NODE *first, TPOSITION_3D adding);  //!< Adding constructor.
  	TPATH_NODE::TPATH_NODE(TPOSITION_3D goal);    //!< Constructor.
#endif
  
private:
#ifdef __GNUC__
	TPATH_NODE(TPATH_NODE &origin, int sx, int sy, int sz);   //!< Constructor creates copy with shift.
#else
  	TPATH_NODE::TPATH_NODE(TPATH_NODE &origin, int sx, int sy, int sz);   //!< Constructor creates copy with shift.
#endif
  
  friend TPATH_LIST* TPATH_LIST::CreateCopy(int, int, int);
};

//forward declaration
class TLOC_MAP;

/**
 *  Class envelopes datas and methods for A* algorithm.
 *  It is interface for working with algorithm.
 *
 *  @sa TPLAYER
 */
class TA_STAR_ALG {
  TA_STAR_MAP *star_map;                              //!<Pointers to marking map.
  TSET_FIELD *open_set;                               //!<Pointer to set OPEN that is storaged in binary heap.
  unsigned int open_node_num;                         //!<Count of nodes in heap of OPEN set.
  TSET_FIELD *close_set;                              //!<Pointer to set CLOSE.
  unsigned int close_node_num;                        //!<Count of nodes in set CLOSE.
  char playerID;                                      //!<Player number
  double unknown[DAT_SEGMENTS_COUNT];                 //!<Array with lowest difficulty of terrain in each segment in map.
  TSET_FIELD closest_goal_field;                      //!<The field, that is closest to the goal; used when whole path form the start to the goal isn't found.

public:
  TA_STAR_ALG();                                      //!< Constructor for using by thread pool
  TA_STAR_ALG(char owner);                            //!<Constructor
  ~TA_STAR_ALG();                                     //!<Destructor   
  void InsertToOpenSet(TSET_FIELD field);             //!<Inserts new node into the heap OPEN set.
  TSET_FIELD * ExtractMinOpenSet();                   //!<Extracs minimum from all the nodes in the OPEN set .
  void InsertToCloseSet(TSET_FIELD * open_min );      //!<Inserts nodes to close set.  
  bool PathFinder(TPOSITION_3D goal, TFORCE_UNIT *unit, TLOC_MAP *loc_map,TPATH_LIST **path,TPOSITION_3D *real_goal,long *area_dist=NULL,int max_steps_cnt =0, int area_widht =0, int area_height =0);
  void GetAdjacent(TSET_FIELD *neighbours, TSET_FIELD *center, TFORCE_ITEM *type, TLOC_MAP_FIELD ***loc_map, TPOSITION_3D goal,double *easiest); //!<Fill array of TSET_FIELD with adjacent area.
  TPATH_LIST * CreatePathList(TPOSITION_3D goal, TPOSITION_3D start);    //!<Create path list from CLOSE set created in PathFinder().
  int CountDistance(TPOSITION_3D pos1,TPOSITION_3D pos2); //!< Function count square of Euclidean distance.
  TSEL_NODE * GetGroup(TSEL_NODE **unit_list);           //<! returns group of units for which the path will be waited.

  TPATH_INFO* ComputePath(TPATH_INFO* path_info);     //!< The method computes new path according to parameter.
  TPATH_INFO* DevideToGroups(TPATH_INFO* path_info);  //!< The method divides  all selected units to groups and for each group searches the path
  TPATH_INFO* MoveGroup(TPATH_INFO* group_info);      //!< Thread function: moves with group of units.  

  TNEAREST_INFO* SearchForNearestBuilding(TNEAREST_INFO* pnearest_info); //!< The method finds nearest building and path to it according to parameter

private:
  /* Functions used in GetAdjacent(). */
  inline unsigned int FieldDifficultyAux (int x, int y, int z, TLOC_MAP_FIELD ***loc_map);
  inline bool IsOccupiedByMe (int x, int y, int z);
  inline bool IsOccupiedByOwn (int x, int y, int z, TLOC_MAP_FIELD ***loc_map);
  inline bool IsUnavailableField (int x, int y, int z, TFORCE_ITEM *type, TLOC_MAP_FIELD ***loc_map);

  enum TDIRECTION { STRAIGHT, DIAGONAL, VERTICAL };

  inline double GetDistance (TDIRECTION direction, double max_diff, float max_speed);
  void GetAdjacentOneDirection(int x, int y, int z, TDIRECTION direction, TSET_FIELD &neighbour, TSET_FIELD *center, TFORCE_ITEM *type, TLOC_MAP_FIELD ***loc_map, TPOSITION_3D goal, double* easiest);
};


//========================================================================
// class TPATH_PARMS - needed for creation of thread
//========================================================================

class TSEL_NODE : public TPOOL_ELEMENT
{
  public:

  TFORCE_UNIT * unit;         //!< Pointer to selected unit.

  TSEL_NODE *next;            //!< Next item of the list.
  TSEL_NODE *prev;            //!< Previous item of the list.


  TSEL_NODE()                 //!< Default constructor.
    { unit = NULL; next = prev = NULL;};
};


class TPATH_INFO : public TPOOL_ELEMENT
{
public:
  TPOSITION_3D goal;        //goal of the unit
  union 
  {
    TFORCE_UNIT * unit;
    TSEL_NODE * unit_list;
  };

  TLOC_MAP *loc_map;        //pointer to local map
  TPATH_LIST *path;         //found path
  TPOSITION_3D real_goal;   //real goal where unit is able to go
  bool succ;                //1- if pathfinder was succesfull, 0 - otherwise  
  int request_id;           //id of request
  T_SIMPLE e_simple1;        //place for saving any simple value
  T_SIMPLE e_simple2;        //place for saving any simple value
  int event_type;           //type of event
  

  TPATH_INFO();
  ~TPATH_INFO() {} ;

  void Clear(bool all); 
};


class TNEAREST_INFO : public TPOOL_ELEMENT
{
  public:
    TSOURCE_UNIT *src_unit;    //which building is start for search    
    TFORCE_UNIT *unit;         //which unit is searching
    TBUILDING_UNIT *nearest;   //nearest_unit
    int request_id;            //id of request 
    int event_type;            //type of event
    T_SIMPLE simple1;
    T_SIMPLE simple2;

    TNEAREST_INFO();           //constructor
    ~TNEAREST_INFO() {};       //destructor

    void Clear(bool all);
};

//========================================================================
// Global variables
//========================================================================

extern TPOOL<TPATH_INFO> * pool_path_info;
extern TPOOL<TSEL_NODE> * pool_sel_node;
extern TPOOL<TNEAREST_INFO> * pool_nearest_info;
extern TTHREAD_POOL<TPATH_INFO, TPATH_INFO, TA_STAR_ALG> *threadpool_astar;
extern TTHREAD_POOL<TNEAREST_INFO, TNEAREST_INFO, TA_STAR_ALG> *threadpool_nearest;

//=========================================================================
// Global functions declaration
//=========================================================================


#endif  // __dowalk_h__

//=========================================================================
// END
//=========================================================================
// vim:ts=2:sw=2:et:

