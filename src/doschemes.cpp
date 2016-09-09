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
 *  @file doschemes.cpp
 *
 *  Resources functions.
 *
 *  @author Peter Knut
 *
 *  @date 2003, 2004
 */

#include <stdlib.h>
#include <string.h>

#include "doconfig.h"
#include "dofile.h"
#include "doschemes.h"
#include "doengine.h"

// Local Defines
#define EMPTY 255

//=========================================================================
// Global variables
//=========================================================================

/** Actual scheme. */
TSCHEME  scheme;


//=========================================================================
// struct TTERR_ITEM
//=========================================================================

void TTERR_ITEM::SetTextures(TTEX_GROUP *group)
{
  if (group) {
    if (animation) ClearAnimations();
    else {
      anim_count = group->count;
      animation = NEW TGUI_ANIMATION *[anim_count];
      for (int i = 0; i < anim_count; i++) animation[i] = NEW TGUI_ANIMATION(group->textures + i);
    }
  }
  else if (animation) ClearAnimations();
}


//=========================================================================
// struct TMATERIAL
//=========================================================================

/**
 *  Constructor.
 */
TMATERIAL::TMATERIAL()
{
  *name = *id  = 0;
  tg_id = -1;
}


//=========================================================================
// TSCHEME
//=========================================================================

/**
 *  Updates fragments and layers. Both of them has only one animation for all instances.
 *
 *  @note Map objects has its own animation. Thay are updated in map.
 *
 *  @param time_shift  Time shift from the last update.
 */
void TSCHEME::UpdateGraphics(double time_shift)
{
  int i, seg;

  for (seg = 0; seg < DAT_SEGMENTS_COUNT; seg++) {
    for (i = 0; i < terrf_count[seg]; i++) terrf[seg][i].UpdateGraphics(time_shift);
    for (i = 0; i < terrl_count[seg]; i++) terrl[seg][i].UpdateGraphics(time_shift);
  }
}


/**
 *  Cleare scheme structure - removes scheme from memory.
 */
void TSCHEME::Clear(void)
{
  int i,j;

  *id_name = 0;
  *name = 0;
  *author = 0;

  // free texture set
  tex_table.Clear();

  // fragments
  for (j = 0; j < DAT_SEGMENTS_COUNT; j++){
    if (terrf[j]) {
      delete[] terrf[j];
      terrf[j] = NULL;
    }
  }

  // layers
  for (j = 0; j < DAT_SEGMENTS_COUNT; j++){
    if (terrl[j]) {
      delete[] terrl[j];
      terrl[j] = NULL;
    }
  }

  // objects
  for (j = 0; j < DAT_SEGMENTS_COUNT; j++){
    if (terro[j]) {
      delete[] terro[j];
      terro[j] = NULL;
    }
  }

  // counts
  for (j = 0; j < DAT_SEGMENTS_COUNT; j++){
    terrf_count[j] = 0;
    terrl_count[j] = 0;
    terro_count[j] = 0;
  }

  // terrain segments
  if (terrain_segments) {
    delete[] terrain_segments;
    terrain_segments = NULL;
  }

  // terrain props
  for (i = 0; i < DAT_SEGMENTS_COUNT; i++) {
    if (terrain_props[i]) delete[] terrain_props[i];
    terrain_props[i] = NULL;
  }
    
  // hashtable
  for (i = 0; i < DAT_SEGMENTS_COUNT; i++) {
    if (hashtable[i]) delete[] hashtable[i];
    hashtable[i] = NULL;
  }

  // materials
  if (materials) {
    for (i = 0; i < materials_count; i++) if (materials[i]) delete materials[i];
    delete[] materials;
    materials = NULL;
  }
}


//=========================================================================
// Scheme functions
//=========================================================================

int compare(const void *el1, const void *el2)
{
  return ((TTERRAIN_PROPS *) el1)->layer - ((TTERRAIN_PROPS *) el2)->layer;
}

/**
 *  Creates hash table for quick translations from user terrain ID to "smart"
 *  terrain id.
 */
bool CreateHashTable(int segment)
{
  int i;
  int maxlayer = scheme.terrain_props[segment][scheme.terrain_segments[segment].max_terrain_id-1].layer+1;  //max user def layer increase by 1

  if (!(scheme.hashtable[segment] = NEW T_SIMPLE[maxlayer])) return false;

  for (i = 0; i < maxlayer; i++) scheme.hashtable[segment][i] = EMPTY;

  for (i = 0; i < scheme.terrain_segments[segment].max_terrain_id; i++) 
    scheme.hashtable[segment][scheme.terrain_props[segment][i].layer] = i;

  return true;
}

/**
 *  Gets "smart" terrain ID from user terrain ID.
 */
char GetTerrainType(int user_terr_id, int to_where, int segment)
{
  int i, ret_val;
  int maxlayer = scheme.terrain_props[segment][(scheme.terrain_segments[segment].max_terrain_id)-1].layer;
  
  
  // if user terrain id is greater than max. layer or lower than min. layer (0)
  if ((user_terr_id > maxlayer) || (user_terr_id < 0)) {
    if (user_terr_id > maxlayer) ret_val = scheme.hashtable[segment][maxlayer]; // return max. smart layer
    else ret_val = 0; // return min. smart layer (0)

    return ret_val;
  }

  // if user defined layer exists, return smart value
  if (scheme.hashtable[segment][user_terr_id] != EMPTY) return scheme.hashtable[segment][user_terr_id];

  // user defined terrain id does not exists
  Warning(LogMsg("Invalid Terrain ID %d", user_terr_id));
    
  switch (to_where){
  case TO_HIGH: 
    for (i = user_terr_id; (i <= maxlayer) && (scheme.hashtable[segment][i]==EMPTY); i++);

    if (scheme.hashtable[segment][i] != EMPTY){
      ret_val = scheme.hashtable[segment][i];
      Warning(LogMsg("Terrain ID %d was replaced with value %d", ret_val));
      return ret_val;
    }

    for (i = user_terr_id; (i >= 0) && (scheme.hashtable[segment][i] == EMPTY); i--);
    if (scheme.hashtable[segment][i] != EMPTY){
      ret_val = scheme.hashtable[segment][i];
      Warning(LogMsg("Can not find Terrain ID greater than %d", user_terr_id));
      Warning(LogMsg("Terrain ID %d was replaced with value %d", ret_val));
      return ret_val;
    }

    // this shouldn't run
    Error(LogMsg("Hash table does not exists"));
    return 0;

  case TO_LOW:   
    for (i = user_terr_id; (i >= 0) && (scheme.hashtable[segment][i]==EMPTY); i--);
    if (scheme.hashtable[segment][i] != EMPTY){
      ret_val = scheme.hashtable[segment][i];
      Warning(LogMsg("Terrain ID %d was replaced with value %d", ret_val));
      return ret_val;
    }

    for (i = user_terr_id; (i <= maxlayer) && (scheme.hashtable[segment][i] == EMPTY); i++);
    if (scheme.hashtable[segment][i] != EMPTY){
      ret_val = scheme.hashtable[segment][i];
      Warning(LogMsg("Can not find Terrain ID smaller than %d", user_terr_id));
      Warning(LogMsg("Terrain ID %d was replaced with value %d", ret_val));
      return ret_val;
    }

    // this shouldn't run
    Error(LogMsg("Hash table does not exists"));
    return 0;

  case TO_MIDDLE:
    for(i = 0; ((user_terr_id+i <= maxlayer) && (user_terr_id-i >= 0) && (scheme.hashtable[segment][user_terr_id+i] == EMPTY) && (scheme.hashtable[segment][user_terr_id-i] == EMPTY)); i++)
    {
      if (user_terr_id+i <= maxlayer){
        for (i = user_terr_id+i; (i <= maxlayer) && (scheme.hashtable[segment][i] == EMPTY); i++);
        if (scheme.hashtable[segment][i] != EMPTY){
          ret_val = scheme.hashtable[segment][i];
          Warning(LogMsg("Terrain ID %d was replaced with value %d", ret_val));
          return ret_val;
        }

        // this shouldn't run
        Error(LogMsg("Hash table does not exists"));
        return 0;
      }
      if (user_terr_id-i >= 0){
        for (i = user_terr_id-i; (i >= 0) && (scheme.hashtable[segment][i]==EMPTY); i--);
        if (scheme.hashtable[segment][i] != EMPTY){
          ret_val = scheme.hashtable[segment][i];
          Warning(LogMsg("Terrain ID %d was replaced with value %d", ret_val));
          return ret_val;
        }

        // this shouldn't run
        Error(LogMsg("Hash table does not exists"));
        return 0;
      }
      
      if (scheme.hashtable[segment][user_terr_id+i] != EMPTY) return scheme.hashtable[segment][user_terr_id+i];
      if (scheme.hashtable[segment][user_terr_id-i] != EMPTY) return scheme.hashtable[segment][user_terr_id-i];
    }

  default: return 0;
  }
}


//=========================================================================
// Create Structures
//=========================================================================


/**
 *  Create table of materials and fill it with default values.
 *  @sa TFORCE_ITEM
 */
bool CreateMaterialsTable(void)
{
  if (!(scheme.materials = NEW TMATERIAL*[scheme.materials_count]))
  {
    SchCriticalTable(scheme, "materials");
    return false;
  }

  int i;

  for (i = 0; i < scheme.materials_count; i++) scheme.materials[i] = NULL;

  return true;
}


bool CreateTerrfTable(int sid)
{
  if (!(scheme.terrf[sid] = NEW TTERRF_ITEM[scheme.terrf_count[sid]])) return false;

  return true;
}


bool CreateTerrlTable(int sid)
{
  if (!(scheme.terrl[sid] = NEW TTERRL_ITEM[scheme.terrl_count[sid]])) return false;

  return true;
}


bool CreateTerroTable(int sid)
{
  if (!(scheme.terro[sid] = NEW TSURFACE_ITEM[scheme.terro_count[sid]])) return false;

  return true;
}


//=========================================================================
// Loading materials
//=========================================================================

/**
 *  Load material from file (one material with specified ID).
 *  @sa TCONF_FILE
 */
bool LoadSchMaterial(TCONF_FILE *cf, int id)
{
  bool ok = true;
  TFILE_LINE pom;

  sprintf(pom, "Material %d", id);
  ok = cf->SelectSection(pom, true);

  scheme.materials[id] = NEW TMATERIAL;

  if (!scheme.materials[id]) ok = false;

  if (ok){
    cf->ReadStr(scheme.materials[id]->id, const_cast<char*>("id"), pom, true);
    cf->ReadStr(scheme.materials[id]->name, const_cast<char*>("name"), scheme.materials[id]->id, true);
  }

  if (ok) ok = cf->ReadTextureGroup(&scheme.materials[id]->tg_id, const_cast<char*>("tg_id"), &scheme.tex_table, true, pom);
    
  cf->UnselectSection();

  return ok;
}

/**
 *  Load materials types from scheme conf. file.
 *  @sa TCONF_FILE
 */
bool LoadSchMaterials(TCONF_FILE *cf)
{
  bool ok = true;

  ok = cf->SelectSection(const_cast<char*>("Materials"), true);

  if (ok) ok = cf->ReadIntRange(&scheme.materials_count, const_cast<char*>("count"), 1, SCH_MAX_MATERIALS_COUNT, 1);
  if (ok) ok = CreateMaterialsTable();

  for (int i = 0; ok && i < scheme.materials_count; i++) ok = LoadSchMaterial(cf, i);

  cf->UnselectSection();
  return ok;
}

/**
 *  Returns index of item with user id "usr_id".
 *  If no instance of item has usr_id, return s -1 as error.
 *  @sa TCONF_FILE
 */
int GetMaterialPrgID(char * usr_id)
{
  int i;  

  for (i = 0; i < scheme.materials_count; i++)
    if (!(strcmp(usr_id, scheme.materials[i]->id))) return i;

    return -1;
};



//=========================================================================
// Loading fragments
//=========================================================================

/**
 *  Sets fragment terrain id.
 */
bool LoadSchFragmentTerrainId(TCONF_FILE *cf, int fid, int sid)
{
  TTERRF_ITEM *fragment = scheme.terrf[sid] + fid;

  int tid;
  int i, j;

  if (!fragment->terrain_field)
    if (!(fragment->terrain_field = CreateTerrainField(fragment->width, fragment->height))) return false;

  for (i = 0; i < fragment->width; i++)
    for (j = 0; j < fragment->height; j++) {
      if (cf->ReadIntGE(&tid, const_cast<char*>("terrain_id"), 0, 0)) fragment->terrain_field[i][j] = GetTerrainType(tid, TO_HIGH, sid);
      else return false;
    }

  return true;
}

/**
 *  Loads terrain fragment.
 */
bool LoadSchFragment(TCONF_FILE *cf, int fid, int sid)
{
  bool ok = true;
  TFILE_LINE pom;
  int ival;

  sprintf(pom, "Fragment %d", fid);
  ok = cf->SelectSection(pom, true);


  cf->ReadSimpleGE(&scheme.terrf[sid][fid].width, const_cast<char*>("size"), 1, 1);
  scheme.terrf[sid][fid].height = scheme.terrf[sid][fid].width;

  sprintf(pom, "'Segment %d' / 'Fragment %d'", sid, fid);
  if (ok) ok = cf->ReadTextureGroup(&ival, const_cast<char*>("tg_id"), &scheme.tex_table, false, pom);
  if (ok && ival >= 0) scheme.terrf[sid][fid].SetTextures(scheme.tex_table.groups + ival);
       
  if (ok) ok = LoadSchFragmentTerrainId(cf, fid, sid);

  cf->UnselectSection();

  return ok;
}

/**
 *  Loads terrain fragments table.
 */
bool LoadSchFragments(TCONF_FILE *cf, int sid)
{
  bool ok = true;
  int tid, i, ival;
  TFILE_LINE pom;

  ok = cf->SelectSection(const_cast<char*>("Fragments"), true);

  if (ok) ok = cf->ReadIntGE(&scheme.terrf_count[sid], const_cast<char*>("count"), 1, 1);
  scheme.terrf_count[sid] ++; // allocete one more "box" for default fragment
  if (ok) ok = CreateTerrfTable(sid);

  for (i = 0; ok && i < scheme.terrf_count[sid] - 1; i++) ok = LoadSchFragment(cf, i, sid);

  
  // set default fragmet to scheme table of fragments
  scheme.terrf[sid][scheme.terrf_count[sid] - 1].height = scheme.terrf[sid][scheme.terrf_count[sid] - 1].width = 1;
  
  sprintf(pom, "'Segment %d' / 'Fragments'", sid);
  if (ok) ok = cf->ReadTextureGroup(&ival, const_cast<char*>("tg_default_id"), &scheme.tex_table, false, pom);
  if (ok && ival >= 0) scheme.terrf[sid][scheme.terrf_count[sid] - 1].SetTextures(scheme.tex_table.groups);

  if (ok) scheme.terrf[sid][scheme.terrf_count[sid] - 1].terrain_field = CreateTerrainField(1, 1);

  if (ok) {
    cf->ReadIntGE(&tid, const_cast<char*>("default_terrain_id"), 0, 0);
    scheme.terrf[sid][scheme.terrf_count[sid] - 1].terrain_field[0][0] = GetTerrainType(tid, TO_HIGH, sid); //!< Default number of terr. id in segment. (used when no fragment is on mapel)
  }
  
  cf->UnselectSection();

  return ok;
}


//=========================================================================
// Loading layers
//=========================================================================

/**
 *  Sets Layer terrain id.
 */
bool LoadSchLayerTerrainId(TCONF_FILE *cf, int lid, int sid)
{
  TTERRL_ITEM *layer = scheme.terrl[sid] + lid;

  int tid;
  int i, j;

  if (!layer->terrain_field)
    if (!(layer->terrain_field = CreateTerrainField(layer->width, layer->height))) return false;

  for (i = 0; i < layer->width; i++)
    for (j = 0; j < layer->height; j++)
      if (cf->ReadIntGE(&tid, const_cast<char*>("terrain_id"), 0, 0)) layer->terrain_field[i][j]=GetTerrainType(tid, TO_HIGH, sid);
      else return false;

  return true;
}

/**
 *  Loads terrain Layer.
 */
bool LoadSchLayer(TCONF_FILE *cf, int lid, int sid)
{
  bool ok = true;
  TFILE_LINE pom;
  char strval[1024]; //buffer for id and name
  int ival;

  sprintf(pom, "Layer %d", lid);
  ok = cf->SelectSection(pom, true);
  
  sprintf(pom, "'Segment %d' / 'Layer %d'", sid, lid);
  if (ok) ok = cf->ReadTextureGroup(&ival, const_cast<char*>("tg_id"), &scheme.tex_table, true, pom);
  if (ok && ival >= 0) scheme.terrl[sid][lid].SetTextures(scheme.tex_table.groups + ival);

  if (ok) {
    cf->ReadSimpleGE(&scheme.terrl[sid][lid].width, const_cast<char*>("width"), 1, 1);
    cf->ReadSimpleGE(&scheme.terrl[sid][lid].height, const_cast<char*>("height"), 1, 1);

    cf->ReadStr(strval, const_cast<char*>("id"), pom, true);
    scheme.terrl[sid][lid].text_id = NEW char[strlen(strval) + 1];
    scheme.terrl[sid][lid].text_id = strcpy(scheme.terrl[sid][lid].text_id, strval);

    cf->ReadStr(strval, const_cast<char*>("name"), scheme.terrl[sid][lid].text_id, true);
    scheme.terrl[sid][lid].name = NEW char[strlen(strval) + 1];
    scheme.terrl[sid][lid].name = strcpy(scheme.terrl[sid][lid].name, strval);
  
    ok = LoadSchLayerTerrainId(cf, lid, sid);
  }

  cf->UnselectSection();

  return ok;
}

/**
 *  Loads terrain Layers table.
 */
bool LoadSchLayers(TCONF_FILE *cf, int sid)
{
  bool ok = true;

  ok = cf->SelectSection(const_cast<char*>("Layers"), true);

  if (ok) ok = cf->ReadIntGE(&scheme.terrl_count[sid], const_cast<char*>("count"), 0, 0);
  if (ok) ok = CreateTerrlTable(sid);

  for (int i = 0; ok && i < scheme.terrl_count[sid]; i++) ok = LoadSchLayer(cf, i, sid);

  cf->UnselectSection();

  return ok;
}


//=========================================================================
// Loading objects
//=========================================================================

/**
 *  Sets fragment terrain id.
 */
bool LoadSchObjectTerrainId(TCONF_FILE *cf, int oid, int sid)
{
  TSURFACE_ITEM * surface = scheme.terro[sid] + oid;

  int tid;
  int i, j;

  if (!surface->terrain_field)
    if (!(surface->terrain_field = CreateTerrainField(surface->GetWidth(), surface->GetHeight()))) return false;

  for (i = 0; i < surface->GetWidth(); i++)
    for (j = 0; j < surface->GetHeight(); j++) {
      if (cf->ReadIntGE(&tid, const_cast<char*>("terrain_id"), 0, 0)) surface->terrain_field[i][j] = GetTerrainType(tid, TO_HIGH, sid);
      else return false;
    }

  return true;
}

/**
 *  Loads terrain Object.
 */
bool LoadSchObject(TCONF_FILE *cf, int oid, int sid)
{
  bool ok = true;
  TFILE_LINE pom;
  char strval[1024]; //buffer for id and name
  int ival;

  sprintf(pom, "Object %d", oid);
  ok = cf->SelectSection(pom, true);
  
  sprintf(pom, "'Segment %d' / 'Object %d'", sid, oid);
  if (ok) ok = cf->ReadTextureGroup(&scheme.terro[sid][oid].tg_stay_id, const_cast<char*>("tg_id"), &scheme.tex_table, true, pom);
  
  if (ok) {
    cf->ReadIntGE(&ival, const_cast<char*>("width"), 1, 1);
    scheme.terro[sid][oid].SetWidth(ival);
    
    cf->ReadIntGE(&ival, const_cast<char*>("height"), 1, 1);
    scheme.terro[sid][oid].SetHeight(ival);
        
    cf->ReadStr(strval, const_cast<char*>("id"), pom, true);
    scheme.terro[sid][oid].text_id = NEW char[strlen(strval) + 1];
    scheme.terro[sid][oid].text_id = strcpy(scheme.terro[sid][oid].text_id, strval);

    cf->ReadStr(strval, const_cast<char*>("name"), scheme.terro[sid][oid].text_id, true);
    scheme.terro[sid][oid].name = NEW char[strlen(strval) + 1];
    scheme.terro[sid][oid].name = strcpy(scheme.terro[sid][oid].name, strval);
  
    ok = LoadSchObjectTerrainId(cf, oid, sid);
  }
  
  cf->UnselectSection();

  return ok;
}

/**
 *  Loads terrain Objects table.
 */
bool LoadSchObjects(TCONF_FILE *cf, int sid)
{
  bool ok = true;

  ok = cf->SelectSection(const_cast<char*>("Objects"), true);

  if (ok) ok = cf->ReadIntGE(&scheme.terro_count[sid], const_cast<char*>("count"), 0, 0);
  
  if (scheme.terro_count[sid] > 0) {
    if (ok) ok = CreateTerroTable(sid);
    for (int i = 0; ok && i < scheme.terro_count[sid]; i++) ok = LoadSchObject(cf, i, sid);
  }

  cf->UnselectSection();

  return ok;
}


//=========================================================================
// Loading Terrain Segments
//=========================================================================

/**
 *  Loads terrain type.
 */
bool LoadSchTerrainType(TCONF_FILE *cf, int tid, int segment)
{
  float fval;
  bool ok = true;
  TFILE_LINE pom;
  
  sprintf(pom, "Terrain type %d", tid);
  ok = cf->SelectSection(pom, true);

  if (ok) {
    cf->ReadStr(scheme.terrain_props[segment][tid].name, const_cast<char*>("name"), pom, true);
    cf->ReadIntGE(&scheme.terrain_props[segment][tid].layer, const_cast<char*>("layer"), 0, 0);
    cf->ReadFloatRange(&fval, const_cast<char*>("difficulty"), 0, 1, 0);
    scheme.terrain_props[segment][tid].difficulty = MIN (MAX (static_cast<unsigned int>(fval * 1000), 1), 999);
  }
 
  cf->UnselectSection();

  return ok;
}


bool LoadSchSegment(TCONF_FILE *cf, int sid)
{
  bool ok = true;
  int tid;
  TFILE_LINE pom;

  sprintf(pom, "Segment %d", sid);
  ok = cf->SelectSection(pom, true);

  if (ok) ok = cf->ReadIntGE(&scheme.terrain_segments[sid].max_terrain_id, const_cast<char*>("count"), 0, 0);
  if (!(scheme.terrain_props[sid] = NEW TTERRAIN_PROPS [scheme.terrain_segments[sid].max_terrain_id]) || !ok) ok = false;
  
  if (ok){
    cf->ReadStr(scheme.terrain_segments[sid].name, const_cast<char*>("name"), pom, true);
  
    for (tid = 0; (ok) && tid < scheme.terrain_segments[sid].max_terrain_id; tid++)
      ok = LoadSchTerrainType(cf, tid, sid);
  }
  
  if (ok) {
    scheme.terrain_segments[sid].layer = sid;
    qsort(scheme.terrain_props[sid], scheme.terrain_segments[sid].max_terrain_id, sizeof(TTERRAIN_PROPS),compare);
  }
  
  if (ok) ok = CreateHashTable(sid);
  if (ok) ok = LoadSchFragments(cf, sid);
  if (ok) ok = LoadSchLayers(cf, sid);
  if (ok) ok = LoadSchObjects(cf, sid);
  
  cf->UnselectSection();
  
  return ok;
}

/**
 *  Loads terrain segments table.
 */
bool LoadSchSegments(TCONF_FILE *cf)
{
  bool ok = true;
  
  ok = cf->SelectSection(const_cast<char*>("Segments"), true);

  if (!(scheme.terrain_segments = NEW TTERRAIN_SEGMENTS [DAT_SEGMENTS_COUNT]) || !ok) ok = false;
  
  for (int segment = 0; ok && segment < DAT_SEGMENTS_COUNT; segment++)
    if (ok) ok = LoadSchSegment(cf, segment);
  

  cf->UnselectSection();

  return ok;
}


//=========================================================================
// Schemes
//=========================================================================

/**
 *  Loads scheme data from file to scheme.
 */
bool LoadScheme(TSCHEME_NAME id_name)
{
  TFILE_NAME schname;
  
  TCONF_FILE *cf;

  bool ok = true;

  scheme.Clear();  // free loaded scheme

  sprintf(schname, "%s%s%s", SCH_PATH, id_name, ".dat");

  if (!scheme.tex_table.Load(schname, config.tex_mag_filter, config.tex_min_filter)) return false;

  sprintf(schname, "%s%s%s", SCH_PATH, id_name, ".sch");

  Info(LogMsg("Loading scheme from '%s'", schname));

  strcpy(scheme.id_name, id_name);

  if (!(cf = OpenConfFile(schname))) return false;

  if (ok) {
    cf->ReadStr(scheme.name, const_cast<char*>("name"), const_cast<char*>(""), true);
    cf->ReadStr(scheme.author, const_cast<char*>("author"), const_cast<char*>(""), true);
  }

  if (ok) ok = LoadSchMaterials(cf);
  if (ok) ok = LoadSchSegments(cf);
  if (ok) ok = LoadRace(id_name, true);
  
  CloseConfFile(cf);

  if (!ok) {
    scheme.Clear();
    Error(LogMsg("Error loading scheme from '%s'", schname));
  }

  return ok;
}


//=========================================================================
// END
//=========================================================================
// vim:ts=2:sw=2:et:

