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
 *  @file dodata.h
 *
 *  Resources declarations and methods.
 *
 *  @author Peter Knut
 *
 *  @date 2002, 2003, 2004
 */

#ifndef __dodata_h__
#define __dodata_h__


//=========================================================================
// Forvard declarations
//=========================================================================

struct TTEX_GROUP;
struct TTEX_TABLE;

struct TSND_TABLE;


//=========================================================================
// Definitions
//=========================================================================

// textures
/** Maximal count of textures. */
#define TEX_MAX_COUNT       50

/** Directory, where data file are located. */
#define DAT_PATH            (app_path + DATA_DIR "dat/").c_str()
/** Filename of common data in #DAT_PATH. */ 
#define DAT_COMMON_NAME     (std::string(DAT_PATH) + "common.dat").c_str()
/** Filename of fonts data in #DAT_PATH. */
#define DAT_FONTS_NAME      (std::string(DAT_PATH) + "fonts.dat").c_str()
/** Filename of mouse cursors data in #DAT_PATH. */
#define DAT_CURSORS_NAME    (std::string(DAT_PATH) + "cursors.dat").c_str()
/** Filename of gui data in #DAT_PATH. */
#define DAT_GUI_NAME        (std::string(DAT_PATH) + "gui.dat").c_str()

#define DAT_SEGMENTS_COUNT  3           //!< Number of segments.

#define DAT_MAPEL_STRAIGHT_SIZE   32.0f                           //!< Straight size of mapel. [pixels]
#define DAT_MAPEL_STRAIGHT_SIZE_2 (DAT_MAPEL_STRAIGHT_SIZE / 2)   //!< Half straight size of mapel. [pixels]
#define DAT_MAPEL_DIAGONAL_SIZE   16.0f                           //!< Diagonal size of mapel. [pixels]
#define DAT_MAPEL_DIAGONAL_SIZE_2 (DAT_MAPEL_DIAGONAL_SIZE / 2)   //!< Hafl diagonal size of mapel. [pixels]

#define DAT_MAX_TEX_MAPELS       (int)(256.0 / DAT_MAPEL_DIAGONAL_SIZE) //!< Count of diagonal mapels in maximal texture height.
#define DAT_MAX_COLOR_TESTED     0.9f   //!< Maximal color value for testing.

#define DAT_MAX_DISTANCE  100000

#define DAT_MAX_FILENAME_LENGTH 256   //!< Maximal length of filename.

/** File header of data file. Every data file must start with this. */
#define DAT_FILE_HEADER     "Dark Oberon data file"
#define DAT_MAX_VERSION     3       //!< Max. allowed file version.
#define DAT_MIN_VERSION     2       //!< Min. allowed file version.

// texture
#define DAT_TEX_NAME_LENGTH 30      //!< Length of texture name in data file.
#define DAT_TEX_RANDOM      -1

// IDs of integrated textures and sounds
#define DAT_SID_MENU_MUSIC      0
#define DAT_SID_GAME_MUSIC      2
#define DAT_SID_MENU_BUTTON     1
#define DAT_SID_MENU_CONTROL    3

#define DAT_TGID_MOUSE_CURSORS  0
#define DAT_TGID_BASIC_FONT     0

#define DAT_TGID_PANELS         0
#define DAT_TGID_ACTION_BUTTONS 1
#define DAT_TGID_MENU_BUTTONS   2
#define DAT_TGID_SEG_BUTTONS    3
#define DAT_TGID_LABELS         4
#define DAT_TGID_GUARD_BUTTONS  5


//=========================================================================
// Included files
//=========================================================================

#include <stdlib.h>

#include "cfg.h"
#include "doalloc.h"

#include "dosimpletypes.h"
#include "dosound.h"
#include "glgui.h"


//=========================================================================
// Typedefs
//=========================================================================

/** Author of races, schemes. */
typedef char TAUTHOR[20];
typedef char TAUTHOR[20];
/** Version of races, schemes. */
typedef char TVERSION[10];


//=========================================================================
// Textures table
//=========================================================================

/**
 *  Group of textures. It is used in textures table.
 */
struct TTEX_GROUP {
  char *name;               //!< Group name.
  TGUI_TEXTURE *textures;   //!< Pointer to table of textures.

  int count;                //!< Count of the textures in the group.

  /** Constructor. */
  TTEX_GROUP(void) { name = NULL; textures = NULL; count = 0; };
  /** Destructor */
  ~TTEX_GROUP(void) { if (name) delete[] name; if (textures) delete[] textures; };
};


/**
 *  Table of textures groups.
 */
struct TTEX_TABLE {
  TTEX_GROUP *groups;       //!< Pointer to table of textures groups.

  int count;                //!< Count of the groups in the table.

  bool Load(const char *file_name, int mag_filter, int min_filter);
  void Clear(void) { if (groups) delete[] groups; groups = NULL; count = 0; };

  TGUI_TEXTURE *GetTexture(int group_id, int tex_id) { 
    if (tex_id == DAT_TEX_RANDOM) tex_id = GetRandomInt(groups[group_id].count);
    if (group_id < 0 || group_id >= count || tex_id < 0 || tex_id >= groups[group_id].count) return NULL;
    else return &groups[group_id].textures[tex_id];
  };

  /** Constructor. */
  TTEX_TABLE(void) { groups = NULL; count = 0; };
  /** Destructor */
  ~TTEX_TABLE(void) { Clear(); };
};


//=========================================================================
// Sounds table
//=========================================================================

#if SOUND

/**
 *  Group of pointers to sounds. It is used in unit items to store various sounds
 *  for one action. Group plays one random sound from its list.
 */
class TSND_GROUP {
public:
  TSOUND **sounds;      //!< Table with pointers to sounds.
  T_BYTE count;         //!< Count of sounds in group.
  double repeat_limit;  //!< Limit for playing double sounds.

  void Play() {
    if (!count)
      return;
    if (repeat_limit > 0 && glfwGetTime() < last_time + repeat_limit)
      return;

    last_time = glfwGetTime();

    if (count == 1)
      last_id = 0;
    else if (count > 1)
      last_id = GetRandomInt(count);

    sounds[last_id]->Play();
  }
  void Stop() {
    if (last_id >= 0)
      sounds[last_id]->Stop();
  }
  bool IsPlaying() {
    return last_id >= 0 ? sounds[last_id]->IsPlaying() : false;
  }

  bool IsEmpty() { return (count == 0); }
  void Clear() { if (sounds) delete[] sounds; sounds = NULL; count = 0; last_id = -1; }

  TSND_GROUP() { sounds = NULL; count = 0; last_id = -1; repeat_limit = 0.0; last_time = 0.0; }
  ~TSND_GROUP() { Clear(); }

private:
  int last_id;          //!< Identifier of last played sound.
  double last_time;     //!< Time of last played sound.
};


/**
 *  Table of sounds.
 **/
struct TSND_TABLE {
  TSOUND **sounds;     //!< Table of sounds pointers.
  int count;           //!< Count of sounds in the table.

  bool Load(const char *fname);
  void Clear(void);

  TSOUND * GetSound(char * id); //!< Finds sound with id in TSOUND table (if not exists, return null)

  /** Constructor. */
  TSND_TABLE(void) { sounds = NULL; count = 0; };
  /** Destructor */
  ~TSND_TABLE(void) { Clear(); };
};
#endif


//=========================================================================
// Variables
//=========================================================================

extern TTEX_TABLE fonts_table;
extern TTEX_TABLE gui_table;
#if SOUND
extern TSND_TABLE sounds_table;
#endif

extern GLFfont  *font0;


//=========================================================================
// Functions
//=========================================================================

bool LoadData(void);
void DeleteData(void);

bool CreateFonts(void);
void DestroyFonts(void);

#endif  // __dodata_h__

//=========================================================================
// END
//=========================================================================
// vim:ts=2:sw=2:et:

