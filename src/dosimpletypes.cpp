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
 *  @file dosimpletypes.cpp
 *
 *  Terrain field functions.
 *
 *  @author Martin Kosalko
 *
 *  @date 2002, 2003
 */

#include <time.h>
#include <stdlib.h>

#include "dosimpletypes.h"


/**
 *  Deletes 2D array of terrain identifiers.
 *
 *  @param width  Width of field.
 */
void DeleteTerrainField(TTERRAIN_FIELD terrain_field, int width)
{
  if (!terrain_field) return;

  int i;

  for (i = 0; i < width; i++)
    if (terrain_field[i]) 
      delete[] terrain_field[i];

  delete[] terrain_field;
}


/**
 *  Creates 2D array of terrain identifiers.
 *
 *  @param width  Width of field.
 *  @param height Height of field.
 */
TTERRAIN_FIELD CreateTerrainField(int width, int height)
{
  TTERRAIN_FIELD terrain_field = NULL;
  int i, j;

  bool ok = true;

  if (!(terrain_field = NEW PTERRAIN_ID[width])) ok = false;

  if (ok) for (i = 0; i < width; i++) {
    terrain_field[i] = NULL;
    if (!(terrain_field[i] = NEW TTERRAIN_ID[height])) ok = false;

    if (ok) for (j = 0; j < height; j++) terrain_field[i][j] = 0;
  }

  if (ok) return terrain_field;
  else {
    if (terrain_field) DeleteTerrainField(terrain_field, width);
    return NULL;
  }
}

/**
 *  Function generates natural number between 0 and @param max.
 */
int GetRandomInt(int max)
{
  if (!max) return 0;
  
  return rand() % max;
}


/**
 *  Returns random float number between zero and one.
 */
float GetRandomFloat()
{
  return static_cast<float>(rand()%1024)/1023;    //generating of random number between 0.0 and 1.0
}

//=========================================================================
// END
//=========================================================================
// vim:ts=2:sw=2:et:

