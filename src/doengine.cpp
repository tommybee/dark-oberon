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
 *  @file doengine.cpp
 *
 *  Menu and game engine + creating GUI and callback functions.
 *
 *  @author Peter Knut
 *  @author Martin Kosalko
 *  @author Marian Cerny
 *
 *  @date 2004, 2005
 */


//========================================================================
// Included files
//========================================================================

#include "cfg.h"

#ifdef WINDOWS
 #include <io.h>
#else // on UNIX
 #include <sys/types.h>
 #include <dirent.h>
#endif


#include "donet.h"

#ifdef NEW_GLFW3
#include <glfw3.h>
#include <tinycthread.h>
#else
#include <glfw.h>
#endif

#include <cmath>
#include <string>

#include "dofollower.h"
#include "doengine.h"
#include "dodraw.h"
#include "domap.h"
#include "domouse.h"
#include "doplayers.h"
#include "doleader.h"
#include "doselection.h"
#include "dosimpletypes.h"
#include "doevents.h"
#include "glgui.h"
#include "dopool.h"
#include "doipc.h"

using std::string;


//========================================================================
// Constants
//========================================================================

// main menu keys
#define MNU_MAIN              0
#define MNU_PLAY              1
#define MNU_OPTIONS           2
#define MNU_CREDITS           3
#define MNU_QUIT              4
#define MNU_QUICK_PLAY        5

// play menu keys
#define MNU_RESUME            6
#define MNU_CREATE            7
#define MNU_CONNECT           8
#define MNU_DISCONNECT        9

// ip menu
#define MNU_CREATE2           10
#define MNU_CONNECT2          11
#define MNU_IP                12
#define MNU_PLAYER_NAME       13

// game menu
#define MNU_PLAY2             15
#define MNU_MAP_LIST          16
#define MNU_KILL_PLAYER       17

// options menu keys
#define MNU_VIDEO             21
#define MNU_AUDIO             22

// video menu key
#define MNU_FULLSCREEN        30
#define MNU_VERT_SYNC         31
#define MNU_640               32
#define MNU_800               33
#define MNU_1024              34
#define MNU_1152              35
#define MNU_1280              36
#define MNU_1600              37

// filters keys
#define MNU_TF_NEAREST        40
#define MNU_TF_LINEAR         41
#define MNU_MF_NONE           42
#define MNU_MF_NEAREST        43
#define MNU_MF_LINEAR         44

// audio menu key
#define MNU_MASTER_VOL        50
#define MNU_MENU_MUSIC        51
#define MNU_MENU_SOUND_VOL    52
#define MNU_MENU_MUSIC_VOL    53
#define MNU_GAME_MUSIC        54
#define MNU_GAME_SOUND_VOL    55
#define MNU_GAME_MUSIC_VOL    56
#define MNU_UNIT_SPEECH       57

// panel keys
#define MNU_TOGGLE_PANEL      100
#define MNU_TOGGLE_RADAR      101

#define MNU_VIEW_SEGMENT      102

// action buttons
#define MNU_ACTION_STAY       110
#define MNU_ACTION_MOVE       111
#define MNU_ACTION_ATTACK     112
#define MNU_ACTION_MINE       113
#define MNU_ACTION_REPAIR     114
#define MNU_ACTION_BUILD      115

// build buttons
#define MNU_BUILD_BUTTON      200     // other build buttons have incremental keys (+1, +2, ...)

// guard
#define MNU_GUARD_BUTTON      300     // other guard buttons have incremental keys (+1, +2, ...)

// dialog keys
#define MNU_OK                400


#define MAX_VID_MODES         100
#define GAME_PANEL_ALPHA      0.9f
#define TOOLTIP_ALPHA         0.7f


//========================================================================
// Macros
//========================================================================

#define SetMenuPanel() \
do { \
  panel->SetColor(0, 0, 0); \
  panel->SetAlpha(0.7f); \
  panel->SetPadding(0); \
  panel->SetVisible(false); \
  panel->SetOnDraw(MenuPanelOnDraw); \
} while (0)


#define SetMenuButton(enabled) \
do { \
  button->SetFaceColor(0.7f, 0.6f, 0.4f); \
  button->SetHoverColor(1, 0.93f, 0.82f); \
  button->SetOnMouseClick(MenuButtonOnClick); \
  button->SetEnabled(enabled); \
} while (0)


#define SetGamePanel(visible) \
do { \
  panel->SetColor(0.3f, 0.3f, 0.3f); \
  panel->SetAlpha(GAME_PANEL_ALPHA); \
  panel->SetPadding(0); \
  panel->SetOnMouseUp(GameOnMouseUp); \
  panel->SetVisible(visible); \
} while (0)


#define SetGameButton(tooltip) \
do { \
  button->SetFaceColor(0.7f, 0.6f, 0.4f); \
  button->SetHoverColor(1, 0.93f, 0.82f); \
  button->SetOnMouseUp(GameOnMouseUp); \
  button->SetOnMouseDown(GameButtonOnMouseDown); \
  button->SetOnMouseClick(GameButtonOnClick); \
  button->SetCanFocus(false); \
  button->SetTooltipText(tooltip); \
} while (0)


#define SetBuildButton() \
do { \
  button->SetFaceColor(0.7f, 0.7f, 0.7f); \
  button->SetHoverColor(1, 1, 1); \
  button->SetOnMouseUp(GameOnMouseUp); \
  button->SetOnMouseClick(GameBuildOnClick); \
  button->SetOnDraw(GameBuildOnDraw); \
  button->SetOnShowTooltip(GameBuildOnTooltip); \
  button->SetTooltipBox(build_tooltip); \
} while (0)


#define SetStayButton() \
do { \
  button->SetFaceColor(0.7f, 0.7f, 0.7f); \
  button->SetHoverColor(1, 1, 1); \
  button->SetOnMouseUp(GameOnMouseUp); \
  button->SetOnMouseClick(GameStayOnClick); \
  button->SetOnDraw(GameBuildOnDraw); \
} while (0)


#define SetProduceButton() \
do { \
  button->SetFaceColor(0.7f, 0.7f, 0.7f); \
  button->SetHoverColor(1, 1, 1); \
  button->SetOnMouseUp(GameOnMouseUp); \
  button->SetOnMouseClick(GameProduceOnClick); \
  button->SetOnShowTooltip(GameBuildOnTooltip); \
  button->SetTooltipBox(build_tooltip); \
} while (0)


#define SetOrderButton(txt) \
do { \
  button->SetFaceColor(0.7f, 0.7f, 0.7f); \
  button->SetHoverColor(1, 1, 1); \
  button->SetOnMouseUp(GameOnMouseUp); \
  button->SetOnMouseDown(GameOrderOnMouseDown); \
  button->SetCaption(txt); \
} while (0)


//========================================================================
// Forward declarations
//========================================================================

class TBUILD_TOOLTIP;
void StopGame();
void BackgroundResolveFinished ();
void SetActiveMenu(TGUI_PANEL *menu);
void UpdateGameMenu();
void MenuUpdateMapInfo (string map_name);
bool Disconnect();

static void ProcessNetEvent (TNET_MESSAGE *msg);
static void ProcessPlayerArray (TNET_MESSAGE *msg);
static void ProcessHello (TNET_MESSAGE *msg);
static void ProcessPingRequest (TNET_MESSAGE *msg);
static void ProcessPingReply (TNET_MESSAGE *msg);
static void ProcessConnectRequest (TNET_MESSAGE *msg);
static void ProcessChangeRace (TNET_MESSAGE *msg);
static void ProcessChatMessage (TNET_MESSAGE *msg);
static void ProcessSynchronise (TNET_MESSAGE *msg);
static void ProcessAllowProcessFunction (TNET_MESSAGE *msg);
static void ProcessDisconnect (TNET_MESSAGE *msg);

static void ProcessDisconnect (int player_id);
static void OnDisconnect (in_addr address, in_port_t port);

//========================================================================
// Variables
//========================================================================

std::string app_path;
std::string user_dir;

TPANEL_INFO panel_info;             //!< Group of variables for drawing unit and player info on main panel.
TGAME_STATE state = ST_MAIN_MENU;
TGAME_ERROR error = ERR_NONE;

TBUILD_TOOLTIP  *build_tooltip = NULL;

string selected_map_name;

/** Update thread ID. */
#ifdef NEW_GLFW3
thrd_t process_thread = NULL;
thrd_t connecting_thread_id = NULL;
#else
GLFWthread process_thread = -1;
GLFWthread connecting_thread_id = -1;
#endif


// menus
TGUI_PANEL *main_menu = NULL;
TGUI_PANEL *play_menu = NULL;
TGUI_PANEL *ip_menu = NULL;
TGUI_PANEL *game_menu = NULL;
TGUI_PANEL *options_menu = NULL;
TGUI_PANEL *video_menu = NULL;
TGUI_PANEL *audio_menu = NULL;
TGUI_PANEL *credits_menu = NULL;
TGUI_PANEL *active_menu = NULL;

// game panels
TGUI_PANEL *main_panel = NULL;
TGUI_PANEL *little_panel = NULL;
TGUI_PANEL *chat_panel = NULL;
TGUI_PANEL *radar_panel = NULL;
bool main_panel_visible = true;

TGUI_EDIT_BOX *chat_edit = NULL;

// menu objects
TGUI_EDIT_BOX *ip_edit = NULL;
TGUI_LABEL    *ip_label = NULL;
TGUI_BUTTON   *connect_button = NULL;
TGUI_BUTTON   *create_button = NULL;

TGUI_LIST_BOX *map_list = NULL;
TGUI_LABEL    *map_label = NULL;
TGUI_BUTTON   *play_button = NULL;
TGUI_BUTTON   *disconn_button = NULL;

TGUI_BUTTON *resume_button = NULL;
TGUI_BUTTON *disconnect_button = NULL;

TBASIC_ITEM *last_item = NULL;

// actions for message box
int action_key = 0;
bool action_force = false;

// map info
TGUI_LABEL  *map_scheme_label = NULL;
TGUI_LABEL  *map_size_label = NULL;
TGUI_LABEL  *map_players_label = NULL;
TGUI_LABEL  *map_races_label = NULL;
TGUI_LABEL  *map_author_label = NULL;
TGUI_SCROLL_BOX *map_info_scroll = NULL;

// players info
TGUI_LABEL     *pl_name_label[PL_MAX_PLAYERS];
TGUI_COMBO_BOX *pl_race_combo[PL_MAX_PLAYERS];
TGUI_BUTTON    *pl_kill_button[PL_MAX_PLAYERS];

// loading game
TGUI_PANEL     *load_panel = NULL;

// menu lists
TMAP_INFO_LIST map_info_list; //!< List of maps info in menu.

// some networking variables
THOST *host = NULL;
bool connected = false;
bool started = false;
bool allowed_to_start_process_function = true;
bool won_lose = false;

/** Specifies, whether leader loaded a map and created all needed game
 *  structures. This is used by synchronisation of start of the game. */
bool leader_ready;

/**
 *  Specifies, whether we need to redraw the screen. This saves a lot of
 *  processor time. This is used only in menu. The reason, why it is not used in
 *  the game, is that the game is too dynamic and this variable will almost
 *  allways be set to true (because of animations, etc.).
 */
TSAFE_BOOL_SWITCH *need_redraw = NULL;


//========================================================================
// Thread for connecting in menu
//========================================================================

struct TCONNECT_DATA {
  string server_name;
  in_port_t port;
};

static void connecting_in_menu_thread (void *_data) {
  TCONNECT_DATA *data = static_cast<TCONNECT_DATA *>(_data);
  string server_name = data->server_name;
  in_port_t port = data->port;
  delete data;

  TNET_RESOLVER resolver;

  try {
    string message;

    Debug ("Phase 1: before resolve");

    message = string ("Resolving host ") + server_name + "...";
    action_key = MNU_CONNECT2;
    gui->ShowMessageBox (message.c_str (), GUI_MB_CANCEL);

    in_addr ip_address = resolver.Resolve (server_name);
    string ip_address_string = resolver.NetworkToAscii (ip_address);

    Debug ("Phase 2: before connect");

    TFOLLOWER *follower;

    if (server_name == ip_address_string)
      message = string ("Connecting to ") + ip_address_string + "...";
    else
      message = string ("Connecting to ") + server_name + " (" + ip_address_string + ")...";

    action_key = MNU_CONNECT2;
    gui->ShowMessageBox (message.c_str (), GUI_MB_CANCEL);

    // Start follower. It will listen on config.net_server_port.
    host = follower = NEW TFOLLOWER (follower_in_queue_size, config.net_server_port, follower_out_queue_size, ip_address, port);
    connected = true;

    gui->HideMessageBox ();

    host->RegisterExtendedFunction (net_protocol_event, ProcessNetEvent);
    host->RegisterExtendedFunction (net_protocol_hello, ProcessHello);
    host->RegisterExtendedFunction (net_protocol_player_array, ProcessPlayerArray);
    host->RegisterExtendedFunction (net_protocol_chat_message, ProcessChatMessage);
    host->RegisterExtendedFunction (net_protocol_synchronise, ProcessAllowProcessFunction);
    host->RegisterExtendedFunction (net_protocol_ping, ProcessPingReply);
    host->RegisterExtendedFunction (net_protocol_disconnect, ProcessDisconnect);

    host->RegisterOnDisconnect (OnDisconnect);

    player_array.RunningOnFollower ();

    follower->Connect (config.player_name);

    Debug ("Phase 3: before ping - background");

#ifdef NEW_GLFW3
	struct timespec interval;
	interval.tv_sec = 0;
  	interval.tv_nsec = 0.05;
	
    for (int i = 0; i < 50; i++) {
      follower->SendPingRequest ();
      thrd_sleep(&interval, NULL);
      interval.tv_nsec *= 1.04;
    }
#else
	double wait_interval = 0.05;
    for (int i = 0; i < 50; i++) {
      follower->SendPingRequest ();
      glfwSleep (wait_interval);
      wait_interval *= 1.04;
    }
#endif
    

    Debug ("Phase 4: Finished");

  } catch (TNET_RESOLVER::ResolveException &) {
    string message = string ("Error:\nCould not look up host ") + server_name;
    action_key = MNU_DISCONNECT;
    gui->ShowMessageBox (message.c_str (), GUI_MB_OK);
    
  } catch (TNET_ADDRESS::ConnectingErrorException &) {
    string message = string ("Error:\nError connecting to host ") + server_name;
    action_key = MNU_DISCONNECT;
    gui->ShowMessageBox (message.c_str (), GUI_MB_OK);

  } catch (...) {
    Debug ("nejaka ina vynimka");
  }
}


//========================================================================
// Sounds
//========================================================================

#if SOUND

// vol [0..100]
void ChangeSoundVolume(T_BYTE vol) {
  vol = (T_BYTE)(vol * 2.55);

  FSOUND_SetSFXMasterVolume((vol * config.snd_master_volume) / 100);
}

// vol [0..100]
void SetMenuMusicVolume(T_BYTE vol) {
  config.snd_menu_music_volume = vol;

  vol = (T_BYTE)(vol * 2.55);

  sounds_table.sounds[DAT_SID_MENU_MUSIC]->SetVolumeAbsolute((vol * config.snd_master_volume) / 100);
}


// vol [0..100]
void SetGameMusicVolume(T_BYTE vol) {
  config.snd_game_music_volume = vol;

  vol = (T_BYTE)(vol * 2.55);

  sounds_table.sounds[DAT_SID_GAME_MUSIC]->SetVolumeAbsolute((vol * config.snd_master_volume) / 100);
}


void SetMasterVolume(T_BYTE vol) {
  config.snd_master_volume = vol;

  SetMenuMusicVolume(config.snd_menu_music_volume);
  SetGameMusicVolume(config.snd_game_music_volume);

  if (state == ST_GAME) ChangeSoundVolume(config.snd_game_sound_volume);
  else ChangeSoundVolume(config.snd_menu_sound_volume);
}


void PrepareSounds()
{
  sounds_table.sounds[DAT_SID_MENU_MUSIC]->SetLoop(true);
  sounds_table.sounds[DAT_SID_GAME_MUSIC]->SetLoop(true);

  SetMasterVolume(config.snd_master_volume);
}

#endif


//========================================================================
// TBUILD_TOOLTIP
//========================================================================

class TBUILD_TOOLTIP: public TGUI_PANEL {
private:
  int mat_count;
  TGUI_LABEL *text;
  TGUI_LABEL *info_pic[SCH_MAX_MATERIALS_COUNT + 2];
  TGUI_LABEL *info_num[SCH_MAX_MATERIALS_COUNT + 2];
  TGUI_LABEL *text2;

  TGUI_BOX_ENVELOPE ch_envelope;

public:
  TBUILD_TOOLTIP():TGUI_PANEL(NULL, 0, 0, 0, 100, 100)
  {
    int i;
    GLfloat line_h = 15.0f;

    SetPadding(3);
    mat_count = scheme.materials_count;

    SetHeight((mat_count + 4) * line_h + 2 * padding);

    text = AddLabel(0, 0, (mat_count + 3) * line_h, "");
    text2 = AddLabel(0, 0, -4, "");

    for (i = 0; i < mat_count + 2; i++) {
      info_pic[i] = AddLabel(0, 0, (mat_count - i + 2) * line_h, 15, 12);
      info_pic[i]->SetColor(1, 1, 1);
      info_num[i] = AddLabel(0, 20, (mat_count - i + 2) * line_h - 2, "");
    }

    // materials
    for (i = 0; i < mat_count; i++) {
      info_pic[i]->SetTexture(scheme.tex_table.GetTexture(scheme.materials[i]->tg_id, 0));
    }

    // food, energy
    info_pic[mat_count + 0]->SetTexture(myself->race->tex_table.GetTexture(myself->race->tg_food_id, 0));
    info_pic[mat_count + 1]->SetTexture(myself->race->tex_table.GetTexture(myself->race->tg_energy_id, 0));
  }


  void SetText(char *txt)
  {
    text->SetCaption(txt);
  }


  void SetText2(char *txt2)
  {
    text2->SetCaption(txt2);
  }

  TGUI_LABEL *GetText2(void)
  {
    return text2;
  }

  void SetInfo(T_BYTE id, int num)
  {
    char txt[10];
    sprintf(txt, "%d", num);
    info_num[id]->SetCaption(txt);
  }


  virtual void RecalculateChildren(TGUI_BOX *sender)
  {
    TGUI_BOX *box;

    ch_envelope.Reset();

    for (box = last_child; box; box = box->GetPrev()) {
      ch_envelope.TestBox(box);
    }

    SetWidth(ch_envelope.GetMaxX() + 2 * padding);
    SetHeight(ch_envelope.GetMaxY() + 2 * padding);
  }
};


//=========================================================================
// Structures of map list used in menu
//=========================================================================

/**
 *  Fills list of maps from directory MAP_PATH.
 */
bool TMAP_INFO_LIST::LoadMapInfo(bool basic, const char *file_name){
  
  TFILE_NAME mapname, racname;
  TCONF_FILE *cf, *cf_rac, *cf_sch;
  TFILE_LINE pom;
  bool ok = true, ok_scheme, ok_race, ok_race_all, warn_race;;
  TMAP_BASIC_INFO_NODE * basic_node = NULL;
  TMAP_RAC_INFO_NODE * rac_node = NULL;
  int rac_count = 0, i;
  char id_name[1024], full_name[1024], schemes_name[1024]; //buffer for name, fullname and schemes ids

  sprintf(mapname, "%s%s", MAP_PATH, file_name);
  if (!(cf = OpenConfFile(mapname)))
    return false;
  
  if (basic){ // fill list of basic info of map
    // creating new instance of basic map info
    if (!(basic_node = NEW TMAP_BASIC_INFO_NODE)){
      Critical("Can not allocate memory for menu structures.");
      return false;
    }

    
    // set variables
    strcpy(basic_node->id_name, file_name);
    if (ok) {
      ok = cf->ReadStr(basic_node->name, const_cast<char*>("name"), const_cast<char*>(""), true);
    }
    if (ok) { // put new instance to list
      basic_node->next = map_list;
      map_list = basic_node;
    }
    else delete basic_node;

  }
  else { // fill extended info of map
 
    // extended map information
    if (ok) cf->ReadStr(map_ext_info.author, const_cast<char*>("author"), const_cast<char*>(""), true); //author is not mandatory
    if (ok) ok = cf->ReadSimpleRange( &(map_ext_info.width), const_cast<char*>("width"), 1, MAP_MAX_SIZE, 1);
    if (ok) ok = cf->ReadSimpleRange( &(map_ext_info.height), const_cast<char*>("height"), 1, MAP_MAX_SIZE, 1);

    // scheme name
    if (ok) ok = cf->ReadStr(map_ext_info.scheme_id_name, const_cast<char*>("scheme"), const_cast<char*>(""), true);
    if (ok) {
      char pom[1024];
      sprintf(pom, "%s%s%s", SCH_PATH, map_ext_info.scheme_id_name, ".sch");
      if ((cf_sch = OpenConfFile(pom))) {
        cf_sch->ReadStr(map_ext_info.scheme_name, const_cast<char*>("name"), const_cast<char*>(map_ext_info.scheme_id_name), false);
        CloseConfFile(cf_sch);
        cf_sch = NULL;
      }
    }

    // loading information about races from map
    // clear list of races info if necesary
    ClearRacList();

    if (ok) cf->SelectSection(const_cast<char*>("Players"), true);
    if (ok) ok = cf->ReadIntGE(&(map_ext_info.max_players), const_cast<char*>("max_count"), 0, 0);
    if (ok){
      // Maximum count of players not including hyper player.
      int max_players = PL_MAX_PLAYERS - 1;

      if (map_ext_info.max_players > max_players) {
        map_ext_info.max_players = max_players;
        Warning(LogMsg("Maximal count players in map %s is greater than '%d'.", file_name, max_players));
      }
    }

    // reading start points
    if (ok) {
      ok = cf->SelectSection(const_cast<char*>("Start Points"), true);

      int start_points_count;

      if (ok) ok = cf->ReadIntGE(&start_points_count, const_cast<char*>("count"), 1, 1);
      if (ok) start_points_count = MIN(start_points_count, PL_MAX_START_POINTS);  // if there is more then PL_MAX_START_POINTS, PL_MAX_START_POINTS is used

      if (ok) player_array.SetStartPointsCount (start_points_count);

      cf->UnselectSection();
    }

    if (ok) ok = cf->SelectSection(const_cast<char*>("Races"), true);
    
    if (ok) ok = cf->ReadIntGE(&rac_count, const_cast<char*>("count"), 1, 1);
    
    ok_race_all = false;
    for (i = 0; ok && i < rac_count; i++) {
      //loading each race info
      sprintf(pom, "Race %d", i);
      if (ok) ok = cf->SelectSection(pom, true);
   
      if (ok) ok = cf->ReadStr(id_name, const_cast<char*>("name"), const_cast<char*>(""), true);
        
      if (ok){
        warn_race = false;
        ok_race = true;
        
        // find out if file race.rac exists and read full name and scheme from it
        sprintf(racname, "%s%s/%s.rac", RAC_PATH, id_name, id_name);
        if (!(cf_rac = OpenConfFile(racname))){
          ok_race = false;
          warn_race = true;
        }
        
        // check if race is compatible vith map scheme
        if (ok_race){
          cf_rac->ReadStr(full_name, const_cast<char*>("name"), id_name, true);
          
          ok_scheme = false;

          do {
            if (ok_race) ok_race = cf_rac->ReadStr(schemes_name, const_cast<char*>("schemes"), const_cast<char*>(""), false);
            if ((ok_race) && (*schemes_name != 0)){
              if (!(strcmp(schemes_name, map_ext_info.scheme_id_name))){
                ok_scheme = true;
              }
            }
          } while (*schemes_name != 0);

          if (!ok_scheme) {
            cf_rac->ReadStr(schemes_name, const_cast<char*>("schemes"), const_cast<char*>(""), true); //only for log with line number
            Warning(LogMsg("Map '%s' and race '%s' are not scheme compatible", file_name, id_name));
            warn_race = true;
          }

          ok_race = ok_scheme;
        }

             
        // creating new TMAP_RAC_INFO_NODE node
        if (ok_race){
    
          if (!(rac_node = NEW TMAP_RAC_INFO_NODE)){
            Critical("Can not allocate memory for menu structures.");
            return false;
          }
    
          // set variables
          strcpy(rac_node->id_name, id_name);
          strcpy(rac_node->name, full_name);
        
          // put new instance to list
          rac_node->next = rac_list;
          rac_list = rac_node;

          if (!ok_race_all) ok_race_all = true; //at least one race was readed
        }
        else if (!warn_race) Warning(LogMsg("Race '%s' is corrupted.", id_name));  //error reading race

      
        CloseConfFile(cf_rac);
      }
      
      


      cf->UnselectSection();  // Race X
    }

    if (ok) ok = ok_race_all;
    
    cf->UnselectSection();  //Races
    cf->UnselectSection();  //Players

    SortRacList ();
  }

  CloseConfFile(cf);

  if (!ok) Warning(LogMsg("Map '%s' is not complet or is corrupted.", file_name));
  
  return ok;
}


/**
 *  Delete list of maps.
 */
void TMAP_INFO_LIST::ClearMapList(void){

  TMAP_BASIC_INFO_NODE *act, *next;

  if (map_list){
    for (act = map_list, next = act->next; next != NULL; act = next, next = next->next){
      delete act;
      act = NULL;
    }
    delete act;
    map_list = NULL;
  }
}

/**
 *  Delete list of races.
 */
void TMAP_INFO_LIST::ClearRacList(void){

  TMAP_RAC_INFO_NODE *act, *next;

  if (rac_list){
    for (act = rac_list, next = act->next; next != NULL; act = next, next = next->next){
      delete act;
    }
    delete act;
    rac_list = NULL;
  }
}

/**
 *  Sorts list of maps.
 */
void TMAP_INFO_LIST::SortMapList(void) {
  TMAP_BASIC_INFO_NODE *p;
  bool change_made = true;

  if (!map_list)
    return; /* Nothing to sort. */

  /* We are using bubble sort here. */
  while (change_made) {
    change_made = false;

    for (p = map_list; p != NULL; p = p->next) {
      TMAP_BASIC_INFO_NODE *next = p->next;
      TMAP_NAME temp_name;
      TMAP_FILENAME temp_id_name;

      /* If the next item exists and should be before the actual one, SWAP them.
       */
      if (next && strcmp (p->name, next->name) > 0) {
        change_made = true;

        strcpy (temp_name, p->name);
        strcpy (temp_id_name, p->id_name);

        strcpy (p->name, next->name);
        strcpy (p->id_name, next->id_name);

        strcpy (next->name, temp_name);
        strcpy (next->id_name, temp_id_name);
      }
    }
  }
}

/**
 *  Sorts list of races.
 */
void TMAP_INFO_LIST::SortRacList(void) {
  /* XXX: THIS IS UGLY COPY AND PASTE FROM SortMapList(). :-( But it's not my
   *      fault, it's the design fault. [jojolaser] */
  TMAP_RAC_INFO_NODE *p;
  bool change_made = true;

  if (!rac_list)
    return; /* Nothing to sort. */

  /* We are using bubble sort here. */
  while (change_made) {
    change_made = false;

    for (p = rac_list; p != NULL; p = p->next) {
      TMAP_RAC_INFO_NODE *next = p->next;
      TRAC_NAME temp_name;
      TRAC_FILENAME temp_id_name;

      /* If the next item exists and should be before the actual one, SWAP them.
       */
      if (next && strcmp (p->name, next->name) > 0) {
        change_made = true;

        strcpy (temp_name, p->name);
        strcpy (temp_id_name, p->id_name);

        strcpy (p->name, next->name);
        strcpy (p->id_name, next->id_name);

        strcpy (next->name, temp_name);
        strcpy (next->id_name, temp_id_name);
      }
    }
  }
}

/**
 *  Finds out the id_name of the race from the @p name of the race.
 *
 *  @return String containing the id_name of the map.
 *
 *  @throw NotFoundException
 */
string TMAP_INFO_LIST::GetRacIdName (string name) {
  TMAP_RAC_INFO_NODE *p;

  for (p = rac_list; p != NULL; p = p->next)
    if (name == p->name)
      return p->id_name;

  throw NotFoundException ();
}

/**
 *  Finds out the name of the race from the @p id_name of the race.
 *
 *  @return String containing the name of the map.
 *
 *  @throw NotFoundException
 */
string TMAP_INFO_LIST::GetRacName (string id_name) {
  TMAP_RAC_INFO_NODE *p;

  for (p = rac_list; p != NULL; p = p->next)
    if (id_name == p->id_name)
      return p->name;

  throw NotFoundException ();
}

/**
 *  Finds out the name of the mape from the @p id_name of the map.
 *
 *  @return String containing the name of the map or @c NULL when no race with
 *          specified @p id_name exists.
 */
string TMAP_INFO_LIST::GetMapName (string id_name) {
  TMAP_BASIC_INFO_NODE *p;

  for (p = map_list; p != NULL; p = p->next) {
    if (id_name == p->id_name)
      return p->name;
  }

  return "";
  // XXX: throw MapNotFoundException(); [majo]
}

/**
 *  Fills list of maps from directory MAP_PATH.
 */
bool TMAP_INFO_LIST::LoadMapList() {

  bool ok = true;
  char * extension;

  ClearMapList(); // if exists any list of maps, clears it

#ifdef WINDOWS  // on WINDOWS systems
  _finddata_t file;         // file in directory 
  long file_handler;        // handler to first find file in directory
  bool next_file = true;
#else  // on UNIX systems
  DIR *dir;
  struct dirent *entry;
#endif


#ifdef WINDOWS  // on WINDOWS systems
  if ((file_handler = _findfirst((string(MAP_PATH) + "*.map").c_str(), &file)) == -1L) {  // gets handler to first file with mask "*.map"
    _findclose(file_handler); // no file exists
    return ok;
  }

  while (ok && next_file) { // loop over all files and directories id MAP_PATH diectory
    extension = strrchr(file.name, '.');
    if (!(strcmp(extension, ".map"))) // filter in _findfirst is not correct (accepts files *.map*)
      ok = LoadMapInfo(true, file.name);  // loads map info for each *.map file
    next_file = (!_findnext(file_handler, &file));
  }

  _findclose(file_handler);

#else  // on UNIX systems
  if (!(dir = opendir (MAP_PATH))) {
    Critical( LogMsg ("%s%s%s", "Error opening directory '", MAP_PATH, "'"));
    return false;
  }

  while (ok && ((entry = readdir(dir)) != NULL)) {
    if (entry->d_type != DT_DIR)
    {
      extension = strrchr(entry->d_name, '.');
      /** XXX: TU TO ASI MOZE SPADNUT, ked tam bude subor bez pripony **/
      if (!(strcmp(extension, ".map")))
        ok = LoadMapInfo(true, entry->d_name); // loads map info for each *.map file
    }
  }
  
  closedir(dir);
#endif

  SortMapList ();

  return ok;
}

char * TMAP_INFO_LIST::GetRandomMap(){
  TMAP_BASIC_INFO_NODE * act;
  int count;
  int i;
  
  if (map_list) {

    for (act = map_list, count = 0; act != NULL; act = act->next, count++);
    for (act = map_list, i = 0; i < GetRandomInt(count); act = act->next, i++);
    
    return act->id_name;
  }
  else return NULL;
}


//========================================================================
// Useful Methods
//========================================================================

void ToggleMainPanel()
{
  main_panel->ToggleVisible();
  main_panel_visible = main_panel->IsVisible();

  if (chat_panel->IsVisible()) {
    chat_panel->SetAlpha(main_panel->IsVisible() ? GAME_PANEL_ALPHA : 0);
  }
  else little_panel->ToggleVisible();

  if (radar.IsHideable()) radar_panel->SetVisible(main_panel->IsVisible());
  if (!radar_panel->IsVisible() && radar.GetMoving()) radar.SetMoving(false);
}


void ToggleRadarPanel()
{
  radar.ToggleHideable();

  if (radar.IsHideable() && !main_panel->IsVisible()) {
    radar.SetMoving(false);
    radar_panel->Hide();
  }
}


void ToggleChatPanel()
{
  if (!chat_panel->IsVisible()) {
    little_panel->Hide();
    chat_panel->SetAlpha(main_panel->IsVisible() ? GAME_PANEL_ALPHA : 0);
    chat_panel->Show();
    chat_edit->Focus();
  }
  else {
    little_panel->SetVisible(main_panel->IsVisible());
    chat_panel->Hide();
    chat_edit->Unfocus();
    chat_edit->SetText(NULL);
  }
}


/**
 *  Updates myself information on little panel.
 */
void UpdateMyselfInfo()
{
  if (state != ST_GAME) {
    myself->update_info = false;
    return;
  }

  int i;

  char txt[100];

  // materials
  for (i = 0; i < scheme.materials_count; i++) {
    sprintf(txt, "%.0f", myself->GetStoredMaterial(i));
    panel_info.material_label[i]->SetCaption(txt);
  }

  // food, energy
  sprintf(txt, "%d/%d", myself->GetOutFood(), myself->GetInFood());
  panel_info.food_label->SetCaption(txt);
  if (myself->GetOutFood() > myself->GetInFood()) panel_info.food_label->SetFontColor(1, 0.4f, 0.4f);
  else panel_info.food_label->SetFontColor(1, 0.93f, 0.82f);

  sprintf(txt, "%d/%d", myself->GetOutEnergy(), myself->GetInEnergy());
  panel_info.energy_label->SetCaption(txt);
  if (myself->GetOutEnergy() > myself->GetInEnergy()) panel_info.energy_label->SetFontColor(1, 0.4f, 0.4f);
  else panel_info.energy_label->SetFontColor(1, 0.93f, 0.82f);

  myself->update_info = false;
}


void SetActiveMenu(TGUI_PANEL *menu)
{
#ifdef NEW_GLFW3
	static mtx_t mutexicek;
	mtx_init(&mutexicek, mtx_plain);
	mtx_lock(&mutexicek);
#else
	static GLFWmutex mutexicek = glfwCreateMutex ();
	glfwLockMutex (mutexicek);
#endif
  

  active_menu->SetVisible(false);
  menu->SetVisible(true);
  active_menu = menu;

#ifdef NEW_GLFW3
	mtx_unlock(&mutexicek);
#else
	glfwUnlockMutex (mutexicek);
#endif
  
}


/**
 *  Synchronises player_array with game menu.
 */
void SynchronisePlayersAndMenu () {
  player_array.Lock ();

  /* For each player we synchronise player's race selected in combo box and
   * race set in players_array. */
  for (int i = 1; i < player_array.GetCount (); i++) {
    string selected_race_id;

    /* Find out, which race is selected in player's combo box. */
    try {
      const char *text = pl_race_combo[i]->GetText ();
      selected_race_id = map_info_list.GetRacIdName (text);
    } catch (...) {
      /* Do nothing if no race was found. */
      continue;
    }

    try {
      string player_race_id = player_array.GetRaceIdName (i);
      string player_race_name = map_info_list.GetRacName (player_race_id.c_str ());

      /* The race of player i exists. So we check out, if the same race is
       * selected also in the combo box and when not, correct the combo
       * box. */
      if (player_race_id != selected_race_id)
        pl_race_combo[i]->SetItem (player_race_name.c_str ());
    } catch (TMAP_INFO_LIST::NotFoundException &) {
      /* The race of player i does not exist. We change his race to those
       * which is selected in his combo box. */
      player_array.SetRaceIdName (i, selected_race_id);
    }
  }

  player_array.Unlock ();
}


void UpdateGameMenu()
{
  giant->Lock ();

  bool none = host == NULL;
  bool leader = !none && host->GetType () == THOST::ht_leader;
  bool follower = !none && host->GetType () == THOST::ht_follower;

  player_array.Lock ();

  map_list->SetVisible(leader);
  map_label->SetVisible(follower);
  map_info_scroll->SetVisible (!none);

  play_button->SetVisible(leader);

  if (leader) disconn_button->SetPos(20, 15);
  if (follower || none) disconn_button->SetPos(157, 15);

  int i;

  /* Update GUI according to connected players. */
  for (i = 1; i < player_array.GetCount (); i++) {
    string player_name = player_array.GetPlayerName (i);
    pl_name_label[i]->SetCaption (player_name.c_str ());
    pl_name_label[i]->SetVisible (true);
    pl_race_combo[i]->SetVisible (true);
    pl_race_combo[i]->SetEnabled (leader || (follower && !player_array.IsRemote (i)));

    /* First player (leader) can never be killed. */
    pl_kill_button[i]->SetVisible((i == 1) ? false : leader);
  }

  /* Disable not connected players from GUI. */
  for (; i < PL_MAX_PLAYERS; i++) {
    pl_name_label[i]->SetVisible (false);
    pl_race_combo[i]->SetVisible (false);
    pl_kill_button[i]->SetVisible (false);
  }

  /* Something could change. */
  need_redraw->SetTrue ();

  player_array.Unlock ();

  giant->Unlock ();
}

/**
 *  Synchronises player_array with game menu and updates the menu.
 */
void UpdatePlayersAndMenu () {
  player_array.Lock ();

  SynchronisePlayersAndMenu ();
  UpdateGameMenu ();

  giant->Lock ();
  if (host->GetType () == THOST::ht_leader)
    dynamic_cast<TLEADER *>(host)->SendPlayerArray (selected_map_name, false);
  giant->Unlock ();

  player_array.Unlock ();
}


//========================================================================
// Engine Methods
//========================================================================

bool Disconnect() {
  /* Destroy connecting thread if it is running. Do it even when not connected.
   */
#ifdef NEW_GLFW3
	mtx_t mutex;
	thrd_t locked_by;
	cnd_t unlocked;
	connecting_thread_id = NULL;
#else
	if (glfwWaitThread (connecting_thread_id, GLFW_NOWAIT) == GL_FALSE) 
	{
	    Debug (LogMsg ("ok, rusim vlakno %d", connecting_thread_id));
	    glfwDestroyThread (connecting_thread_id);
  	}
  	connecting_thread_id = -1;
#endif
  
  

  /* one never knows... (if the process thread is not still waiting :-) */
  allowed_to_start_process_function = true;

  if (!connected) {
    action_force = false;
    return true;
  }

  // show message box first
  if (config.show_disconnect_warning && !action_force) {
    gui->ShowMessageBox("Do you really want to disconnect?", GUI_MB_YES | GUI_MB_NO);
    return false;
  }

  action_force = false;

  if (started) {
    for (int i = 0; i < player_array.GetCount(); i++){
      if (players[i]->active && !player_array.IsRemote(i)){
        players[i]->active = false;
      }
    }
    StopGame();  // have to be called before play player_array.Clear()
  }

  player_array.Clear();
  connected = false;

  THOST *to_delete = NULL;

  giant->Lock ();
  if (host) {
    to_delete = host;
    host = NULL; 
  }
  giant->Unlock ();

  if (to_delete != NULL) {
    to_delete->RegisterOnDisconnect (NULL);
    delete to_delete;
  }

  return true;
}

bool CreateGame()
{
  /*
   * @@FIXME@@
   */
  if (!Disconnect())
    return false;

  giant->Lock ();

  player_array.AddLocalPlayer (config.player_name);

  // start leader
  host = NEW TLEADER (leader_in_queue_size, config.net_server_port,
                      leader_out_queue_size);

  host->RegisterExtendedFunction (net_protocol_event, ProcessNetEvent);
  host->RegisterExtendedFunction (net_protocol_connect, ProcessConnectRequest);
  host->RegisterExtendedFunction (net_protocol_change_race, ProcessChangeRace);
  host->RegisterExtendedFunction (net_protocol_chat_message, ProcessChatMessage);
  host->RegisterExtendedFunction (net_protocol_synchronise, ProcessSynchronise);
  host->RegisterExtendedFunction (net_protocol_ping, ProcessPingRequest);
  host->RegisterExtendedFunction (net_protocol_disconnect, ProcessDisconnect);

  host->RegisterOnDisconnect (OnDisconnect);

  leader_ready = false;

  host->AddEmptyAddress (); /* player on leader */
  host->AddEmptyAddress (); /* hyper player */

  // XXX: skontrolovat, ci sa podarilo spustit server
  connected = true;

  giant->Unlock ();

  return true;
}

bool Connect (string server)
{
  if (!Disconnect ())
    return false;

  TCONNECT_DATA *data = NEW TCONNECT_DATA;
  data->port = config.net_server_port;
  data->server_name = server;

  /* Create connecting thread. */
#ifdef NEW_GLFW3
	if(thrd_create(&connecting_thread_id, connecting_in_menu_thread, data) == thrd_error)
		Critical ("Error creating thread");
#else
	connecting_thread_id = glfwCreateThread (connecting_in_menu_thread, data);
	 if (connecting_thread_id < 0)
    	Critical ("Error creating thread");
#endif
 
  Debug (LogMsg ("Yep, vytvaram to vlakno %d", connecting_thread_id));

  return true;
}


bool Quit() {
  if (!Disconnect())
    return false;

  state = ST_QUIT;
  return true;
}


//========================================================================
// Menu Callbacks
//========================================================================

/**
 *  Callback function that is called everytime a menu button is clicked.
 *
 *  @param key Key of the button that has called this function.
 */
void MenuButtonOnClick(int key, TGUI_BOX *sender = NULL)
{
#if SOUND
  if (!action_force) sounds_table.sounds[DAT_SID_MENU_BUTTON]->Play();
#endif

  switch (key) {
  case GUI_MB_OK:
  case GUI_MB_YES:
    if (action_key) {
      action_force = true;
      MenuButtonOnClick(action_key);
    }
    gui->HideMessageBox();
    break;

  case GUI_MB_NO:
  case GUI_MB_CANCEL:
    /* If user clicked on Cancel when "Waiting for other players..." */
    if (action_key == MNU_PLAY2) {
      action_force = true;
      state = ST_MAIN_MENU;
      Disconnect();
    }

    /* If user clicked on Cancel when connecting */
    if (action_key == MNU_CONNECT2) {
    	
      if (glfwWaitThread (connecting_thread_id, GLFW_NOWAIT) == GL_FALSE) {
        Debug (LogMsg ("ok, rusim vlakno %d", connecting_thread_id));
        glfwDestroyThread (connecting_thread_id);
      }
      connecting_thread_id = -1;
      action_force = true;
      MenuButtonOnClick(MNU_DISCONNECT, sender);
    }

    action_key = 0;
    gui->HideMessageBox();
    break;

  case MNU_PLAY:
    SetActiveMenu(play_menu);
    state = ST_PLAY_MENU;
    break;

  case MNU_QUIT:
    action_key = MNU_QUIT;

    Quit();
    break;

  case MNU_QUICK_PLAY:
    action_key = MNU_QUICK_PLAY;

    allowed_to_start_process_function = true;

    if (!CreateGame())
      break;

    selected_map_name = map_info_list.GetRandomMap();
    if (!map_info_list.LoadMapInfo(false, selected_map_name.c_str ())) {
      action_key = 0;
      gui->ShowMessageBox ((string ("Error loading map '") + selected_map_name + "'").c_str (), GUI_MB_OK);
      break;
    }

    /* Set race for hyper player. */
    player_array.SetRaceIdName(0, map_info_list.map_ext_info.scheme_id_name);

    {
      int max_players = map_info_list.map_ext_info.max_players;

      if (max_players >= 1) {
        player_array.SetRaceIdName(1, map_info_list.rac_list->id_name);
      }
      if (max_players >= 2) {
        player_array.AddComputerPlayer ();
        host->AddEmptyAddress (); // computer_player
        player_array.SetRaceIdName(2, map_info_list.rac_list->next->id_name);
      }
      /*
      if (max_players >= 3) {
        player_array.AddComputerPlayer ();
        host->AddEmptyAddress (); // computer_player
        player_array.SetRaceIdName(3, map_info_list.rac_list->next->next->id_name);
      }
      */
    }

    state = ST_GAME;
    break;
  
  case MNU_CREDITS:       SetActiveMenu(credits_menu); break;

  case MNU_OPTIONS:       SetActiveMenu(options_menu); break;
  case MNU_VIDEO:
    SetActiveMenu(video_menu);
    state = ST_VIDEO_MENU;
    break;
  case MNU_AUDIO:         SetActiveMenu(audio_menu); break;
  case MNU_RESUME:        state = ST_GAME; break;

  case MNU_CREATE:
    ip_edit->Hide();
    connect_button->Hide();

    ip_label->Show();
    create_button->Show();
    
    SetActiveMenu(ip_menu);
    break;

  case MNU_CONNECT:
    ip_edit->Show();
    connect_button->Show();

    ip_label->Hide();
    create_button->Hide();

    SetActiveMenu(ip_menu);
    ip_edit->Focus();
    break;

  case MNU_KILL_PLAYER:
    player_array.Lock ();

    {
      TGUI_BUTTON *button = (TGUI_BUTTON *)sender;

      for (int i = 1; i < PL_MAX_PLAYERS; i++)
        if (button == pl_kill_button[i]) {
          host->RemoveAddress (i);
          player_array.RemovePlayer (i);
        }
    }

    UpdatePlayersAndMenu ();
    player_array.Unlock ();
    break;

  case MNU_CREATE2:
    action_key = MNU_CREATE2;

    if (CreateGame()) {
      UpdateGameMenu();
      SetActiveMenu(game_menu);
    }
    break;

  case MNU_CONNECT2:
    action_key = MNU_CONNECT2;

    if (Connect(ip_edit->GetText())) {
      UpdateGameMenu();
      SetActiveMenu(game_menu);
    }
    break;

  case MNU_PLAY2:
    player_array.Lock ();

    allowed_to_start_process_function = player_array.AllPlayersAreLocal ();

    {
      int max_players = map_info_list.map_ext_info.max_players;
      enum { none, too_many_players, not_different_races } error = none;

      if (player_array.GetCount () > max_players + 1)
        error = too_many_players;
      else if (!player_array.EveryPlayerHasDifferentRace ())
        error = not_different_races;

      if (error != none) {
        switch (error) {
        case too_many_players:
          action_key = 0;
          gui->ShowMessageBox ((string ("Too many players! Maximum count\n of players for this map is ") + char(max_players + '0') + ".").c_str(), GUI_MB_OK);
          break;
        case not_different_races:
          action_key = 0;
          gui->ShowMessageBox ("Every player must have different race!", GUI_MB_OK);
          break;
        case none:
          /* Will never happen. */
          break;
        }
        player_array.Unlock ();
        break;
      }

      giant->Lock ();

      /* Leader will inform all followers that the game is starting. */
      if (host->GetType () == THOST::ht_leader) {
        TLEADER *leader = dynamic_cast<TLEADER *>(host);

        /* XXX: netreba... leader->FillRemoteAddresses (); */
        leader->SendPlayerArray (selected_map_name, true);
      }
 
      state = ST_GAME;

      giant->Unlock ();

      /* XXX: tu je potrebne este nejako zablokovat, aby sa uz nepridali dalsi
       *      hraci... Takze nieco ako player_array.NoMoreChanges (); */
    }

    player_array.Unlock ();
    break;

  case MNU_DISCONNECT:
    action_key = MNU_DISCONNECT;

    if (Disconnect()) {
      resume_button->SetEnabled(false);
      disconnect_button->SetEnabled(false);
      SetActiveMenu(play_menu);

      // clear menu structures
      map_info_list.ClearRacList();
    }
    break;

  case MNU_MAIN:          SetActiveMenu(main_menu); break;
  }
}


/**
 *  Callback function that is called everytime a menu button is clicked.
 *
 *  @param sender Button object that has called this function.
 */
void MenuButtonOnClick(TGUI_BOX *sender)
{
    MenuButtonOnClick(sender->GetKey(), sender);
}


/**
 *  This function is called when we are in menu (#state == #ST_MENU) and a key
 *  was pressed.
 *
 *  @param key  GLFW key identifier of released key.
 */
void MenuOnKeyDown(int key)
{
  if (gui->KeyDown(key)) return;

  switch (key) {
  // On ESC we end Menu and change to Quit.
  case GLFW_KEY_ESC:
    if (main_menu->IsVisible())
      MenuButtonOnClick(MNU_QUIT);

    else if (video_menu->IsVisible() || (audio_menu && audio_menu->IsVisible()))
      MenuButtonOnClick(MNU_OPTIONS);

    else if (ip_menu->IsVisible())
      MenuButtonOnClick(MNU_PLAY);

    else if (game_menu->IsVisible())
      MenuButtonOnClick(MNU_DISCONNECT);

    else MenuButtonOnClick(MNU_MAIN);

    break;

  case GLFW_KEY_ENTER:
  case GLFW_KEY_KP_ENTER:
    if (ip_menu->IsVisible())
    {
      if (create_button->IsVisible()) MenuButtonOnClick(MNU_CREATE2);
      else MenuButtonOnClick(MNU_CONNECT2);
    }
    break;
  }
}


/**
 *  Callback function that is called everytime a menu checkbox button is
 *  clicked.
 *
 *  @param key Key of the checkbox button that has called this function.
 */
void MenuCheckBoxOnClick(int key)
{
#if SOUND
  sounds_table.sounds[DAT_SID_MENU_CONTROL]->Play();
#endif

  switch (key) {
  case MNU_FULLSCREEN:
    config.fullscreen = !config.fullscreen;
    config.file->WriteBool(const_cast<char*>("fullscreen"), config.fullscreen);
    break;

  case MNU_VERT_SYNC:
    config.vert_sync = !config.vert_sync;
    config.file->WriteBool(const_cast<char*>("vert_sync"), config.vert_sync);
    #if UNIX
      /* We are not going to synchronise with monitor refresh rate on UNIX, because
      * it seems xorg does not support this feature and the program aborts. This
      * is a bug of glfw, which tries to set nonexistent feature. */
      glfwSwapInterval(0);
    #else
      glfwSwapInterval(config.vert_sync ? 1 : 0);
    #endif
    break;

  case MNU_640:
    config.scr_width = 640;
    config.scr_height = 480;
    config.file->WriteStr(const_cast<char*>("resolution"), const_cast<char*>("640x480"));
    glfwSetWindowSize(640, 480);
    state = ST_RESET_VIDEO_MENU;
    break;

  case MNU_800:
    config.scr_width = 800;
    config.scr_height = 600;
    config.file->WriteStr(const_cast<char*>("resolution"), const_cast<char*>("800x600"));
    glfwSetWindowSize(800, 600);
    state = ST_RESET_VIDEO_MENU;
    break;

  case MNU_1024:
    config.scr_width = 1024;
    config.scr_height = 768;
    config.file->WriteStr(const_cast<char*>("resolution"), const_cast<char*>("1024x768"));
    glfwSetWindowSize(1024, 768);
    state = ST_RESET_VIDEO_MENU;
    break;

  case MNU_1152:
    config.scr_width = 1152;
    config.scr_height = 864;
    config.file->WriteStr(const_cast<char*>("resolution"), const_cast<char*>("1152x864"));
    glfwSetWindowSize(1152, 864);
    state = ST_RESET_VIDEO_MENU;
    break;

  case MNU_1280:
    config.scr_width = 1280;
    config.scr_height = 1024;
    config.file->WriteStr(const_cast<char*>("resolution"), const_cast<char*>("1280x1024"));
    glfwSetWindowSize(1280, 1024);
    state = ST_RESET_VIDEO_MENU;
    break;

  case MNU_1600:
    config.scr_width = 1600;
    config.scr_height = 1200;
    config.file->WriteStr(const_cast<char*>("resolution"), const_cast<char*>("1600x1200"));
    glfwSetWindowSize(1600, 1200);
    state = ST_RESET_VIDEO_MENU;
    break;

  case MNU_TF_NEAREST:
    config.tex_mag_filter = GL_NEAREST;

    if (config.tex_min_filter == GL_LINEAR) config.tex_min_filter = GL_NEAREST;
    else if (config.tex_min_filter == GL_LINEAR_MIPMAP_NEAREST) config.tex_min_filter = GL_NEAREST_MIPMAP_NEAREST;
    else if (config.tex_min_filter == GL_LINEAR_MIPMAP_LINEAR) config.tex_min_filter = GL_NEAREST_MIPMAP_LINEAR;
    config.file->WriteStr(const_cast<char*>("texture_filter"), const_cast<char*>("nearest"));
    break;

  case MNU_TF_LINEAR:
    config.tex_mag_filter = GL_LINEAR;

    if (config.tex_min_filter == GL_NEAREST) config.tex_min_filter = GL_LINEAR;
    else if (config.tex_min_filter == GL_NEAREST_MIPMAP_NEAREST) config.tex_min_filter = GL_LINEAR_MIPMAP_NEAREST;
    else if (config.tex_min_filter == GL_NEAREST_MIPMAP_LINEAR) config.tex_min_filter = GL_LINEAR_MIPMAP_LINEAR;
    config.file->WriteStr(const_cast<char*>("texture_filter"), const_cast<char*>("linear"));
    break;

  case MNU_MF_NONE:
    config.tex_min_filter = config.tex_mag_filter;
    config.file->WriteStr(const_cast<char*>("mipmap_filter"), const_cast<char*>("none"));
    break;

  case MNU_MF_NEAREST:
    if (config.tex_mag_filter == GL_NEAREST) config.tex_min_filter = GL_NEAREST_MIPMAP_NEAREST;
    else config.tex_min_filter = GL_LINEAR_MIPMAP_NEAREST;
    config.file->WriteStr(const_cast<char*>("mipmap_filter"), const_cast<char*>("nearest"));
    break;

  case MNU_MF_LINEAR:
    if (config.tex_mag_filter == GL_NEAREST) config.tex_min_filter = GL_NEAREST_MIPMAP_LINEAR;
    else config.tex_min_filter = GL_LINEAR_MIPMAP_LINEAR;
    config.file->WriteStr(const_cast<char*>("mipmap_filter"), const_cast<char*>("linear"));
    break;

#if SOUND

  case MNU_MENU_MUSIC:
    config.snd_menu_music = !config.snd_menu_music;
    config.file->WriteBool(const_cast<char*>("snd_menu_music"), config.snd_menu_music);

    if (config.snd_menu_music) sounds_table.sounds[DAT_SID_MENU_MUSIC]->Play();
    else sounds_table.sounds[DAT_SID_MENU_MUSIC]->Stop();
    break;

  case MNU_GAME_MUSIC:
    config.snd_game_music = !config.snd_game_music;
    config.file->WriteBool(const_cast<char*>("snd_game_music"), config.snd_game_music);
    break;

  case MNU_UNIT_SPEECH:
    config.snd_unit_speech = !config.snd_unit_speech;
    config.file->WriteBool(const_cast<char*>("snd_unit_speech"), config.snd_unit_speech);
    break;

#endif

  }

  config.file->Save();
}


/**
 *  Callback function that is called everytime a menu checkbox button is
 *  clicked.
 *
 *  @param sender Checkbox button object that has called this function.
 */
void MenuCheckBoxOnClick(TGUI_BOX *sender)
{
  MenuCheckBoxOnClick(sender->GetKey());
}


void MenuPanelOnDraw(TGUI_BOX *sender)
{
  glDisable(GL_TEXTURE_2D);

  glColor3f(0.8f, 0.8f, 0.8f);
  glBegin(GL_LINE_STRIP);
    glVertex2f(0, 0);
    glVertex2f(0, sender->GetHeight() - 1);
    glVertex2f(sender->GetWidth() - 1, sender->GetHeight() - 1);
  glEnd();

  glBegin(GL_LINE_STRIP);
    glVertex2f(sender->GetWidth() - 1, sender->GetHeight());
    glVertex2f(sender->GetWidth() - 1, 0);
    glVertex2f(0, 0);
  glEnd();

  glEnable(GL_TEXTURE_2D);
}

void MenuUpdateMapInfo (string map_name) {
  char buff[4096];
  TMAP_RAC_INFO_NODE * act_rac;
  int i, count_rac;
  string races;

  player_array.Lock ();

  map_info_list.ClearRacList();
        
  if (map_info_list.LoadMapInfo(false, map_name.c_str ())) { // selected map exists
    selected_map_name = map_name; // put map id_name to global variable ... if menu quits, this map will be loaded

    map_label->SetCaption (map_info_list.GetMapName (map_name).c_str ());

    play_button->SetEnabled(true);

    // author
    map_author_label->SetCaption(map_info_list.map_ext_info.author);

    // scheme
    map_scheme_label->SetCaption(map_info_list.map_ext_info.scheme_name);
    player_array.SetRaceIdName(0, map_info_list.map_ext_info.scheme_id_name);
    
    // map size
    sprintf(buff, "%dx%d", map_info_list.map_ext_info.width, map_info_list.map_ext_info.height);
    map_size_label->SetCaption(buff);

    // races
    count_rac = 0;
    for (act_rac=map_info_list.rac_list; act_rac != NULL; act_rac = act_rac->next){
      races += string (act_rac->name) + "\n";
      count_rac++;
    }
    races.erase (races.end () - 1, races.end());   // Erase the last "\n".

    map_races_label->SetCaption(races.c_str ());
    map_races_label->SetPosY(40 - map_races_label->GetHeight() + map_races_label->GetLineHeight());

    sprintf(buff, "%d", MIN(map_info_list.map_ext_info.max_players, count_rac));
    map_players_label->SetCaption(buff);

    for (i = 1; i < PL_MAX_PLAYERS; i++)
      pl_race_combo[i]->SetItems(races.c_str ());

    SynchronisePlayersAndMenu ();
  }
  else{ // can not find file of selected map
    map_label->SetCaption ("Missing or corrupted map");

    // clear global map name and disable play button
    selected_map_name = "";
    play_button->SetEnabled(false);
    
    // set empty info map labels
    map_scheme_label->SetCaption("");
    map_size_label->SetCaption("");
    map_author_label->SetCaption("");
    map_players_label->SetCaption("");
    map_races_label->SetCaption("");

    // hide combos and buttons
    for (i = 1; i < PL_MAX_PLAYERS; i++){
      pl_race_combo[i]->Hide();
      pl_kill_button[i]->Hide();
      pl_name_label[i]->Hide();

      pl_race_combo[i]->SetItems("");
    }
  }

  player_array.Unlock ();
}

void MenuListOnChange(TGUI_BOX *sender, int item_index)
{
  TMAP_BASIC_INFO_NODE * act;
  int i;

  if (sender->GetKey() == MNU_MAP_LIST) {
    for (act = map_info_list.map_list, i = 0; (act != NULL) && (i < item_index); act = act->next, i++); // finds pointer to item_index.th map

    if (act != NULL) {
      MenuUpdateMapInfo (act->id_name);
      UpdatePlayersAndMenu ();
    }
  }
}


/**
 *  Callback function called everytime a player race has been changed in combo
 *  box. It updates player race in #player_array.
 *
 *  @param sender     Sender of the message which is the list in combo box.
 *  @param item_index Index of item in the list.
 */
void MenuPlayerRaceComboOnChange (TGUI_BOX *sender, int item_index) {
  
  player_array.Lock();

  TGUI_LIST_BOX *list = (TGUI_LIST_BOX *)sender;

  for (int i = 1; i < PL_MAX_PLAYERS; i++)
    if (list == pl_race_combo[i]->GetList ()) {
      string selected_race_name = list->GetItem (item_index);
      string selected_race_id = map_info_list.GetRacIdName (selected_race_name);
      player_array.SetRaceIdName (i, selected_race_id);

      giant->Lock ();

      if (host->GetType () == THOST::ht_follower)
        dynamic_cast<TFOLLOWER *>(host)->ChangeRace (i,
            player_array.GetPlayerName (i), selected_race_id);

      giant->Unlock ();

      break;
    }

  UpdatePlayersAndMenu ();
  
  player_array.Unlock();
}


void MenuPanelOnShow(TGUI_BOX *sender)
{
  giant->Lock ();

  if (sender == game_menu) {
    /* If the actual host is a leader, set map_list listbox content according
     * to map_info_list and read extended information about the first map in
     * the list. */
    if (host && host->GetType () == THOST::ht_leader) {
      TMAP_BASIC_INFO_NODE *act;
      string maps;

      // fill full map names into string separated by 'newline'
      if (map_info_list.map_list){
        for (act = map_info_list.map_list; act != NULL; act = act->next)
          maps += string (act->name) + "\n";
        maps.erase (maps.end () - 1, maps.end());   // Erase the last "\n".
      }
      else
        maps = "No maps";

      map_list->SetItems(maps.c_str ()); // assign string to menu structure
      
      if (map_info_list.map_list) 
        MenuListOnChange(map_list, 0); // read extended information about first map in list
    }
  }

  giant->Unlock ();
}


void MenuEditOnChange(TGUI_BOX *sender, char *text)
{
  switch (sender->GetKey()) {
  case MNU_PLAYER_NAME:
    strcpy(config.player_name, text);
    config.file->WriteStr(const_cast<char*>("player_name"), text);
    break;

  case MNU_IP:
    strcpy(config.address, text);
    config.file->WriteStr(const_cast<char*>("address"), text);
    connect_button->SetEnabled(*text > 0);
    break;
  }

  config.file->Save();
}


#if SOUND

void MenuSliderOnChange(TGUI_BOX *sender, GLfloat pos)
{
  switch (sender->GetKey()) {
  case MNU_MASTER_VOL:
    SetMasterVolume(T_BYTE(pos * 100));
    config.file->WriteInt(const_cast<char*>("snd_master_volume"), config.snd_master_volume);
    break;

  case MNU_MENU_SOUND_VOL:
    config.snd_menu_sound_volume = T_BYTE(pos * 100);
    ChangeSoundVolume(config.snd_menu_sound_volume);
    config.file->WriteInt(const_cast<char*>("snd_menu_sound_volume"), config.snd_menu_sound_volume);
    break;

  case MNU_MENU_MUSIC_VOL:
    SetMenuMusicVolume(T_BYTE(pos * 100));
    config.file->WriteInt(const_cast<char*>("snd_menu_music_volume"), config.snd_menu_music_volume);
    break;

  case MNU_GAME_SOUND_VOL:
    config.snd_game_sound_volume = T_BYTE(pos * 100);
    config.file->WriteInt(const_cast<char*>("snd_game_sound_volume"), config.snd_game_sound_volume);
    break;

  case MNU_GAME_MUSIC_VOL:
    SetGameMusicVolume(T_BYTE(pos * 100));
    config.file->WriteInt(const_cast<char*>("snd_game_music_volume"), config.snd_game_music_volume);
    break;
  }

  config.file->Save();
}

#endif


//========================================================================
// Game Mouse Callbacks
//========================================================================

/**
 *   Callback function for events in game.
 */
void GameOnMouseDown(TGUI_BOX *sender, GLfloat x, GLfloat y, int button)
{
  if (!started) return;

  switch (button) {
  case GLFW_MOUSE_BUTTON_LEFT:
    mouse.down_x = mouse.x;
    mouse.down_y = mouse.y;

    if (mouse.cursor_id == MC_ARROW || 
      mouse.cursor_id == MC_CAN_MOVE || mouse.cursor_id == MC_CAN_BUILD || 
      mouse.cursor_id == MC_CANT_MOVE || mouse.cursor_id == MC_CANT_BUILD)
    {
      mouse.down_cursor_id = MC_ARROW;
      mouse.down_circle_id = MCC_NONE;
    }
    else {
      mouse.down_cursor_id = MC_SELECT;
      mouse.down_circle_id = MCC_ARROWS_IN;
    }

    mouse.action = UA_NONE;
    myself->build_item = NULL;

    mouse.draw_selection = true;
    break;

  case GLFW_MOUSE_BUTTON_MIDDLE:
    if (!radar.GetMoving()) map.drag_moving = true;
    break;

  case GLFW_MOUSE_BUTTON_RIGHT:
    switch (mouse.cursor_id) {
    case MC_SELECT:
    case MC_CAN_MOVE:
    case MC_CANT_MOVE:

      if (selection->MoveUnits(mouse.map_pos)) 
      {
        mouse.action = UA_NONE;
      }
      break;

    case MC_CAN_ATTACK:
    case MC_CANT_ATTACK:
    case MC_CAN_MINE:
    case MC_CANT_MINE:
    case MC_CAN_REPAIR:
    case MC_CANT_REPAIR:
    case MC_CAN_HIDE:
    case MC_CANT_HIDE:

      if (selection->ReactUnits()) {
        mouse.action = UA_NONE;
      }
      break;

    case MC_CAN_BUILD:
    case MC_CANT_BUILD:
      {
        TBUILDING_UNIT *building;

        if (myself->build_item && ((building = ((TWORKER_UNIT *)selection->GetFirstUnit())->StartBuild(myself->build_item, mouse.map_pos, false)))) {

          // other units iterract with new building
          TNODE_OF_UNITS_LIST *ul = selection->GetUnitsList();
          for (; ul; ul = ul->next)
            (ul->unit != mouse.over_unit && ul->unit->SelectReaction(building, UA_REPAIR));

          // reset mouse action
          myself->build_item = NULL;
          mouse.action = UA_NONE;
          UncheckBuildButton();
        }
      }
      break;

    case MC_EJECT:
      mouse.over_unit->EjectUnits();

      mouse.action = UA_NONE;
      break;

    default: break;
    }

#if SOUND
    // error sound
    switch (mouse.cursor_id) {
    case MC_CANT_MOVE:
    case MC_CANT_ATTACK:
    case MC_CANT_MINE:
    case MC_CANT_REPAIR:
    case MC_CANT_BUILD:
      myself->race->snd_error.Play();
      break;

    default: break;
    }
#endif

    break;
  }
}


/**
 *   Callback function for events in game.
 */
void GameOnMouseUp(TGUI_BOX *sender, GLfloat x, GLfloat y, int button)
{
  if (!started) return;

  switch (button) {
  case GLFW_MOUSE_BUTTON_LEFT:

    if (mouse.draw_selection) {
      if (mouse.cursor_id != MC_SELECT_PLUS)
        selection->UnselectAll();   // unselects all selected units

      // mouse selection
      if (fabs(x - mouse.down_x) > MC_SELECT_RADIUS || fabs(y - mouse.down_y) > MC_SELECT_RADIUS) {
        mouse.RectSelect();

        if (selection->IsEmpty() && mouse.over_unit)                 // selects all unit in selection rectangle
          selection->SelectUnit(mouse.over_unit);
      }
      else {
        if (mouse.cursor_id == MC_SELECT_PLUS)
          selection->AddDeleteUnit(mouse.over_unit);

        else if (mouse.cursor_id == MC_SELECT)
          selection->SelectUnit(mouse.over_unit);
      }

      mouse.draw_selection = false;       // end of selection rectangle drawing
    }

    // map moving by radar
    if (radar.GetMoving()) radar.SetMoving(false);
    break;

  case GLFW_MOUSE_BUTTON_MIDDLE:
    map.drag_moving = false;
    break;

  case GLFW_MOUSE_BUTTON_RIGHT:
    break;
    }
}


/**
 *  Callback function for click events on some of the game GUI buttons.
 *
 *  @param key Key of the game GUI button that has called this function.
 */
void GameButtonOnClick(int key)
{
  if (!started) return;

  switch (key) {
  case MNU_TOGGLE_PANEL:
    ToggleMainPanel(); break;

  case MNU_TOGGLE_RADAR:
    ToggleRadarPanel(); break;

  case MNU_QUIT:
    action_key = MNU_QUIT;
    Quit();
    break;

  case MNU_VIEW_SEGMENT:
    view_segment++;
    if (view_segment > DRW_ALL_SEGMENTS) view_segment = 0;
    panel_info.seg_button->SetTexture(GUI_BS_UP, gui_table.GetTexture(DAT_TGID_SEG_BUTTONS, view_segment));    
    break;

  case MNU_ACTION_STAY:
    myself->build_item = NULL;
    mouse.action = UA_NONE;
    UpdateGuardButtons();
    selection->StopUnits();
    selection->UpdateInfo(true);
    if (panel_info.stay_button->IsChecked())
      ChangeActionPanel(MNU_PANEL_STAY);
    break;

  case MNU_ACTION_MOVE:
    myself->build_item = NULL;
    mouse.action = UA_MOVE;
    ChangeActionPanel(MNU_PANEL_MOVE);
    break;

  case MNU_ACTION_ATTACK:
    myself->build_item = NULL;
    mouse.action = UA_ATTACK;
    ChangeActionPanel(MNU_PANEL_ATTAK);
    break;

  case MNU_ACTION_MINE:
    myself->build_item = NULL;
    mouse.action = UA_MINE;
    ChangeActionPanel(MNU_PANEL_MINE);
    break;

  case MNU_ACTION_REPAIR:
    myself->build_item = NULL;
    mouse.action = UA_REPAIR;
    ChangeActionPanel(MNU_PANEL_REPAIR);
    break;

  case MNU_ACTION_BUILD:
    mouse.action = UA_NONE;
    CreateBuildButtons();
    ChangeActionPanel(MNU_PANEL_BUILD);
    break;
  }
}


/**
 *  Callback function for mouse events on some of the game GUI buttons.
 *
 *  @param sender Game GUI button that has called this function.
 */
void GameButtonOnMouseDown(TGUI_BOX *sender, GLfloat, GLfloat, int button)
{
  if (!started) return;
  if (button != GLFW_MOUSE_BUTTON_LEFT) return;

  switch (sender->GetKey()) {
  case MNU_ACTION_STAY:
    if (panel_info.stay_button->IsChecked()) {
      GameButtonOnClick(MNU_ACTION_STAY);
    }
    break;

  case MNU_ACTION_MOVE:
    if (panel_info.move_button->IsChecked()) {
      GameButtonOnClick(MNU_ACTION_MOVE);
    }
    break;

  case MNU_ACTION_ATTACK:
    if (panel_info.attack_button->IsChecked()) {
      GameButtonOnClick(MNU_ACTION_ATTACK);
    }
    break;

  case MNU_ACTION_MINE:
    if (panel_info.mine_button->IsChecked()) {
      GameButtonOnClick(MNU_ACTION_MINE);
    }
    break;

  case MNU_ACTION_REPAIR:
    if (panel_info.repair_button->IsChecked()) {
      GameButtonOnClick(MNU_ACTION_REPAIR);
    }
    break;

  default:
    break;
  }
}


/**
 *  Callback function for click events on some of the game GUI buttons.
 *
 *  @param sender Game GUI button that has called this function.
 */
void GameButtonOnClick(TGUI_BOX *sender)
{
  GameButtonOnClick(sender->GetKey());
}


/**
 *   Callback function for events of build buttons.
 *
 *   @param key   Key of menu object that has called this function.
 */
void GameBuildOnClick(TGUI_BOX *sender)
{
  if (!started) return;

  mouse.action = UA_BUILD;
  myself->build_item = (TBUILDING_ITEM *)sender->GetKey();
  panel_info.act_build_button = (TGUI_BUTTON *)sender;
}


void GameStayOnClick(TGUI_BOX *sender)
{
  if (!started) return;

  selection->SetAggressivity(TAGGRESSIVITY_MODE(sender->GetKey() - MNU_GUARD_BUTTON));
}


void GameProduceOnClick(TGUI_BOX *sender)
{
  if (!started) return;

  static_cast<TFACTORY_UNIT *>(selection->GetFirstUnit())->AddUnitToOrder((TFORCE_ITEM *)sender->GetKey());
  CreateOrderButtons();
}


void GameOrderOnClick(TGUI_BOX *sender)
{
  if (!started) return;

  static_cast<TFACTORY_UNIT *>(selection->GetFirstUnit())->TogglePausedProducing();
}


void GameOrderOnMouseDown(TGUI_BOX *sender, GLfloat x, GLfloat y, int button)
{
  if (button == GLFW_MOUSE_BUTTON_RIGHT) {
    static_cast<TFACTORY_UNIT *>(selection->GetFirstUnit())->CancelProducing(panel_info.order_panel->GetChildOrder(sender));
    CreateOrderButtons();
  }
}


void GameBuildOnDraw(TGUI_BOX *sender)
{
  if (!started) return;

  TGUI_BUTTON *b = (TGUI_BUTTON *)sender;

  if (!b->IsChecked()) return;

  glDisable(GL_TEXTURE_2D);

  // frame rectangle
  glColor3f(0.7f, 0.6f, 0.4f);
  glBegin(GL_LINE_STRIP);
    glVertex2f(0.0f, 0.0f);
    glVertex2f(0.0f, sender->GetHeight() - 1);
    glVertex2f(sender->GetWidth(), sender->GetHeight() - 1);
  glEnd();

  glBegin(GL_LINE_STRIP);
    glVertex2f(sender->GetWidth() - 1, sender->GetHeight());
    glVertex2f(sender->GetWidth() - 1, 0.0f);
    glVertex2f(0.0f, 0.0f);
  glEnd();

  glEnable(GL_TEXTURE_2D);
}


void GameBuildOnTooltip(TGUI_BOX *sender)
{
  if (!started) return;

  TBASIC_ITEM *itm = (TBASIC_ITEM *)(sender->GetKey());

  // unit name
  build_tooltip->SetText(itm->name);

  // ancestor of buildings
  build_tooltip->GetText2()->Hide();
  TBUILDING_ITEM *bitm = dynamic_cast<TBUILDING_ITEM *>(itm);
  if (bitm) {
    if (bitm->ancestor) {
      build_tooltip->SetText2(bitm->ancestor->name);
      build_tooltip->GetText2()->Show();
    }
  }

  // materials
  for (int i = 0; i < scheme.materials_count; i++)
    build_tooltip->SetInfo(i, (int)itm->materials[i]);

  // aids
  build_tooltip->SetInfo(scheme.materials_count + 0, itm->food < 0 ? -itm->food : 0);
  build_tooltip->SetInfo(scheme.materials_count + 1, itm->energy < 0 ? -itm->energy : 0);
}


//========================================================================
// Game Key Callbacks
//========================================================================

/**
 *  This function is called when we are in game (#state == #ST_GAME) and a key
 *  was pressed.
 *
 *  @param key  GLFW key identifier of released key.
 */
void GameOnKeyDown(int key)
{
  if (!started) return;
  if (gui->KeyDown(key)) return;

  switch (key) {
 
  case GLFW_KEY_ESC:
    if (chat_panel->IsVisible()) ToggleChatPanel();
    else if (mouse.draw_selection) mouse.draw_selection = false;
    else if (mouse.action == UA_NONE) {
      radar.SetMoving(false);
      state = ST_PLAY_MENU;
    }
    else {
      myself->build_item = NULL;
      mouse.action = UA_NONE;

      // uncheck build button
      UncheckBuildButton();

      selection->UpdateAction();
    }
    break;

  case GLFW_KEY_ENTER:
  case GLFW_KEY_KP_ENTER:
    if (chat_panel->IsVisible()) {
      char txt[1024];
      sprintf(txt, "%s: %s", myself->name, chat_edit->GetText());
      host->SendChatMessage(chat_edit->GetText());
      ost->AddText(txt);
      ToggleChatPanel();
    }
    break;

  case GLFW_KEY_TAB:
    ToggleMainPanel();
    break;

  // On 'Q' we change the state of the game to Quit.
  case 'Q':
    GameButtonOnClick(MNU_QUIT);
    break;
  
  case 'Y':
  case 'U':
    ToggleChatPanel();
    break;

  case 'S':
    if (panel_info.stay_button->IsEnabled()) {
      panel_info.stay_button->SetChecked(true);
      GameButtonOnClick(MNU_ACTION_STAY);
    }
    break;

  case 'M':
    if (panel_info.move_button->IsEnabled()) {
      panel_info.move_button->SetChecked(true);
      GameButtonOnClick(MNU_ACTION_MOVE);
    }
    break;

  case 'A':
    if (panel_info.attack_button->IsEnabled()) {
      panel_info.attack_button->SetChecked(true);
      GameButtonOnClick(MNU_ACTION_ATTACK);
    }
    break;

  case 'I':
    if (panel_info.mine_button->IsEnabled()) {
      panel_info.mine_button->SetChecked(true);
      GameButtonOnClick(MNU_ACTION_MINE);
    }
    break;

  case 'R':
    if (panel_info.repair_button->IsEnabled()) {
      panel_info.repair_button->SetChecked(true);
      GameButtonOnClick(MNU_ACTION_REPAIR);
    }
    break;

  case 'B':
    if (panel_info.build_button->IsEnabled()) {
      panel_info.build_button->SetChecked(true);
      GameButtonOnClick(MNU_ACTION_BUILD);
    }
    break;

  case 'G':
    reduced_drawing = !reduced_drawing;
    break;

  /*
   * RIGHT, LEFT, DOWN and UP starts to move the map in the selected
   * direction. The map stops to move, when the apropriate key is released.
   */
  case GLFW_KEY_LEFT:
    map.StartKeyMove(MAP_KEY_MOVE_RIGHT);
    break;
  case GLFW_KEY_RIGHT:
    map.StartKeyMove(MAP_KEY_MOVE_LEFT);
    break;
  case GLFW_KEY_UP:
    map.StartKeyMove(MAP_KEY_MOVE_DOWN);
    break;
  case GLFW_KEY_DOWN:
    map.StartKeyMove(MAP_KEY_MOVE_UP);
    break;

  /*
   * Map can be moved with mouse while ALT key is pressed.
   */
  case GLFW_KEY_LALT:
    if (!radar.GetMoving()) map.mouse_moving = true;
    break;

  /*
   * On '+' we zoom in the map. Both '+'s (on numeric keyboard and on
   * standard keyboard) are recognized. Also the key '=' is recognized as
   * '+', because it's on the same key as standard '+', but one does not
   * need to press the SHIFT key, too.
   */
  case GLFW_KEY_KP_ADD:
  case '+':
  case '=':
    map.Zoom(MAP_ZOOM_IN);
    break;

  /*
   * On '-' we zoom out the map. Both '-'s (on numeric keyboard and on
   * standard keyboard) are recognized.
   */
  case GLFW_KEY_KP_SUBTRACT:
  case '-':
    map.Zoom(MAP_ZOOM_OUT);
    break;

  /*
   * On '*' we reset zoom of the map. Both '*'s (on numeric keyboard and on
   * standard keyboard) are recognized.
   */
  case GLFW_KEY_KP_DIVIDE:
  case '/':
    map.Zoom(MAP_ZOOM_RESET);
    break;

  /*
   * With 'F5' - 'F8' you can change which segment should be shown.
   */
  case GLFW_KEY_F5:
  case GLFW_KEY_F6:
  case GLFW_KEY_F7:
  case GLFW_KEY_F8:
    view_segment = key - GLFW_KEY_F5;
    panel_info.seg_button->SetTexture(GUI_BS_UP, gui_table.GetTexture(DAT_TGID_SEG_BUTTONS, view_segment));
    break;

  case GLFW_KEY_F9:
    MenuCheckBoxOnClick(MNU_640);
    break;
  case GLFW_KEY_F10:
    MenuCheckBoxOnClick(MNU_800);
    break;
  case GLFW_KEY_F11:
    MenuCheckBoxOnClick(MNU_1024);
    break;


#if DEBUG
  case GLFW_KEY_INSERT:
    queue_events->LogQueue();
    break;

  case GLFW_KEY_BACKSPACE:
    show_all = !show_all;
    break;
#endif


  case GLFW_KEY_F1:
  case GLFW_KEY_F2:
  case GLFW_KEY_F3:
  case GLFW_KEY_F4:    
    if (!selection->IsEmpty()) {
      selection->SetAggressivity(TAGGRESSIVITY_MODE(key - GLFW_KEY_F1));
      if (panel_info.stay_button->IsChecked())
        selection->UpdateAction();
    }
    break;
  
  /*
   * With numbers you can manipulate with groups of units.
   */
  case '1':
  case '2':
  case '3':
  case '4':
  case '5':
  case '6':
  case '7':
  case '8':
  case '9':
    {
      static double key_time[9] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };

      key -= '1';

      if (glfwGetKey(GLFW_KEY_LCTRL))
        selection->StoreSelection(key);
      else {
        double delta = glfwGetTime() - key_time[key];
        bool double_key = delta < 0.5;
        bool center = glfwGetKey(GLFW_KEY_LALT) == GL_TRUE || double_key;

        key_time[key] += delta;

        selection->RestoreSelection(key, center, !double_key);
      }
      break;
    }
  }
}


/**
 *  This function is called when we are in game (#state == #ST_GAME) and a key
 *  was released.
 *
 *  @param key  GLFW key identifier of released key.
 */
void GameOnKeyUp(int key)
{
  if (!started) return;
  switch (key) {

  /*
   * The map stops it's move, when a key LEFT, RIGHT, UP or DOWN was
   * released. The map started it's move, when the same key was pressed.
   */
  case GLFW_KEY_LEFT:
    map.StopKeyMove(MAP_KEY_MOVE_RIGHT);
    break;
  case GLFW_KEY_RIGHT:
    map.StopKeyMove(MAP_KEY_MOVE_LEFT);
    break;
  case GLFW_KEY_UP:
    map.StopKeyMove(MAP_KEY_MOVE_DOWN);
    break;
  case GLFW_KEY_DOWN:
    map.StopKeyMove(MAP_KEY_MOVE_UP);
    break;

  /*
   * Map moving with mouse is stopped when ALT key is released.
   */
  case GLFW_KEY_LALT:
    map.mouse_moving = false;
    break;
  }
}


//========================================================================
// Radar Callbacks
//========================================================================

/**
 *   Callback function for mouse moving in radar.
 */
void GameOnRadarMouseMove(TGUI_BOX *sender, GLfloat x, GLfloat y)
{
  if (radar.GetMoving()) {
    GLfloat koef = radar.zoom / (GLfloat)DAT_MAPEL_DIAGONAL_SIZE;

    map.SetPosition(-(x - radar.dx) / koef, -(y / koef / 2));
  }
}


/**
 *   Callback function for mouse events in radar.
 */
void GameOnRadarMouseDown(TGUI_BOX *sender, GLfloat x, GLfloat y, int button)
{
  if (!started) return;

  switch (button) {
  case GLFW_MOUSE_BUTTON_LEFT:
    if (!map.mouse_moving && !map.drag_moving) {
      radar.SetMoving(true);
      GameOnRadarMouseMove(sender, x, y);
    }
    break;
  }
}


void GameOnRadarDraw(TGUI_BOX *sender)
{
  if (!started) return;

  radar.Draw();
  MenuPanelOnDraw(sender);
}


//========================================================================
// Callbacks for network messages
//========================================================================

/**
 *  Callback function which is called whenever a connect request is received.
 */
static void ProcessConnectRequest (TNET_MESSAGE *msg) {
  giant->Lock ();

  if (host == NULL) {
    giant->Unlock ();
    return;
  }

  in_port_t port;
  string player_name;

  msg->Extract (&port, sizeof (port));
  player_name = msg->ExtractString ();

  player_array.Lock ();

  dynamic_cast<TLEADER *>(host)->ConnectFollower (msg->GetAddress (), port, msg->GetFileDescriptor ());

  player_array.AddRemotePlayer (player_name, msg->GetAddress (), port);
  UpdatePlayersAndMenu ();

  player_array.Unlock ();

  giant->Unlock ();
}

/**
 *  Callback function which is called whenever a change of a race request is
 *  received.
 */
static void ProcessChangeRace (TNET_MESSAGE *msg) {
  giant->Lock ();

  if (host == NULL) {
    giant->Unlock ();
    return;
  }

  int index = msg->ExtractByte ();
  string name = msg->ExtractString ();
  string new_race = msg->ExtractString ();

  player_array.Lock ();

  if (player_array.GetPlayerName (index) != name) {
    Warning ("Players not synchronised");
    player_array.Unlock ();
    giant->Unlock ();
    return;
  }

  player_array.SetRaceIdName (index, new_race);

  UpdatePlayersAndMenu ();
  player_array.Unlock ();

  giant->Unlock ();
}

static void ProcessPlayerArray (TNET_MESSAGE *msg) {
  giant->Lock ();

  if (host == NULL) {
    giant->Unlock ();
    return;
  }

  double time;
  msg->Extract (&time, sizeof time);

  /* If the time, when the message originated is less greater than actual time,
   * we have wrong time and we'll correct it according to the received time. */
  if (glfwGetTime () < time)
    glfwSetTime (time);

  player_array.Lock ();

  TFOLLOWER *follower = dynamic_cast<TFOLLOWER *>(host);

  MenuUpdateMapInfo (msg->ExtractString ());    // Map name.
  T_BYTE player_count = msg->ExtractByte ();    // Count of players.

  /* Clear the array completely, remove hyper player too. */
  player_array.Clear ();
  player_array.RemovePlayer (0);

  int i;

  for (i = 0; i < player_count; i++) {
    string name = msg->ExtractString ();
    string race = msg->ExtractString ();
    bool computer = msg->ExtractByte () != 0;
    bool on_leader = !msg->ExtractByte ();
    int start_point = msg->ExtractByte ();

    if (on_leader) {
      player_array.AddRemotePlayer (name, follower->GetRemoteAddress (), follower->GetRemotePort (), computer);
    } else {
      in_addr addr;
      in_port_t port;

      msg->Extract (&addr, sizeof (in_addr));
      msg->Extract (&port, sizeof (in_port_t));

      /* Check, if the actual player is a local or a remote player. */
      if (TNET_RESOLVER::NetworkToAscii (addr) == TNET_RESOLVER::NetworkToAscii (follower->GetMyAddress ()) &&
          port == follower->GetMyPort ())
      {
        player_array.AddLocalPlayer (name, race, computer);
      } else
        player_array.AddRemotePlayer (name, addr, port, computer);
          
    }

    player_array.SetStartPoint (i, start_point);
    player_array.SetRaceIdName (i, race);
  }

  UpdatePlayersAndMenu ();

  /* Start the game if requested. */
  bool start_game = (msg->GetSubtype() == 1);
  if (start_game) {
    int my_id = player_array.GetMyPlayerID ();
    Debug (LogMsg ("My_id je %d", my_id));

    /* We are already connected to leader with remote_address 0, which is
     * hyperplayer. */

    /* Fill the follower's talker array of remote addresses with addresses of
     * all remote players' hosts. For local players add empty address. */
    for (i = 1; i < player_array.GetCount (); i++) {
      if (i < my_id) {
        /* Wait until player with id < my_id gets connected to me. */
        int fd;


#ifdef NEW_GLFW3
		struct timespec interval;
		interval.tv_sec = 0;
	  	interval.tv_nsec = 0.5;
	
    	for (int j = 0; j < 180; j++) {
          fd = host->GetListener()->GetListenersFileDescriptor (player_array.GetAddress (i));
          if (fd != -1)
            break;
          Debug ("Este stale nemam tu adresu");
          thrd_sleep(&interval, NULL);
        }
#else
		for (int j = 0; j < 180; j++) {
          fd = host->GetListener()->GetListenersFileDescriptor (player_array.GetAddress (i));
          if (fd != -1)
            break;
          Debug ("Este stale nemam tu adresu");
          glfwSleep (0.5);
        }
#endif
        

        if (fd != -1) {
          host->AddRemoteAddress (player_array.GetAddress (i), player_array.GetPort (i), fd);
          Debug (LogMsg ("%d: Pridavam adresu s filedescriptorom", i));
        } else {
          Error (LogMsg ("%d: host not connected in 15 seconds...", i));
        }
      } else if (player_array.IsRemote (i)) {
        host->AddRemoteAddress (player_array.GetAddress (i), player_array.GetPort (i));
        host->GetListener ()->AddListenerByFileDescriptor (player_array.GetAddress (i), player_array.GetPort (i), host->GetTalker ()->GetRemoteFiledescriptor (i));
        Debug (LogMsg ("%d: Pridavam remote adresu", i));
      } else {
        host->AddEmptyAddress ();
        Debug (LogMsg ("%d: Pridavam prazdnu adresu", i));
      }
    }

    MenuButtonOnClick (MNU_PLAY2);
  }

  player_array.Unlock ();
  giant->Unlock ();
}

static void ProcessHello (TNET_MESSAGE *msg) {
  giant->Lock ();

  if (host == NULL) {
    giant->Unlock ();
    return;
  }

  double received = glfwGetTime ();

  TFOLLOWER *follower = dynamic_cast<TFOLLOWER *>(host);

  in_addr my_address;
  in_port_t my_port;

  msg->Extract (&my_address, sizeof my_address);
  msg->Extract (&my_port, sizeof my_port);

  double time;
  msg->Extract (&time, sizeof time);

  /* Time shift is time that the request took divided by 2 (we beleive both
   * parts of the communication took the same amount of time. */
  double time_shift = (received - follower->GetPingRequestTime ()) / 2;
  glfwSetTime (time + time_shift);
  follower->SetMinimalTimeshift (time_shift);

  Debug (LogMsg ("Reply from Leader received in %.2f miliseconds", time_shift * 2000));

  TNET_RESOLVER resolver;

  follower->SetMyAddress (my_address, my_port);

  giant->Unlock ();
}

static void ProcessPingRequest (TNET_MESSAGE *msg) {
  giant->Lock ();

  if (host == NULL) {
    giant->Unlock ();
    return;
  }

  TLEADER *leader = dynamic_cast<TLEADER *>(host);

  double request_time;
  msg->Extract (&request_time, sizeof request_time);

  leader->SendPingReply (request_time);

  giant->Unlock ();
}

static void ProcessPingReply (TNET_MESSAGE *msg) {
  giant->Lock ();

  if (host == NULL) {
    giant->Unlock ();
    return;
  }

  double received = glfwGetTime ();

  TFOLLOWER *follower = dynamic_cast<TFOLLOWER *>(host);

  double request_time;
  double time;

  msg->Extract (&request_time, sizeof request_time);
  msg->Extract (&time, sizeof time);

  /* Time shift is time that the request took divided by 2 (we beleive both
   * parts of the communication took the same amount of time. */
  if (request_time == follower->GetPingRequestTime ()) {
    double time_shift = (received - follower->GetPingRequestTime ()) / 2;

    if (time_shift < follower->GetMinimalTimeshift ()) {
      glfwSetTime (time + time_shift);
      follower->SetMinimalTimeshift (time_shift);
      Debug (LogMsg ("Ping reply from Leader received in %.2f miliseconds", time_shift * 2000));
    }
  }

  giant->Unlock ();
}

static void ProcessChatMessage (TNET_MESSAGE *msg) {
  giant->Lock ();

  if (host == NULL) {
    giant->Unlock ();
    return;
  }

  player_array.Lock ();
  
  for (int id = 1; (id = player_array.GetPlayerID (msg->GetAddress (), msg->GetPort (), id)) != -1; id++) {
    if (!player_array.IsComputer (id)) {
      string name = player_array.GetPlayerName (id);

      string message = name + ": " + msg->ExtractString ();

      ost->AddText(message.c_str ());

      break;
    }
  }

  player_array.Unlock ();

  giant->Unlock ();
}

static void ProcessNetEvent (TNET_MESSAGE *msg) {
  giant->Lock ();

  if (host == NULL) {
    giant->Unlock ();
    return;
  }

  int size = msg->GetSize();
  char data[128];
  TEVENT *pevent;

  msg->Extract(data, size);
  
  pevent = pool_events->GetFromPool();
  pevent->DelinearizeEvent(data, size);
  queue_events->PutEvent(pevent);

  giant->Unlock ();
}

static void ProcessSynchronise (TNET_MESSAGE *msg) {
  giant->Lock ();

  if (host == NULL) {
    giant->Unlock ();
    return;
  }

  player_array.PlayerReady (msg->GetAddress (), msg->GetPort ());

  if (leader_ready && player_array.AllRemoteReady ()) {
    allowed_to_start_process_function = true;

    TLEADER *leader = dynamic_cast<TLEADER *>(host);
    leader->SendAllowProcessFunction ();

    gui->HideMessageBox ();
  }

  giant->Unlock ();
}

static void ProcessAllowProcessFunction (TNET_MESSAGE *msg) {
  giant->Lock ();

  if (host == NULL) {
    giant->Unlock ();
    return;
  }

  allowed_to_start_process_function = true;
  gui->HideMessageBox ();

  giant->Unlock ();
}

static void ProcessDisconnect (TNET_MESSAGE *msg) {
  T_BYTE player_id = msg->ExtractByte();
  
  ProcessDisconnect(player_id);
}

static void ProcessDisconnect (int player_id) {
  giant->Lock ();

  if (host == NULL) {
    giant->Unlock ();
    return;
  }

  if (!players[player_id]->active) {
    Debug(LogMsg("Not disconnecting player %d (%s) - he is not active.",
          player_id, player_array.GetPlayerName (player_id).c_str ()));

    giant->Unlock ();
    return;
  }
  bool exists_player;

  if (player_array.IsRemote(player_id)) {
    players[player_id]->Disconnect();
  }
  
  players[player_id]->active = false;

  Info(LogMsg("Player %d (%s) was disconnected.",
       player_id, player_array.GetPlayerName (player_id).c_str ()));

  /* when hyper player was disconnected, dont write "You won!" message */
  if (player_id == 0) {
    giant->Unlock ();
    return;
  }

  exists_player = false;
  for (int i = 0; i < player_array.GetCount(); i++) {
    if ((players[i] != hyper_player) && (players[i] != myself) && (players[i]->active == true)){
      exists_player = true;
      break;
    }
  }

  if (!exists_player && !won_lose) {
    action_key = 0;
    gui->ShowMessageBox("You WON!", GUI_MB_OK);
    allowed_to_start_process_function = true;
    won_lose = true;
  }

  giant->Unlock ();
}

struct TDISCONNECT_DATA {
  in_addr address;
  in_port_t port;
};

static void OnDisconnectThread (void *d) {
  TDISCONNECT_DATA *data = static_cast<TDISCONNECT_DATA *>(d);

  Debug ("niekto sa odpojil");

  giant->Lock ();

  if (host == NULL) {
    Debug ("Ale nemam hosta");
    giant->Unlock ();
    return;
  }

  if (state == ST_PLAY_MENU) {
    Debug ("som v menu");

    if (host->GetType () == THOST::ht_follower) {
      action_force = true;
      MenuButtonOnClick (MNU_DISCONNECT);

      action_key = 0;
      gui->ShowMessageBox ("You were disconnected!", GUI_MB_OK);
    } else {
      player_array.Lock ();

      int player_id = player_array.GetPlayerID (data->address, data->port, 0);

      if (player_id != -1) {
        string player_name = player_array.GetPlayerName (player_id);

        Info (LogMsg ("Player %d (%s) disconnected", player_id, player_name.c_str ()));

        host->RemoveAddress (player_id);
        player_array.RemovePlayer (player_id);
        UpdatePlayersAndMenu ();
      }

      player_array.Unlock ();
    }
  } else if (state == ST_GAME) {
      Debug ("som v hre");

      player_array.Lock ();

      for (int id = 0; (id = player_array.GetPlayerID (data->address, data->port, id)) != -1; id++) {
        host->DisconnectAddress (id);
        ProcessDisconnect (id);
      }    

      player_array.Unlock ();
  }

  giant->Unlock ();
}

static void OnDisconnect (in_addr address, in_port_t port) {
  TDISCONNECT_DATA *data = NEW TDISCONNECT_DATA;

  data->address = address;
  data->port = port;

  glfwCreateThread (OnDisconnectThread, data);
}


//========================================================================
// Creating GUI
//========================================================================

/**
 *  Clears gui variables.
 */
void ClearGuiVars()
{
  int i;

  main_menu = NULL;
  play_menu = NULL;
  ip_menu = NULL;
  game_menu = NULL;
  options_menu = NULL;
  video_menu = NULL;
  audio_menu = NULL;
  credits_menu = NULL;
  active_menu = NULL;

  main_panel = NULL;
  little_panel = NULL;
  chat_panel = NULL;
  radar_panel = NULL;

  chat_edit = NULL;

  ip_edit = NULL;
  ip_label = NULL;
  connect_button = NULL;
  create_button = NULL;

  map_list = NULL;
  map_label = NULL;
  play_button = NULL;
  disconn_button = NULL;

  resume_button = NULL;
  disconnect_button = NULL;

  map_scheme_label = NULL;
  map_size_label = NULL;
  map_players_label = NULL;
  map_races_label = NULL;
  map_author_label = NULL;

  for (i = 0; i < PL_MAX_PLAYERS; i++) {
    pl_name_label[i] = NULL;
    pl_race_combo[i] = NULL;
    pl_kill_button[i] = NULL;
  }

  load_panel = NULL;
}


/**
 *  Creates main menu GUI.
 */
void CreateMenuGUI()
{
  TGUI_PANEL *panel;
  TGUI_BUTTON *button;
  TGUI_CHECK_BOX *check;
  TGUI_LABEL *label;
  TGUI_LIST_BOX *list;
  TGUI_EDIT_BOX *edit;
  TGUI_COMBO_BOX *combo;
  TGUI_SCROLL_BOX *scroll;

  int i;

#if SOUND
  TGUI_SLIDER *slider;
#endif

  GLfloat y;
  GLfloat check_width = 160.0f;

  // gui
  gui->SetSize((GLfloat)config.scr_width, (GLfloat)config.scr_height);
  gui->SetFont(font0);
  gui->SetFontColor(1, 1, 1);
  gui->SetColor(0.75f, 0.7f, 0.6f);

  panel = gui->AddPanel(
    0, 0, 0, 
    (GLfloat)config.scr_width, (GLfloat)config.scr_height,
    gui_table.GetTexture(DAT_TGID_PANELS, 0)
  );

  // main menu
  if (config.scr_height >= 600) y = GLfloat(config.scr_height / 2 - 128 + 40);
  else y = GLfloat(config.scr_height / 2 - 128 + 10);

  main_menu = panel = gui->AddPanel(0, GLfloat(config.scr_width / 2 - 128), y, 256, 240);
  panel->SetPadding(0.0f);
  panel->SetAlpha(0.0f);
  panel->SetVisible(false);

  button = panel->AddButton(MNU_PLAY, 0, 190, gui_table.GetTexture(DAT_TGID_MENU_BUTTONS, 0));
  SetMenuButton(true);

  button = panel->AddButton(MNU_QUICK_PLAY, 0, 155, gui_table.GetTexture(DAT_TGID_MENU_BUTTONS, 1));
  SetMenuButton(true);

  button = panel->AddButton(MNU_OPTIONS, 0, 120, gui_table.GetTexture(DAT_TGID_MENU_BUTTONS, 2));
  SetMenuButton(true);
  
  button = panel->AddButton(MNU_CREDITS, 0, 85, gui_table.GetTexture(DAT_TGID_MENU_BUTTONS, 3));
  SetMenuButton(true);
  
  button = panel->AddButton(MNU_QUIT, 0, 15, gui_table.GetTexture(DAT_TGID_MENU_BUTTONS, 4));
  SetMenuButton(true);
  
  // test menu
  /*{
    TGUI_SCROLL_BOX *scroll;
    TGUI_MESSAGE_BOX *message;

    gui->SetFontColor(0, 0, 0);
    panel = gui->AddPanel(0, 10, 10, 300, 500);
    panel->SetPadding(10);

    label = panel->AddLabel(0, 120, 400, "Label - Line 1\nLine 2");

    panel->AddChild(NEW TGUI_LIST(panel, 0, 0, 390, "List - Item 1\nItem 2\nItem 3"));

    button = panel->AddCheckButton(0, 0, 360, 100, 20, "Button");
    button->SetTooltipText("Tooltip for Button\nNew line");

    check = panel->AddCheckBox(0, 0, 330, 100, 20, "Check Box");

    edit = panel->AddEditBox(0, 0, 300, 200, 20, 100);

    slider = panel->AddSlider(0, 0, 270, 200, 20, GUI_ST_HORIZONTAL);
    
    combo = panel->AddComboBox(0, 0, 240, 200, 20);
    combo->SetItems("Combo Box - Item 1\nItem 2\nItem 3\nItem 4\nItem 5\nItem 6");
    
    list = panel->AddListBox(0, 0, 150, 200, 80);
    list->SetItems("List Box - Line 1\nLine 2\nLine 3");
    list->ShowSlider(GUI_ST_HORIZONTAL);

    scroll = panel->AddScrollBox(0, 0, 0, 200, 100);
    scroll->SetTooltipText("This is a scroll box");
    //scroll->HideSlider(GUI_ST_VERTICAL);
    //scroll->HideSlider(GUI_ST_HORIZONTAL);

    button = scroll->AddButton(0, -20, -20, 300, 40, "Inside Button");
    button->SetTooltipText("Tooltip for Inside Button");
    scroll->AddChild(NEW TGUI_LIST(scroll, 0, 20, 30, "Inside List - Item 1\nItem 2\nItem 3"));

    //message = NEW TGUI_MESSAGE_BOX(gui);
    //gui->AddChild(message);
    //message->Show("Message Box - Line 1\nLine 2", GUI_MB_YES | GUI_MB_NO | GUI_MB_CANCEL);
  }*/

  // play menu
  if (config.scr_height >= 600) y = GLfloat(config.scr_height / 2 - 128 + 40);
  else y = GLfloat(config.scr_height / 2 - 128 + 10);

  play_menu = panel = gui->AddPanel(0, GLfloat(config.scr_width / 2 - 128), y, 256, 240);
  panel->SetPadding(0.0f);
  panel->SetAlpha(0.0f);
  panel->SetVisible(false);

  resume_button = button = panel->AddButton(MNU_RESUME, 0, 190, gui_table.GetTexture(DAT_TGID_MENU_BUTTONS, 5));
  SetMenuButton(connected);

  button = panel->AddButton(MNU_CREATE, 0, 155, gui_table.GetTexture(DAT_TGID_MENU_BUTTONS, 6));
  SetMenuButton(true);

  button = panel->AddButton(MNU_CONNECT, 0, 120, gui_table.GetTexture(DAT_TGID_MENU_BUTTONS, 7));
  SetMenuButton(true);

  disconnect_button = button = panel->AddButton(MNU_DISCONNECT, 0, 85, gui_table.GetTexture(DAT_TGID_MENU_BUTTONS, 8));
  SetMenuButton(connected);

  button = panel->AddButton(MNU_MAIN, 0, 15, gui_table.GetTexture(DAT_TGID_MENU_BUTTONS, 9));
  SetMenuButton(true);

  // ip menu
  if (config.scr_height >= 600) y = GLfloat(config.scr_height / 2 - 75 - 50);
  else y = GLfloat(config.scr_height / 2 - 85);

  ip_menu = panel = gui->AddPanel(MNU_CONNECT, GLfloat(config.scr_width / 2 - 250), y, 500, 170);
  SetMenuPanel();

  panel->AddLabel(0, 30, 110, gui_table.GetTexture(DAT_TGID_LABELS, 10));
  ip_label = panel->AddLabel(0, 110, 110, TNET_RESOLVER::GetHostName ().c_str ());

  ip_edit = edit = panel->AddEditBox(MNU_IP, 110, 107, 170, 20, PL_MAX_ADDRESS_LENGTH);
  edit->SetFontColor(0, 0, 0);
  edit->SetText(config.address);
  edit->SetOnChange(MenuEditOnChange);

  panel->AddLabel(0, 30, 80, gui_table.GetTexture(DAT_TGID_LABELS, 9));
  edit = panel->AddEditBox(MNU_PLAYER_NAME, 110, 77, 170, 20, PL_MAX_PLAYER_NAME_LENGTH);
  edit->SetFontColor(0, 0, 0);
  edit->SetText(config.player_name);
  edit->SetOnChange(MenuEditOnChange);

  button = panel->AddButton(MNU_PLAY, 0, 15, gui_table.GetTexture(DAT_TGID_MENU_BUTTONS, 9));
  SetMenuButton(true);
  connect_button = button = panel->AddButton(MNU_CONNECT2, 244, 15, gui_table.GetTexture(DAT_TGID_MENU_BUTTONS, 7));
  SetMenuButton(false);
  connect_button->SetEnabled(*config.player_name > 0);
  create_button = button = panel->AddButton(MNU_CREATE2, 244, 15, gui_table.GetTexture(DAT_TGID_MENU_BUTTONS, 6));
  SetMenuButton(true);

  // game menu
  if (config.scr_height >= 600) y = GLfloat(config.scr_height / 2 - 230 - 50);
  else y = GLfloat(config.scr_height / 2 - 230);

  game_menu = panel = gui->AddPanel(0, GLfloat(config.scr_width / 2 - 285), y, 570, 460);
  SetMenuPanel();
  panel->SetOnShow(MenuPanelOnShow);

  disconn_button = button = panel->AddButton(MNU_DISCONNECT, 20, 15, gui_table.GetTexture(DAT_TGID_MENU_BUTTONS, 8));
  SetMenuButton(true);
  play_button = button = panel->AddButton(MNU_PLAY2, 280, 15, gui_table.GetTexture(DAT_TGID_MENU_BUTTONS, 0));
  SetMenuButton(true);

  panel = panel->AddPanel(0, 0, 60, 570, 480);
  panel->SetPadding(0.0f);
  panel->SetAlpha(0);

  panel->AddLabel(0, 30, 353, gui_table.GetTexture(DAT_TGID_LABELS, 7));
  map_list = list = panel->AddListBox(MNU_MAP_LIST, 30, 185, 215, 150);
  list->SetFontColor(0, 0, 0);
  list->SetOnChange(MenuListOnChange);

  map_label = panel->AddLabel(0, 30, 320, "");

  panel->AddLabel(0, 265, 350, gui_table.GetTexture(DAT_TGID_LABELS, 8));
  scroll = map_info_scroll = panel->AddScrollBox(0, 260, 185, 285, 150);
  scroll->HideSlider(GUI_ST_HORIZONTAL);
  scroll->SetAlpha(0);

  scroll->AddLabel(0, 0, 120, "Author:");
  map_author_label =  scroll->AddLabel(0, 65, 120, "");
  scroll->AddLabel(0, 0, 100, "Scheme:");
  map_scheme_label =  scroll->AddLabel(0, 65, 100, "");
  scroll->AddLabel(0, 0, 80, "Size:");
  map_size_label =    scroll->AddLabel(0, 65, 80, "");
  scroll->AddLabel(0, 0, 60, "Players:");
  map_players_label = scroll->AddLabel(0, 65, 60, "");
  scroll->AddLabel(0, 0, 40, "Races:");
  map_races_label =   scroll->AddLabel(0, 65, 40, "");

  panel->AddLabel(0, 30, 150, gui_table.GetTexture(DAT_TGID_LABELS, 11));

  for (i = 1; i < PL_MAX_PLAYERS; i++) {
    pl_name_label[i] = panel->AddLabel(0, 30, 126 - GLfloat(i * 18), "");

    pl_race_combo[i] = combo = panel->AddComboBox(0, 325, 126 - GLfloat(i * 18), 200, 17);
    combo->SetItems("");
    combo->SetPadding(3.0f);
    combo->SetFontColor(0, 0, 0);
    combo->SetOnChange (MenuPlayerRaceComboOnChange);

    pl_kill_button[i] = button = panel->AddButton(MNU_KILL_PLAYER, 530, 126 - GLfloat(i * 18), 16, 16, "X");
    button->SetOnMouseClick(MenuButtonOnClick);
    button->SetFontColor(0, 0, 0);
  }

  /*
  add_comp_button = button = panel->AddButton(MNU_ADD_COMPUTER, 446, 150, 100, 16, "Add computer");
  button->SetFontColor(0, 0, 0);
  button->SetOnMouseClick(MenuButtonOnClick);
  */

  // options menu
  if (config.scr_height >= 600) y = GLfloat(config.scr_height / 2 - 128 + 40);
  else y = GLfloat(config.scr_height / 2 - 128 + 10);

  options_menu = panel = gui->AddPanel(0, GLfloat(config.scr_width / 2 - 128), y, 256, 240);
  panel->SetPadding(0.0f);
  panel->SetAlpha(0.0f);
  panel->SetVisible(false);

  button = panel->AddButton(MNU_VIDEO, 0, 190, gui_table.GetTexture(DAT_TGID_MENU_BUTTONS, 10));
  SetMenuButton(true);

  button = panel->AddButton(MNU_AUDIO, 0, 155, gui_table.GetTexture(DAT_TGID_MENU_BUTTONS, 11));
#if SOUND
  SetMenuButton(true);
#else
  SetMenuButton(false);
#endif

  button = panel->AddButton(MNU_MAIN, 0, 15, gui_table.GetTexture(DAT_TGID_MENU_BUTTONS, 9));
  SetMenuButton(true);

  // video options
  if (config.scr_height >= 600) y = GLfloat(config.scr_height / 2 - 230 - 50);
  else y = GLfloat(config.scr_height / 2 - 230);

  video_menu = panel = gui->AddPanel(0, GLfloat(config.scr_width / 2 - 250), y, 500, 460);
  SetMenuPanel();

  panel->AddLabel(0, 30, 400, gui_table.GetTexture(DAT_TGID_LABELS, 0));

  check = panel->AddCheckBox(MNU_FULLSCREEN, 50, 370, check_width, 17, "fullscreen *");
  check->SetChecked(config.fullscreen);
  check->SetOnMouseClick(MenuCheckBoxOnClick);

  check = panel->AddCheckBox(MNU_VERT_SYNC, 50, 350, check_width, 17, "v. synchronisation");
  check->SetChecked(config.vert_sync);
  check->SetOnMouseClick(MenuCheckBoxOnClick);

  panel->AddLabel(0, 30, 300, gui_table.GetTexture(DAT_TGID_LABELS, 1));

  // get supported video modes
  {
    GLFWvidmode *vid_modes = NEW GLFWvidmode[MAX_VID_MODES];
    int vid_modes_count = glfwGetVideoModes(vid_modes, MAX_VID_MODES);
    GLfloat y = 270.0f;
    GLfloat x = 50.0f;
    int w = 0;

    for (int i = 0; i < vid_modes_count; i++)
      if (vid_modes[i].RedBits == 8 && vid_modes[i].GreenBits == 8 && vid_modes[i].BlueBits == 8) {
        if (vid_modes[i].Width == 640 && vid_modes[i].Height == 480) {
          check = panel->AddGroupBox(MNU_640, x, y, check_width, 17, "640x480", 1);
          w = 640;
        }
        else if (vid_modes[i].Width == 800 && vid_modes[i].Height == 600) {
          check = panel->AddGroupBox(MNU_800, x, y, check_width, 17, "800x600", 1);
          w = 800;
        }
        else if (vid_modes[i].Width == 1024 && vid_modes[i].Height == 768) {
          check = panel->AddGroupBox(MNU_1024, x, y, check_width, 17, "1024x768", 1);
          w = 1024;
        }
        else if (vid_modes[i].Width == 1280 && vid_modes[i].Height == 1024) {
          check = panel->AddGroupBox(MNU_1280, x, y, check_width, 17, "1280x1024", 1);
          w = 1280;
        }
        else if (vid_modes[i].Width == 1152 && vid_modes[i].Height == 864) {
          check = panel->AddGroupBox(MNU_1152, x, y, check_width, 17, "1152x864", 1);
          w = 1152;
        }
        else if (vid_modes[i].Width == 1600 && vid_modes[i].Height == 1200) {
          check = panel->AddGroupBox(MNU_1600, x, y, check_width, 17, "1600x1200", 1);
          w = 1600;
        }

        if (w) {
          check->SetChecked(config.scr_width == w);
          check->SetOnMouseClick(MenuCheckBoxOnClick);
          w = 0;
          y -= 20.0f;
        }
      }

    delete[] vid_modes;
  }

  panel->AddLabel(0, 240, 400, gui_table.GetTexture(DAT_TGID_LABELS, 2));
  panel->AddLabel(0, 438, 400, "**");

  check = panel->AddGroupBox(MNU_TF_NEAREST, 260, 370, check_width, 18, "nearest", 2);
  check->SetChecked(config.tex_mag_filter == GL_NEAREST);
  check->SetOnMouseClick(MenuCheckBoxOnClick);
  check = panel->AddGroupBox(MNU_TF_LINEAR, 260, 350, check_width, 18, "linear", 2);
  check->SetChecked(config.tex_mag_filter == GL_LINEAR);
  check->SetOnMouseClick(MenuCheckBoxOnClick);

  panel->AddLabel(0, 240, 300, gui_table.GetTexture(DAT_TGID_LABELS, 3));
  panel->AddLabel(0, 425, 300, "**");

  check = panel->AddGroupBox(MNU_MF_NONE, 260, 270, check_width, 18, "none", 3);
  check->SetChecked(config.tex_min_filter == config.tex_mag_filter);
  check->SetOnMouseClick(MenuCheckBoxOnClick);
  check = panel->AddGroupBox(MNU_MF_NEAREST, 260, 250, check_width, 18, "nearest", 3);
  check->SetChecked(config.tex_min_filter == GL_NEAREST_MIPMAP_NEAREST || config.tex_min_filter == GL_LINEAR_MIPMAP_NEAREST);
  check->SetOnMouseClick(MenuCheckBoxOnClick);
  check = panel->AddGroupBox(MNU_MF_LINEAR, 260, 230, check_width, 18, "linear", 3);
  check->SetChecked(config.tex_min_filter == GL_NEAREST_MIPMAP_LINEAR || config.tex_min_filter == GL_LINEAR_MIPMAP_LINEAR);
  check->SetOnMouseClick(MenuCheckBoxOnClick);

  label = panel->AddLabel(0, 30, 80, " * restart application to apply this option");
  label->SetColor(0.5f, 0.5f, 0.5f);
  label = panel->AddLabel(0, 30, 65, "** start new game to apply this option");
  label->SetColor(0.5f, 0.5f, 0.5f);

  button = panel->AddButton(MNU_OPTIONS, 122, 15, gui_table.GetTexture(DAT_TGID_MENU_BUTTONS, 9));
  SetMenuButton(true);

#if SOUND

  // audio options
  if (config.scr_height >= 600) y = GLfloat(config.scr_height / 2 - 230 - 50);
  else y = GLfloat(config.scr_height / 2 - 230);

  audio_menu = panel = gui->AddPanel(0, GLfloat(config.scr_width / 2 - 250), y, 500, 460);
  SetMenuPanel();

  panel->AddLabel(0, 30, 400, gui_table.GetTexture(DAT_TGID_LABELS, 4));
  panel->AddLabel(0, 50, 370, "volume");
  slider = panel->AddSlider(MNU_MASTER_VOL, 180, 370, 280, 17, GUI_ST_HORIZONTAL);
  slider->SetPosition(config.snd_master_volume / 100.0f);
  slider->SetOnChange(MenuSliderOnChange);

  panel->AddLabel(0, 30, 320, gui_table.GetTexture(DAT_TGID_LABELS, 5));
  check = panel->AddCheckBox(MNU_MENU_MUSIC, 50, 290, check_width, 17, "play music");
  check->SetChecked(config.snd_menu_music);
  check->SetOnMouseClick(MenuCheckBoxOnClick);

  panel->AddLabel(0, 50, 260, "sound volume");
  slider = panel->AddSlider(MNU_MENU_SOUND_VOL, 180, 260, 280, 17, GUI_ST_HORIZONTAL);
  slider->SetPosition(config.snd_menu_sound_volume / 100.0f);
  slider->SetOnChange(MenuSliderOnChange);

  panel->AddLabel(0, 50, 240, "music volume");
  slider = panel->AddSlider(MNU_MENU_MUSIC_VOL, 180, 240, 280, 17, GUI_ST_HORIZONTAL);
  slider->SetPosition(config.snd_menu_music_volume / 100.0f);
  slider->SetOnChange(MenuSliderOnChange);

  panel->AddLabel(0, 30, 190, gui_table.GetTexture(DAT_TGID_LABELS, 6));
  check = panel->AddCheckBox(MNU_GAME_MUSIC, 50, 160, check_width, 17, "play music");
  check->SetChecked(config.snd_game_music);
  check->SetOnMouseClick(MenuCheckBoxOnClick);
  check = panel->AddCheckBox(MNU_UNIT_SPEECH, 50, 140, check_width, 17, "play unit speech");
  check->SetChecked(config.snd_unit_speech);
  check->SetOnMouseClick(MenuCheckBoxOnClick);

  panel->AddLabel(0, 50, 110, "sound volume");
  slider = panel->AddSlider(MNU_GAME_SOUND_VOL, 180, 110, 280, 17, GUI_ST_HORIZONTAL);
  slider->SetPosition(config.snd_game_sound_volume / 100.0f);
  slider->SetOnChange(MenuSliderOnChange);

  panel->AddLabel(0, 50, 90, "music volume");
  slider = panel->AddSlider(MNU_GAME_MUSIC_VOL, 180, 90, 280, 17, GUI_ST_HORIZONTAL);
  slider->SetPosition(config.snd_game_music_volume / 100.0f);
  slider->SetOnChange(MenuSliderOnChange);

  button = panel->AddButton(MNU_OPTIONS, 122, 15, gui_table.GetTexture(DAT_TGID_MENU_BUTTONS, 9));
  SetMenuButton(true);

#endif

  // credids
  if (config.scr_height >= 600) y = GLfloat(config.scr_height / 2 - 230 - 50);
  else y = GLfloat(config.scr_height / 2 - 230);

  credits_menu = panel = gui->AddPanel(0, GLfloat(config.scr_width / 2 - 250), y, gui_table.GetTexture(DAT_TGID_PANELS, 2));
  SetMenuPanel();
  panel->SetColor(1, 1, 1);

  button = panel->AddButton(MNU_MAIN, 122, 15, gui_table.GetTexture(DAT_TGID_MENU_BUTTONS, 9));
  SetMenuButton(true);

  // messagebox
  panel = gui->GetMessageBox();
  SetMenuPanel();
  panel->SetPadding(15);

  button = gui->GetMessageBox()->GetButton(GUI_MB_OK);
  button->SetTexture(GUI_BS_UP, gui_table.GetTexture(DAT_TGID_MENU_BUTTONS, 12));
  button->SetCaption(NULL);
  SetMenuButton(true);

  button = gui->GetMessageBox()->GetButton(GUI_MB_YES);
  button->SetTexture(GUI_BS_UP, gui_table.GetTexture(DAT_TGID_MENU_BUTTONS, 13));
  button->SetCaption(NULL);
  SetMenuButton(true);

  button = gui->GetMessageBox()->GetButton(GUI_MB_NO);
  button->SetTexture(GUI_BS_UP, gui_table.GetTexture(DAT_TGID_MENU_BUTTONS, 14));
  button->SetCaption(NULL);
  SetMenuButton(true);

  button = gui->GetMessageBox()->GetButton(GUI_MB_CANCEL);
  button->SetTexture(GUI_BS_UP, gui_table.GetTexture(DAT_TGID_MENU_BUTTONS, 15));
  button->SetCaption(NULL);
  SetMenuButton(true);

  // activate menu
  if (state == ST_PLAY_MENU) active_menu = play_menu;
  else if (state == ST_VIDEO_MENU) active_menu = video_menu;
  else active_menu = main_menu;

  active_menu->SetVisible(true);
}


/**
 *   Creates game GUI.
 */
void CreateGameGUI()
{
  TGUI_PANEL *panel;
  TGUI_LABEL *label;
  TGUI_BUTTON *button;
  TGUI_EDIT_BOX *edit;
  TGUI_SCROLL_BOX *scroll;
  TGUI_SLIDER *slider;
  int i;

  gui->SetFont(font0);
  gui->SetSize((GLfloat)config.scr_width, (GLfloat)config.scr_height);
  gui->SetFontColor(1, 0.93f, 0.82f);
  gui->SetOnMouseDown(GameOnMouseDown);
  gui->SetOnMouseUp(GameOnMouseUp);
  gui->GetDefTooltip()->SetColor(0, 0, 0);
  gui->GetDefTooltip()->SetAlpha(TOOLTIP_ALPHA);

  // little panel
  little_panel = panel = gui->AddPanel(0, 0, 0, GLfloat(config.scr_width - 200), 20);
  SetGamePanel(true);

  // materials
  for (i = 0; i < scheme.materials_count; i++) {
    label = panel->AddLabel(0, GLfloat(15 + i * 65), 4, 15, 12);
    label->SetTexture(scheme.tex_table.GetTexture(scheme.materials[i]->tg_id, 0));
    label->SetColor(1, 1, 1);
    label->SetOnMouseUp(GameOnMouseUp);

    panel_info.material_label[i] = label = panel->AddLabel(0, GLfloat(34 + i * 65), 1, "0");
    label->SetOnMouseUp(GameOnMouseUp);
  }

  // food, energy
  label = panel->AddLabel(0, little_panel->GetWidth() - 200, 4, 15, 12);
  label->SetTexture(myself->race->tex_table.GetTexture(myself->race->tg_food_id, 0));
  label->SetColor(1, 1, 1);
  label->SetOnMouseUp(GameOnMouseUp);

  panel_info.food_label = label = panel->AddLabel(0, little_panel->GetWidth() - 180, 1, "0");
  label->SetOnMouseUp(GameOnMouseUp);

  label = panel->AddLabel(0, little_panel->GetWidth() - 100, 4, 15, 12);
  label->SetTexture(myself->race->tex_table.GetTexture(myself->race->tg_energy_id, 0));
  label->SetColor(1, 1, 1);
  label->SetOnMouseUp(GameOnMouseUp);

  panel_info.energy_label = label = panel->AddLabel(0, little_panel->GetWidth() - 80, 1, "0");
  label->SetOnMouseUp(GameOnMouseUp);

  // chat panel
  chat_panel = panel = gui->AddPanel(0, 0, 0, GLfloat(config.scr_width - 200), 20);
  SetGamePanel(false);
  
  label = panel->AddLabel(0, 10, 2, "Say:");
  label->SetOnMouseUp(GameOnMouseUp);

  chat_edit = edit = panel->AddEditBox(0, 40, 2, panel->GetWidth() - 40, 17, 256);
  edit->SetAlpha(0);
  edit->SetPadding(0);
  edit->SetOnMouseUp(GameOnMouseUp);

  // main panel
  main_panel = panel = gui->AddPanel(0, GLfloat(config.scr_width - 200), 0, 200, GLfloat(config.scr_height), gui_table.GetTexture(DAT_TGID_PANELS, 1));
  panel->SetAlpha(GAME_PANEL_ALPHA);
  panel->SetOnMouseUp(GameOnMouseUp);

  // unit picture
  panel_info.picture_image = label = panel->AddLabel(0, 15, GLfloat(config.scr_height - 235), 50, 40);
  label->SetColor(1, 1, 1);
  label->SetOnMouseUp(GameOnMouseUp);

  // unit info
  panel_info.name_label = label = panel->AddLabel(0, 75, GLfloat(config.scr_height - 215), "Unit name");
  label->SetFontColor(1, 0.93f, 0.82f);
  label->SetOnMouseUp(GameOnMouseUp);

  panel_info.life_label = label = panel->AddLabel(0, 75, GLfloat(config.scr_height - 227), "Unit life");
  label->SetFontColor(1, 0.93f, 0.82f);
  label->SetOnMouseUp(GameOnMouseUp);

  panel_info.hided_label = label = panel->AddLabel(0, 75, GLfloat(config.scr_height - 239), "Hided units");
  label->SetFontColor(1, 0.93f, 0.82f);
  label->SetOnMouseUp(GameOnMouseUp);

  panel_info.info_label = label = panel->AddLabel(0, 15, GLfloat(config.scr_height - 255), "Unit info");
  label->SetFontColor(0.7f, 0.6f, 0.4f);
  label->SetLineHeight(12);
  label->SetOnMouseUp(GameOnMouseUp);

  // unit materials
  for (i = 0; i < scheme.materials_count; i++) {
    panel_info.material_image[i] = label = panel->AddLabel(0, GLfloat(100 + i * 20), 333, 15, 12);
    label->SetTexture(scheme.tex_table.GetTexture(scheme.materials[i]->tg_id, 0));
    label->SetColor(1, 1, 1);
    label->SetOnMouseUp(GameOnMouseUp);
  }

  // progress info
  panel_info.progress_label = label = panel->AddLabel(0, 15, 12, "Progress");
  label->SetColor(1, 0.93f, 0.82f);
  label->SetOnMouseUp(GameOnMouseUp);

  // action panels
  for (i = 0; i < MNU_PANELS_COUNT; i++) {
    panel_info.action_panel[i] = scroll = main_panel->AddScrollBox(0, 15, 40, 170, GLfloat(config.scr_height - 412));
    scroll->SetColor(0, 0, 0);
    scroll->SetAlpha(0.3f);
    scroll->SetPadding(3);
    scroll->HideSlider(GUI_ST_HORIZONTAL);
    scroll->SetOnMouseUp(GameOnMouseUp);
    scroll->Hide();

    slider = scroll->GetVSlider();
    slider->SetWidth(10);
    slider->SetFaceColor(0.3f, 0.3f, 0.3f);
    slider->SetHoverColor(0.45f, 0.4f, 0.3f);
    slider->SetAlpha(0.3f);
    slider->SetOnMouseUp(GameOnMouseUp);
  }
  panel_info.act_action_panel = NULL;
  panel_info.act_build_button = NULL;
  last_item = NULL;

  // quard buttons
  {
    GLfloat y = panel_info.action_panel[MNU_PANEL_STAY]->GetClientHeight() - gui_table.GetTexture(DAT_TGID_GUARD_BUTTONS, 0)->frame_height;
    GLfloat x = 0.0f;

    for (i = 0; i < RAC_AGGRESIVITY_MODE_COUNT; i++) {
      button = panel_info.guard_button[i] = panel_info.action_panel[MNU_PANEL_STAY]->AddGroupButton(
        MNU_GUARD_BUTTON + i, x, y, gui_table.GetTexture(DAT_TGID_GUARD_BUTTONS, i), 4
      );
      SetStayButton();

      switch (i) {
        case AM_IGNORE:       button->SetTooltipText(const_cast<char*>("Stay (F1)"));              break;
        case AM_GUARDED:      button->SetTooltipText(const_cast<char*>("Guard (F2)"));             break;
        case AM_OFFENSIVE:    button->SetTooltipText(const_cast<char*>("Offensive Guard (F3)"));   break;
        case AM_AGGRESSIVE:   button->SetTooltipText(const_cast<char*>("Aggressive Guard (F4)"));  break;
        default:              break;
      }

      x += button->GetWidth() + 2;
      if (x + button->GetWidth() > panel_info.action_panel[MNU_PANEL_STAY]->GetClientWidth())
      {
        y -= button->GetHeight() + 2;
        x = 0.0f;
      }
    }

    panel_info.act_guard_button = NULL;
  }

  // build tooltip
  build_tooltip = NEW TBUILD_TOOLTIP();
  build_tooltip->SetColor(0, 0, 0);
  build_tooltip->SetAlpha(TOOLTIP_ALPHA);

  // order panel
  if (config.scr_height > 480)
    panel_info.order_panel = scroll = main_panel->AddScrollBox(0, 15, 40, 170, 56);
  else
    panel_info.order_panel = scroll = main_panel->AddScrollBox(0, 138, 40, 47, 56);
  scroll->SetColor(0, 0, 0);
  scroll->SetAlpha(0.3f);
  scroll->SetPadding(3);
  scroll->HideSlider(GUI_ST_VERTICAL);
  scroll->SetOnMouseUp(GameOnMouseUp);

  slider = scroll->GetHSlider();
  slider->SetHeight(10);
  slider->SetFaceColor(0.3f, 0.3f, 0.3f);
  slider->SetHoverColor(0.45f, 0.4f, 0.3f);
  slider->SetAlpha(0.3f);
  slider->SetOnMouseUp(GameOnMouseUp);

  // unit actions
  panel = main_panel->AddPanel(0, 18.0f, GLfloat(config.scr_height - 364), 170, 30);
  panel->SetPadding(0.0f);
  panel->SetColor(0, 0, 0);
  panel->SetAlpha(0.0f);
  panel->SetOnMouseUp(GameOnMouseUp);

  panel_info.stay_button = button = panel->AddGroupButton(MNU_ACTION_STAY, 3, 3, gui_table.GetTexture(DAT_TGID_ACTION_BUTTONS, 0), 2);
  button->SetTexture(GUI_BS_DOWN, gui_table.GetTexture(DAT_TGID_ACTION_BUTTONS, 6));
  SetGameButton(const_cast<char*>("Stay (S)"));

  panel_info.move_button = button = panel->AddGroupButton(MNU_ACTION_MOVE, 31, 3, gui_table.GetTexture(DAT_TGID_ACTION_BUTTONS, 1), 2);
  button->SetTexture(GUI_BS_DOWN, gui_table.GetTexture(DAT_TGID_ACTION_BUTTONS, 7));
  SetGameButton(const_cast<char*>("Move (M)"));

  panel_info.attack_button = button = panel->AddGroupButton(MNU_ACTION_ATTACK, 59, 3, gui_table.GetTexture(DAT_TGID_ACTION_BUTTONS, 2), 2);
  button->SetTexture(GUI_BS_DOWN, gui_table.GetTexture(DAT_TGID_ACTION_BUTTONS, 8));
  SetGameButton(const_cast<char*>("Attack (A)"));

  panel_info.mine_button = button = panel->AddGroupButton(MNU_ACTION_MINE, 87, 3, gui_table.GetTexture(DAT_TGID_ACTION_BUTTONS, 3), 2);
  button->SetTexture(GUI_BS_DOWN, gui_table.GetTexture(DAT_TGID_ACTION_BUTTONS, 9));
  SetGameButton(const_cast<char*>("Mine (I)"));

  panel_info.repair_button = button = panel->AddGroupButton(MNU_ACTION_REPAIR, 114, 3, gui_table.GetTexture(DAT_TGID_ACTION_BUTTONS, 4), 2);
  button->SetTexture(GUI_BS_DOWN, gui_table.GetTexture(DAT_TGID_ACTION_BUTTONS, 10));
  SetGameButton(const_cast<char*>("Repair (R)"));

  panel_info.build_button = button = panel->AddGroupButton(MNU_ACTION_BUILD, 142, 3, gui_table.GetTexture(DAT_TGID_ACTION_BUTTONS, 5), 2);
  button->SetTexture(GUI_BS_DOWN, gui_table.GetTexture(DAT_TGID_ACTION_BUTTONS, 11));
  SetGameButton(const_cast<char*>("Build (B)"));

  // radar panel
  radar_panel = panel = gui->AddPanel(0, GLfloat(config.scr_width - 185), GLfloat(config.scr_height - 185), DRW_RADAR_SIZE, DRW_RADAR_SIZE);
  panel->SetFaceColor(0, 0, 0);
  panel->SetPadding(0);
  panel->SetOnDraw(GameOnRadarDraw);
  panel->SetOnMouseUp(GameOnMouseUp);
  panel->SetOnMouseDown(GameOnRadarMouseDown);
  panel->SetOnMouseMove(GameOnRadarMouseMove);

  button = panel->AddCheckButton(MNU_TOGGLE_RADAR, 2, DRW_RADAR_SIZE - 17, 15, 15, "");
  button->SetColor(0.3f, 0.3f, 0.3f);
  button->SetOnMouseUp(GameOnMouseUp);
  button->SetOnMouseClick(GameButtonOnClick);
  button->SetTooltipText(const_cast<char*>("Clip Radar"));
  if (!radar.IsHideable())
    button->SetChecked(true);

  // toggle panel button
  button = gui->AddButton(MNU_TOGGLE_PANEL, GLfloat(config.scr_width - 15), GLfloat(config.scr_height - 15), 15, 15, "");
  SetGameButton(const_cast<char*>("Toggle Panels (Tab)"));

  // view segment button
  panel = main_panel->AddPanel(0, 18.0f, 10.0f, 170, 24);
  panel->SetPadding(0);
  panel->SetColor(0, 0, 0);
  panel->SetAlpha(0.0f);
  panel->SetOnMouseUp(GameOnMouseUp);

  panel_info.seg_button = button = panel->AddButton(MNU_VIEW_SEGMENT, 142, 0, gui_table.GetTexture(DAT_TGID_SEG_BUTTONS, 3));
  SetGameButton(const_cast<char*>("Change View Segment (F5-F8)"));

  // update panel
  selection->UpdateInfo(false);
  ChangeActionPanel(MNU_PANEL_EMPTY);

  // toggle main panel
  if (!main_panel_visible)
    ToggleMainPanel();
}


void ChangeActionPanel(int panel)
{
  if (panel_info.act_action_panel)
    panel_info.act_action_panel->Hide();

  panel_info.act_action_panel = panel_info.action_panel[panel];
  panel_info.act_action_panel->Show();
}


void CreateBuildButtons()
{
  if (state != ST_GAME) return;

  TGUI_BUTTON *button;
  int i;
  GLfloat y = panel_info.action_panel[MNU_PANEL_BUILD]->GetClientHeight() - 40.0f;
  GLfloat x = 0.0f;

  // uncheck button
  UncheckBuildButton();

  // worker's build buttons
  if (selection->GetFirstUnit()->TestItemType(IT_WORKER)) {
    TLIST<TBUILDING_ITEM>::TNODE<TBUILDING_ITEM> *node;
    TWORKER_ITEM *itm = static_cast<TWORKER_ITEM *>(selection->GetBuilderItem());

    if (last_item == itm)
      return;

    panel_info.action_panel[MNU_PANEL_BUILD]->Clear();

    for (i = 0, node = itm->build_list.GetFirst(); node; node = node->GetNext(), i++) {
      button = panel_info.action_panel[MNU_PANEL_BUILD]->AddGroupButton(
        (int)node->GetPitem(), x, y, myself->race->tex_table.GetTexture(node->GetPitem()->tg_picture_id, 0), 3
      );
      SetBuildButton();
      
      if (i % 3 == 2) {
        y -= 42.0f;
        x = 0.0f;
      }
      else x += 52.0f;
    }

    last_item = itm;
  }

  // factory's produce buttons
  else if (selection->GetFirstUnit()->TestItemType(IT_FACTORY)) {
    TPRODUCEABLE_NODE *node;
    TFACTORY_ITEM *itm = static_cast<TFACTORY_ITEM *>(selection->GetBuilderItem());

    if (last_item == itm)
      return;

    panel_info.action_panel[MNU_PANEL_BUILD]->Clear();
  
    for (i = 0, node = itm->GetProductsList().GetFirstNode(); node; node = node->GetNextNode(), i++) {
      button = panel_info.action_panel[MNU_PANEL_BUILD]->AddButton(
        (int)node->GetProduceableItem(), x, y, myself->race->tex_table.GetTexture(node->GetProduceableItem()->tg_picture_id, 0)
      );

      SetProduceButton();
    
      if (i % 3 == 2) {
        y -= 42.0f;
        x = 0.0f;
      }
      else x += 52.0f;
    }

    last_item = itm;
  }
}


void UncheckBuildButton()
{
  // uncheck button
  if (panel_info.act_build_button) {
    panel_info.act_build_button->SetChecked(false);
    panel_info.act_build_button = NULL;
  }
}


void UpdateGuardButtons()
{
  if (state != ST_GAME) return;

  panel_info.guard_button[AM_GUARDED]->SetEnabled(selection->GetCanAttack());
  panel_info.guard_button[AM_OFFENSIVE]->SetEnabled(selection->GetCanAttack() && selection->GetCanMove());
  panel_info.guard_button[AM_AGGRESSIVE]->SetEnabled(selection->GetCanAttack() && selection->GetCanMove());
}


void CheckGuardButton(TAGGRESSIVITY_MODE button)
{
  if (panel_info.act_guard_button)
    panel_info.act_guard_button->SetChecked(false);

  if (button != AM_NONE) {
    panel_info.act_guard_button = panel_info.guard_button[button];
    panel_info.act_guard_button->SetChecked(true);
  }
  else
    panel_info.act_guard_button = NULL;
}


void CreateOrderButtons()
{
  if (state != ST_GAME) return;
  if (!selection->GetFirstUnit()->TestItemType(IT_FACTORY)) return;

  panel_info.order_panel->Clear();

  TFACTORY_UNIT * funit = static_cast<TFACTORY_UNIT *>(selection->GetFirstUnit());
  TGUI_BUTTON *button;
  int i;
  char txt[4];

  for (i = 0; i < funit->GetOrderSize(); i++) {
    button = panel_info.order_panel->AddButton(
      (int)funit->GetOrderedUnit(i)->GetProduceableItem(), i * 52.0f, 0.0f, myself->race->tex_table.GetTexture(funit->GetOrderedUnit(i)->GetProduceableItem()->tg_picture_id, 0)
    );

    sprintf(txt, "%d", i + 1);
    SetOrderButton(txt);

    if (i == 0)
      button->SetOnMouseClick(GameOrderOnClick);
    else
      button->SetAlpha(0.5f);
  }
}


void SetOrderVisibility(bool vis)
{
  int i;
  panel_info.order_panel->SetVisible(vis);

  if (config.scr_height > 480) {
    if (vis) {
      for (i = 0; i < MNU_PANELS_COUNT; i++) {
        panel_info.action_panel[i]->SetPosY(40 + panel_info.order_panel->GetHeight() + 5);
        panel_info.action_panel[i]->SetHeight(GLfloat(config.scr_height - 412 - panel_info.order_panel->GetHeight() - 5));
      }
    }
    else {
      for (i = 0; i < MNU_PANELS_COUNT; i++) {
        panel_info.action_panel[i]->SetPosY(40);
        panel_info.action_panel[i]->SetHeight(GLfloat(config.scr_height - 412));
      }
    }
  }
  else {
    if (vis) {
      for (i = 0; i < MNU_PANELS_COUNT; i++)
        panel_info.action_panel[i]->SetWidth(170 - panel_info.order_panel->GetWidth() - 5);
    }
    else {
      for (i = 0; i < MNU_PANELS_COUNT; i++)
        panel_info.action_panel[i]->SetWidth(170);
    }
  }
}


//========================================================================
// GLFW Size Callback
//========================================================================

/**
 *  GLFW window size callback function. This function is called by GLFW every
 *  time the window size is changed.
 *
 *  @param w New width of the window.
 *  @param h New height of the window.
 */
#ifdef NEW_GLFW3
void GLFWCALL SizeCallback(GLFWwindow* window, int w, int h)
#else
void GLFWCALL SizeCallback(int w, int h)
#endif
{
  config.scr_width = w;
  config.scr_height = h;

  glViewport(0, 0, w, h);

  glfSetFontDisplayMode(font0, w, h);
}


//========================================================================
// GLFW Key Callback
//========================================================================

/**
 *  GLFW key callback function. This function is called every time a key is
 *  pressed or released.
 *
 *  @param key    A key identifier (uppercase ASCII or a special key
 *                identifier).
 *  @param action Either GLWF_PRESS or GLFW_RELEASE.
 */
#ifdef NEW_GLFW3
void GLFWCALL KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
#else
void GLFWCALL KeyCallback(int key, int action)
#endif
{
  need_redraw->SetTrue ();

  switch (state) {
  case ST_MAIN_MENU:
  case ST_VIDEO_MENU:
  case ST_PLAY_MENU:
    if (action == GLFW_PRESS) MenuOnKeyDown(key);
    break;

  case ST_GAME:
    if (action == GLFW_PRESS) GameOnKeyDown(key);
    else GameOnKeyUp(key);
    break;

  case ST_QUIT:
  default:
    break;
  }
}


//========================================================================
// GLFW Mouse Callbacks
//========================================================================

/**
 *  GLFW mouse button callback function. This function is called every time a
 *  mouse button is pressed or released.
 *
 *  @param button  A mouse button identifier (one of
 *                 GLFW_MOUSE_BUTTON_{LEFT|MIDDLE|RIGHT}).
 *  @param action  Either GLFW_PRESS or GLFW_RELEASE.
 */
#ifdef NEW_GLFW3
void GLFWCALL MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
#else
void GLFWCALL MouseButtonCallback(int button, int action)
#endif
{
  need_redraw->SetTrue ();

  if (action == GLFW_PRESS) gui->MouseDown(GLfloat(mouse.x), GLfloat(mouse.y), button);
  else gui->MouseUp(GLfloat(mouse.x), GLfloat(mouse.y), button);
}

/**
 *  GLFW mouse position callback function. This function is called every time a
 *  mouse has changed position.
 *
 *  @param x  X coordinate.
 *  @param y  Y coordinate.
 */
#ifdef NEW_GLFW3
void GLFWCALL MousePosCallback(GLFWwindow* window, double x, double y)
#else
void GLFWCALL MousePosCallback(int x, int y)
#endif
{
  need_redraw->SetTrue ();

  y = config.scr_height - y;

  int dx = x - mouse.rx;
  int dy = y - mouse.ry;

  if (state == ST_GAME && map.mouse_moving)
  {
    map.Move(-dx * projection.game_h_coef, -dy * projection.game_v_coef);
  }
  else {
    int last_x = mouse.x;
    int last_y = mouse.y;

    mouse.x += dx;
    mouse.y += dy;
    if (mouse.x < 0) mouse.x = 0;
    if (mouse.y < 0) mouse.y = 0;
    if (mouse.x >= config.scr_width) mouse.x = config.scr_width - 1;
    if (mouse.y >= config.scr_height) mouse.y = config.scr_height - 1;

    if (state == ST_GAME && map.drag_moving) {
      map.Move((mouse.x - last_x) * projection.game_h_coef, (mouse.y - last_y) * projection.game_v_coef);
    }
  }

  mouse.rx = x;
  mouse.ry = y;

  gui->MouseMove(GLfloat(mouse.x), GLfloat(mouse.y));
}


/**
 *  GLFW mouse wheel callback function. This function is called every time a
 *  mouse wheel changes position.
 *
 *  @param pos Actual wheel position.
 */
#ifdef NEW_GLFW3
void GLFWCALL MouseWheelCallback(GLFWwindow* window, double xoffset, double yoffset);
#else
void GLFWCALL MouseWheelCallback(int pos)
#endif
{
  need_redraw->SetTrue ();

  static int last_pos;

  switch (state) {
  case ST_GAME:
    map.Zoom(pos - last_pos);

  default: break;
  }

  last_pos = pos;
}

/**
 *  GLFW window refresh callback function. This function is called every time
 *  part of the window client area needs to be repainted - for instance when
 *  another window lying on top of our window has changed its position.
 */
#ifdef NEW_GLFW3
void GLFWCALL WindowRefreshCallback(GLFWwindow* window);
#else
void GLFWCALL WindowRefreshCallback () 
#endif
{
  need_redraw->SetTrue ();
}


//========================================================================
// Menu & Game
//========================================================================

/**
 *  Main function for menu. Contains menu loop.
 */
void Menu()
{
  TTIME clock;

//  Info("Running menu");
  CreateMenuGUI();

  if (error == ERR_LOAD_MAP) {
    action_key = 0;
    gui->ShowMessageBox ((string ("Error loading map '") + selected_map_name + "'").c_str(), GUI_MB_OK);
    error = ERR_NONE;
  }

  map_info_list.LoadMapList();

#if SOUND
  ChangeSoundVolume(config.snd_menu_sound_volume);
  if (config.snd_menu_music && !sounds_table.sounds[DAT_SID_MENU_MUSIC]->IsPlaying())
  {
    sounds_table.sounds[DAT_SID_MENU_MUSIC]->Play();
    sounds_table.sounds[DAT_SID_MENU_MUSIC]->Stop();   // hack for looping :((
    sounds_table.sounds[DAT_SID_MENU_MUSIC]->Play();
  }
#endif

  projection.SetProjection(PRO_MENU);
  mouse.ResetCursor();
  gui->MouseMove(GLfloat(mouse.x), GLfloat(mouse.y));  // update gui under mouse

  // menu loop
  while (state == ST_MAIN_MENU || state == ST_PLAY_MENU || state == ST_VIDEO_MENU) {
    // updates events
    clock.Update();
    mouse.Update(false, clock.GetShift());
    gui->Update(clock.GetShift());

    // Updates ost (On Screen Text). If there has been some change made, set
    // need_redraw to true.
    if (ost->Update(clock.GetActual()))
      need_redraw->SetTrue ();

    // Draw the screen, but only when there has been some change made.
    if (need_redraw->IsTrue ()) {
      gui->Draw();
      ost->Draw();
      mouse.Draw();

      glfwSwapBuffers ();
      gui->PollEvents();
    }
#ifdef NEW_GLFW3
		struct timespec interval;
		interval.tv_sec = 0;
	  	interval.tv_nsec = 0.01;
	
        thrd_sleep (&interval, NULL); // 100 fps
#else
		glfwSleep (0.01); // 100 fps
#endif
    
    glfwPollEvents ();

    gui->PollEvents();
  }

  if (state == ST_RESET_VIDEO_MENU) state = ST_VIDEO_MENU;

  // delete gui
  ClearGuiVars();
  gui->Reset();
  
  // clear menu maps and players structures
  map_info_list.ClearMapList();
}


/**
 *  Update thread function. It is runned by glfwCreateThread() from Game().
 *
 *  @param arg Arguments passed to glfwCreateThread. They are not used.
 */
static void GLFWCALL ProcessFunction(void *arg)
{
  TTIME time;
  TEVENT * act_event;
  TPLAYER_UNIT * act_unit;

  fps_of_update.Reset ();
  
#ifdef NEW_GLFW3
		struct timespec interval;
		interval.tv_sec = 0;
	  	interval.tv_nsec = 0.02;
	  	
		while (!allowed_to_start_process_function)
        	thrd_sleep (&interval, NULL); 
#else
		while (!allowed_to_start_process_function)
    		glfwSleep (0.02);
#endif

  

  Info ("Update: Running");

  while (started) {
    time.Update ();
    fps_of_update.Update (time.GetShift ());

    // cycle which get from queue all events with time_stamp <= actual time.
    while ((queue_events->GetFirstEventTimeStamp() != -1) && (queue_events->GetFirstEventTimeStamp() <= time.GetActual())) {
      process_mutex->Lock();

      act_event = queue_events->GetFirstEvent();

      act_unit = ((TPLAYER_UNIT *)players[act_event->GetPlayerID()]->hash_table_units.GetUnitPointer(act_event->GetUnitID()));

      // process event only in case that unit exists
      if (act_unit) {
      

        // local (not remote) units
        if (!player_array.IsRemote(act_event->GetPlayerID())) {
          
          // units running on my computer can have in queue only one event (events are NULLed only by US_... (not by requests))
          if ((act_event->GetEvent() < RQ_FIRST))
            act_unit->pevent = NULL;
          
          
          #if DEBUG_EVENTS
            Debug(LogMsg("PROC_L: P:%d U:%d E:%s RQ:%d X:%d Y:%d Z:%d R:%d I1:%d TS:%f RT:%f COUNT:%d", act_event->GetPlayerID(), act_event->GetUnitID(), EventToString(act_event->GetEvent()), act_event->GetRequestID(), act_event->simple1, act_event->simple2, act_event->simple3, act_event->simple4, act_event->int1, act_event->GetTimeStamp(), glfwGetTime(), queue_events->GetQueueLength()));
          #endif

          act_unit->ProcessEvent(act_event);
        }
        
        // units of not local players must be checked for right order of events according to time stamp
        if (player_array.IsRemote(act_event->GetPlayerID())) {
          // test if timestamp of las processed event is smaller than actual event time stamp
          if ((act_unit->last_event_time_stamp <= act_event->GetTimeStamp()) || (act_event->GetEvent() >= RQ_FIRST)){
            
            if (act_event->GetEvent() < RQ_FIRST) act_unit->last_event_time_stamp = act_event->GetTimeStamp();

            #if DEBUG_EVENTS
              if (act_unit->pevent)
                Error(LogMsg("Remote unit has PEVENT!"));

              Debug(LogMsg("PROC_R: P:%d U:%d E:%s RQ:%d X:%d Y:%d Z:%d R:%d I1:%d TS:%f RT:%f COUNT:%d", act_event->GetPlayerID(), act_event->GetUnitID(), EventToString(act_event->GetEvent()), act_event->GetRequestID(), act_event->simple1, act_event->simple2, act_event->simple3, act_event->simple4, act_event->int1, act_event->GetTimeStamp(), glfwGetTime(), queue_events->GetQueueLength()));
            #endif

            act_unit->ProcessEvent(act_event);
          }
        }
      }

      process_mutex->Unlock();
      
      pool_events->PutToPool(act_event);
    }

    // sleep that long, we get 50 fps
    time.SleepToGetExpectedFrameDuration (0.02);
  }
}


bool StartGame(double stime)
{
  state = ST_GAME;
  won_lose = false;

  // int threads (only for identification that CreateMutex was not called)
#ifdef NEW_GLFW3
	process_thread = NULL;
#else
	process_thread = -1;
#endif
  

  // create loading gui
  gui->SetFont(font0);
  gui->SetFontColor(1, 0.93f, 0.82f);

  load_panel = gui->AddPanel(0, 0, 0, (GLfloat)config.scr_width, (GLfloat)config.scr_height, gui_table.GetTexture(DAT_TGID_PANELS, 0));
  load_panel->AddLabel(0, GLfloat(config.scr_width/2) - 40, GLfloat(config.scr_height/2) - 7, "LOADING...");

  gui->Draw();
  glfwSwapBuffers();

  // create mutexes
  delete_mutex  = glfwCreateMutex ();

  if (!delete_mutex) {
    Critical ("Could not create mutex");
    goto error;
  }

  // create instances of TPOOL for events and TQUEUE_EVENTS
  // create instance of TPOOL for path info 
  pool_path_info    = NEW TPOOL<TPATH_INFO>(EV_MIN_POOL_ELEMENTS, 0, EV_MIN_POOL_ELEMENTS);
  pool_sel_node     = NEW TPOOL<TSEL_NODE>(EV_MIN_POOL_ELEMENTS, 0, EV_MIN_POOL_ELEMENTS);

  pool_nearest_info = NEW TPOOL<TNEAREST_INFO>(EV_MIN_POOL_ELEMENTS, 0, EV_MIN_POOL_ELEMENTS);
  pool_events       = NEW TPOOL<TEVENT>(2 * EV_MIN_POOL_ELEMENTS, 0, EV_MIN_POOL_ELEMENTS);

  // load map
  char map_name[MAP_MAX_NAME_LENGTH];
  char * last;
  
  strcpy (map_name, selected_map_name.c_str ());
  last = strrchr(map_name, '.');
  *last = 0;

  // set player options
  view_segment = DRW_ALL_SEGMENTS;

  if (!map.LoadMap(map_name)) {
    error = ERR_LOAD_MAP;
    state = ST_MAIN_MENU;
    goto error_with_own_state;
  }

  //create thread pool for path finding but only if doesn't exist yet
  if (threadpool_astar == NULL)
    threadpool_astar = threadpool_astar->CreateNewThreadPool(5, 50);
  //create thread pool for searching of the nearest building but only if doesn't exist yet
  if (threadpool_nearest == NULL)
    threadpool_nearest = threadpool_nearest->CreateNewThreadPool(3, 30);

  //check success
  if (!threadpool_astar || !threadpool_nearest) 
  {
    Critical ("Could not create thread pools");
    goto error;
  }

  selection = NEW TSELECTION;

  strcpy(myself->name, config.player_name);

  // moves map to player's initial view position
  map.CenterMapel(myself->initial_x, myself->initial_y);

  // reset events
  map.start_time = stime;

  started = true;

  // start Update thread
#ifdef NEW_GLFW3
	if(thrd_create(&process_thread, ProcessFunction, NULL) == thrd_error){
		Critical ("Could not create threads");
    	goto error;
    }
#else
	process_thread = glfwCreateThread (ProcessFunction, NULL);

  if (process_thread < 0) {
    Critical ("Could not create threads");
    goto error;
  }
#endif
  

  gui->Reset();
  
  // clear menu structures
  map_info_list.ClearRacList();
  return true;

error:
  state = ST_QUIT;
error_with_own_state:
 
  // clear menu structures
  map_info_list.ClearRacList();
  gui->Reset();

  // wait for Update and AI thread to finish
  if (process_thread == -1) glfwWaitThread(process_thread, GLFW_WAIT);
  
  started = false;
  
  // delete selection
  if (selection){
    delete selection;
    selection = NULL;
  }

  // clear map
  map.DeleteMap();
  
  // clear pools
  if (pool_events){ delete pool_events; pool_events = NULL;}
  if (pool_path_info){ delete pool_path_info; pool_path_info = NULL;}
  if (pool_nearest_info){ delete pool_nearest_info; pool_nearest_info = NULL;}
  if (pool_sel_node){ delete pool_sel_node; pool_sel_node = NULL;}
  
  if (delete_mutex) {
    glfwDestroyMutex(delete_mutex);
    delete_mutex = NULL;
  }

  return false;
}


void StopGame()
{
  started = false;

  // wait for Update thread to finish
  glfwWaitThread(process_thread, GLFW_WAIT);

  // delete selection
  if (selection) {
    delete selection;
    selection = NULL;
  }

  // delete map
  map.DeleteMap();
  
  // delete instances of pools
  if (pool_events){ delete pool_events; pool_events = NULL;}
  if (pool_path_info){ delete pool_path_info; pool_path_info = NULL;}
  if (pool_nearest_info){ delete pool_nearest_info; pool_nearest_info = NULL;}
  if (pool_sel_node){ delete pool_sel_node; pool_sel_node = NULL;}

  // kill all temporary threads
  if (threadpool_astar) { delete threadpool_astar; threadpool_astar = NULL; }
  if (threadpool_nearest) { delete threadpool_nearest; threadpool_nearest = NULL; }

  if (delete_mutex){
    glfwDestroyMutex(delete_mutex);
    delete_mutex = NULL;
  }

  // queue is only cleared (it is destroyed in the end of program)
  queue_events->Clear();
}

/**
 *  Game function. Game loop is here.
 */
void Game(void)
{
  TTIME clock;
  int i;

  giant->Lock ();

  if (!started) {
    if (!StartGame(clock.GetActual())) {
      giant->Unlock ();
      return;
    }

    if (host->GetType () == THOST::ht_follower) {
      TFOLLOWER *follower = dynamic_cast<TFOLLOWER *>(host);

      follower->SendSynchronise ();
    } else {
      leader_ready = true;

      if (player_array.AllRemoteReady ()) {
        allowed_to_start_process_function = true;

        TLEADER *leader = dynamic_cast<TLEADER *>(host);
        leader->SendAllowProcessFunction ();
      }
    }
  }

  giant->Unlock ();

#if SOUND
  sounds_table.sounds[DAT_SID_MENU_MUSIC]->Stop();

  ChangeSoundVolume(config.snd_game_sound_volume);
  if (config.snd_game_music) {
    sounds_table.sounds[DAT_SID_GAME_MUSIC]->Play();
    sounds_table.sounds[DAT_SID_GAME_MUSIC]->Stop();   // hack for looping :((
    sounds_table.sounds[DAT_SID_GAME_MUSIC]->Play();
  }
#endif

  // create game GUI
  CreateGameGUI();

  fps.Reset();
  mouse.ResetCursor();
  gui->MouseMove(GLfloat(mouse.x), GLfloat(mouse.y));  // update gui under mouse
  myself->update_info = true;                         // update myself information on panels
  selection->UpdateInfo(true);

  if (!allowed_to_start_process_function) {
    action_key = MNU_PLAY2;
    gui->ShowMessageBox ("Waiting for other players...", GUI_MB_CANCEL);
  }

  // projection
  //projection.SetProjection(PRO_GAME);
  
  // main loop used for drawing
  while (state == ST_GAME) 
  {
    clock.Update();

    // gui environment
    gui->Update(clock.GetShift());

    // infos
    fps.Update(clock.GetShift());
    ost->Update(clock.GetActual());

    // mouse (MUST be called before selection update)
    mouse.Update(true, clock.GetShift());

    // selection
    selection->Update(clock.GetShift());

    // map position and active area
    map.UpdateMoving(clock.GetShift());

    // projection
    projection.Update();

    // active area
    map.UpdateActiveArea();

    if (!reduced_drawing) {
      // units and buildings (have to be called after updating active area)
      int pl_count = player_array.GetCount();
      for (i = 0; i < pl_count; i++) 
      {
        if (players[i]->active) players[i]->UpdateGraphics(clock.GetShift());
      }
      // map graphics (with sorting of units -> have to be called after updating units)
      map.UpdateGraphics(clock.GetShift());

      scheme.UpdateGraphics(clock.GetShift ());
    }

    // myself information
    if (myself->update_info) UpdateMyselfInfo();

    // draw game
    DrawGame();

    // change buffers
    glfwSwapBuffers();
    gui->PollEvents();
    
    if (!glfwGetWindowParam(GLFW_OPENED)) state = ST_QUIT;

    // sleep to get expected frame duration
    clock.SleepToGetExpectedFrameDuration (config.pr_expected_frame_duration);

    /* Process events to respon a bit quicker to some events (for example to
     * not to render one more frame, when QUIT key was pressed). */
    glfwPollEvents ();
    gui->PollEvents();

  } // while (state == ST_GAME)

  if (state == ST_QUIT && connected) Disconnect();

  if (state == ST_RESET_VIDEO_MENU) state = ST_GAME;
  else {

#if SOUND
  sounds_table.sounds[DAT_SID_GAME_MUSIC]->Stop();
#endif

  }

  // delete gui
  ClearGuiVars();
  gui->Reset();
  panel_info.Clear();

  if (build_tooltip) {
    delete build_tooltip;
    build_tooltip = NULL;
  }
}


//=========================================================================
// END
//=========================================================================
// vim:ts=2:sw=2:et:

