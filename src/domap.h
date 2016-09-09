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
 *  @file domap.h
 *
 *  Map working declarations and methods.
 *
 *  @author Peter Knut
 *  @author Jiri Krejsa
 *
 *  @date 2002, 2003, 2004
 */

#ifndef __domap_h__
#define __domap_h__

//=========================================================================
// Forward declarations
//=========================================================================

class TTERR_BASIC;
class TTERR_FRAG;
class TTERR_LAYER;
struct TMAP_SURFACE;
struct TMAP_SEGMENT;
struct TWARFOG;
class TRADAR;
class TMAP;

//=========================================================================
// Definitions
//=========================================================================

#define MAP_PATH  (app_path + DATA_DIR "maps/").c_str()  //!< Directory containing maps.

#define MAP_MAX_SIZE          240   //!< Maximal map width or height.
#define MAP_AREA_SIZE         10    //!< Map area size.
#define MAP_MAX_NAME_LENGTH   30    //!< Maximal length of map name.

#define MAP_MAX_ZOOM          5.0f  //!< Maximal zoom coeficient.
#define MAP_MIN_ZOOM          0.2f  //!< Minimal zoom coeficient.
#define MAP_MAX_TERRAIN_DIFF  999   //!< Highest possible terrain difficulty.

// map zooming
#define MAP_ZOOM_RESET  0           //!< Resets map zoomimg.
#define MAP_ZOOM_IN     1           //!< Zooms map in - one step.
#define MAP_ZOOM_OUT    -1          //!< Zooms map out - one step.

// map moving
#define MAP_MOVE_NONE         0x00000000   //!< Does not move map.

#define MAP_KEY_MOVE_LEFT     0x00000001   //!< Moves map left with keyboard.
#define MAP_KEY_MOVE_RIGHT    0x00000010   //!< Moves map right with keyboard.
#define MAP_KEY_MOVE_UP       0x00000100   //!< Moves map up with keyboard.
#define MAP_KEY_MOVE_DOWN     0x00001000   //!< Moves map down with keyboard.
#define MAP_KEY_MOVE          0x00001111   //!< Map moves with keyboard.

#define MAP_MOUSE_MOVE_LEFT   0x00010000   //!< Moves map left with mouse.
#define MAP_MOUSE_MOVE_RIGHT  0x00100000   //!< Moves map right with mouse.
#define MAP_MOUSE_MOVE_UP     0x01000000   //!< Moves map up with mouse.
#define MAP_MOUSE_MOVE_DOWN   0x10000000   //!< Moves map down with mouse.
#define MAP_MOUSE_MOVE        0x11110000   //!< Map moves with mouse.

#define MAP_MOVE_LEFT         0x00010001   //!< Map moves left.
#define MAP_MOVE_RIGHT        0x00100010   //!< Map moves right.
#define MAP_MOVE_UP           0x01000100   //!< Map moves up.
#define MAP_MOVE_DOWN         0x10001000   //!< Map moves down.

#define MAP_MOVE_COEF         10           //!< Coeficiet to regulate speed of map moving.

// map terrains
#define MAP_EMPTY_SURFACE  255    //!< Special value in map surface

// activity of actions
#define ACTIV_ATTACK 3                    //!< Activity koeficient of attack
#define ACTIV_MINE 3                      //!< Activity koeficient of mine
#define ACTIV_REPAIR 2                    //!< Activity koeficient of repair
#define ACTIV_MOVE 2                      //!< Activity koeficient of move
#define ACTIV_DEFAULT 1                   //!< Default activity koeficient

//=========================================================================
// Included files
//=========================================================================

#include "cfg.h"
#include "doalloc.h"

#include "dounits.h"
#include "doconfig.h"

//=========================================================================
// Typedefs
//=========================================================================

typedef char TMAP_NAME[MAP_MAX_NAME_LENGTH];              //!< Used for map name.
typedef char TMAP_FILENAME[DAT_MAX_FILENAME_LENGTH];      //!< Used for maximal length fo map filename.


//=========================================================================
// Fragments, Layers, Objects
//=========================================================================

/**
 *  Common terrain object.
 */
class TTERR_BASIC {
public:
  TPOSITION_3D pos;       //!< Position.
  TTERR_ITEM *pitem;      //!< Pointer to specific item type.

  void Draw(void);
  void UpdateGraphics(void);

  bool IsInActiveArea() { return in_active_area; }

  TTERR_BASIC(int tx, int ty, int tz);

protected:
  bool in_active_area;    //!< If terrain is active map area.
  int  anim_id;           //!< Random id of animation.
};


/**
 *  Terrain fragment.
 */
class TTERR_FRAG: public TTERR_BASIC {
public:
  TTERR_FRAG(int tid, int tx, int ty, int tz);
};

typedef TTERR_FRAG *PTERR_FRAG;   //!< Pointer to terrain fragment.


/**
 *  Terrain layer.
 */
class TTERR_LAYER: public TTERR_BASIC {
public:
  TTERR_LAYER(int tid, int tx, int ty, int tz);
};



//=========================================================================
// Map segments
//=========================================================================


/**
 *  Map surface.
 */
struct TMAP_SURFACE {
  TTERRAIN_ID t_id;     //!< Terrain id.
  TMAP_UNIT *unit;      //!< Unit that stays here.
  TMAP_UNIT *ghost;     //!< Ghost that stays here.
  TNEURON_VALUE *activity;     //<! Activity of every player.

  TMAP_SURFACE();        //!< Basic constructor.
  ~TMAP_SURFACE();

  void GetActivity(const T_SIMPLE PlayerID, TNEURON_VALUE *my_activity, TNEURON_VALUE *enemy_activity);
  void DecreaseActivity(T_SIMPLE factor);
  void IncreaseActivity(const T_SIMPLE PlayerID, TNEURON_VALUE added) {activity[PlayerID] += added;};

  void Clear() {
    if (activity) delete [] activity;
    activity = NULL;
  }

  /** @return The method returns the list of units which watch this field.*/
  TMAP_POOLED_LIST* GetWatchersList() const
    { return watchers;}
  /** @return The method returns the list of units which could aim this field.*/
  TMAP_POOLED_LIST* GetAimersList() const
    { return aimers;}
  
private:
  TMAP_POOLED_LIST *aimers;
  TMAP_POOLED_LIST *watchers;
};

typedef TMAP_SURFACE *PMAP_SURFACE;   //!< Pointer to map surface.


/**
 *  Map segment.
 */
struct TMAP_SEGMENT {
  int terrf_count;              //!< Count of terrain fragments.
  int terro_count;              //!< Count of terrain objects.

  TTERR_FRAG  **terrf;          //!< Filled of pointers to terrain fragments.
  TDRAW_UNIT  **terro;          //!< Filled of pointers to terrain objects.
  TLIST<TTERR_LAYER> terrl;     //!< List of terrain layers.
  
  TMAP_SURFACE **surface;       //!< Characteristics of map segment surface.

  GLenum tex_radar_id;          //!< Identifiers for radar textures.

  //! Arithmetic average of the difficulty of surface in the segment.
  double average_surface_difficulty;

  void Clear(void);
  void Draw(void);
  void DrawSurface();
  void UpdateGraphics(double time_shift);

  void UpdateTerrainId(int x, int y, int width, int height, TTERRAIN_FIELD field);
  bool AddLayer(int lid, int lx, int ly, int lz);

  double CalculateAverageDifficulty(T_SIMPLE width, T_SIMPLE height);

  bool LoadMapSegment();
  bool LoadMapFragments(T_BYTE sid);          //!< Load terrain fragments.
  bool LoadMapFragment(T_BYTE sid, int id);   //!< Set map fragment
  bool LoadMapLayers(T_BYTE sid);             //!< Load terrain layers.
  bool LoadMapLayer(T_BYTE sid, int id);      //!< Set player
  bool LoadMapObjects(T_BYTE sid);            //!< Load terrain objects.
  bool LoadMapObject(T_BYTE sid, int id);     //!< Set map object

  TMAP_SEGMENT();
  ~TMAP_SEGMENT();
};


class TSEG_UNITS {
private:
  T_BYTE id;
  TDRAW_UNIT *units;          //!< Circle list with head of all map units, that stays on the segment.
  int units_count;            //!< Count of units that stays on the segment.
  
#ifdef NEW_GLFW3
	mtx_t mutex;
#else
	GLFWmutex mutex;            //!< Units mutex.
#endif
  

public:
  void Clear();

  void AddUnit(TDRAW_UNIT *unit);
  void DeleteUnit(TDRAW_UNIT *unit);
  void SortUnits();

  void Draw(T_BYTE style = DS_NORMAL);
  void DrawToRadar();

  TSEG_UNITS(T_BYTE seg_id);
  ~TSEG_UNITS();
};


/**
 *  Warfog structure.
 */
struct TWARFOG {
  GLenum tex_id;
  GLenum radar_tex_id;

  GLubyte *(tex[DAT_SEGMENTS_COUNT + 1]);

  GLfloat x1_coord;             //!< Coordinates in texture (x1 component).
  GLfloat y1_coord;             //!< Coordinates in texture (y1 component).
  GLfloat x2_coord;             //!< Coordinates in texture (x2 component).
  GLfloat y2_coord;             //!< Coordinates in texture (y2 component).
  GLfloat x3_coord;             //!< Coordinates in texture (x2 component) used for radar drawing.
  GLfloat y3_coord;             //!< Coordinates in texture (y2 component) used for radar drawing.

  bool Create(void);
  void Clear(void);
  void Update(void);
  void Draw(void);
  void DrawToRadar(void);

  TWARFOG() {
    for (int i = 0; i <= DAT_SEGMENTS_COUNT; i++) {
      tex[i] = NULL;
    }
    tex_id = radar_tex_id = 0;
    x1_coord = y1_coord = x2_coord = y2_coord = x3_coord = y3_coord = 0;
  }
  ~TWARFOG() { Clear(); }
};


/**
 *  Radar structure.
 */
class TRADAR {
public:
  GLfloat dx;      //!< DeltaX for drawing.
  GLfloat zoom;    //!< Zoom constant for drawing.

  void Draw(void);
  void ToggleHideable() { hideable = !hideable; };

  bool IsHideable() { return hideable; };
  bool GetMoving() { return moving; };
  void SetMoving(bool mov) { moving = mov; };

  TRADAR(void) { dx = zoom = 0; moving = false; hideable = true; };

private:
  bool moving;      //!< True if map is moving by clicking to radar.
  bool hideable;    //!< Radar is hideable together with panel.
};


//=========================================================================
// Map
//=========================================================================

class TMAP_AREA {
public:
  TMAP_AREA() { x1 = x2 = y1 = y2 = 0; changed = true; }

  bool IsInArea(T_SIMPLE x, T_SIMPLE y)
    { return (x >= x1 && x <= x2 && y >= y1 && y <= y2); }
  bool IsInArea(T_SIMPLE x, T_SIMPLE y, T_SIMPLE width, T_SIMPLE height)
    { return (x + width > x1 && x <= x2 && y + height > y1 && y <= y2); }
  void SetArea(T_SIMPLE sx1, T_SIMPLE sx2, T_SIMPLE sy1, T_SIMPLE sy2)
  {
    if (x1 != sx1 || x2 != sx2 || y1 != sy1 || y2 != sy2) {
      x1 = sx1; x2 = sx2; y1 = sy1; y2 = sy2;
      changed = true;
    }
    else changed = false;
  }

  bool IsChanged() { return changed; }

  T_SIMPLE GetX() { return x1; }
  T_SIMPLE GetY() { return y1; }
  T_SIMPLE GetX2() { return x2; }
  T_SIMPLE GetY2() { return y2; }
  T_SIMPLE GetWidth() { return x2 - x1; }
  T_SIMPLE GetHeight() { return y2 - y1; }

private:
  T_SIMPLE x1, x2;
  T_SIMPLE y1, y2;

  bool changed;
};


/**
 *  Map.
 */
class TMAP {
public:
  TMAP_NAME name;               //!< Map name.
  TMAP_NAME id_name;            //!< Map idname.
  TAUTHOR author;               //!< Author.
  TSCHEME_NAME scheme_name;     //!< Scheme name.

  T_SIMPLE width;               //!< Map width.
  T_SIMPLE height;              //!< Map height.

  double start_time;            //!< Time when playing was started. [seconds]

  GLfloat dx;                   //!< Map position shift (x coordinate).
  GLfloat dy;                   //!< Map position shift (y coordinate).
  bool mouse_moving;            //!< If map is currently moving with a mouse and ALT key.
  bool drag_moving;             //!< If map is currently moving with a mouse by drag&drop method.
  unsigned move_flag;           //!< Flag that indicates direction of moving with keyboard or mouse.
  
  TMAP_SEGMENT segments[DAT_SEGMENTS_COUNT];            //!< Map segments.
  TSEG_UNITS   *segment_units[DAT_SEGMENTS_COUNT];      //!< Units in segments.

  TWARFOG war_fog;              //!< Warfog structure.
  TCONF_FILE *file;             //!< Configuration file handler.

  TMAP_AREA active_area;        //!< Envelope for active area, that is visible on the screen.

  void UpdateMoving(double time_shift);
  void UpdateGraphics(double time_shift);
  void UpdateActiveArea();

  void Draw();
  void DrawBorder();
  void DrawToRadar();
  void Zoom(int dzoom);
  void Clear();

  float GetZoom() { return zoom; }
  
  void StartKeyMove(int mflag);
  void StopKeyMove(int mflag);
  void MouseMove(int mflag);
  void Move(GLfloat mdx, GLfloat mdy);
  void CenterMapel(T_SIMPLE x, T_SIMPLE y);
  void SetPosition(GLfloat ndx, GLfloat ndy);

  /** Tests whether position is in borders of map. Does not test segment coordinate. */
  bool IsInMap(const T_SIMPLE x, const T_SIMPLE y)
    { return ((x < width) && (y < height)); }
  //!< Tests whether position is in borders of map.
  bool IsInMap(const TPOSITION_3D &pos)
    { return ((pos.x < width) && (pos.y < height) && (pos.segment < DAT_SEGMENTS_COUNT)); }
  //!< Tests whether position is in borders of map.
  bool IsInMap(const T_SIMPLE x, const T_SIMPLE y, const T_SIMPLE segment)
    { return ((x < width) && (y < height) && (segment < DAT_SEGMENTS_COUNT)); }
  //!< Tests whether position is in borders of map.
  bool IsInMap(const double x, const double y)
    { return ((x >= 0) && (x < width) && (y >= 0) && (y < height)); }
  //!< Tests whether position is in borders of map.
  bool IsInMap(const int x, const int y)
    { return ((x >= 0) && (x < width) && (y >= 0) && (y < height)); }

  //!< The method tests whether position is free.
  bool IsPositionFree(T_SIMPLE width, T_SIMPLE height, const TPOSITION_3D &pos);
  
  //! Returns seconds how long has been the map played.
  double GetPlayTime (double time_actual) { return (time_actual - start_time); }

  TMAP();
  ~TMAP() { Clear(); };

  bool LoadMap(char *name);     //!< Load map from file
  void DeleteMap();             //!< Deletes map and depend structures.

  /** @return The method returns pool with preallocated nodes for the aimers 
   *  lists.*/
  void* GetAimersPool() const
    { return aimers_pool;}
  /** @return The method returns pool with preallocated nodes for the watchers
   *  lists.*/
  void* GetWatchersPool() const
    { return watchers_pool;}

  void InitPools(void);

  bool LoadMapUnit(int pid, int id);          //!< set map unit
  bool LoadMapUnits(int pid);                 //!< load units
  bool LoadMapBuilding(int pid, int id);      //!< set map buidlding
  bool LoadMapBuildings(int pid);             //!< load units
  bool LoadMapSource(int pid, int id);        //!< set map buidlding
  bool LoadMapSources(int pid);               //!< load sources
  bool LoadMapPlayer(int pid);                //!< set player
  bool LoadMapPlayers();                      //!< load players
  void RenderRadarTextures();

private:
  void Initialise();

  float zoom;                   //!< Map zoom.
  int zoom_flag;                //!< Flag that indicates direction of zooming.

  /** Pool of preallocated objects for aimers lists in the map. */
  void *aimers_pool;
  /** Pool of preallocated objects for watchers lists in the map. */
  void *watchers_pool;
};

//=========================================================================
// Variables
//=========================================================================

extern TMAP map;
extern TRADAR radar;


//=========================================================================
// Macros
//=========================================================================

#define RadarPosition(x, y) \
do { \
  glTranslated(radar.zoom * ((x) - (y)), \
               radar.zoom * ((x) + (y)), \
               0); \
} while (0)


#define MapPosition(x, y) \
do { \
  glTranslated(DAT_MAPEL_STRAIGHT_SIZE_2 * (x) - DAT_MAPEL_STRAIGHT_SIZE_2 * (y), \
               DAT_MAPEL_DIAGONAL_SIZE_2 * (x) + DAT_MAPEL_DIAGONAL_SIZE_2 * (y), \
               0); \
} while (0)


#define SetMapPosition(x, y) \
do { \
  glLoadIdentity(); \
  MapPosition((x), (y)); \
} while (0)


//=========================================================================
// Functions
//=========================================================================

TMAP_SURFACE **CreateMapSurface();
void DeleteMapSurface(TMAP_SURFACE **surface);


#endif  // __domap_h__

//=========================================================================
// END
//=========================================================================
// vim:ts=2:sw=2:et:

