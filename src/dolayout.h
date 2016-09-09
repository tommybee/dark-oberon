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
 *  @file dolayout.h
 *
 *  Work with coordinates and terrain.
 *
 *  @author Jiri Krejsa
 *
 *  @date 2003
 */

#ifndef __dolayout_h__
#define __dolayout_h__

//========================================================================
// Forward declarations
//========================================================================

class TPOSITION;
class TPOSITION_3D;


//========================================================================
// Definitions
//========================================================================

#define LAY_UNAVAILABLE_POSITION  255     //!<  Special value that indicates unavailable position.

// layout
#define LAY_SOUTH                   0     //!<  Constants that says direction of moving unit.
#define LAY_SOUTH_WEST              1     //!<  Constants that says direction of moving unit.
#define LAY_WEST                    2     //!<  Constants that says direction of moving unit.
#define LAY_NORTH_WEST              3     //!<  Constants that says direction of moving unit.
#define LAY_NORTH                   4     //!<  Constants that says direction of moving unit.
#define LAY_NORTH_EAST              5     //!<  Constants that says direction of moving unit.
#define LAY_EAST                    6     //!<  Constants that says direction of moving unit.
#define LAY_SOUTH_EAST              7     //!<  Constants that says direction of moving unit.

#define LAY_UP                      8     //!< Constants that says direction of moving unit. Moving to higher segment.
#define LAY_DOWN                    9     //!< Constants that says direction of moving unit. Moving to lower segment.
#define LAY_UNDEFINED               -1    //!< Undefined moving direction.

#define LAY_DIRECTIONS_COUNT        10    //!< Count of the directions.
#define LAY_HOR_DIRECTIONS_COUNT    8     //!< Count of the directions in the one segment.

//========================================================================
// Included files
//========================================================================

//#include <math.h>
#include "cfg.h"

#include "dosimpletypes.h"


//========================================================================
// Positions
//========================================================================

/**
 *  Coordinates of position in one segment.
 */
class TPOSITION {
public:
  T_SIMPLE x;                               //!< X coordinate.
  T_SIMPLE y;                               //!< Y coordinate.
  TPOSITION()                               //!< Basic constructor.
    {x = y = LAY_UNAVAILABLE_POSITION;};
  TPOSITION(T_SIMPLE c_x, T_SIMPLE c_y)     //!< Constructor that sets coordinates to value of parameters.
    {x = c_x; y = c_y;};
  TPOSITION& operator= (const TPOSITION& pos)         //!< Assigning operator.
    {x = pos.x; y = pos.y; return *this;};

  void SetPosition(const T_SIMPLE set_x, const T_SIMPLE set_y)  //!< assigning method sets value of coordinates to params
    {x = set_x; y = set_y;};

  bool operator== ( const TPOSITION& pos)               //!< comparing operator
    {if ((x == pos.x) && (y == pos.y)) return true; else return false;};

  bool operator!= ( const TPOSITION& pos)               //!< comparing operator
    {if ((x == pos.x) && (y == pos.y)) return false; else return true;};

  bool Compare(const T_SIMPLE a_x, const T_SIMPLE a_y) const    //!< method compare objects values with params
    {if ((x == a_x) && (y == a_y)) return true; else return false;};

  /** The method determines the new direction while passing to the next step.*/
  int GetDirection(TPOSITION new_pos) const;
  /** The method determines the direction from the angle in the plane.*/
  int GetDirection(double angle) const;
  /** The method returns angle between position and point.*/
  double GetAngle(TPOSITION point) const;
};


/**
 *  Coordinates of position including number of segment.
 *  
 *  @sa TPOSITION
 */
class TPOSITION_3D : public TPOSITION {
public:
  T_SIMPLE segment;                 //!< Segment coordinate.

  TPOSITION_3D(): TPOSITION()       //!< Basic constructor.
    {segment = LAY_UNAVAILABLE_POSITION;};
  //! Constructor that sets coordinates to value of params.
  TPOSITION_3D(const T_SIMPLE c_x,const T_SIMPLE c_y,const T_SIMPLE c_z) : TPOSITION(c_x, c_y)
    {segment = c_z;};

  //! Assigning method sets value of coordinates to params.
  void SetPosition(const T_SIMPLE set_x, const T_SIMPLE set_y, const T_SIMPLE set_z)
    {TPOSITION::SetPosition(set_x, set_y); segment = set_z;};
  //! Method compares objects values with params.
  bool Compare(const T_SIMPLE c_x, const T_SIMPLE c_y, const T_SIMPLE c_z) const
    {if (TPOSITION::Compare(c_x, c_y) && (segment == c_z)) return true; else return false; };
  /**
   *  The method determines the new direction while passing to the next step.
   *  
   *  @param new_pos  Coordinates of second position
   *  @return The method returns direction constants.
   */
  int GetDirection(TPOSITION_3D new_pos)
  {
    int dir = TPOSITION::GetDirection(new_pos);         //identifies directions in one segments
    if (dir == LAY_UNDEFINED)
      if (segment < new_pos.segment) return LAY_UP;                  //to higher segment
      else if (segment > new_pos.segment) return LAY_DOWN;           //to lower segment
      else return LAY_UNDEFINED;                        // same field
    else 
      return dir;
  };

  //! Assigning operator.
  TPOSITION_3D& operator= (const TPOSITION_3D& pos)
    {x = pos.x; y = pos.y; segment = pos.segment; return *this;};

  //! Comparing operator.
  bool operator== ( const TPOSITION_3D& pos)
    {if ((x == pos.x) && (y == pos.y) && (segment == pos.segment)) return true; else return false;};
  //! Comparing operator.
  bool operator!= ( const TPOSITION_3D& pos)
    {if ((x == pos.x) && (y == pos.y) && (segment == pos.segment)) return false; else return true;};

  //! Adding operator.
  TPOSITION_3D operator+(const TPOSITION_3D& pos) const
    {TPOSITION_3D sum = *this; sum.x += pos.x; sum.y += pos.y; sum.segment += pos.segment; return sum;};
  //! Subtracting operator.
  TPOSITION_3D operator-(const TPOSITION_3D& pos) const
    {TPOSITION_3D sum = *this; sum.x -= pos.x; sum.y -= pos.y; sum.segment -= pos.segment; return sum;};  
};


//=========================================================================
// Functions
//=========================================================================

void InitPositionChanges();


//=========================================================================
// Variables
//=========================================================================

extern TPOSITION_3D changes[];

#endif  //__dolayout_h__

//========================================================================
// End
//========================================================================
// vim:ts=2:sw=2:et:

