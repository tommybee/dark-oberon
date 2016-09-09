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
 *  @file dolayout.cpp
 *
 *  Work with coordinates and terrain.
 *
 *  @author Jiri Krejsa
 *  @author Peter Knut
 *  @author Valeria Sventova
 *
 *  @date 2003
 */

#include <stdlib.h>
#include <math.h>

#include "doconfig.h"
#include "dodraw.h"
#include "dolayout.h"
#include "domap.h"
#include "doplayers.h"
#include "dosimpletypes.h"


//=========================================================================
// Variables
//=========================================================================

TPOSITION_3D changes[LAY_DIRECTIONS_COUNT];

//=========================================================================
// Definitions of global functions.
//=========================================================================

/** 
 *  Initialize array with changes of position in dependence to the direction.
 */
void InitPositionChanges()
{
  changes[0].SetPosition(           1,(T_SIMPLE)-1,           0);
  changes[1].SetPosition(           0,(T_SIMPLE)-1,           0);
  changes[2].SetPosition((T_SIMPLE)-1,(T_SIMPLE)-1,           0);
  changes[3].SetPosition((T_SIMPLE)-1,           0,           0);
  changes[4].SetPosition((T_SIMPLE)-1,           1,           0);
  changes[5].SetPosition(           0,           1,           0);
  changes[6].SetPosition(           1,           1,           0);
  changes[7].SetPosition(           1,           0,           0);
  changes[8].SetPosition(           0,           0,           1);
  changes[9].SetPosition(           0,           0,(T_SIMPLE)-1);
}


//=========================================================================
// TPOSITION
//=========================================================================

/**
 *  The method determines the new direction while passing to the next step.
 *  
 *  @param new_pos  Coordinates of new position
 *  @return The method returns direction constants.
 */
int TPOSITION::GetDirection(TPOSITION new_pos) const
{
  if ((new_pos.x == this->x) && (new_pos.y == this->y)) 
    return LAY_UNDEFINED;
  else
    return GetDirection(GetAngle(new_pos));
}


/** 
 *  The method returns angle between position and point.
 *
 *  @param  point The point which determine angel.
 *  @return The method returns angel in the range [0, 2*PI).
 */
double TPOSITION::GetAngle(TPOSITION point) const
{
  double dx, dy, angel;

  dx = static_cast<short>(point.x) - static_cast<short>(x);
  dy = static_cast<short>(point.y) - static_cast<short>(y);

  //get angel from interval, be carrefuly to right angle
  if (dx == 0.0)
  {
    if (dy > 0)
      angel = PI * 0.5;
    else
      angel = PI * (-0.5);
  }
  else
    angel = atan2(dy, dx);        //calculate angel between fire and impact position

  if (angel < 0.0)
    angel += 2*PI;

  return angel;
}


/**
 *  The method determines the direction from the angle  in the plane.
 *  
 *  @param angle Angle which will be transformate to direction.
 *  @return The method returns direction constants.
 */
int TPOSITION::GetDirection(double angle) const
{
  int result;
  //set angle to interval [0, 2*PI)
  while (angle >= 2*PI)
    angle -= 2*PI;

  while (angle < 0)
    angle += 2*PI;

  //get return direction
  if ((angle > (1.875*PI)) || (angle <= (0.125*PI)))
  {
    result = LAY_EAST;
  }
  else if ((angle > (0.125*PI)) && (angle <= (0.375*PI)))
  {
    result = LAY_NORTH_EAST;
  }
  else if ((angle > (0.375*PI)) && (angle <= (0.625*PI)))
  {
    result = LAY_NORTH;
  }
  else if ((angle > (0.625*PI)) && (angle <= (0.875*PI)))
  {
    result = LAY_NORTH_WEST;
  }
  else if ((angle > (0.875*PI)) && (angle <= (1.125*PI)))
  {
    result = LAY_WEST;
  }
  else if ((angle > (1.125*PI)) && (angle <= (1.375*PI)))
  {
    result = LAY_SOUTH_WEST;
  }
  else if ((angle > (1.375*PI)) && (angle <= (1.625*PI)))
  {
    result = LAY_SOUTH;
  }
  else
  {
    result = LAY_SOUTH_EAST;
  }
  return result;
}


//=========================================================================
// END
//=========================================================================
// vim:ts=2:sw=2:et:

