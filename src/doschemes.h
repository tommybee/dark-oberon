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
 *  @file doschemes.h
 *
 *  Schemes declarations and methods.
 *
 *  @author Peter Knut
 *
 *  @date 2003
 */

#ifndef __doschemes_h_
#define __doschemes_h_

//=========================================================================
// Forward declarations
//=========================================================================

struct TTERRAIN_SEGMENTS;
struct TTERRAIN_PROPS;
class TTERR_ITEM;
class TTERRF_ITEM;
class TTERRL_ITEM;
class TMATERIAL;
struct TSCHEME;

//=========================================================================
// Macros
//=========================================================================

#define SCH_PATH    (app_path + DATA_DIR "schemes/").c_str()

#define SCH_MAX_NAME_LENGTH  30
#define SCH_SEGMENT_MAX_NAME_LENGTH  30
#define SCH_ID_MAX_NAME_LENGTH  30


#define TO_MIDDLE 0 //!< Macro used in GetTerrainType for changing input value to middle value
#define TO_HIGH   1 //!< Macro used in GetTerrainType for changing input value to the nearees higher value
#define TO_LOW    2 //!< Macro used in GetTerrainType for changing input value to the neares lower value


#define SCH_MAX_MATERIALS_COUNT     4     //!< Maximum count kinds of material.
#define SCH_MAX_MATERIAL_NAME       30    //!< Max. lenght of material name

#define SchCriticalTable(scheme, table)    Critical(LogMsg("Can not allocate memory for '%s' %s table", scheme.name, table))

//=========================================================================
// Included files
//=========================================================================

#include "cfg.h"
#include "doalloc.h"

#include "dodata.h"
#include "dolayout.h"


//=========================================================================
// Typedefs
//=========================================================================

typedef char TSCHEME_NAME[SCH_MAX_NAME_LENGTH];
typedef char TMATERIAL_NAME[SCH_MAX_MATERIAL_NAME];

struct TTERRAIN_SEGMENTS {
  char name[SCH_SEGMENT_MAX_NAME_LENGTH]; //!< User segment name.
  int layer;            //!< Height of segment.
  int max_terrain_id;   //!< Highest number of terrain ID in segment.
};


/**
 *  Terrain properties.
 */
struct TTERRAIN_PROPS {
  char name[SCH_ID_MAX_NAME_LENGTH];    //!< Segment name.
  int layer;                            //!< User defined high of terrain.
  unsigned int difficulty;              //!< Difficulty of walking on this terrain.
};


/**
 *  ???
 *
 *  @todo okomentovat triedu
 */
class TTERR_ITEM {
public:
  T_SIMPLE width;               //!< Width. [mapels]
  T_SIMPLE height;              //!< Height. [mapels]

  TTERRAIN_ID **terrain_field;  //!< Terrain id.

  void Draw(int anim_id) { if (animation && anim_id >= 0) animation[anim_id]->Draw(); }
  void UpdateGraphics(double time_shift) { 
    if (animation) for (int i = 0; i < anim_count; i++) animation[i]->Update(time_shift);
  }

  void SetTextures(TTEX_GROUP *group);
  void SetUsed(bool use) { used = use; }
  bool IsUsed() { return used; }
  int  GetAnimCount() { return anim_count; }
  void ClearAnimations() {
    if (animation) {
      for (int i = 0; i < anim_count; i++) if (animation[i]) delete animation[i];
      delete[] animation;
      animation = NULL;
      anim_count = 0;
    }
  }

  TTERR_ITEM() {
    width = height = 0;
    animation = NULL;
    terrain_field = NULL;
    used = false;
    anim_count = 0;
  }

  ~TTERR_ITEM() {
    ClearAnimations();
    DeleteTerrainField(terrain_field, width);
  }

protected:
  bool used;                     //!< If item is used by some fragment or layer.
  TGUI_ANIMATION **animation;    //!< Field of fragment animations. All same fragments in the map use the same animation.
  int anim_count;
};


class TTERRF_ITEM: public TTERR_ITEM {
public:
  TTERRF_ITEM():TTERR_ITEM() {};
};


class TTERRL_ITEM: public TTERR_ITEM {
public:
  TTERRL_ITEM():TTERR_ITEM() {text_id = NULL; name = NULL;};
  
  char * text_id;           //!< User text identifikator used in conf. files
  char * name;              //!< Text name of item.

  ~TTERRL_ITEM()     //!< Destructor.
  {
    if (text_id) delete[] text_id;
    if (name) delete[] name;
  };
};


/**
 *  Class containing material properties.
 */
class TMATERIAL {
public:
  TMATERIAL_NAME name;            //!< Name of the material.
  TMATERIAL_NAME id;              //!< Id of the material.
  int tg_id;                      //!< Texture identifier for material picture.

  TLIST<TWORKER_ITEM> mine_list;  //!< List of workers that can mine material.

  TMATERIAL();
};


struct TSCHEME {
  TSCHEME_NAME name;    //!< Scheme name.
  TSCHEME_NAME id_name; //!< Name of scheme file.

  TAUTHOR author;     //!< Author.

  TTEX_TABLE tex_table; //!< Set of terrain textures.

  TTERRF_ITEM *terrf[DAT_SEGMENTS_COUNT];   //!< Fragments table for each segment.
  TTERRL_ITEM *terrl[DAT_SEGMENTS_COUNT];   //!< Layers table for each segment.
  TSURFACE_ITEM *terro[DAT_SEGMENTS_COUNT]; //!< Objects table for each segment.

  /** Table of terrain ID for each segment. */
  TTERRAIN_PROPS (*terrain_props[DAT_SEGMENTS_COUNT]);
  /** Table of segments. */
  TTERRAIN_SEGMENTS *terrain_segments;

  /** Table of user terrain IDs. */
  T_SIMPLE *(hashtable[DAT_SEGMENTS_COUNT]);

  int terrf_count[DAT_SEGMENTS_COUNT];      //!< Fragments count in each segment.
  int terrl_count[DAT_SEGMENTS_COUNT];      //!< Layers count in each segment.
  int terro_count[DAT_SEGMENTS_COUNT];      //!< Objects count in each segment.

  TMATERIAL **materials;        //!< Materials table.
  int materials_count;          //!< Count of materials.
  
  TRACE *race;

  void UpdateGraphics(double time_shift);
  void Clear(void);

  TSCHEME() {
    int i;

    name[0] = '\0';
    id_name[0] = '\0';
    author[0] = '\0';
    
    for (i = 0; i < DAT_SEGMENTS_COUNT; i++){
      terrf[i] = NULL;
      terrl[i] = NULL;
      terro[i] = NULL;
    }
    materials = NULL; 
    materials_count = 0;
    race = NULL;
  };

  ~TSCHEME() {Clear();};

};

//=========================================================================
// Variables
//=========================================================================

extern TSCHEME scheme;  // actual scheme

//=========================================================================
// Functions
//=========================================================================

bool LoadScheme(TSCHEME_NAME idname);
char GetTerrainType(int user_terr_id, int to_where, int segment);
int GetMaterialPrgID(char * usr_id);

#endif  // __doschemes_h_

//=========================================================================
// END
//=========================================================================
// vim:ts=2:sw=2:et:

