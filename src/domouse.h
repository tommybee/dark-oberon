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
 *  @file domouse.h
 *
 *  Mouse.
 *
 *  @author Peter Knut
 *
 *  @date 2003, 2004
 */


#ifndef __domouse_h__
#define __domouse_h__


//========================================================================
// Forward declarations
//========================================================================

class TMOUSE;


//========================================================================
// Included files
//========================================================================

#include "cfg.h"
#include "doalloc.h"

#include "dodraw.h"
#include "dounits.h"


//========================================================================
// Typedefs
//========================================================================

#define MC_SELECT_RADIUS  8

// mouse cursor
#define MC_COUNT  16  //!< Count of mouse cursor types.

enum TMOUSE_CURSOR {
  MC_ARROW,           //!< Default mouse cursor.
  MC_SELECT,          //!< Mouse cursor for units selection.
  MC_SELECT_PLUS,     //!< Mouse cursor for units multi selection.
  MC_CAN_MOVE,        //!< Mouse cursor for moving.
  MC_CANT_MOVE,       //!< Mouse cursor for disabled moving.
  MC_CAN_ATTACK,      //!< Mouse cursor for attack enemy unit.
  MC_CANT_ATTACK,     //!< Mouse cursor for disbled attack enemy unit.
  MC_CAN_MINE,        //!< Mouse cursor for mining.
  MC_CANT_MINE,       //!< Mouse cursor for disabled mining.
  MC_CAN_REPAIR,      //!< Mouse cursor for repairing.
  MC_CANT_REPAIR,     //!< Mouse cursor for disabled repairing.
  MC_CAN_BUILD,       //!< Mouse cursor for building.
  MC_CANT_BUILD,      //!< Mouse cursor for disabled building.
  MC_CAN_HIDE,        //!< Mouse cursor for hiding units into others.
  MC_CANT_HIDE,       //!< Mouse cursor for disabled hiding units into others.
  MC_EJECT,           //!< Mouse cursor for ejecting units.
};

// mouse circle
#define MCC_COUNT 3   //!< Count of mouse cursor circle types.

enum TMOUSE_CIRCLE {
  MCC_ARROWS_IN,      //!< Mouse cursor circle for permited actions.
  MCC_ARROWS_OUT,     //!< Mouse cursor circle for permited actions.
  MCC_CROSS,          //!< Mouse cursor circle for not permited actions.
  MCC_NONE = 255      //!< Disabled mouse cursor circle.
};


//========================================================================
// TMOUSE
//========================================================================

/**
 *  Mouse class.
 */
class TMOUSE {
public:
  TMOUSE_CURSOR cursor_id;  //!< Cursor identifier.
  TMOUSE_CIRCLE circle_id;  //!< Cursor circle identifier.
  TUNIT_ACTION  action;     //!< Units action.

  int x;                    //!< Mouse x position.
  int y;                    //!< Mouse y position.
  int rx;                   //!< Real x mouse position.
  int ry;                   //!< Real y mouse position.

  int down_x, down_y;       //!< Position of cursor whem left button was pressed.
  TMOUSE_CURSOR down_cursor_id;
  TMOUSE_CIRCLE down_circle_id;

  TPOSITION map_pos;        //!< Map position in the map. [mapels]

  GLfloat rpos_x;           //!< Real x mouse position used for drawing. [pixels]
  GLfloat rpos_y;           //!< Real y mouse position used for drawing. [pixels]
  GLfloat sel_x;            //!< Start x position of selection. [pixels]
  GLfloat sel_y;            //!< Start y position of selection. [pixels]
  double rmap_x;            //!< Real x mouse position in the map. [mapels]
  double rmap_y;            //!< Real y mouse position in the map. [mapels]

  bool draw_selection;      //!< Selection is drawed.
  bool **build_map;         //!< 2D map - where building could be build.

  TMAP_UNIT *over_unit;     //!< Actual unit under cursor that will by selected after click.

  bool LoadData(const char *path);
  void DeleteData(void);

  void Update(bool in_game, double time_shift);
  void Draw(void);            // draws mouse cursor
  void DrawSelection(void);   // draws selection rectangle
  void DrawBuildMap(void);    // draws build map
  void ResetCursor(bool full = true) { cursor_id = MC_ARROW; circle_id = MCC_NONE; ReleaseCountedPointer(over_unit); if (full) draw_selection = false; }; 

  void Center(void);          // center mouse cursor
  bool RectSelect(void);      // Selects all units from selection rectangle.

  TMOUSE(void);

private:
  TTEX_TABLE tex_set;                       //!< Texture set for mouse cursors and circles.

  TGUI_ANIMATION cursors_anim[MC_COUNT];    //!< Field of animations for mouse cursors types.
  TGUI_ANIMATION circles_anim[MCC_COUNT];   //!< Field of animations for mouse cursor circles types.

  void UpdateCursorID(void);
  void FindOverUnit(int seg);
};


//=========================================================================
// Functions
//=========================================================================

void MouseToMap(GLfloat rpos_x, GLfloat rpos_y, double *map_x, double *map_y);


//=========================================================================
// Variables
//=========================================================================

extern TMOUSE mouse;


#endif  //__domouse_h__

//========================================================================
// End
//========================================================================
// vim:ts=2:sw=2:et:

