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
 *  @file doselection.h
 *
 *  Selection of units.
 *
 *  @author Peter Knut
 *
 *  @date 2003, 2004
 */

#ifndef __doselection_h__
#define __doselection_h__

//=========================================================================
// Forward declarations
//=========================================================================

struct TNODE_OF_UNITS_LIST;
class TSELECTION;


//=========================================================================
// Definitions
//=========================================================================

#define PL_MAX_SELECTIONS   9
#define PL_SHOW_LINES_TIME  1.0


//=========================================================================
// Included files
//=========================================================================

#include "cfg.h"
#include "doalloc.h"

#include "dounits.h"


//=========================================================================
// Classes
//=========================================================================

/** Node of list of units. */
struct TNODE_OF_UNITS_LIST 
{
  TMAP_UNIT *unit;          //!< Pointer to unit.
  bool has_path;

  TNODE_OF_UNITS_LIST *next;            //!< Next item of the list.
  TNODE_OF_UNITS_LIST *prev;            //!< Previous item of the list.


  TNODE_OF_UNITS_LIST()                 //!< Default constructor.
    { unit = NULL; next = prev = NULL; has_path = false; };
};


/** Struct for one stored selection. */
struct TSTORED_SELECTION {
  TMAP_UNIT **units;                            //!< Array of stored units.
  int units_count;                              //!< Count of stored units.
  int active_units;                             //!< Count of active stored units.

  void Reset();
  void PrepareField(int count);
  void DeleteUnit(TMAP_UNIT *unit);

  TSTORED_SELECTION()                                       //!< Constructor.
    { units = NULL; units_count = active_units = 0; };
  ~TSTORED_SELECTION() { if (units) delete[] units; };      //!< Destructor.
};



/** Selection of units or one building. */
class TSELECTION {
public:
  TNODE_OF_UNITS_LIST *GetUnitsList() { return units; }

  //! Determines if selection is empty.
  bool IsEmpty() { return (units_count == 0); }
  bool IsMy() { return is_my; }
  bool OnlyOne() { return (units_count == 1); }
  int  GetUnitsCount() { return units_count; }
  TMAP_UNIT *GetFirstUnit() { if (units) return units->unit; else return NULL; }
  TMAP_ITEM *GetBuilderItem() { return builder_item; }

  void UnselectAll() { _UnselectAll(true); }

  void SelectUnit(TMAP_UNIT *punit);            // Selects one unit.
  void AddUnit(TMAP_UNIT *punit, bool sound)    // Add unit into selection.
    { _AddUnit(punit, sound, true); }
  bool DeleteUnit(TMAP_UNIT *punit);            // Deletes unit from selection.
  void AddDeleteUnit(TMAP_UNIT *punit);         // Inverts unit's selection.

  void Update(double time_shift);
  void UpdateInfo(bool update_action, bool lock = true);  // Updates panel information depends on selection.
  void UpdateAction(bool lock = true);

  void DrawUnitsLines();

  void StartTimer()   { timer = PL_SHOW_LINES_TIME; };
  bool CanDrawLines() { return can_move && timer > 0; }

  bool TestCanMove() { return can_move; }
  bool TestCanHide(TMAP_UNIT *over_unit);
  bool TestCanAttack(TMAP_UNIT *over_unit);
  bool TestCanMine(TSOURCE_UNIT *over_unit);
  bool TestCanUnload(TBUILDING_UNIT *over_unit);
  bool TestCanBuildOrRepair(TBASIC_UNIT *over_unit);
  bool TestCanBuild(TBUILDING_ITEM *building, TPOSITION pos, bool **build_map);  

  bool MoveUnits(TPOSITION position);   // Moves with selected units.
  bool ReactUnits();                    // React with selected units.
  void StopUnits();                     // Stops all selected units.

  void SetAggressivity(TAGGRESSIVITY_MODE mode);

  bool GetCanMove() { return can_move; }
  bool GetCanAttack() { return can_attack; }

  // stored selections
  void StoreSelection(int gid);
  void RestoreSelection(int gid, bool center, bool sound = true);
  void DeleteStoredUnit(int gid, TMAP_UNIT *unit);

  TSELECTION();                         // Constructor.
  ~TSELECTION();                        // Destructor.

private:
#ifdef NEW_GLFW3
	mtx_t mutex;
#else
	GLFWmutex mutex;                      //!< Units mutex.
#endif

  TNODE_OF_UNITS_LIST *units;           //!< List of selected units.
  int    units_count;                   //!< Count of selected units.
  double timer;                         //!< Timer for selection operations.

  bool is_my;                           //!< Whether only my units are in the selection.

  bool can_move;                        //!< Player can move with all units in selection.
  bool can_attack;                      //!< Some unit in selection can attack enemies.
  bool can_mine;                        //!< Some unit in selection can mine resources.
  bool can_repair;                      //!< Some unit in selection can repair units.
  bool can_build;                       //!< Some unit in selection can build buildings.

  TMAP_ITEM *builder_item;              //!< Workers/factory of this item are selected.

  TUNIT_ACTION units_action;            //!< Action of all units.
  TAGGRESSIVITY_MODE aggressivity_mode; //!< Aggressivity mode of all units.

  void _AddUnit(TMAP_UNIT *punit, bool sound, bool lock);   // Add unit into selection.
  void _UnselectAll(bool lock);                             // Unselects all units or building.

  TSTORED_SELECTION groups[PL_MAX_SELECTIONS];
};

//=========================================================================
// Variables
//=========================================================================

extern TSELECTION *selection;


#endif  // __doselection_h__

//=========================================================================
// END
//=========================================================================
// vim:ts=2:sw=2:et:

