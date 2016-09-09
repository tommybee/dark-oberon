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
 *  @file doconfig.h
 *
 *  Game declarations and methods.
 *
 *  @author Peter Knut
 *
 *  @date 2002, 2003
 */

#ifndef __doconfig_h__
#define __doconfig_h__

//========================================================================
// Forward declarations
//========================================================================

struct TCONFIG;


//========================================================================
// Definitions
//========================================================================

/** Filename of the configuration file.
 *  @sa DATA_DIR */
#define CFG_FILE_NAME      (user_dir + DATA_DIR "config.cfg").c_str()

/** Default configuration's screen resolution. From this value the
 *  TCONFIG::width and TCONFIG::height are set. */
#define CFG_DEF_RESOLUTION      "1024x768"
#define CFG_DEF_SCR_WIDTH       1024
#define CFG_DEF_SCR_HEIGHT      768
/** Default configuration's texture filter.
 *  @sa TCONFIG::texture_filter */
#define CFG_DEF_TEXTURE_FILTER  "linear"
/** Default configuration's mipmap filter.
 *  @sa TCONFIG::mipmap_filter */
#define CFG_DEF_MIPMAP_FILTER   "none"
/** Default player's name.
 *  @sa TCONFIG::player_name */
#define CFG_DEF_TEX_MAG_FILTER GL_LINEAR
#define CFG_DEF_TEX_MIN_FILTER GL_LINEAR
#define CFG_DEF_PLAYER_NAME     "Player"
/** Default IP address.
 *  @sa TCONFIG::address */
#define CFG_DEF_ADDRESS         ""
/** Default configuration's mouse sensitivity.
 *  @sa TCONFIG::ip_address */
#define CFG_DEF_SENSITIVITY     40
/** Default configuration's show_fps.
 *  @sa TCONFIG::densitivity */
#define CFG_DEF_SHOW_FPS        false
/** Default configuration's show_process_fps.
 *  @sa TCONFIG::show_process_fps */
#define CFG_DEF_SHOW_PROCESS_FPS     false
/** Default configuration's show_disconnect_warning.
 *  @sa TCONFIG::show_disconnect_warning */
#define CFG_DEF_SHOW_DISCONNECT_WARNING true
/** Default configuration's warfog alpha.
 *  @sa TCONFIG::warfog_alpha */
#define CFG_DEF_WARFOG_ALPHA    40
/** Default configuration's warfog color (red part).
 *  @sa TCONFIG::warfog_color */
#define CFG_DEF_WARFOG_COLOR_R  50
/** Default configuration's warfog color (green part).
 *  @sa TCONFIG::warfog_color */
#define CFG_DEF_WARFOG_COLOR_G  35
/** Default configuration's warfog color (blue part).
 *  @sa TCONFIG::warfog_color */
#define CFG_DEF_WARFOG_COLOR_B  15
/** Default configuration's fullscreen.
 *  @sa TCONFIG::fullscreen */
#define CFG_DEF_FULLSCREEN      true
/** Default configuration's vertical synchronisation.
 *  @sa TCONFIG::vert_sync */
#ifdef UNIX
# define CFG_DEF_VERT_SYNC      false
#else
# define CFG_DEF_VERT_SYNC      true
#endif
/** Default configuration's map move speed.
 *  @sa TCONFIG::map_move_speed */
#define CFG_DEF_MAP_MOVE_SPEED  50
/** Default configuration's zoom speed.
 *  @sa TCONFIG::map_zoom_speed */
#define CFG_DEF_MAP_ZOOM_SPEED  50
/** Default configuration's maximum frame rate.
 *  @sa TCONFIG::max_frame_rate */
#define CFG_DEF_MAX_FRAME_RATE  60
/** Default configuration's master volume.
 *  @sa TCONFIG::snd_master_volume */
#define CFG_DEF_SND_MASTER_VOLUME  100 
/** Default configuration's music volume in game.
 *  @sa TCONFIG::snd_game_music_volume */
#define CFG_DEF_SND_GAME_MUSIC_VOLUME  70
/** Default configuration's sound volume in game.
 *  @sa TCONFIG::snd_game_sound_volume */
#define CFG_DEF_SND_GAME_SOUND_VOLUME  100
/** Default configuration's music volume in menu.
 *  @sa TCONFIG::snd_menu_music_volume */
#define CFG_DEF_SND_MENU_MUSIC_VOLUME  100
/** Default configuration's sound volume in menu.
 *  @sa TCONFIG::snd_menu_sound_volume */
#define CFG_DEF_SND_MENU_SOUND_VOLUME  100
/** Default configuration's menu music toogle.
 *  @sa TCONFIG::snd_menu_music */
#define CFG_DEF_SND_MENU_MUSIC  true
/** Default configuration's game music toogle.
 *  @sa TCONFIG::snd_game_music */
#define CFG_DEF_SND_GAME_MUSIC  true
/** Default configuration's sound unit speech toogle in game.
 *  @sa TCONFIG::snd_unit_speech */
#define CFG_DEF_SND_UNIT_SPEECH  true
/** Default configuration's server port.
 *  @sa TCONFIG::net_server_port */
#define CFG_DEF_NET_SERVER_PORT  17000


//========================================================================
// Included files
//========================================================================

#ifdef NEW_GLFW3
#include <glfw3.h>
#else
#include <glfw.h>
#endif

#include "cfg.h"
#include "doalloc.h"

#include "dofile.h"
#include "doplayers.h"


//========================================================================
// struct TCONFIG
//========================================================================

/**
 *  Contains information stored in configuration file and some precompiled
 *  values.
 */
struct TCONFIG {

  TCONF_FILE *file;       //!< Associated configaration file handler.

  // information stored in configuration file

  bool fullscreen;        //!< Specifies, if application runs in fullscreen mode.
  bool vert_sync;         //!< Specifies, if vertical synchronisation is enabled.

  int scr_width;          //!< Width of the screen. [pixels]
  int scr_height;         //!< Height of the screen. [pixels]

  char player_name[PL_MAX_PLAYER_NAME_LENGTH];    //!< Player name.
  char address[PL_MAX_ADDRESS_LENGTH];            //!< Last conntected address.
  T_BYTE sensitivity;      //!< Mouse sensitivity. [1..100]

  int tex_mag_filter;     //!< Magnification texture filter. [GL_NEAREST, GL_LINEAR]
  int tex_min_filter;     //!< Minification texture filter. [GL_NEAREST, GL_LINEAR, GL_LINEAR_MIPMAP_NEAREST, ...]
  
  T_BYTE warfog_intensity; //!< Warfog intensity (alfa-channel). [0..100]
  T_BYTE warfog_color[3];  //!< Warfog color. [R: 0..255, G: 0..255, B: 0..255]

  bool show_fps;          //!< Specifies, if count of frames per second will be drawn.
  bool show_process_fps;  //!< Specifies, if count of frames per second of UpdateFunction() will be drawn.
  bool show_disconnect_warning;   //!< Specifies, if warning message box before disconnecting from server will be drawn.

  T_BYTE map_move_speed;  //!< Speed of map moving. [1..100]
  T_BYTE map_zoom_speed;  //!< Speed of map zooming. [1..100]
  
  /** Maximum frame rate per one second (fps). This is an approximate value,
   *  which is achieved using glfwSleep(). Usually the precision of glfwSleep()
   *  is in miliseconds and smallest possible sleep time is 1ms. That is why
   *  high values of #max_frame_rate cause real fps to be rounded to some
   *  values which correspond to 1ms, 2ms and 3ms (eg. 33 fps, 50 fps and 100
   *  fps). Values higher than 112 gives too small sleep times (smaller than
   *  1ms) and are ignored. Therefore setting this value higher than 112 gives
   *  maximum possible frame rate. [1..1000] */
  int max_frame_rate;

  // Sounds
  T_BYTE snd_master_volume;      //!< Specifies master volume. [0..100]
  T_BYTE snd_game_music_volume;  //!< Specifies volume of music in game. [0..100]
  T_BYTE snd_game_sound_volume;  //!< Specifies volume of sounds in game. [0..100]
  T_BYTE snd_menu_music_volume;  //!< Specifies volume of music in music. [0..100]
  T_BYTE snd_menu_sound_volume;  //!< Specifies volume of sounds in music. [0..100]
  bool snd_unit_speech;         //!< Unit's speech.
  bool snd_menu_music;          //!< Mute menu music.
  bool snd_game_music;          //!< Mute game music.

  // Network
  int net_server_port;          //!< Server port.

  // Precomputed values
  int pr_wnd_mode;                    //!< Precomputed window mode. [GLFW_WINDOW, GLFW_FULLSCREEN]
  GLubyte pr_warfog_color[4];         //!< Precomputed warfog color and alfa-channel. [-128..127]
  double pr_expected_frame_duration;  //!< Precomputed expected frame duration from max_frame_rate. [seconds]
#ifdef __GNUC__
	TCONFIG();
#else
  	TCONFIG::TCONFIG();
#endif
  
};


//========================================================================
// Variables
//========================================================================

extern TCONFIG config;


//========================================================================
// Functions
//========================================================================

bool LoadConfig(void);
void SaveConfig(void);


#endif // __doconfig_h__

//========================================================================
// End
//========================================================================
// vim:ts=2:sw=2:et:

