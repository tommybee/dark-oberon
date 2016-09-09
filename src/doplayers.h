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
 *  @file doplayers.h
 *
 *  Working with players.
 *
 *  @author Martin Kosalko
 *  @author Peter Knut
 *  @author Jiri Krejsa
 *  @author Michal Kral
 *
 *  @date 2003, 2004, 2005
 */

#ifndef __doplayers_h__
#define __doplayers_h__

//=========================================================================
// Forward declarations
//=========================================================================

class THASH_UNIT;
class THASHTABLE_UNITS;
class TLOC_MAP;
class TPLAYER;
class TCOMPUTER_PLAYER;

//=========================================================================
// Definitions
//=========================================================================

#define PL_MAX_PLAYER_NAME_LENGTH   20
#define PL_MAX_ADDRESS_LENGTH       40
#define PL_MAX_SELECTIONS   9
/** Maximum count of players including hyper player. */
#define PL_MAX_PLAYERS      8
#define PL_HASHTABLE_UNITS_SIZE  100
#define PL_MAX_START_POINTS  32

// area visibility
#define AV_NOT_VISIBLE      0
#define AV_VISIBLE          1
#define AV_WARFOG           2

// building height coef
#define MAP_BUILDING_COEF   100

// types of player
#define PT_HUMAN            0      //!< Human player
#define PT_COMPUTER         1      //!< Computer player

//=========================================================================
// Typedefs
//=========================================================================

//=========================================================================
// Included files
//=========================================================================

#include "cfg.h"
#include "doalloc.h"

#include <string>

#include "doipc.h"
#include "donet.h"
#include "dowalk.h"

//=========================================================================
// Classes
//=========================================================================


/** Node of hash unit. Contians unique identificator of unit and pointer to unit. */
class THASH_UNIT{
public:  
  int unit_id;                  //!< Unique identificator of unit.
  TPLAYER_UNIT * player_unit;   //!< Pointer to unit.
  THASH_UNIT * next;            //!< Pointer to next THASH_UNIT for one hash value.

  THASH_UNIT(void) {unit_id = 0; player_unit = NULL; next = NULL;}; //!< Constructor. Only zeroize data.
  THASH_UNIT(int a_unit_id, TPLAYER_UNIT * a_player_unit) {unit_id = a_unit_id; player_unit = a_player_unit; next = NULL;}; //!< Constructor. Fill data.
};

/**
 *  Hashtable for quick translation between unit identificator and pointer to unit.
 */

class THASHTABLE_UNITS {
private:
  THASH_UNIT * table[PL_HASHTABLE_UNITS_SIZE]; //!< Array of pointers to hash nodes.

  int HashFunction(int h_unit_id) { return ((abs(h_unit_id)) % PL_HASHTABLE_UNITS_SIZE);}; //!< hash function.

public: 
  TPLAYER_UNIT * GetUnitPointer(int g_unit_id);   // Returns pointer to unit identified by unit_id.
  int GetUnitID(TPLAYER_UNIT * g_player_unit);    // Returns identificator of unit identified by pointer.

  void AddToHashTable(int a_unit_id, TPLAYER_UNIT * a_player_unit); // Add new hash unit to table.
  void RemoveFromHashTable(int r_unit_id);                          // Remove hash unit from table.

  THASHTABLE_UNITS(void);   // Constructor.
  ~THASHTABLE_UNITS(void);  // Destructor.
};


/**
 *  Local map for each player. Contains information about actual state of map
 *  fields in all segments.
 *
 *  @sa TLOC_MAP_FIELD
 */
class TLOC_MAP {
public:

  TLOC_MAP_FIELD ***map;      //!< Pointer to 3D array of fields.

  bool IsVisibleArea(int segment, T_SIMPLE x, T_SIMPLE y);
  bool IsVisibleArea(TPOSITION_3D pos);
  void UnitLeftPosition(TPOSITION_3D pos, int size);
  void UnitFillPosition(TPOSITION_3D pos, int size, T_SIMPLE player_id);
  bool IsMoveablePosition(TFORCE_ITEM * const type, const TPOSITION_3D &pos);
  bool IsLandablePosition(TFORCE_ITEM * const type, const TPOSITION_3D &pos);
  bool IsNextPositionEmpty(const TPOSITION_3D pos, const int dir, TFORCE_UNIT *unit);

  bool GetAreaVisibility(TPOSITION_3D pos, const T_SIMPLE width, const T_SIMPLE height);
  bool GetAreaVisibility(const T_SIMPLE pos_x, const T_SIMPLE pos_y, 
    const T_BYTE seg_min, const T_BYTE seg_max, 
    const T_SIMPLE width, const T_SIMPLE height);

  bool IsAreaUnknown(const T_SIMPLE x, const T_SIMPLE y, const T_BYTE seg, const T_SIMPLE width, const T_SIMPLE height);
  bool IsAnyAreaUnknown(const T_SIMPLE x, const T_SIMPLE y, const T_BYTE seg, const T_SIMPLE width, const T_SIMPLE height);

  TLOC_MAP();
  ~TLOC_MAP();

  /** @return The method returns width of the map.*/ 
  T_SIMPLE GetMapWidth() const
    { return width;}
  /** @return The method returns height of the map.*/ 
  T_SIMPLE GetMapHeight() const
    { return height;}
  /** @return The method returns depth of the map.*/ 
  T_SIMPLE GetMapDepth() const
    { return depth;}

  /** Tests whether position is in borders of map. Does not test segment coordinate. */
  bool IsInMap(const T_SIMPLE x, const T_SIMPLE y)
    { return ((x < width) && (y < height)); }
  //! Tests whether position is in borders of map.
  bool IsInMap(const TPOSITION_3D &pos)
    { return ((pos.x < width) && (pos.y < height) && (pos.segment < depth)); }
  //! Tests whether position is in borders of map.
  bool IsInMap(const T_SIMPLE x, const T_SIMPLE y, const T_SIMPLE segment)
    { return ((x < width) && (y < height) && (segment < depth)); }
  //! Tests whether position is in borders of map.
  bool IsInMap(const double x, const double y)
    { return ((x >= 0) && (x < width) && (y >= 0) && (y < height)); }
  //! Tests whether position is in borders of map.
  bool IsInMap(const int x, const int y)
    { return ((x >= 0) && (x < width) && (y >= 0) && (y < height)); }

private:
  void CreateLocalMap(T_SIMPLE width, T_SIMPLE height, T_SIMPLE depth = DAT_SEGMENTS_COUNT);
  void DeleteLocalMap();

private:
  T_SIMPLE depth;     //!< Depth of the map (count of segments).
  T_SIMPLE width;     //!< Width of the map.
  T_SIMPLE height;    //!< Height of the map.
};


/**
 *  All player information.
 */
class TPLAYER {
public:
  char name[PL_MAX_PLAYER_NAME_LENGTH + 1];    //!< Name.
  bool active;                          //!< If player is active.
  bool update_info;                     //!< If I must update info (on main panel).

  TRACE *race;                          //!< Race.

  TPLAYER_UNIT *units;                  //!< List of all units owned by player.

  TBUILDING_ITEM *build_item;           //!< Building to be built.  

  T_SIMPLE initial_x;                   //!< Initial x position in the map. [mapels]
  T_SIMPLE initial_y;                   //!< Initial y position in the map. [mapels]

  TA_STAR_ALG *pathtools;               //!< Tools for pathfinding.

  TGUI_ANIMATION *need_animation[SCH_MAX_MATERIALS_COUNT + 2];    //!< Animations of missing food, energy and materials.
  
  TLIST<TSOURCE_UNIT> sources[SCH_MAX_MATERIALS_COUNT];  //!< Array of sources, which belong to "hyperplayer", devides intho 4 groups according to the material,which offer

  TLIST<TBUILDING_UNIT> material_array[SCH_MAX_MATERIALS_COUNT]; //!< Array of buildings, that accept one or more of 4 materials, devided into 4 groups according to the accepted material
  void IncreaseBuildingCount(int,int);  //!<increases number of new building in sources array  

  T_BYTE GetPlayerID() { return this->player_id; }; //!< The method gets player ID.
  void SetPlayerID(T_BYTE p_id) { player_id = p_id; }; //!< The method sets player ID.

  TLOC_MAP *GetLocalMap() { return &local_map; };

  THASHTABLE_UNITS hash_table_units;

  void AddUnit(TPLAYER_UNIT *punit);
  void DeleteUnit(TPLAYER_UNIT *punit);

  void UpdateGraphics(double time_shift);
  void Disconnect(void);
  
  //!< Increments global units counter of player.
  int IncrementGlobalUnitCounter(){global_unit_counter++; return global_unit_counter;};
  //!< Sets global units counter of player.
  void SetGlobalUnitCounter(int new_global_unit_counter){global_unit_counter = new_global_unit_counter;};
  
  //!< Increments local units counter of player.
  int DecrementLocalUnitCounter(){local_unit_counter--; return local_unit_counter;};  
  //!< Increments local units counter of player.
  void SetLocalUnitCounter(int new_local_unit_counter){local_unit_counter = new_local_unit_counter;};

  //!< Increments requests counter of player.
  int IncrementRequestCounter(){request_counter++; return request_counter;};

  //!< Returns actual global_unit_counter.
  int GetGlobalUnitCounter(void){return global_unit_counter;};
  //!< Returns atual local_unit_counter.
  int GetLocalUnitCounter(void){return local_unit_counter;};
  //!< Updates local map for building.
  void UpdateLocalMap(const TPOSITION_3D position, TBUILDING_ITEM * const pitem, const bool my_building, const T_SIMPLE player_ID, const bool built);
  //!< Updates local map for new built units.
  void UpdateLocalMap(const TPOSITION_3D position, const TFORCE_ITEM *pitem, const T_SIMPLE player_id);
  //!< Update local map during unit move.
  void UpdateLocalMap(const TPOSITION_3D old_pos, const TPOSITION_3D new_pos, const TFORCE_UNIT *unit);
  //!< The method updates local map for source.
  void UpdateLocalMap(const TPOSITION_3D position, TSOURCE_ITEM * const pitem, const T_SIMPLE player_ID, const bool collapsing);

  float GetStoredMaterial(int mat) { return stored_material[mat]; };

  void SetStoredMaterial(int mat, float val) { stored_material[mat] = val; update_info = true; };
  void IncStoredMaterial(int mat, float val) { stored_material[mat] += val; update_info = true; };
  void DecStoredMaterial(int mat, float val) { stored_material[mat] -= val; update_info = true; };

  void AddUnitEnergyFood(int e, int f);      //Add value of @param e to player's energy and @param f to player's food.
  void RemoveUnitEnergyFood(int e, int f);   //Remove value of @param e from player's energy and @param f from player's food.

  //!< Returns sum of energy given to player from his units and buildings (positive energy).
  int GetInEnergy() {return energy_in;};
  //!< Returns sum of energy taken from player by his units and buildings (negative energy).
  int GetOutEnergy() {return energy_out;};

  T_BYTE GetPercentEnergy() { if (!energy_out || energy_in >= energy_out) return 100; else return ((energy_in * 100) / energy_out); }

  //!< Returns sum of food given to player from his units and buildings (positive food).
  int GetInFood() {return food_in;};
  //!< Returns sum of food taken from player by his units and buildings (negative food).
  int GetOutFood() {return food_out;};

  T_BYTE GetPercentFood() { if (!food_out || food_in >= food_out) return 100; else return ((food_in * 100) / food_out); }

  //!< Indicates whether the player is AI
  int GetPlayerType() { return player_type; }

  //!< Reset all have_order (set them to false)
  void ResetOrders();

  TPLAYER(void);
  virtual ~TPLAYER(void);

  void SetPlayerType(int pt_type) { player_type = pt_type; }

  int GetPlayerUnitsCount() {return player_units_counter;};
  void IncPlayerUnitsCount();
  void DecPlayerUnitsCount();

private:
  T_BYTE player_id;                      //!< Identificator of player.
  
  TLOC_MAP local_map;                    //!< Local map. It includes information: here is warfog, unknown area, known area, etc.

  float stored_material[SCH_MAX_MATERIALS_COUNT]; //!<Amount of material from all of the workers.

  int energy_in;                         //!< Global value of energy (sum of positive energy of all buildings and units) (always >=0).
  int energy_out;                        //!< Global value of energy (sum of negative energy of all buildings and units) (always >=0).
  
  int food_in;                           //!< Global value of food (sum of positive food of all buildings and units) (always >=0).
  int food_out;                          //!< Global value of food (sum of negative food of all buildings and units) (always >=0).

  int global_unit_counter;               //!< Identificator of last created global unit (same on all remote comuters).
  int local_unit_counter;                //!< Identificator of last created local unit (special on local comuter).
  int request_counter;                   //!< Identificator of last created request.

  int player_units_counter;              //!< Count of active players unis (TBASIC_UNIT)

  int player_type;                       //!< PT_COMPUTER for computer_players, PT_HUMAN for humans
  
#ifdef NEW_GLFW3
	mtx_t mutex;
#else
	GLFWmutex mutex;                        //!< Mutex for locking list of units.
#endif
  
};


/**
 *  Contains info about players.
 */
struct TPLAYER_ARRAY {
public:
  class MutexException {};
  class TooManyPlayersException {};
  class PlayerNotFoundException {};

  TPLAYER_ARRAY ();
  ~TPLAYER_ARRAY ();

  void Initialise ();
  void Clear ();

  void AddComputerPlayer ();
  void AddLocalPlayer (std::string player_name, std::string race_id_name = "",
                       bool computer = false);
  void AddRemotePlayer (std::string player_name, struct in_addr addr,
                        in_port_t port, bool computer = false);
  void RemovePlayer (int player_index);

  std::string GetPlayerName (int player_index);
  std::string GetRaceIdName (int player_index);
  in_addr GetAddress (int player_index);
  in_port_t GetPort (int player_index);

  bool IsRemote (int player_index);
  bool IsComputer (int player_index);

  void SetRaceIdName (int player_index, std::string race_id_name);
  bool EveryPlayerHasDifferentRace ();

  void Lock ()    { lock->Lock (); }
  void Unlock ()  { lock->Unlock (); }

  /** Returns count of (connected) players including hyper player. */
  int GetCount ()
  { return count; }

  void ChooseRandomStartPoints ();
  void SetStartPointsCount (int value);
  int GetStartPoint (int player_index);
  void SetStartPoint (int player_index, int value);

  int GetMyPlayerID ();
  int GetPlayerID (in_addr address, in_port_t port, int min_id);

  void RunningOnFollower () { running_on_leader = false; }

  bool AllPlayersAreLocal ();

  void PlayerReady (in_addr address, in_port_t port);
  bool AllRemoteReady ();

private:
  void AddPlayer (std::string player_name, std::string race_id_name,
                  bool computer, bool lock = true, bool remote = false,
                  in_addr *addr_p = NULL, in_port_t port = 0);

  /** Array which holds information about players. */
  struct TPLAYER {
    /** Constructor. */
    TPLAYER () { computer = false; }

    std::string player_name;    //!< Player name.
    std::string race_id_name;   //!< Player id_name of selected race.

    bool computer;    //!< Specifies whether the player is a computer player.
    bool remote;      //!< Specifies whether the player is a network player.
    in_addr addr;     //!< Address of a remote player.
    in_port_t port;   //!< Port of a remote player.
    
    int start_point;

    bool ready_to_start_game;
  } player[PL_MAX_PLAYERS];

  int count;              //!< Actual count of (connected) players.
  int start_points_count; //!< Count of start points in actual selected map.

  bool running_on_leader;

  TRECURSIVE_LOCK *lock;
};


//=========================================================================
// Variables
//=========================================================================

extern TPLAYER ** players;
extern TPLAYER *myself;
extern TPLAYER *hyper_player;
extern TPLAYER_ARRAY player_array;


//=========================================================================
// Functions
//=========================================================================

bool CreatePlayers();
void DeleteUnits(TMAP_UNIT *units);
void DeletePlayers();

#endif  // __doplayers_h__


//=========================================================================
// END
//=========================================================================
// vim:ts=2:sw=2:et:

