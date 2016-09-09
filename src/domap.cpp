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
 *  @file domap.cpp
 *
 *  Map functions.
 *
 *  @author Peter Knut
 *  @author Jiri Krejsa
 *
 *  @date 2002, 2003, 2004
 */

#include "cfg.h"

#include <glfw.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <string>

#include "dodata.h"
#include "dodraw.h"
#include "dofile.h"
#include "domap.h"
#include "domouse.h"
#include "doraces.h"
#include "doplayers.h"
#include "doschemes.h"
#include "dounits.h"
#include "doengine.h"

using std::string;

//=========================================================================
// Global Variables
//=========================================================================

TMAP map;       //!< Game's map.
TRADAR radar;   //!< Game's radar.

//=========================================================================
// BasicTerr
//=========================================================================

/**
 *  ???
 *
 *  @param tx  ???
 *  @param ty  ???
 *  @param tz  ???
 *
 *  @todo okomentovat funkciu a parametere
 */
TTERR_BASIC::TTERR_BASIC(int tx, int ty, int tz)
{
  // set common variables
  pos.SetPosition(tx, ty, tz);
  pitem = NULL;
  in_active_area = true;
  anim_id = -1;
}


/**
 *  ???
 *
 *  @todo okomentovat funkciu
 */
void TTERR_BASIC::Draw(void)
{
  if (anim_id < 0 || !in_active_area) return;

  SetMapPosition(pos.x, pos.y);
  pitem->Draw(anim_id);
}


/**
 *  ???
 *
 *  @todo okomentovat funkciu
 */
void TTERR_BASIC::UpdateGraphics(void)
{
  if (map.active_area.IsChanged()) {
    in_active_area = map.active_area.IsInArea(pos.x, pos.y, pitem->width, pitem->height);
  }
}


//=========================================================================
// Terrain fragment
//=========================================================================

/**
 *  ???
 *
 *  @param tid ???
 *  @param tx  ???
 *  @param ty  ???
 *  @param tz  ???
 *
 *  @todo okomentovat funkciu a parametere
 */
TTERR_FRAG::TTERR_FRAG(int tid, int tx, int ty, int tz)
: TTERR_BASIC(tx, ty, tz)
{
  pitem = scheme.terrf[tz] + tid;
  pitem->SetUsed(true);

  if (pitem->GetAnimCount()) anim_id = GetRandomInt(pitem->GetAnimCount());

  // update terrain id
  map.segments[tz].UpdateTerrainId(tx, ty, pitem->width, pitem->height, pitem->terrain_field);
}


//=========================================================================
// Terrain layer
//=========================================================================

/**
 *  ???
 *
 *  @param tid ???
 *  @param tx  ???
 *  @param ty  ???
 *  @param tz  ???
 *
 *  @todo okomentovat funkciu a parametere
 */
TTERR_LAYER::TTERR_LAYER(int tid, int tx, int ty, int tz)
: TTERR_BASIC(tx, ty, tz)
{
  pitem = scheme.terrl[tz] + tid;
  pitem->SetUsed(true);
  if (pitem->GetAnimCount()) anim_id = GetRandomInt(pitem->GetAnimCount());

  // update terrain id
  map.segments[tz].UpdateTerrainId(tx, ty, pitem->width, pitem->height, pitem->terrain_field);
}


//=========================================================================
// class TMAP_SURFACE - methods definition
//=========================================================================


/** 
 *  The method starts attack for each member of the list.
 *
 *  @param enemy  Pointer to enemy unit which disturbs watchers or aimers.
 *  @param watchers_list  Is true if it is watchers list.
 */
void TMAP_POOLED_LIST::AttackEnemy(TMAP_UNIT *enemy, bool watchers_list)
{
  for (TNODE* aux = first; aux != NULL; aux = static_cast<TNODE*>(aux->GetNext()))
  {
    TMAP_UNIT* attacker = aux->GetUnit();
    //attack only enemy units which hyperplayer doesn't own
    if ((enemy->GetPlayerID() != 0) && 
        (!player_array.IsRemote(attacker->GetPlayerID())) && 
        (enemy->GetPlayer() != attacker->GetPlayer()) && 
        (!attacker->HasTarget()) && 
        ((attacker->TestState(US_STAY)) || (attacker->TestState(US_ANCHORING))) &&
        (attacker->GetPlayer()->GetLocalMap()->GetAreaVisibility(enemy->GetPosition(), enemy->GetUnitWidth(), enemy->GetUnitHeight()))
       )
    {
      if (watchers_list && (attacker->GetAggressivity() == AM_AGGRESSIVE))
        attacker->StartAttacking(enemy, true);
      else if ((!watchers_list)  && ((attacker->GetAggressivity() == AM_OFFENSIVE) || (attacker->GetAggressivity() == AM_GUARDED)))
        attacker->StartAttacking(enemy, true);
    }
  }
}

//=========================================================================
// class TMAP_SURFACE - methods definition
//=========================================================================


TMAP_SURFACE::TMAP_SURFACE()        //!< Basic constructor.
{
  //TODO
  int i = 8,j=0;

  t_id = 0;
  unit = ghost = NULL;
  activity = NEW TNEURON_VALUE[8];       //<! Every player have his own activity. There should be player_array.GetCount()
  for (j=0;j<i;j++)
    activity[j]=0;

  aimers = NEW TMAP_POOLED_LIST(reinterpret_cast<TPOOL<TPOOLED_LIST::TNODE>*>(map.GetAimersPool()));
  watchers = NEW TMAP_POOLED_LIST(reinterpret_cast<TPOOL<TPOOLED_LIST::TNODE>*>(map.GetWatchersPool()));
}


TMAP_SURFACE::~TMAP_SURFACE()
{
  if (activity) 
    delete[] activity;

  delete aimers;
  delete watchers;
};


/**
 *  Returns activity of my and enemy units separately.
 *
 *  @param PlayerID        ID of player I want to know his actitivy (I've got his and enemy activity).
 *  @param my_activity     I will have here activity of player with @PlayerID.
 *  @param enemy_activity  Here will be sum of activity of other players.
 */
void TMAP_SURFACE::GetActivity(const T_SIMPLE PlayerID,  TNEURON_VALUE *my_activity, TNEURON_VALUE *enemy_activity)
{
  T_SIMPLE i=0;
  TNEURON_VALUE my = 0, enemy = 0;

  for (i=1;i<player_array.GetCount();i++)     //<! For every player. (I don't care abour hyper player's activity)
    if (i == PlayerID)                        //<! If it's me.
      my = activity[i];                       //<! My activity is separately.
    else
      enemy += activity[i];                   //<! Enemy activity is sum.

  *my_activity += my;                           //<! Just for consistence.
  *enemy_activity += enemy;
}

/**
 *  Decrease activity counters of all players (so it will not grow to infinity)
 *
 *  @param factor     Factor of decreasing.
 */
void TMAP_SURFACE::DecreaseActivity(T_SIMPLE factor)
{
  T_SIMPLE i=0;

  for (i=0;i<player_array.GetCount();i++)         //<! For every player.
    activity[i] = activity[i]/factor;             //<! Decrease by factor.
}

//=========================================================================
// class TMAP_SEGMENT - methods definition
//=========================================================================

/**
 *  Constructor.
 */
TMAP_SEGMENT::TMAP_SEGMENT()
{
  terrf_count = terro_count = 0;
  terrf = NULL;
  terro = NULL;
  surface = NULL;
  average_surface_difficulty = 0;
  tex_radar_id = 0;
}


/**
 *  Destructor.
 */
TMAP_SEGMENT::~TMAP_SEGMENT()
{
  Clear();

  if (tex_radar_id) glDeleteTextures(1, &tex_radar_id);
}


/**
 *  Clears all segment's data (fragments, objects, layers, etc.).
 */
void TMAP_SEGMENT::Clear(void)
{
  int i;

  // delete fragments
  if (terrf) {
    for (i = 0; i < terrf_count; i++) if (terrf[i]) delete terrf[i];
    delete[] terrf;
    terrf = NULL;
    terrf_count = 0;
  }

  // delete objects
  if (terro) {
    for (i = 0; i < terro_count; i++) delete terro[i];
    delete[] terro;
    terro = NULL;
    terro_count = 0;
  }

  // delete layers
  TLIST<TTERR_LAYER>::TNODE<TTERR_LAYER> *node;

  for (node = terrl.GetFirst(); node; node = node->GetNext())
    if (node->GetPitem()) delete node->GetPitem();

  terrl.DestroyList();

  // surface
  if (surface) DeleteMapSurface(surface);
  surface = NULL;

  average_surface_difficulty = 0;
}


/**
 *  Adds layer to segment.
 *
 *  @todo Comment function's parameters.
 */
bool TMAP_SEGMENT::AddLayer(int lid, int lx, int ly, int sid)
{

  TTERR_LAYER *l;

  if (!(l = NEW TTERR_LAYER(lid, lx, ly, sid))) return false;

  // add object into segment
  terrl.AddNode(l);

  return true;
}


/**
 *  @todo Comment.
 */
void TMAP_SEGMENT::UpdateTerrainId(int x, int y, int width, int height, TTERRAIN_FIELD field)
{
  for (int i = 0; i < width; i++)
    for (int j = 0; j < height; j++)
      surface[x + i][y + j].t_id = field[i][j];
}


/**
 *  Updates segment. Updates all terrain objets and sorts units in the segment.
 *  
 *  @note Fragments and layers hasn't its own animation. They are updated in scheme.
 *
 *  @param time_shift  Time shift from the last update.
 */
void TMAP_SEGMENT::UpdateGraphics(double time_shift)
{
  int i;

  if (map.active_area.IsChanged()) {
    for (i = 0; i < terrf_count; i++) terrf[i]->UpdateGraphics();
    terrl.ApplyFunction(&TTERR_LAYER::UpdateGraphics);
  }

  for (i = 0; i < terro_count; i++) terro[i]->UpdateGraphics(time_shift);
}


#define TestNeighbours(x, y) \
do { \
  id = (y) * mapw * 4 + (x) * 4 + 3; \
  ok = wtex[id] < 255.0f \
      || ((x) > 1 && lmap[(x) - 2][(y) - 1].state == WLK_UNKNOWN_AREA && wtex[id - 4] < 255) \
      || (lmap[(x) - 0][(y) - 1].state == WLK_UNKNOWN_AREA && wtex[id + 4] < 255); \
  \
  if ((y) > 1) { \
    id = ((y) - 1) * mapw * 4 + (x) * 4 + 3; \
    ok = ok \
      || ((x) > 1 && lmap[(x) - 2][(y) - 2].state == WLK_UNKNOWN_AREA && wtex[id - 4] < 255) \
      || (lmap[(x) - 1][(y) - 2].state == WLK_UNKNOWN_AREA && wtex[id] < 255) \
      || (lmap[(x) - 0][(y) - 2].state == WLK_UNKNOWN_AREA && wtex[id + 4] < 255); \
  } \
  \
  if ((y) < map.height) { \
    id = ((y) + 1) * mapw * 4 + (x) * 4 + 3; \
    ok = ok \
      || ((x) > 1 && lmap[(x) - 2][(y) - 0].state == WLK_UNKNOWN_AREA && wtex[id - 4] < 255) \
      || (lmap[(x) - 1][(y) - 0].state == WLK_UNKNOWN_AREA && wtex[id] < 255) \
      || (lmap[(x) - 0][(y) - 0].state == WLK_UNKNOWN_AREA && wtex[id + 4] < 255); \
  } \
  \
  ok = ok && lmap[(x) - 1][(y) - 1].state == WLK_UNKNOWN_AREA; \
} while(0)


/**
 *  Draws segment.
 */
void TMAP_SEGMENT::Draw(void)
{
  int i, j, k;
  T_BYTE seg_id = this - map.segments;
  double w;
  double h = 1.01;
  T_SIMPLE mapw = map.width + MAP_AREA_SIZE + 1;
  bool ok;
  int id;
  TLOC_MAP_FIELD **lmap = myself->GetLocalMap()->map[seg_id];
  GLubyte *wtex = map.war_fog.tex[view_segment];

  if (view_segment == DRW_ALL_SEGMENTS) {
    glEnable(GL_DEPTH_TEST);
    glClear(GL_DEPTH_BUFFER_BIT);
    glDepthFunc(GL_LEQUAL);

    glDisable(GL_TEXTURE_2D);
    glColorMask(0, 0, 0, 0);

    for (j = map.active_area.GetY() + 1; j <= map.active_area.GetY2(); j++)
      for (i = map.active_area.GetX() + 1; i <= map.active_area.GetX2(); i++) {
        
        TestNeighbours(i, j);

        if (ok) {
          for (k = i + 1; k <= map.active_area.GetX2(); k++) {
            TestNeighbours(k, j);
            if (!ok) break;
          }

          w = k - i + 0.01;

          SetMapPosition(i - 1, j - 1);
          glTranslated(0, 0, 0.1);

          glBegin(GL_QUADS);
            glVertex2d(0.0, 0.0);
            glVertex2d(w * DAT_MAPEL_STRAIGHT_SIZE / 2, w * DAT_MAPEL_DIAGONAL_SIZE / 2);
            glVertex2d((w - h) * DAT_MAPEL_STRAIGHT_SIZE / 2, (w + h) * DAT_MAPEL_DIAGONAL_SIZE / 2);
            glVertex2d(-h * DAT_MAPEL_STRAIGHT_SIZE / 2, h * DAT_MAPEL_DIAGONAL_SIZE / 2);
          glEnd();

          i = k;
        } // if ok
      }

    glColorMask(1, 1, 1, 1);
    glEnable(GL_TEXTURE_2D);
  }

  // draw surface in segments 0, 1
  /*if (seg_id != 2)*/ DrawSurface();

  // draw underground units in segment 1
  if (view_segment == DRW_ALL_SEGMENTS && seg_id == 1)
    map.segment_units[0]->Draw(DS_UNDERGROUND);

  if (view_segment == DRW_ALL_SEGMENTS) glDisable(GL_DEPTH_TEST);

  // draw units
  map.segment_units[seg_id]->Draw();

  /*
  // draw surface in segment 2
  if (seg_id == 2) {
    if (view_segment == DRW_ALL_SEGMENTS) glEnable(GL_DEPTH_TEST);

    DrawSurface();

    if (view_segment == DRW_ALL_SEGMENTS) glDisable(GL_DEPTH_TEST);
  }
  */
}


/**
 *  Draws units from segment to radar.
 */
void TMAP_SEGMENT::DrawSurface(void)
{
  int i;

  glColor4f(1.0, 1.0, 1.0, 1.0);
  for (i = 0; i < terrf_count; i++) if (terrf[i]->IsInActiveArea()) terrf[i]->Draw();
  terrl.ApplyFunction(&TTERR_LAYER::Draw);
}


/**
 *  Calculates arithmetic average of the difficulty of surface in the segment.
 *  Returns calculated value.
 *
 *  @param width  Width of the map.
 *  @param height Height of the map.
 */ 
double TMAP_SEGMENT::CalculateAverageDifficulty(T_SIMPLE width, T_SIMPLE height)
{
  int i, j;
  int seg = this - map.segments;      //segment number

  average_surface_difficulty = 0;

  //count in each field
  for (i = 0; i < width; i++)
    for (j = 0; j < height; j++)
    {
      average_surface_difficulty += scheme.terrain_props[seg][surface[i][j].t_id].difficulty;
    }

  average_surface_difficulty /= (width*height);

  return average_surface_difficulty;    //returning result of calculation
}


/**
 *  Loads information about segment into segment structure.
 */
bool TMAP_SEGMENT::LoadMapSegment()
{
  T_SIMPLE sid = this - map.segments;

  TFILE_LINE sec_name;
  bool ok = true;

  sprintf(sec_name, "Segment %d", sid);
  ok = map.file->SelectSection(sec_name, true);

  if (!(map.segments[sid].surface = CreateMapSurface()) || !ok) return false;
  
  if (ok) ok = LoadMapFragments(sid);
  if (ok) ok = LoadMapLayers(sid);
  if (ok) ok = LoadMapObjects(sid);

  map.segments[sid].CalculateAverageDifficulty(map.width, map.height);

  map.file->UnselectSection();

  return ok;
}


/**
 *  Load terrain fragments.
 *
 *  @param sid segment ID
 */
bool TMAP_SEGMENT::LoadMapFragments(T_BYTE sid)
{
  bool ok = true, stop_cycle;
  TMAP_SEGMENT *seg = map.segments + sid;
  int i, j, k, l;
  int count_empty_frag = 0; // count of missing fragments in fragment array
  int count_empty_surf = 0; // count of missing fragments in surface (map)
  TTERR_FRAG **terrf_new = NULL; //new array of fragments

  ok = map.file->SelectSection(const_cast<char*>("Fragments"), true);
  
  if (ok) ok = map.file->ReadIntGE(&seg->terrf_count, const_cast<char*>("count"), 0, 0);
  
  // allocate memory for terrain fragments
  if (ok) {
    if (!(seg->terrf = NEW PTERR_FRAG[seg->terrf_count])) {
      Critical("Can not allocate memory for map data.");
      ok = false;
    }

    for (i = 0; i < seg->terrf_count; i++) seg->terrf[i] = NULL;
  }

  for (i = 0; ok && i < seg->terrf_count; i++)
    ok = LoadMapFragment(sid, i);
  
  map.file->UnselectSection();

  for (i = 0; i < seg->terrf_count; i++){
    if (!(seg->terrf[i])) count_empty_frag++;
  }

  for (i = 0; i < map.width; i++)
    for (j = 0; j < map.height; j++){
      if (seg->surface[i][j].t_id == MAP_EMPTY_SURFACE) count_empty_surf++;
    }

  // in case that some fragments are missing
  if ((count_empty_surf) || (count_empty_frag)){
    // allocate memory for new terrain fragments
    if (!(terrf_new = NEW PTERR_FRAG[seg->terrf_count - count_empty_frag + count_empty_surf])) {
      Critical("Can not allocate memory for map data.");
      ok = false;
    }

    // copy from old fragment array to new array tterrf_new
    for (i = 0, j = 0; i < seg->terrf_count; i++){
      if (seg->terrf[i]) terrf_new[j++] = seg->terrf[i];
    }
    
    // adding default fragments to array
    k = l = 0;
    for (;((j < seg->terrf_count - count_empty_frag + count_empty_surf)&&ok); j++){
      
      stop_cycle = true;
      for (;(k < map.width) && (stop_cycle); k++){
        if (l >= map.height) l = 0;
        for (;(l < map.height) && (stop_cycle); l++){
          if (seg->surface[k][l].t_id == MAP_EMPTY_SURFACE)
            stop_cycle = false;
        }
      }
      

      if (!(terrf_new[j] = NEW TTERR_FRAG(scheme.terrf_count[sid] - 1, --k, --l, sid))){
        ok = false;
        Critical("Can not allocate memory for map data.");
      }
    }
    
    // delete old terrf array
    delete[] seg->terrf;

    // assign new terrf to segment
    seg->terrf = terrf_new;
    seg->terrf_count = seg->terrf_count - count_empty_frag + count_empty_surf;
  }
  return ok;
}


/**
 *   Set map fragment.
 */
bool TMAP_SEGMENT::LoadMapFragment(T_BYTE sid, int id)
{
  TFILE_LINE item;
  int fid;
  int x, y;
  TMAP_SEGMENT *seg = map.segments + sid;
  int i, j;

  sprintf(item, "fragment_%d", id);

  if (!map.file->ReadIntGE(&fid, item, 0, 0)) return false;
  if (!map.file->ReadIntGE(&x, item, 0, 0)) return false;
  if (!map.file->ReadIntGE(&y, item, 0, 0)) return false;

  if ((map.IsInMap(x,y)) && (map.IsInMap(x + scheme.terrf[sid][fid].width - 1, y + scheme.terrf[sid][fid].height - 1))){
    
    // test overlapping fragments
    for (i = x; i < x + scheme.terrf[sid][fid].width - 1; i++)
      for (j = y; j < y + scheme.terrf[sid][fid].height - 1; j++){
        if (seg->surface[i][j].t_id != MAP_EMPTY_SURFACE){
          Warning(LogMsg("Map fragment %d located on another fragment in segment %d.", id, sid));
          return true;
        }
      }

    if (!(seg->terrf[id] = NEW TTERR_FRAG(fid, x, y, sid))) return false;
  }
  else 
    Warning(LogMsg("Map fragment %d located out of map in segment %d.", id, sid));

  return true;
}


/**
 *  Load terrain layers.
 *
 *  @param sid
 */
bool TMAP_SEGMENT::LoadMapLayers(T_BYTE sid)
{
  bool ok = true;
  int count;

  ok = map.file->SelectSection(const_cast<char*>("Layers"), true);
  
  if (ok) ok = map.file->ReadIntGE(&count, const_cast<char*>("count"), 0, 0);

  for (int i = 0; ok && i < count; i++) ok = LoadMapLayer(sid, i);

  map.file->UnselectSection();

  return ok;
}


/**
 *  set player
 */
bool TMAP_SEGMENT::LoadMapLayer(T_BYTE sid, int id)
{
  TFILE_LINE item;
  int lid;
  int x, y;
  TMAP_SEGMENT *seg = map.segments + sid;

  sprintf(item, "layer_%d", id);

  if (!map.file->ReadIntGE(&lid, item, 0, 0)) return false;
  if (!map.file->ReadIntGE(&x, item, 0, 0)) return false;
  if (!map.file->ReadIntGE(&y, item, 0, 0)) return false;

  if ((map.IsInMap(x,y)) && (map.IsInMap(x + scheme.terrl[sid][lid].width - 1, y + scheme.terrl[sid][lid].height - 1))){
    if (!(seg->AddLayer(lid, x, y, sid))) return false;
  }
  else 
    Warning(LogMsg("Map layer %d located out of map in segment %d.", id, sid));


  return true;
}


/**
 *  Load terrain objects.
 */
bool TMAP_SEGMENT::LoadMapObjects(T_BYTE sid)
{
  bool ok = true;
  TMAP_SEGMENT *seg = map.segments + sid;

  ok = map.file->SelectSection(const_cast<char*>("Objects"), true);
  
  if (ok) ok = map.file->ReadIntGE(&seg->terro_count, const_cast<char*>("count"), 0, 0);

  // allocate memory for terrain objects
  if (ok) {
    if (!(seg->terro =  NEW TDRAW_UNIT*[seg->terro_count])) {
      Critical("Can not allocate memory for map data.");
      ok = false;
    }

    for (int i = 0; i < seg->terro_count; i++) seg->terro[i] = NULL;
  }

  if (ok) for (int i = 0; ok && i < seg->terro_count; i++)
    ok = LoadMapObject(sid, i);

  map.file->UnselectSection();

  return ok;
}


/**
 *  Set map object
 */
bool TMAP_SEGMENT::LoadMapObject(T_BYTE sid, int id)
{
  TFILE_LINE item;
  int oid;
  int x, y;
  TMAP_SEGMENT *seg = map.segments + sid;

  sprintf(item, "object_%d", id);

  if (!map.file->ReadIntGE(&oid, item, 0, 0)) return false;
  if (!map.file->ReadIntGE(&x, item, 0, 0)) return false;
  if (!map.file->ReadIntGE(&y, item, 0, 0)) return false;

  if ((map.IsInMap(x,y)) && (map.IsInMap(x + scheme.terro[sid][oid].GetWidth() - 1, y + scheme.terro[sid][oid].GetHeight() - 1))){
    if (!(seg->terro[id] = NEW TDRAW_UNIT(x, y, sid, (scheme.terro[sid]+oid), &scheme.tex_table, true))) return false;

    seg->terro[id]->AddToSegments();
    seg->terro[id]->SetVisible(true);
    map.segments[sid].UpdateTerrainId(x, y, scheme.terro[sid][oid].GetWidth(), scheme.terro[sid][oid].GetHeight(), scheme.terro[sid][oid].terrain_field);
  }
  else 
    Warning(LogMsg("Map object %d located out of map in segment %d.", id, sid));

  return true;
}


//=========================================================================
// class TSEG_UNITS - methods definition - Segments units
//=========================================================================

TSEG_UNITS::TSEG_UNITS(T_BYTE seg_id)
{
  // create mutex
  if ((mutex = glfwCreateMutex ()) == NULL) {
    Critical ("Could not create units mutex");
  }

  id = seg_id;
  units = NEW TDRAW_UNIT();   // create head of units list

  Clear();
}


TSEG_UNITS::~TSEG_UNITS()
{
  delete units;

  glfwDestroyMutex(mutex);
}


void TSEG_UNITS::Clear()
{
  glfwLockMutex(mutex);

  for (int i = 0; i <= DAT_SEGMENTS_COUNT; i++) {
    units->SetNextInSegment(i, units);
    units->SetPrevInSegment(i, units);
  }
  units_count = 0;

  glfwUnlockMutex(mutex);
}


/**
 *  Adds unit to segment.
 *
 *  @param unit  Pointer to unit to add.
 */
void TSEG_UNITS::AddUnit(TDRAW_UNIT *unit)
{
  TDRAW_UNIT *mu = NULL;
  TDRAW_UNIT *next_unit = NULL;

  glfwLockMutex(mutex);

  // logging
  /*
  if (id == 1) {
    Debug(LogMsg("*** AddUnit(%s) - before", unit->GetPointerToItem()->id));
    for (mu = units->GetPrevInSegment(id); (mu != units); mu = mu->GetPrevInSegment(id))
    {
      Debug(LogMsg("%s", mu->GetPointerToItem()->id));
    }
  }
  */

  if (unit->GetNextInSegment(id)) {
    //if (id == 1) Debug(LogMsg("*** AddUnit(%s) - warning: DOUBLE!", unit->GetPointerToItem()->id));
    glfwUnlockMutex(mutex);
    return;
  }

  // find position in sorted list
  for (mu = units->GetPrevInSegment(id); (mu != units) && (mu != unit) && !unit->IsCloserThan(mu); mu = mu->GetPrevInSegment(id));

  next_unit = mu->GetNextInSegment(id);

  unit->SetNextInSegment(id, next_unit);
  unit->SetPrevInSegment(id, next_unit->GetPrevInSegment(id));
  unit->GetNextInSegment(id)->SetPrevInSegment(id, unit);
  unit->GetPrevInSegment(id)->SetNextInSegment(id, unit);
  units_count++;

  // logging
  /*
  if (id == 1) {
    Debug("*** AddUnit - after");
    for (mu = units->GetPrevInSegment(id); (mu != units); mu = mu->GetPrevInSegment(id))
    {
      Debug(LogMsg("%s", mu->GetPointerToItem()->id));
    }
    Debug("*** AddUnit - end");
  }
  */

  glfwUnlockMutex(mutex);
}


/**
 *  Removes unit from segment.
 *
 *  @param unit  Pointer to unit to remove.
 */
void TSEG_UNITS::DeleteUnit(TDRAW_UNIT *unit)
{
  glfwLockMutex(mutex);

  // logging
  /*
  if (id == 1) {
    Debug(LogMsg("*** DeleteUnit(%s) - before", unit->GetPointerToItem()->id));
    for (TDRAW_UNIT *mu = units->GetPrevInSegment(id); (mu != units); mu = mu->GetPrevInSegment(id))
    {
      Debug(LogMsg("%s", mu->GetPointerToItem()->id));
    }
  }
  */

  if (!unit->GetNextInSegment(id) || !units_count) {
    //if (id == 1) Debug(LogMsg("*** DeleteUnit(%s) - warning: DELETED", unit->GetPointerToItem()->id));
    glfwUnlockMutex(mutex);
    return;
  }

  // remove unit from list
  // prev and next unit in segment is allways defined because of head
  unit->GetNextInSegment(id)->SetPrevInSegment(id, unit->GetPrevInSegment(id));
  unit->GetPrevInSegment(id)->SetNextInSegment(id, unit->GetNextInSegment(id));

  unit->SetNextInSegment(id, NULL);
  unit->SetPrevInSegment(id, NULL);

  units_count--;

  // logging
  /*
  if (id == 1) {
    Debug("*** DeleteUnit - after");
    for (TDRAW_UNIT *mu = units->GetPrevInSegment(id); (mu != units); mu = mu->GetPrevInSegment(id))
    {
      Debug(LogMsg("%s", mu->GetPointerToItem()->id));
    }
    Debug("*** DeleteUnit - end");
  }
  */

  glfwUnlockMutex(mutex);
}


/**
 *  Sorts units in segment according to distance from the observer.
 */
void TSEG_UNITS::SortUnits(void)
{
#if DEBUG_SORTING
  int counter_test = 0;
  int counter = 0;
#endif

  glfwLockMutex(mutex);

  if (units_count < 2) {
    glfwUnlockMutex(mutex);
    return;   // nothing to sort
  }

  TDRAW_UNIT *p;
  TDRAW_UNIT *next_unit;

  TDRAW_UNIT *left_unit = units->GetNextInSegment(id);
  TDRAW_UNIT *next_left_unit;

  int left = 0;
  int next_left;

  int right = units_count - 1;
  int next_right;

  int i, j;

  for (i = 0; i < units_count; i++) {

    next_right = 0;
    next_left = units_count - 1;
    next_left_unit = units->GetPrevInSegment(id);

    for (j = left, p = left_unit; j < right; j++) {

      #if DEBUG_SORTING
      counter_test++;
      #endif

      if (p->IsCloserThan(p->GetNextInSegment(id))) {

        #if DEBUG_SORTING
        counter++;
        #endif

        next_unit = p->GetNextInSegment(id);

        // remove unit
        p->GetPrevInSegment(id)->SetNextInSegment(id, p->GetNextInSegment(id));
        p->GetNextInSegment(id)->SetPrevInSegment(id, p->GetPrevInSegment(id));

        // insert unit
        p->SetPrevInSegment(id, next_unit);
        p->SetNextInSegment(id, next_unit->GetNextInSegment(id));
        p->GetNextInSegment(id)->SetPrevInSegment(id, p);
        p->GetPrevInSegment(id)->SetNextInSegment(id, p);

        next_right = j;
        if (j - 1 < next_left) {
          if (!j) {
            next_left = j;
            next_left_unit = next_unit;
          }
          else {
            next_left = j - 1;
            next_left_unit = next_unit->GetPrevInSegment(id);
          }
        }
      }
      else p = p->GetNextInSegment(id);
    }

    if (next_right <= next_left) break;
    right = next_right;
    left = next_left;
    left_unit = next_left_unit;
  }

#if DEBUG_SORTING
  if (id == 3 && counter) {
    Debug(LogMsg("units: %d, tests: %d, sort: %d", units_count, counter_test, counter));
  }
#endif

  glfwUnlockMutex(mutex);
}


/**
 *  Draws units from segment to radar.
 */
void TSEG_UNITS::Draw(T_BYTE style)
{
  TDRAW_UNIT *unit;

  glfwLockMutex(mutex);

  // draw units in one segment
  for (unit = units->GetNextInSegment(id); unit != units; unit = unit->GetNextInSegment(id))
    unit->Draw(style);

  glfwUnlockMutex(mutex);
}


/**
 *  Draws units from segment to radar.
 */
void TSEG_UNITS::DrawToRadar(void)
{
  TDRAW_UNIT *unit;

  glfwLockMutex(mutex);

  // draw units
  for (unit = units->GetNextInSegment(id); unit != units; unit = unit->GetNextInSegment(id)) {
    unit->DrawToRadar();
  }

  glfwUnlockMutex(mutex);
}


//=========================================================================
// struct TWARFOG
//=========================================================================


/**
 *  Initializes warfog.
 *
 *  @return @c true on success, @c false otherwise.
 */
bool TWARFOG::Create(void)
{
  int i, j;
  int w, h;
  int map_all, tex_all;
  GLubyte *pom_tex;

  for (w = 64; w < map.width + MAP_AREA_SIZE + 1; w *= 2);
  for (h = 64; h < map.height + MAP_AREA_SIZE + 1; h *= 2);
  map_all = (map.width + MAP_AREA_SIZE + 1) * (map.height + MAP_AREA_SIZE + 1) * 4;
  tex_all = w * h * 4;

  // create temporary texture
  pom_tex = NEW GLubyte[tex_all];
  if (pom_tex == NULL) return false;

  // coordinates
  x1_coord = (GLfloat)1 / w;
  y1_coord = (GLfloat)1 / h;
  x2_coord = (GLfloat)(map.width + MAP_AREA_SIZE + 1) / w;
  y2_coord = (GLfloat)(map.height + MAP_AREA_SIZE + 1) / h;
  x3_coord = (GLfloat)(map.width + 1) / w;
  y3_coord = (GLfloat)(map.height + 1) / h;

  // fill texture and field
  for (i = 0; i < tex_all; i++) pom_tex[i] = (i % 4 == 3) ? 255 : 0;
  
  glEnable(GL_TEXTURE_2D);
  
  // warfog texture
  glGenTextures(1, &tex_id);
  glBindTexture(GL_TEXTURE_2D, tex_id);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, (void *)pom_tex);

  // radar warfog texture
  glGenTextures(1, &radar_tex_id);
  glBindTexture(GL_TEXTURE_2D, radar_tex_id);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, (void *)pom_tex);

  // delete temporary texture
  delete[] pom_tex;

  // create fields for generating warfog
  for (i = 0; i <= DAT_SEGMENTS_COUNT; i++) {
    tex[i] = NEW GLubyte[map_all];
    for (j = 0; j < map_all; j++) tex[i][j] = ((j % 4 == 3) ? 255 : 0);
  }

  return true;
}


/**
 *  Cleanes warfog.
 */
void TWARFOG::Clear(void)
{
  int i;

  // coordinates
  x1_coord = y1_coord = 0;
  x2_coord = y2_coord = 0;
  x3_coord = y3_coord = 0;

  if (tex_id) {
    glDeleteTextures(1, &tex_id);
    tex_id = 0;
  }
  if (radar_tex_id) {
    glDeleteTextures(1, &radar_tex_id);
    radar_tex_id = 0;
  }

  for (i = 0; i <= DAT_SEGMENTS_COUNT; i++) {
    if (tex[i]) delete[] tex[i];
    tex[i] = NULL;
  }
}


/**
 *  Fills warfog texture with values from local map segment.
 */
void TWARFOG::Update(void)
{
  // warfog texture
  glBindTexture(GL_TEXTURE_2D, tex_id);
  glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, map.width + MAP_AREA_SIZE + 1, map.height + MAP_AREA_SIZE + 1, GL_RGBA, GL_UNSIGNED_BYTE, tex[view_segment]);

  // radar warfog texture
  if (radar_panel->IsVisible()) {
    glBindTexture(GL_TEXTURE_2D, radar_tex_id);

    if (view_segment == DRW_ALL_SEGMENTS)
      glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, map.width + MAP_AREA_SIZE + 1, map.height + MAP_AREA_SIZE + 1, GL_RGBA, GL_UNSIGNED_BYTE, tex[1]);
    else
      glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, map.width + MAP_AREA_SIZE + 1, map.height + MAP_AREA_SIZE + 1, GL_RGBA, GL_UNSIGNED_BYTE, tex[view_segment]);
  }
}


/**
 *  Draws warfog texture.
 */
void TWARFOG::Draw(void)
{
  GLfloat px1 = (GLfloat)map.active_area.GetX();
  GLfloat py1 = (GLfloat)map.active_area.GetY();
  GLfloat pw = (GLfloat)map.active_area.GetWidth() + MAP_AREA_SIZE;
  GLfloat ph = (GLfloat)map.active_area.GetHeight() + MAP_AREA_SIZE;

  GLfloat x1 =  x1_coord * (px1 + 1);
  GLfloat y1 =  y1_coord * (py1 + 1);

  GLfloat x2 =  x1_coord * (px1 + pw + 1);
  GLfloat y2 =  y1_coord * (py1 + ph + 1);

  // draw warfog
  glColor3f(1, 1, 1);
  SetMapPosition(px1, py1);

  glBindTexture(GL_TEXTURE_2D, tex_id);

  glBegin(GL_QUADS);
    glTexCoord2f(x1, y1);
    glVertex2f(0, 0);
    glTexCoord2f(x2, y1);
    glVertex2f(pw * DAT_MAPEL_STRAIGHT_SIZE_2, pw * DAT_MAPEL_DIAGONAL_SIZE_2);
    glTexCoord2f(x2, y2);
    glVertex2f((pw - ph) * DAT_MAPEL_STRAIGHT_SIZE_2, (pw + ph) * DAT_MAPEL_DIAGONAL_SIZE_2);
    glTexCoord2f(x1, y2);
    glVertex2f(-ph * DAT_MAPEL_STRAIGHT_SIZE_2, ph * DAT_MAPEL_DIAGONAL_SIZE_2);
  glEnd();
}


/**
 *  Draws warfog texture to radar window.
 */
void TWARFOG::DrawToRadar(void)
{
  T_SIMPLE w = map.width;
  T_SIMPLE h = map.height;
  GLdouble zoom = radar.zoom;

  // draw warfog
  glColor3f(1, 1, 1);
  glPushMatrix();
  glTranslated(radar.dx, 0, 0);

  glBindTexture(GL_TEXTURE_2D, radar_tex_id);

  glBegin(GL_QUADS);
    // bottom left
    glTexCoord2f(x1_coord, y1_coord);
    glVertex2d(0.0, 0.0);
    // bottom right
    glTexCoord2f(x3_coord, y1_coord);
    glVertex2d(w * zoom, w * zoom);
    // top right
    glTexCoord2f(x3_coord, y3_coord);
    glVertex2d((w - h) * zoom, (w + h) * zoom);
    // top left
    glTexCoord2f(x1_coord, y3_coord);
    glVertex2d(-h * zoom, h * zoom);
  glEnd();

  glPopMatrix();
}


//=========================================================================
// struct TMAP
//=========================================================================

/**
 *  Initialises map structure.
 */
void TMAP::Initialise () {
  *name = 0;
  *id_name = 0;
  *author = 0;
  *scheme_name = 0;

  width = height = 0;

  start_time = 0.0;
  file = NULL;

  Zoom(MAP_ZOOM_RESET);
  zoom_flag = MAP_ZOOM_RESET;

  dx = dy = 0.0f;
  move_flag = MAP_MOVE_NONE;
  mouse_moving = drag_moving = false;

  for (int i = 0; i < DAT_SEGMENTS_COUNT; i++) 
    segment_units[i] = NULL;
}


/**
 *  Constructor.
 */
TMAP::TMAP() {
  Initialise();

  aimers_pool = NULL;
  watchers_pool = NULL;
}


void TMAP::InitPools(void) {
  aimers_pool = NEW TPOOL<TMAP_POOLED_LIST::TNODE>(Sqr(MAP_MAX_SIZE / 2)*DAT_SEGMENTS_COUNT*3, 0, 10000);
  watchers_pool = NEW TPOOL<TMAP_POOLED_LIST::TNODE>(Sqr(MAP_MAX_SIZE / 2)*DAT_SEGMENTS_COUNT*5, 0, 20000);
}


/**
 *  Clears map structure.
 */
void TMAP::Clear()
{
  int i;

  for (i = 0; i < DAT_SEGMENTS_COUNT; i++) {
    segments[i].Clear();
     if (segment_units[i]) delete segment_units[i];
  }

  /* pools are initialised in InitAll() function, deleted with application end
  if (aimers_pool)
    delete static_cast<TPOOL<TMAP_POOLED_LIST::TNODE> *> (aimers_pool);

  if (watchers_pool)
    delete static_cast<TPOOL<TMAP_POOLED_LIST::TNODE> *> (watchers_pool);
  */
  
  war_fog.Clear();
  
  Initialise();
}


/**
 *  Updates map's moving.
 *
 *  @param time_shift  Time shift from the last update.
 */
void TMAP::UpdateMoving(double time_shift)
{
  if (move_flag) {
    // compute move difference
    GLfloat diff = GLfloat(config.map_move_speed * MAP_MOVE_COEF * time_shift);
    GLfloat mdx = 0.0;
    GLfloat mdy = 0.0;

    if (move_flag & MAP_MOVE_LEFT)    mdx -= diff;
    if (move_flag & MAP_MOVE_RIGHT)   mdx += diff;
    if (move_flag & MAP_MOVE_UP)      mdy += diff;
    if (move_flag & MAP_MOVE_DOWN)    mdy -= diff;

    // move
    Move(mdx, mdy);

    // reset moving with mouse
    move_flag &= ~MAP_MOUSE_MOVE;
  }
}


/**
 *  Updates map's graphics.
 *
 *  @param time_shift  Time shift from the last update.
 */
void TMAP::UpdateGraphics(double time_shift)
{
  int i;

  // map segments
  for (i = 0; i < DAT_SEGMENTS_COUNT; i++) {
    map.segments[i].UpdateGraphics(time_shift);
    map.segment_units[i]->SortUnits();
  }

  // update warfog
  war_fog.Update();
}


/**
 *  Zooms in / out map or resets map zoom.
 *
 *  @param dzoom  Zoom difference - one of constants: MAP_ZOOM_* or any number:
 *                positive value - zoom in, negative - zoom out.
 */
void TMAP::Zoom(int dzoom)
{
  // reset zoom
  if (dzoom == MAP_ZOOM_RESET) zoom = 1.0;
  else {

    // compute zoom difference
    float diff = ((1.0f + ((float)config.map_zoom_speed / 100)) * dzoom);

    // set zoom
    if (dzoom > 0) zoom /= diff;
    else zoom *= -diff;

    // check zoom range
    if (zoom > MAP_MAX_ZOOM) zoom = MAP_MAX_ZOOM;
    else if (zoom < MAP_MIN_ZOOM) zoom = MAP_MIN_ZOOM;
  }
}


/**
 *  Moves with a map.
 *
 *  @param mdx  X position difference (screen axis).
 *  @param mdy  Y position difference (screen axis).
 */
void TMAP::Move(GLfloat mdx, GLfloat mdy)
{
  GLfloat pom_dx = dx + mdx;
  GLfloat pom_dy = dy + mdy;

  GLfloat mindx = -(GLfloat)(map.width * DAT_MAPEL_STRAIGHT_SIZE / 2);
  GLfloat maxdx = (GLfloat)(map.height * DAT_MAPEL_STRAIGHT_SIZE / 2);

  GLfloat maxdy = 0.0;
  GLfloat mindy = -(GLfloat)((map.height + map.width) * DAT_MAPEL_DIAGONAL_SIZE / 2);

  // check moving range
  if (pom_dx > maxdx) pom_dx = maxdx;
  else if (pom_dx < mindx) pom_dx = mindx;

  if (pom_dy > maxdy) pom_dy = maxdy;
  else if (pom_dy < mindy) pom_dy = mindy;


  if (mouse.draw_selection) {
    mouse.sel_x += (pom_dx - dx) / projection.game_h_coef;
    mouse.sel_y += (pom_dy - dy) / projection.game_v_coef;
  }

  dx = pom_dx;
  dy = pom_dy;
}


/**
 *  Moves with a map and centers specified mapel.
 *
 *  @param x  X position of mapel [mapels].
 *  @param y  Y position of mapel [mapels].
 */
void TMAP::CenterMapel(T_SIMPLE x, T_SIMPLE y)
{
  dx = GLfloat(-DAT_MAPEL_STRAIGHT_SIZE_2 * x + DAT_MAPEL_STRAIGHT_SIZE_2 * y);
  dy = GLfloat(-DAT_MAPEL_DIAGONAL_SIZE_2 * x - DAT_MAPEL_DIAGONAL_SIZE_2 * y);
}


/**
 *  Directly sets map position.
 *
 *  @param ndx  New horizontal position [pixels].
 *  @param ndy  New vertical position [pixels].
 */
void TMAP::SetPosition(GLfloat ndx, GLfloat ndy)
{
  dx = ndx;
  dy = ndy;
}

/**
 *  The method tests whether position is free.
 *
 *  @param w        Width of tested area.
 *  @param h        Height of tested area.
 *  @param pos      Test start position.
 */
bool TMAP::IsPositionFree(T_SIMPLE w, T_SIMPLE h, const TPOSITION_3D &pos)
{
  int i,j;
  
  for (i = pos.x; i < pos.x + w; i++) 
    for (j = pos.y; j < pos.y + h; j++) { //testing through new position
      
      if (!IsInMap(i,j)) return false;   //test, whether the position is in map
      if (segments[pos.segment].surface[i][j].unit) return false; //test, whether there's any unit standing at the position
    }

  return true;
}


/**
 *  Updates active area.
 */
void TMAP::UpdateActiveArea()
{
  float w = (float)config.scr_width;
  float h = (float)config.scr_height;
  double x1, x2, y1, y2, pom;

  MouseToMap(0, 0, &x1, &pom);
  x1--;
  if (x1 < 0) x1 = 0;
  if (x1 > width) x1 = width;

  MouseToMap(w, h, &x2, &pom);
  x2++;
  if (x2 < 0) x2 = 0;
  if (x2 > width) x2 = width;

  MouseToMap(w, 0, &pom, &y1);
  y1--;
  if (y1 < 0) y1 = 0;
  if (y1 > height) y1 = height;

  MouseToMap(0, h, &pom, &y2);
  y2++;
  if (y2 < 0) y2 = 0;
  if (y2 > height) y2 = height;

  active_area.SetArea((T_SIMPLE)x1, (T_SIMPLE)x2, (T_SIMPLE)y1, (T_SIMPLE)y2);
}


/**
 *  Starts map moving with a key in asked direction.
 *
 *  @param mflag  Move flag. One of direction constants #MAP_KEY_MOVE_UP,
 *                #MAP_KEY_MOVE_DOWN, #MAP_KEY_MOVE_LEFT, #MAP_KEY_MOVE_RIGHT.
 */
void TMAP::StartKeyMove(int mflag)
{
  move_flag |= mflag;
}


/**
 *  Ends map moving with a key in asked direction.
 *
 *  @param mflag  Move flag. One of direction constants #MAP_KEY_MOVE_UP,
 *                #MAP_KEY_MOVE_DOWN, #MAP_KEY_MOVE_LEFT, #MAP_KEY_MOVE_RIGHT.
 */
void TMAP::StopKeyMove(int mflag)
{
  move_flag &= ~mflag;
}


/**
 *  Starts map moving with a mouse in asked direction.
 *
 *  @param mflag  Move flag. One of direction constants #MAP_MOUSE_MOVE_UP,
 *                #MAP_MOUSE_MOVE_DOWN, #MAP_MOUSE_MOVE_LEFT,
 *                #MAP_MOUSE_MOVE_RIGHT.
 */
void TMAP::MouseMove(int mflag)
{
  move_flag |= mflag;
}


/**
 *  Draws a map.
 */
void TMAP::Draw()
{
  // terrain segments
  if (view_segment == DRW_ALL_SEGMENTS) {
    for (int i = 0; i < DAT_SEGMENTS_COUNT; i++) segments[i].Draw();
  }
  else segments[view_segment].Draw();

  // draw warfog
  if (!show_all) war_fog.Draw();

  DrawBorder();
}


/**
 *  Draws border around map. Border is drawn only next to unknow map fields.
 */
void TMAP::DrawBorder()
{
  int i, j;
  int tex_id;

  T_SIMPLE map_w = map.width + MAP_AREA_SIZE + 1;

  glDisable(GL_TEXTURE_2D);  

  glColor3f(0.3f, 0.3f, 0.3f);
  SetMapPosition(0, 0);

  tex_id = map_w * 4 + 4 + 3;

  // bottom border
  for (i = 0; i < map.width; i++, tex_id += 4) {

    // black field is found
    if (map.war_fog.tex[view_segment][tex_id] == 255) {

      // find non black field
      for (j = i + 1, tex_id += 4; j < map.width; j++, tex_id += 4) {

        if (map.war_fog.tex[view_segment][tex_id] < 255) {
          glBegin(GL_LINES);
            glVertex2d(i * DAT_MAPEL_STRAIGHT_SIZE_2, i * DAT_MAPEL_DIAGONAL_SIZE_2);
            glVertex2d(j * DAT_MAPEL_STRAIGHT_SIZE_2, j * DAT_MAPEL_DIAGONAL_SIZE_2);
          glEnd();

          break;
        }
      }

      if (j == map.width) {
        glBegin(GL_LINES);
          glVertex2d(i * DAT_MAPEL_STRAIGHT_SIZE_2, i * DAT_MAPEL_DIAGONAL_SIZE_2);
          glVertex2d(j * DAT_MAPEL_STRAIGHT_SIZE_2, j * DAT_MAPEL_DIAGONAL_SIZE_2);
        glEnd();
      }

      i = j;
    }
  }

  tex_id = map.height * map_w * 4 + 4 + 3;

  // top border
  for (i = 0; i < map.width; i++, tex_id += 4) {

    // black field is found
    if (map.war_fog.tex[view_segment][tex_id] == 255) {

      // find non black field
      for (j = i + 1, tex_id += 4; j < map.width; j++, tex_id += 4) {

        if (map.war_fog.tex[view_segment][tex_id] < 255) {
          glBegin(GL_LINES);
            glVertex2d((i - map.height) * DAT_MAPEL_STRAIGHT_SIZE_2, (i + map.height) * DAT_MAPEL_DIAGONAL_SIZE_2);
            glVertex2d((j - map.height) * DAT_MAPEL_STRAIGHT_SIZE_2, (j + map.height) * DAT_MAPEL_DIAGONAL_SIZE_2);
          glEnd();

          break;
        }
      }

      if (j == map.width) {
        glBegin(GL_LINES);
          glVertex2d((i - map.height) * DAT_MAPEL_STRAIGHT_SIZE_2, (i + map.height) * DAT_MAPEL_DIAGONAL_SIZE_2);
          glVertex2d((j - map.height) * DAT_MAPEL_STRAIGHT_SIZE_2, (j + map.height) * DAT_MAPEL_DIAGONAL_SIZE_2);
        glEnd();
      }

      i = j;
    }
  }

  tex_id = map_w * 4 + 4 + 3;

  // left border
  for (i = 0; i < map.height; i++, tex_id += 4 * map_w) {

    // black field is found
    if (map.war_fog.tex[view_segment][tex_id] == 255) {

      // find non black field
      for (j = i + 1, tex_id += 4 * map_w; j < map.height; j++, tex_id += 4 * map_w) {

        if (map.war_fog.tex[view_segment][tex_id] < 255) {
          glBegin(GL_LINES);
            glVertex2d(-i * DAT_MAPEL_STRAIGHT_SIZE_2, i * DAT_MAPEL_DIAGONAL_SIZE_2);
            glVertex2d(-j * DAT_MAPEL_STRAIGHT_SIZE_2, j * DAT_MAPEL_DIAGONAL_SIZE_2);
          glEnd();

          break;
        }
      }

      if (j == map.height) {
        glBegin(GL_LINES);
          glVertex2d(-i * DAT_MAPEL_STRAIGHT_SIZE_2, i * DAT_MAPEL_DIAGONAL_SIZE_2);
          glVertex2d(-j * DAT_MAPEL_STRAIGHT_SIZE_2, j * DAT_MAPEL_DIAGONAL_SIZE_2);
        glEnd();
      }

      i = j;
    }
  }

  tex_id = map_w * 4 + map.width * 4 + 3;

  // right border
  for (i = 0; i < map.height; i++, tex_id += 4 * map_w) {

    // black field is found
    if (map.war_fog.tex[view_segment][tex_id] == 255) {

      // find non black field
      for (j = i + 1, tex_id += 4 * map_w; j < map.height; j++, tex_id += 4 * map_w) {

        if (map.war_fog.tex[view_segment][tex_id] < 255) {
          glBegin(GL_LINES);
            glVertex2d((map.width - i) * DAT_MAPEL_STRAIGHT_SIZE_2, (map.width + i) * DAT_MAPEL_DIAGONAL_SIZE_2);
            glVertex2d((map.width - j) * DAT_MAPEL_STRAIGHT_SIZE_2, (map.width + j) * DAT_MAPEL_DIAGONAL_SIZE_2);
          glEnd();

          break;
        }
      }

      if (j == map.height) {
        glBegin(GL_LINES);
          glVertex2d((map.width - i) * DAT_MAPEL_STRAIGHT_SIZE_2, (map.width + i) * DAT_MAPEL_DIAGONAL_SIZE_2);
          glVertex2d((map.width - j) * DAT_MAPEL_STRAIGHT_SIZE_2, (map.width + j) * DAT_MAPEL_DIAGONAL_SIZE_2);
        glEnd();
      }

      i = j;
    }
  }

  glEnable(GL_TEXTURE_2D);
}


/**
 *  Draws map view to radar.
 */
void TMAP::DrawToRadar()
{
  glColor3f(1, 1, 1);
  GLenum tex_id = 0;

  // draw surface texture
  if (view_segment == DRW_ALL_SEGMENTS) tex_id = segments[1].tex_radar_id;
  else tex_id = segments[view_segment].tex_radar_id;

  if (tex_id) {
    glBindTexture(GL_TEXTURE_2D, tex_id);

    glBegin(GL_QUADS);
      glTexCoord2f(0, 0); glVertex2f(0, 0);
      glTexCoord2f(1, 0); glVertex2f(DRW_RADAR_SIZE, 0);
      glTexCoord2f(1, 1); glVertex2f(DRW_RADAR_SIZE, DRW_RADAR_SIZE);
      glTexCoord2f(0, 1); glVertex2f(0, DRW_RADAR_SIZE);
    glEnd();
  }

  // draw warfog
  war_fog.DrawToRadar();

  glPushMatrix();
  glTranslated(radar.dx, 0, 0);

  // draw units
  for (int i = 0; i < DAT_SEGMENTS_COUNT; i++)
    segment_units[i]->DrawToRadar();

  // border around map
  glDisable(GL_TEXTURE_2D);  
  glColor3f(0.3f, 0.3f, 0.3f);

  glBegin(GL_LINE_LOOP);
    glVertex2d(0.0, 0.0);
    glVertex2d(map.width * radar.zoom, map.width * radar.zoom);
    glVertex2d((map.width - map.height) * radar.zoom, (map.width + map.height) * radar.zoom);
    glVertex2d(-map.height * radar.zoom, map.height * radar.zoom);
  glEnd();

  // active area - for testing
  /*{
    GLfloat px = active_area.GetX() * radar.zoom;
    GLfloat py = active_area.GetY() * radar.zoom;
    GLfloat pw = active_area.GetWidth() * radar.zoom;
    GLfloat ph = active_area.GetHeight() * radar.zoom;

    glColor3f(1.0f, 1.0f, 0.0f);

    glBegin(GL_LINE_LOOP);
      glVertex2d(px - py, px + py);
      glVertex2d(px + pw - py, px + pw + py);
      glVertex2d(px + pw - ph - py, px + pw + ph + py);
      glVertex2d(px - py - ph, px + py + ph);
    glEnd();
  }*/

  glEnable(GL_TEXTURE_2D);
  glPopMatrix();
}



/**
 *   Deletes map and depend structures.
 */
void TMAP::DeleteMap()
{
  map.Clear();
  DeletePlayers();
  DeleteRaces();
  scheme.Clear();
}


/**
 *  Loads map from a file.
 *
 *  @param name  Name of the map (filename without extension).
 *
 *  @return @c true on succes, @c false otherwise.
 */
bool TMAP::LoadMap(char *name) // load map from file
{
  TFILE_NAME  mapname;

  bool ok = true;

  map.Clear();  // free loaded map and initialize all variables

  // create segments units
  for (int i = 0; i < DAT_SEGMENTS_COUNT; i++) map.segment_units[i] = NEW TSEG_UNITS(i);

  sprintf(mapname, "%s%s%s", MAP_PATH, name, ".map");

  Info(LogMsg("Loading map from '%s'", mapname));

  strcpy(map.id_name, name);

  if (!(map.file = OpenConfFile(mapname)))
    return false;

  // info
  if (ok) {
    map.file->ReadStr(map.name, const_cast<char*>("name"), const_cast<char*>(""), true);
    map.file->ReadStr(map.author, const_cast<char*>("author"), const_cast<char*>(""), true);
  }

  // map scheme
  if (ok) ok = map.file->ReadSimpleRange(&map.width, const_cast<char*>("width"), 1, MAP_MAX_SIZE, 1);
  if (ok) ok = map.file->ReadSimpleRange(&map.height, const_cast<char*>("height"), 1, MAP_MAX_SIZE, 1);

  if (ok) ok = map.file->ReadStr(map.scheme_name, const_cast<char*>("scheme"), const_cast<char*>(""), true);
  if (ok) ok = LoadScheme(map.scheme_name);
  if (ok) ok = LoadRaces();
 
  if (ok)
    for (int i = 0; i < DAT_SEGMENTS_COUNT && ok; i++)
      ok = map.segments[i].LoadMapSegment();

    if (ok) ok = map.war_fog.Create();
    if (ok) ok = LoadMapPlayers();
    
  CloseConfFile(map.file);

  if (!ok) {
    DeleteMap();
    Error(LogMsg("Error loading map from '%s'", mapname));
  }

  radar.dx = GLfloat(map.height) * DRW_RADAR_SIZE / (map.height + map.width);
  radar.zoom = radar.dx / map.height;

  RenderRadarTextures();
     
  return ok;
}


/**
 *  set map unit
 */
bool TMAP::LoadMapUnit(int pid, int id)    // set map unit
{
  TFILE_LINE item;
  int uid;
  int x, y, z;
  int dir;
  int life;
  char strval[1024]; //buffer
  TFORCE_UNIT *unit;

  sprintf(item, "unit_%d", id);
    
  if (!map.file->ReadStr(strval, item, const_cast<char*>(""), false)) return false;
  
  uid = GetItemPrgID(strval, (TMAP_ITEM **) players[pid]->race->units, players[pid]->race->units_count);
  if (uid == -1){
    Warning(LogMsg("Bad force/worker unit id '%s' in item '%s'", strval, item));
    return false;
  }

  if (!map.file->ReadInt(&x, item, 0)) return false;
  if (!map.file->ReadInt(&y, item, 0)) return false;
  if (!map.file->ReadIntGE(&z, item, 0, 0)) return false;
  if (!map.file->ReadIntRange(&dir, item, 0, 7, 0)) return false;
  if (!map.file->ReadIntRange(&life, item, 0, 100, 100)) return false;

  x += players[pid]->initial_x;
  y += players[pid]->initial_y;


  if (players[pid]->race->units[uid]->IsPositionAvailable(x,y,z)){
    switch (players[pid]->race->units[uid]->GetItemType()) {
    case IT_FORCE:
      unit = NEW TFORCE_UNIT(pid, x, y, z, dir, *(players[pid]->race->units + uid), 0, true);
      break;
    case IT_WORKER:
      unit = NEW TWORKER_UNIT(pid, x, y, z, dir, *(players[pid]->race->units + uid), 0, true);
      break;
    default:
      unit = NULL;
      break;
    }
  
    if (!unit) return false;

    if (unit->AddToMap(true, true)) {
      
      unit->SetLife((life * static_cast<TMAP_ITEM *>(unit->GetPointerToItem())->GetMaxLife()) / 100.0f);
    
      players[pid]->AddUnitEnergyFood(((TBASIC_ITEM *)(unit->GetPointerToItem()))->energy, ((TBASIC_ITEM *)(unit->GetPointerToItem()))->food);
      players[pid]->IncPlayerUnitsCount(); //increase count of active units of player
      ((TBASIC_ITEM *)(unit->GetPointerToItem()))->IncreaseActiveUnitCount();  // increase count of units of this kind
      
      // send init event to queue to local (not remote) units
      if (!player_array.IsRemote(pid)){
        process_mutex->Lock();
        unit->SendEvent(false, glfwGetTime(), US_NEXT_STEP, -1, unit->GetPosition().x, unit->GetPosition().y, unit->GetPosition().segment, unit->GetMoveDirection());
        process_mutex->Unlock();
      }
    }
    else {
      delete unit;
      Warning(LogMsg("Can not add unit %d ('%s') into the map.", id, strval));
      return false;
    }
  }
  else 
    Warning(LogMsg("Can not add unit %d ('%s') into the map.", id, strval));

  return true;
}


/**
 *  load units
 */
bool TMAP::LoadMapUnits(int pid)
{
  bool ok = true;
  int count;

  ok = map.file->SelectSection(const_cast<char*>("Units"), false);
  
  if (ok) ok = map.file->ReadIntGE(&count, const_cast<char*>("count"), 0, 0);

  for (int i = 0; ok && i < count; i++) ok = LoadMapUnit(pid ,i);

  map.file->UnselectSection();

  return ok;
}


/**
 *  set map buidlding
 */
bool TMAP::LoadMapBuilding(int pid, int id)
{
  TFILE_LINE item;
  int bid;
  int x, y;
  int life;
  TBUILDING_UNIT *unit;
  char strval[1024]; //buffer
  
  sprintf(item, "building_%d", id);

  if (!map.file->ReadStr(strval, item, const_cast<char*>(""), false)) return false;
  
  bid = GetItemPrgID(strval, (TMAP_ITEM **) players[pid]->race->buildings, players[pid]->race->buildings_count);
  if (bid == -1){
    Warning(LogMsg("Bad building/factory unit id '%s' in item '%s'", strval, item));
    return false;
  }

  if (!map.file->ReadInt(&x, item, 0)) return false;
  if (!map.file->ReadInt(&y, item, 0)) return false;
  if (!map.file->ReadIntRange(&life, item, 0, 100, 100)) return false;
  
  x += players[pid]->initial_x;
  y += players[pid]->initial_y;
  
  if (players[pid]->race->buildings[bid]->IsPositionAvailable(x, y, false)) {
    switch (players[pid]->race->buildings[bid]->GetItemType()) {
    case IT_FACTORY:
      unit = NEW TFACTORY_UNIT(pid, x, y, *(players[pid]->race->buildings + bid), 0, true);
      if (static_cast<TBUILDING_ITEM *>(unit->GetPointerToItem())->AllowAnyMaterial())   
        unit->AddToPlayerArray();//add unit to material_array of player and increase counter in every source
      break;
    case IT_BUILDING:
      unit = NEW TBUILDING_UNIT(pid, x, y, *(players[pid]->race->buildings + bid), 0, true);
      if (static_cast<TBUILDING_ITEM *>(unit->GetPointerToItem())->AllowAnyMaterial())   
        unit->AddToPlayerArray();//add unit to material_array of player and increase counter in every source
      break;
    default:
      unit = NULL;
      break;
    }
   
    if (!unit) return false;

    if (unit->AddToMap(true, true)) {
      unit->SetLife((life * static_cast<TMAP_ITEM *>(unit->GetPointerToItem())->GetMaxLife()) / 100.0f);

      players[pid]->AddUnitEnergyFood(((TBASIC_ITEM *)(unit->GetPointerToItem()))->energy, ((TBASIC_ITEM *)(unit->GetPointerToItem()))->food);
      players[pid]->IncPlayerUnitsCount(); //increase count of active units of player
      ((TBASIC_ITEM *)(unit->GetPointerToItem()))->IncreaseActiveUnitCount();  // increase count of units of this kind

      // send init event to queue to local (not remote) buildings
      if (!player_array.IsRemote(pid)){
        process_mutex->Lock();
        unit->SendEvent(false, glfwGetTime(), US_STAY, -1);
        process_mutex->Unlock();
      }
    }
    else {
      delete unit;
      Warning(LogMsg("Can not add building %d ('%s') into the map.", id, strval));
      return false;
    }
  }
  else
    Warning(LogMsg("Can not add building %d ('%s') into the map.", id, strval));

  return true;
}


/**
 *  load units
 */
bool TMAP::LoadMapBuildings(int pid)
{
  bool ok = true;
  int count;

  ok = map.file->SelectSection(const_cast<char*>("Buildings"), true);
  
  if (ok) ok = map.file->ReadIntGE(&count, const_cast<char*>("count"), 0, 0);

  for (int i = 0; ok && i < count; i++) ok = LoadMapBuilding(pid, i);

  map.file->UnselectSection();

  return ok;
}


/**
 *  set map buidlding
 */
bool TMAP::LoadMapSource(int pid, int id)
{
  TFILE_LINE item;
  int sid;
  int x, y;
  int life;
  int balance;
  bool ok;
  TSOURCE_UNIT *unit;
  char strval[1024]; //buffer

  sprintf(item, "source_%d", id);
  
  if (!map.file->ReadStr(strval, item, const_cast<char*>(""), false)) return false;
  
  sid = GetItemPrgID(strval, (TMAP_ITEM **) players[pid]->race->sources, players[pid]->race->sources_count);
  if (sid == -1){
    Warning(LogMsg("Bad source unit id '%s' in item '%s'", strval, item));
    return false;
  }

  if (!map.file->ReadInt(&x, item, 0)) return false;
  if (!map.file->ReadInt(&y, item, 0)) return false;
  if (!map.file->ReadIntRange(&life, item, 0, 100, 100)) return false;
  if (!map.file->ReadIntGE(&balance, item, 0, 0)) return false;
  
  x += players[pid]->initial_x;
  y += players[pid]->initial_y;
  
  if (players[pid]->race->sources[sid]->IsPositionAvailable(x, y)) {
    unit = NEW TSOURCE_UNIT(pid, sid, x, y, *(players[pid]->race->sources + sid), 0, true); 
    if (!unit) return false;

    unit->SetLife((life * static_cast<TMAP_ITEM *>(unit->GetPointerToItem())->GetMaxLife()) / 100.0f); // in precents

    ok = unit->SetMaterialBalance(balance);
    if (players[pid]->race->sources[sid]->IsHideable()) unit->AddToSegments();
    else ok = unit->AddToMap(true, false);

    if (!ok) {
      delete unit;
      Warning(LogMsg("Can not add source %d ('%s') into the map.", id, strval));
      return false;
    }
    else{
      // send init event to queue to local (not remote) sources
      if (!player_array.IsRemote(pid)){
        process_mutex->Lock();
        unit->SendEvent(false, glfwGetTime(), US_STAY, -1);
        process_mutex->Unlock();
      }
    }
  }
  else
    Warning(LogMsg("Can not add source %d ('%s') into the map.", id, strval));

  return true;
}


/**
 *  load sources
 */
bool TMAP::LoadMapSources(int pid)
{
  bool ok = true;
  int count;

  ok = map.file->SelectSection(const_cast<char*>("Sources"), true);
  
  if (ok) ok = map.file->ReadIntGE(&count, const_cast<char*>("count"), 0, 0);

  for (int i = 0; ok && i < count; i++) ok = LoadMapSource(pid, i);

  map.file->UnselectSection();

  return ok;
}


/**
 *  set player
 */
bool TMAP::LoadMapPlayer(int pid)
{
  TFILE_LINE item;
  int i, intval;
  bool ok = true, ok_all;
  char strval[1024]; //buffer
  
  // loading hyper player structures
  if (pid == 0){
    hyper_player = players[pid];

    ok = map.file->SelectSection(const_cast<char*>("SchemeRace"), true);
  }
 
  // loading real map players
  else{
    // user don't know hyper player => in *.map file are player marked from index 0
    
    // finding right race
    
    if (ok){
      ok = (map.file->GetActSection()->ResetValue(const_cast<char*>("count")) > 0);
      ok = map.file->ReadIntGE(&intval, const_cast<char*>("count"), 1, 1); //reading count of races
    }

    if (ok) {
      ok_all = false;
      for (i = 0; (ok) && (i < intval); i++){
        sprintf (item, "Race %d", i);
        if (ok) ok = map.file->SelectSection(item, true);
        
        if (ok) ok = (map.file->GetActSection()->ResetValue(const_cast<char*>("name")) > 0);
        if (ok) ok = map.file->ReadStr(strval, const_cast<char*>("name"), const_cast<char*>(""), true);
        
        if (!(strcmp(players[pid]->race->id_name, strval))){
          ok_all = true;
          break;
        }
        map.file->UnselectSection();
      }
      if (ok) ok = ok_all;
    }
  }
    
  if (pid != 0){ // hyper player do not have init_materials_amount and sets
    if (ok) ok = map.file->SelectSection(const_cast<char*>("Sets"), true);
    if (ok) ok = map.file->ReadIntGE(&intval, const_cast<char*>("count"), 1, 1);

    if (ok){
      sprintf(item, "Set %d", GetRandomInt(intval)); // choose random set and select it
      ok = map.file->SelectSection(item, true);
    }
    
    //init amount of materials
    for ( i = 0; i < scheme.materials_count; i++) {
      map.file->ReadIntGE(&intval, const_cast<char*>("init_materials_amount"), 0, 0);
      players[pid]->SetStoredMaterial(i, float(intval));
    }
  }
  
  if ((ok) && (pid == 0)) ok = LoadMapSources(pid);    // sources must be loaded befor buildings and for hyperplayer only
  if (ok) ok = LoadMapBuildings(pid);
  if (ok) ok = LoadMapUnits(pid);

  // global animations
  if ((players[pid]) == myself) {
    // food & energy
    players[pid]->need_animation[0] = NEW TGUI_ANIMATION(players[pid]->race->tex_table.GetTexture(players[pid]->race->tg_food_id, 1));
    players[pid]->need_animation[1] = NEW TGUI_ANIMATION(players[pid]->race->tex_table.GetTexture(players[pid]->race->tg_energy_id, 1));

    // materials
    for (i = 0; i < scheme.materials_count; i++)
      players[pid]->need_animation[i + 2] = NEW TGUI_ANIMATION(scheme.tex_table.GetTexture(scheme.materials[i]->tg_id, 1));
  }

  if (pid != 0){
    map.file->UnselectSection(); // Set x
    map.file->UnselectSection(); // Sets
  }
  map.file->UnselectSection(); // Race x
  
  return ok;
}


/**
 *  load players
 */
bool TMAP::LoadMapPlayers()
{

  // struct used for saving information about startpoints
  struct TSTART_POINT {
    T_SIMPLE x, y;

    TSTART_POINT() { x = y = 0; }
  };
    
  bool ok = true;
  int i, start_points_count;
  TRACE *b;
  TSTART_POINT start_points[PL_MAX_START_POINTS];
  TFILE_LINE item;

  CreatePlayers ();       // allocate place for active players

  /* Find out playerID of player playing on local computer. */
  int my_player_id = player_array.GetMyPlayerID ();
  myself = players[my_player_id];

  ok = map.file->SelectSection(const_cast<char*>("Players"), true);
  
  // reading start points
  
  if (ok) ok = map.file->SelectSection(const_cast<char*>("Start Points"), true);

  if (ok) ok = map.file->ReadIntGE(&start_points_count, const_cast<char*>("count"), 1, 1);
  start_points_count = MIN(start_points_count, PL_MAX_START_POINTS);  // if there is more then PL_MAX_START_POINTS, PL_MAX_START_POINTS is used

  if (ok) {
    // test if there are enough start points in map
    if (start_points_count < player_array.GetCount() - 1) {
      ok = false;
      Error(LogMsg("No enough start points in map."));
    }
  
    for (i = 0; ok && i < start_points_count; i++){ 
      sprintf (item, "start_point_%d", i);
      if (ok) ok = map.file->ReadSimpleRange(&start_points[i].x, item, 0, map.width, 0);
      if (ok) ok = map.file->ReadSimpleRange(&start_points[i].y, item, 0, map.height, 0);
    }

    map.file->UnselectSection();
  }
  
  // for every player
  for (i = 0; ok && i < player_array.GetCount(); i++) {
    string race_name = player_array.GetRaceIdName (i);
    
    // set race to player
    for (b = races; b; b = b->next)
      if (race_name == b->id_name)
        break;

    if (!b) {
      if (race_name == "")
        Error("Can not find race '(null)'");
      else
        Error(LogMsg("Can not find race '%s'", race_name.c_str ()));
      return false;
    }

    players[i]->race = b;
    players[i]->active = true;
  }
    
  if (ok) ok = map.file->SelectSection(const_cast<char*>("Races"), true);
  
  // hyperplayer starts automaticly at [0,0]
  if (ok) {
    players[0]->initial_x = 0;
    players[0]->initial_y = 0;
    LoadMapPlayer(0);
  }

  for (i = 1; ok && i < player_array.GetCount(); i++){
    int start = player_array.GetStartPoint (i);

    players[i]->initial_x = start_points[start].x;
    players[i]->initial_y = start_points[start].y;
   
    ok = LoadMapPlayer(i);
  }
  
  map.file->UnselectSection(); // end of races
  map.file->UnselectSection(); // end of players

  return ok;
}

/**
 *
 */
void TMAP::RenderRadarTextures()
{
  GLubyte pom_tex[DRW_RADAR_TEX_SIZE * DRW_RADAR_TEX_SIZE * 3];
  int i;

  // set projection
  for (i = 0; i < DAT_SEGMENTS_COUNT; i++) {
    glClear(GL_COLOR_BUFFER_BIT);
    projection.SetProjection(PRO_RENDER_RADAR);

    // draw map segment
    map.segments[i].DrawSurface();

    projection.SetProjection(PRO_MENU);

    // read image to temp. field
    glReadPixels(0, 0, DRW_RADAR_TEX_SIZE, DRW_RADAR_TEX_SIZE, GL_RGB, GL_UNSIGNED_BYTE, &pom_tex);

    // generate new texture
    glGenTextures(1, &map.segments[i].tex_radar_id);
    glBindTexture(GL_TEXTURE_2D, map.segments[i].tex_radar_id);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, DRW_RADAR_TEX_SIZE, DRW_RADAR_TEX_SIZE, 0, GL_RGB, GL_UNSIGNED_BYTE, (void *)pom_tex);
  }
}


//=========================================================================
// Class TRADAR
//=========================================================================

void TRADAR::Draw(void)
{
  map.DrawToRadar();

  double koef = radar.zoom / DAT_MAPEL_DIAGONAL_SIZE;

  double cx   = -map.dx * koef + radar.dx;
  double cy   = -map.dy * koef * 2;

  double w = PRO_DEF_WIDTH  * koef * map.GetZoom() / 2;
  double h = PRO_DEF_HEIGHT * koef * map.GetZoom(); 

  double x1, x2, y1, y2;

  x1 = cx - w;
  y1 = cy - h;
  x2 = cx + w;
  y2 = cy + h;

  // screen border in radar
  glDisable(GL_TEXTURE_2D);  
  glColor3f(0.5f, 0.5f, 0.5f);

  glBegin(GL_LINE_LOOP);
    glVertex2d(x1, y1);
    glVertex2d(x1, y2);
    glVertex2d(x2, y2);
    glVertex2d(x2, y1);
  glEnd();

  glEnable(GL_TEXTURE_2D);
}


//=========================================================================
// global methods definition
//=========================================================================

//=========================================================================
// MapSurface
//=========================================================================

/**
 *  Creates map surface - 2 dimensional field of surface information.
 *
 *  @return Pointer to 2D surface.
 */
TMAP_SURFACE **CreateMapSurface()
{
  TMAP_SURFACE **surface = NULL;
  int i = 0, j = 0;

  bool ok = true;

  if (!(surface = NEW PMAP_SURFACE[map.width]))   // allocating memory
    ok = false;

  if (ok) for (i = 0; i < map.width && ok; i++) {
    surface[i] = NULL;
    if (!(surface[i] = NEW TMAP_SURFACE[map.height])) ok = false;
  }
  
  // setting speciah surface value to detect mapels without fragments
  for (i = 0; i < map.width; i++)
    for (j = 0; j < map.height; j++)
      surface[i][j].t_id = MAP_EMPTY_SURFACE;
  
  if (ok)       // whether allocation has been succesful
    return surface;
  else {        // allocation failed
    if (surface) DeleteMapSurface(surface);
    return NULL;
  }
}


void DeleteMapSurface(TMAP_SURFACE **surface)
{
  if (!surface) return;

  for (int i = 0; i < map.width; i++)
    if (surface[i]) {
      for (int j = 0; j < map.height; j++)
        surface[i][j].Clear();
      
      delete [] surface[i];
      surface[i] = NULL;
    }

  delete[] surface;
  surface = NULL;
}


//=========================================================================
// END
//=========================================================================
// vim:ts=2:sw=2:et:

