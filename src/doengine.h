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
 *  @file doengine.h
 *
 *  Menu and game engine + creating GUI and callback functions.
 *
 *  @author Peter Knut
 *
 *  @date 2004
 */


#ifndef __doengine_h__
#define __doengine_h__


//=========================================================================
// Forward declarations
//=========================================================================

// menu map lists
class TMAP_RAC_INFO_NODE;
class TMAP_BASIC_INFO_NODE;
class TMAP_BASIC_INFO_NODE;
class TMAP_INFO_LIST;


//========================================================================
// Enumerations
//========================================================================

/**
 *  Type, which represents the actual state of the game.
 */
enum TGAME_STATE {
  /** The game should quit as soon as the control is returned to the main
   *  cycle. */
  ST_QUIT,

  /** The game is in main menu. */
  ST_MAIN_MENU,

  /** The game is in video menu. */
  ST_VIDEO_MENU,

  /** The game is in play menu. */
  ST_PLAY_MENU,

  /** Used while changing video options. */
  ST_RESET_VIDEO_MENU,

  /** The game is in the state of playing. */
  ST_GAME
};


/**
 *  Type, which represents the actual error.
 */
enum TGAME_ERROR {
  ERR_NONE,
  ERR_LOAD_MAP
};


//========================================================================
// Includes
//========================================================================

#include "cfg.h"
#include "doalloc.h"

#include <string>

#include "dosimpletypes.h"
#include "domap.h"
#include "dohost.h"

//=========================================================================
// Defines
//=========================================================================


// return values from functions working with menu players informations.
#define MN_PL_OK 0
#define MN_PL_ERROR 1
#define MN_PL_FULL 2
#define MN_PL_NOPLAYER 3

// actions panels
#define MNU_PANELS_COUNT       7
#define MNU_PANEL_EMPTY        0
#define MNU_PANEL_STAY         1
#define MNU_PANEL_MOVE         2
#define MNU_PANEL_ATTAK        3
#define MNU_PANEL_MINE         4
#define MNU_PANEL_REPAIR       5
#define MNU_PANEL_BUILD        6


//========================================================================
// Variables
//========================================================================

/**
 *  Game state. It is initialized to #ST_MENU - the game should start in menu.
 */
extern TGAME_STATE state;

/**
 *  Game error. It is initialized to #ERR_NONE.
 */
extern TGAME_ERROR error;
extern TMAP_INFO_LIST map_info_list;

//=========================================================================
// Structures of map list used in menu
//=========================================================================

/**
 *  Basic information about races in map (used in program menu).
 */
class TMAP_RAC_INFO_NODE {
public:
  
  TRAC_NAME name;              //!< Race full name.
  TRAC_FILENAME id_name;       //!< Race filename (id_name).

  TMAP_RAC_INFO_NODE * next;   //!< Pointer to next node.

  TMAP_RAC_INFO_NODE(void) { *name = *id_name = 0;};  //!< Constructor.
};

/**
 *  Basic information about map (used in program menu).
 */
class TMAP_BASIC_INFO_NODE {
public:

  TMAP_NAME name;               //!< Map name.
  TMAP_FILENAME id_name;        //!< Map filename.

  TMAP_BASIC_INFO_NODE * next;  //!< Pointer to next node.

  TMAP_BASIC_INFO_NODE (void) { *name = *id_name = 0; };  //!< Constructor.
};

/**
 *  Extended information about map (used in program menu).
 */
class TMAP_EXT_INFO_NODE {
public:

  TAUTHOR author;               //!< Author.
  TSCHEME_NAME scheme_name;     //!< Full scheme name.
  TSCHEME_NAME scheme_id_name;  //!< Scheme id name.


  T_SIMPLE width;               //!< Map width.
  T_SIMPLE height;              //!< Map height.

  int max_players;              //!< Maximum players count in map NOT including hyper player.
  

  TMAP_EXT_INFO_NODE (void) { *author = *scheme_name = *scheme_id_name = 0; width = 0; height = 0; max_players = 0;};
};

/**
 *  List of maps and functions for working with list (used in program menu).
 */
class TMAP_INFO_LIST {
public: 
  class NotFoundException { };

  TMAP_BASIC_INFO_NODE * map_list;      //!< List of maps.
  TMAP_EXT_INFO_NODE map_ext_info;      //!< Extended info about one map.
  TMAP_RAC_INFO_NODE * rac_list;    //!< List of races information in map.

  // Fills list of maps from directory MAP_PATH.
  bool LoadMapList ();
  
  // Loads Info. If basic is true load basic info, else load ext. map info of map with given name.
  bool LoadMapInfo(bool basic, const char *file_name); 

  // Delete list of maps information.
  void ClearMapList();

  // Delete list of races information.
  void ClearRacList();

  // Get filename of map at random.
  char * GetRandomMap (void);

  void SortMapList();
  void SortRacList();

  std::string GetRacIdName (std::string name);
  std::string GetRacName (std::string id_name);
#ifdef __GNUC__
	std::string GetMapName (std::string id_name);
#else
  	std::string TMAP_INFO_LIST::GetMapName (std::string id_name);
#endif
  

  TMAP_INFO_LIST (void) { map_list = NULL; rac_list = NULL;};
  ~TMAP_INFO_LIST (void) { ClearMapList(); ClearRacList();};
};


//========================================================================
// Panel info
//========================================================================

/**
 *  Group of variables for drawing unit and player info on main panel.
 */
struct TPANEL_INFO {
public:
  TGUI_LABEL  *material_label[SCH_MAX_MATERIALS_COUNT];   //!< Pinter to labels contains materials amounts.
  TGUI_LABEL  *material_image[SCH_MAX_MATERIALS_COUNT];
  TGUI_LABEL  *food_label;
  TGUI_LABEL  *energy_label;

  TGUI_LABEL  *picture_image;

  TGUI_LABEL  *name_label;      //!< Pinter to label contains unit name.
  TGUI_LABEL  *life_label;      //!< Pinter to label contains unit life / max. life.
  TGUI_LABEL  *hided_label;     //!< Pinter to label contains hided units / max. count of hided units.
  TGUI_LABEL  *info_label;
  TGUI_LABEL  *progress_label;  //!< Pinter to label contains progress of action.

  TGUI_BUTTON *stay_button;
  TGUI_BUTTON *move_button;
  TGUI_BUTTON *attack_button;
  TGUI_BUTTON *mine_button;
  TGUI_BUTTON *repair_button;
  TGUI_BUTTON *build_button;

  TGUI_BUTTON *seg_button;

  TGUI_BUTTON *guard_button[RAC_AGGRESIVITY_MODE_COUNT];
  TGUI_BUTTON *act_guard_button;

  TGUI_SCROLL_BOX *action_panel[MNU_PANELS_COUNT];
  TGUI_SCROLL_BOX *act_action_panel;
  TGUI_BUTTON     *act_build_button;
  TGUI_SCROLL_BOX *order_panel;

  void Clear() {
    int i;
    for (i = 0; i < SCH_MAX_MATERIALS_COUNT; i++) {
      material_image[i] = material_label[i] = NULL;
    }
    food_label = energy_label = picture_image = NULL;
    name_label = life_label = hided_label = info_label = progress_label = NULL;
    stay_button = move_button = attack_button = mine_button = repair_button = build_button = NULL;
    seg_button = NULL;
    for (i = 0; i < RAC_AGGRESIVITY_MODE_COUNT; i++)
      guard_button[i] = NULL;
    act_guard_button = NULL;
    for (i = 0; i < MNU_PANELS_COUNT; i++)
      action_panel[i] = NULL;
    act_build_button = NULL;
    order_panel = NULL;
  }
#ifdef __GNUC__
	TPANEL_INFO() { Clear(); }
#else
  	TPANEL_INFO::TPANEL_INFO() { Clear(); }
#endif
  
};

extern TPANEL_INFO panel_info;

extern TGUI_PANEL *radar_panel;


//========================================================================
// TSAFE_BOOL_SWITCH
//========================================================================

/**
 *  A safe boolean switch with a mutex. Main methods on this switch are atomic
 *  operations SetTrue() and IsTrue().
 */
class TSAFE_BOOL_SWITCH {
public:
  /**
   *  Constructor which initializes the value of the switch and creates a mutex
   *  needed to proper initialisations.
   *
   *  @param init_value Initial value of the switch.
   *
   *  @note This constructor NEEDS to be called after glfwInit(). Otherwise the
   *        mutex won't be created.
   */
  TSAFE_BOOL_SWITCH (bool init_value) {
    value = init_value;
#ifdef NEW_GLFW3
	if(mtx_init(&mutex, mtx_plain) == thrd_error) 
		Critical ("Error creating mutex for TSAFE_BOOL_SWITCH");
#else
	mutex = glfwCreateMutex ();
    if (!mutex)
      Critical ("Error creating mutex for TSAFE_BOOL_SWITCH");
#endif
    
  }

  /**
   *  Sets the value of the switch to true;
   */
  void SetTrue () {
#ifdef NEW_GLFW3
	mtx_lock(&mutex);
#else
	glfwLockMutex (mutex);
#endif
    
    value = true;
    
#ifdef NEW_GLFW3
	mtx_unlock(&mutex);
#else
	glfwUnlockMutex (mutex);
#endif
    
  }

  /**
   *  Returns true if the switch is on (value is true) and atomically change
   *  the value of the switch to false.
   */
  bool IsTrue () {
#ifdef NEW_GLFW3
	mtx_lock(&mutex);
#else
	glfwLockMutex (mutex);
#endif
    

    if (value) {
      value = false;
#ifdef NEW_GLFW3
		mtx_lock(&mutex);
#else
		glfwUnlockMutex (mutex);
#endif
      
      return true;
    }
#ifdef NEW_GLFW3
	mtx_unlock(&mutex);
#else
	glfwUnlockMutex (mutex);
#endif
    
    return false;
  }

private:
	
#ifdef NEW_GLFW3
	mtx_t mutex;
#else
	GLFWmutex mutex;  //!< Object's mutex.
#endif
  
  bool value;       //!< Value of the switch.
};


//========================================================================
// Functions
//========================================================================

// GLWF callbacks
#ifdef NEW_GLFW3
void GLFWCALL SizeCallback(GLFWwindow* window, int w, int h);
void GLFWCALL KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
void GLFWCALL MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void GLFWCALL MousePosCallback(GLFWwindow* window, double x, double y);
void GLFWCALL MouseWheelCallback(GLFWwindow* window, double xoffset, double yoffset);
void GLFWCALL WindowRefreshCallback (GLFWwindow* window);
#else
void GLFWCALL SizeCallback(int w, int h);
void GLFWCALL KeyCallback(int key, int action);
void GLFWCALL MouseButtonCallback(int button, int action);
void GLFWCALL MousePosCallback(int x, int y);
void GLFWCALL MouseWheelCallback(int pos);
void GLFWCALL WindowRefreshCallback ();
#endif


// engine methods
void Menu(void);
void Game(void);

void ChangeActionPanel(int panel);
void UpdateGuardButtons();
void CheckGuardButton(TAGGRESSIVITY_MODE button);
void CreateBuildButtons();
void UncheckBuildButton();
void CreateOrderButtons();
void SetOrderVisibility(bool vis);

// preparing methods
void PrepareSounds();


//========================================================================
// Variables
//========================================================================

extern THOST *host;
extern TSAFE_BOOL_SWITCH *need_redraw;

extern int action_key;
extern bool action_force;
extern bool won_lose;

extern std::string app_path;
extern std::string user_dir;


#endif  //__doengine_h__

//========================================================================
// End
//========================================================================
// vim:ts=2:sw=2:et:

