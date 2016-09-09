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
 *  @file doconfig.cpp
 *
 *  Game configuration.
 *
 *  @author Peter Knut
 *
 *  @date 2002, 2003
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dofile.h"
#include "doconfig.h"
#include "doengine.h"


//========================================================================
// Global Variables
//========================================================================

/** Game configuration. */
TCONFIG config;


//=========================================================================
// Loading functions
//=========================================================================

void ComputePrecompiled()
{
  // compute precomputed values
  config.pr_expected_frame_duration = 1.0 / config.max_frame_rate;
#ifdef NEW_GLFW3
	if (config.fullscreen) config.pr_wnd_mode = OBERON_FULLSCREEN;
  	else config.pr_wnd_mode = OBERON_WINDOW;
#else
	if (config.fullscreen) config.pr_wnd_mode = GLFW_FULLSCREEN;
  else config.pr_wnd_mode = GLFW_WINDOW;
#endif
  

  config.pr_warfog_color[0] = (GLubyte)(config.warfog_color[0]);
  config.pr_warfog_color[1] = (GLubyte)(config.warfog_color[1]);
  config.pr_warfog_color[2] = (GLubyte)(config.warfog_color[2]);
  config.pr_warfog_color[3] = (GLubyte)(config.warfog_intensity * 2.55);
}


/** Constructor. Sets default configuration. */
TCONFIG::TCONFIG()
{
  fullscreen = CFG_DEF_FULLSCREEN;
  vert_sync = CFG_DEF_VERT_SYNC;

  scr_width = CFG_DEF_SCR_WIDTH;
  scr_height = CFG_DEF_SCR_HEIGHT;

  strcpy(player_name, CFG_DEF_PLAYER_NAME);
  strcpy(address, CFG_DEF_ADDRESS);
  sensitivity = CFG_DEF_SENSITIVITY;

  tex_mag_filter = CFG_DEF_TEX_MAG_FILTER;
  tex_min_filter = CFG_DEF_TEX_MIN_FILTER;
  
  warfog_intensity = CFG_DEF_WARFOG_ALPHA;
  warfog_color[0] = CFG_DEF_WARFOG_COLOR_R;
  warfog_color[1] = CFG_DEF_WARFOG_COLOR_G;
  warfog_color[2] = CFG_DEF_WARFOG_COLOR_B;

  show_fps = CFG_DEF_SHOW_FPS;
  show_process_fps = CFG_DEF_SHOW_PROCESS_FPS;
  show_disconnect_warning = CFG_DEF_SHOW_DISCONNECT_WARNING;

  map_move_speed = CFG_DEF_MAP_MOVE_SPEED;
  map_zoom_speed = CFG_DEF_MAP_ZOOM_SPEED;
  
  max_frame_rate = CFG_DEF_MAX_FRAME_RATE;

  snd_master_volume = CFG_DEF_SND_MASTER_VOLUME;
  snd_game_music_volume = CFG_DEF_SND_GAME_MUSIC_VOLUME;
  snd_game_sound_volume = CFG_DEF_SND_GAME_SOUND_VOLUME;
  snd_menu_music_volume = CFG_DEF_SND_MENU_MUSIC_VOLUME;
  snd_menu_sound_volume = CFG_DEF_SND_MENU_SOUND_VOLUME;
  snd_unit_speech = CFG_DEF_SND_UNIT_SPEECH;
  snd_menu_music = CFG_DEF_SND_MENU_MUSIC;
  snd_game_music = CFG_DEF_SND_GAME_MUSIC;

  net_server_port = CFG_DEF_NET_SERVER_PORT;

  ComputePrecompiled();
}


//=========================================================================
// Loading functions
//=========================================================================

/**
 *  Sets the screen resolution for the fulscreen mode.
 */
void LoadCfgResolution(void)
{
  TFILE_LINE value;

  config.file->ReadStr(value, const_cast<char*>("resolution"), const_cast<char*>(CFG_DEF_RESOLUTION), true);

  /* Check, whether the value is in the set of supported values. If not, write
   * some warning and change the value to default resolution
   * (CFG_DEF_RESOLUTION). */
  if (strcmp(value, "640x480")
      && strcmp(value, "800x600")
      && strcmp(value, "1024x768")
      && strcmp(value, "1152x864") 
      && strcmp(value, "1280x1024")
      && strcmp(value, "1600x1200"))
  {
    Warning(LogMsg("Invalid resolution value '%s'. Possible values are: 640x480 | 800x600 | 1024x768 | 1152x864 | 1280x1024 | 1600x1200", value));
    strcpy (value, CFG_DEF_RESOLUTION);
    Info (LogMsg ("Using default value %s", value));
  }

  /* The resolution string is parsed here. E.g. from "800x600" we get width =
   * 800 and height = 600. */
  char *endptr;
  config.scr_width = strtol (value, &endptr, 10);
  config.scr_height = strtol (endptr + 1, NULL, 10); 
}


/**
 *  Sets texture filter.
 */
void LoadCfgTextureFilter(void)
{
  TFILE_LINE value;

  config.file->ReadStr(value, const_cast<char*>("texture_filter"), const_cast<char*>(CFG_DEF_TEXTURE_FILTER), true);

  /* Check, whether the value is in the set of supported values. If not, write
   * some warning and change the value to default value for texture filter
   * (CFG_DEF_TEXTURE_FILTER). */
  if (strcmp(value, "nearest")
      && strcmp(value, "linear"))
  {
    Warning(LogMsg("Invalid texture_filter value '%s'. Possible values are: nearest | linear", value));
    strcpy (value, CFG_DEF_TEXTURE_FILTER);
    Info (LogMsg ("Using default value %s", value));
  }

  switch (value[0]) {
  case 'n': /* nearest */
    config.tex_mag_filter = GL_NEAREST;

    if (config.tex_min_filter == GL_LINEAR) config.tex_min_filter = GL_NEAREST;
    else if (config.tex_min_filter == GL_LINEAR_MIPMAP_NEAREST) config.tex_min_filter = GL_NEAREST_MIPMAP_NEAREST;
    else if (config.tex_min_filter == GL_LINEAR_MIPMAP_LINEAR) config.tex_min_filter = GL_NEAREST_MIPMAP_LINEAR;
    break;

  case 'l': /* linear */
    config.tex_mag_filter = GL_LINEAR;

    if (config.tex_min_filter == GL_NEAREST) config.tex_min_filter = GL_LINEAR;
    else if (config.tex_min_filter == GL_NEAREST_MIPMAP_NEAREST) config.tex_min_filter = GL_LINEAR_MIPMAP_NEAREST;
    else if (config.tex_min_filter == GL_NEAREST_MIPMAP_LINEAR) config.tex_min_filter = GL_LINEAR_MIPMAP_LINEAR;
    break;
  }
}


/**
 *  Set mipmap texture filter.
 */
void LoadCfgMipmapFilter(void)
{
  TFILE_LINE value;

  config.file->ReadStr(value, const_cast<char*>("mipmap_filter"), const_cast<char*>(CFG_DEF_MIPMAP_FILTER), true);

  /* Check, whether the value is in the set of supported values. If not, write
   * some warning and change the value to default value for texture filter
   * (CFG_DEF_TEXTURE_FILTER). */
  if (strcmp(value, "nearest")
      && strcmp(value, "none")
      && strcmp(value, "linear"))
  {
    Warning(LogMsg("Invalid mipmap_filter value '%s'. Possible values are: none | nearest | linear", value));
    strcpy (value, CFG_DEF_MIPMAP_FILTER);
    Info (LogMsg ("Using default value %s", value));
  }

  /* XXX: We check the second character here! */
  switch (value[1]) {
  case 'o': /* none */
    config.tex_min_filter = config.tex_mag_filter;
    break;

  case 'e': /* nearest */
    if (config.tex_mag_filter == GL_NEAREST) config.tex_min_filter = GL_NEAREST_MIPMAP_NEAREST;
    else config.tex_min_filter = GL_LINEAR_MIPMAP_NEAREST;
    break;

  case 'i': /* linear */
    if (config.tex_mag_filter == GL_NEAREST) config.tex_min_filter = GL_NEAREST_MIPMAP_LINEAR;
    else config.tex_min_filter = GL_LINEAR_MIPMAP_LINEAR;
    break;
  }
}


//=========================================================================
// Configuration
//=========================================================================

/**
 *  Writes default configuration into configuration file.
 */
void WriteDefConfig(void)
{
  // info
  config.file->WriteLine(const_cast<char*>("#"));
  config.file->WriteLine(const_cast<char*>("#  Dark Oberon - configuration file"));
  config.file->WriteLine(const_cast<char*>("#"));
  config.file->WriteLine(const_cast<char*>(""));

  // display
  config.file->WriteLine(const_cast<char*>("# *** Display ***"));
  config.file->WriteBool(const_cast<char*>("fullscreen"), CFG_DEF_FULLSCREEN);
  config.file->WriteStr(const_cast<char*>("resolution"), const_cast<char*>(CFG_DEF_RESOLUTION));
  config.file->WriteBool(const_cast<char*>("vert_sync"), CFG_DEF_VERT_SYNC);
  config.file->WriteLine(const_cast<char*>(""));

  // graphics
  config.file->WriteLine(const_cast<char*>("# *** Graphics ***"));
  config.file->WriteStr(const_cast<char*>("texture_filter"), const_cast<char*>(CFG_DEF_TEXTURE_FILTER));
  config.file->WriteStr(const_cast<char*>("mipmap_filter"), const_cast<char*>(CFG_DEF_MIPMAP_FILTER));
  config.file->WriteInt(const_cast<char*>("warfog_color"), CFG_DEF_WARFOG_COLOR_R);
  config.file->WriteInt(const_cast<char*>("warfog_color"), CFG_DEF_WARFOG_COLOR_G);
  config.file->WriteInt(const_cast<char*>("warfog_color"), CFG_DEF_WARFOG_COLOR_B);
  config.file->WriteInt(const_cast<char*>("warfog_intensity"), CFG_DEF_WARFOG_ALPHA);
  config.file->WriteBool(const_cast<char*>("show_fps"), CFG_DEF_SHOW_FPS);
  config.file->WriteBool(const_cast<char*>("show_process_fps"),CFG_DEF_SHOW_PROCESS_FPS);
  config.file->WriteBool(const_cast<char*>("show_disconnect_warning"), CFG_DEF_SHOW_DISCONNECT_WARNING);
  config.file->WriteInt(const_cast<char*>("max_frame_rate"), CFG_DEF_MAX_FRAME_RATE);
  config.file->WriteInt(const_cast<char*>("map_move_speed"), CFG_DEF_MAP_MOVE_SPEED);
  config.file->WriteInt(const_cast<char*>("map_zoom_speed"), CFG_DEF_MAP_ZOOM_SPEED);
  config.file->WriteLine(const_cast<char*>(""));
  
  //sounds
  config.file->WriteLine(const_cast<char*>("# *** Audio ***"));
  config.file->WriteInt(const_cast<char*>("snd_master_volume"), CFG_DEF_SND_MASTER_VOLUME);
  config.file->WriteInt(const_cast<char*>("snd_menu_music_volume"), CFG_DEF_SND_MENU_MUSIC_VOLUME);
  config.file->WriteInt(const_cast<char*>("snd_menu_sound_volume"), CFG_DEF_SND_MENU_SOUND_VOLUME);
  config.file->WriteInt(const_cast<char*>("snd_game_music_volume"), CFG_DEF_SND_GAME_MUSIC_VOLUME);
  config.file->WriteInt(const_cast<char*>("snd_game_sound_volume"), CFG_DEF_SND_GAME_SOUND_VOLUME);
  config.file->WriteBool(const_cast<char*>("snd_menu_music"), CFG_DEF_SND_GAME_MUSIC);
  config.file->WriteBool(const_cast<char*>("snd_game_music"), CFG_DEF_SND_UNIT_SPEECH);
  config.file->WriteBool(const_cast<char*>("snd_unit_speech"), CFG_DEF_SND_MENU_MUSIC);
  config.file->WriteLine(const_cast<char*>(""));

  // customize
  config.file->WriteLine(const_cast<char*>("# *** Customize ***"));
  config.file->WriteStr(const_cast<char*>("player_name"), const_cast<char*>(CFG_DEF_PLAYER_NAME));
  config.file->WriteStr(const_cast<char*>("address"), const_cast<char*>(CFG_DEF_ADDRESS));
  config.file->WriteInt(const_cast<char*>("sensitivity"), CFG_DEF_SENSITIVITY);
  config.file->WriteLine(const_cast<char*>(""));

  // Networking options.
  config.file->WriteLine(const_cast<char*>("# *** Networking options ***"));
  config.file->WriteInt(const_cast<char*>("net_server_port"), CFG_DEF_NET_SERVER_PORT);
}


/**
 *  Opens and loads configuration file.
 *  If configuration file does not exist, creates new one and writes default values into it.
 */
bool LoadConfig(void)
{
  Info(LogMsg("%s%s%s", "Loading configuration from '", CFG_FILE_NAME, "'"));

  // Tries to open config file. If the file does not exists, new config file is
  // created, default values are written into it and the file is saved.
  if (!(config.file = OpenConfFile(CFG_FILE_NAME))) {
    Info(LogMsg("%s%s%s", "Creating new configuration file '", CFG_FILE_NAME, "'"));

    if (!(config.file = CreateConfFile(CFG_FILE_NAME))) {
      Error(LogMsg("%s%s%s", "Can not creating configuration file '", CFG_FILE_NAME, "'"));
      return false;
    }

    WriteDefConfig();           // write default values into it
    config.file->Save();        // and save (create) file
  }

  // display
  config.file->ReadBool(&config.fullscreen, const_cast<char*>("fullscreen"), CFG_DEF_FULLSCREEN);
  config.file->ReadBool(&config.vert_sync, const_cast<char*>("vert_sync"), CFG_DEF_VERT_SYNC);
  LoadCfgResolution();

  // graphics
  LoadCfgTextureFilter();
  LoadCfgMipmapFilter();

  config.file->ReadByteRange(config.warfog_color, const_cast<char*>("warfog_color"), 0, 255, CFG_DEF_WARFOG_COLOR_R);
  config.file->ReadByteRange(config.warfog_color+1, const_cast<char*>("warfog_color"), 0, 255, CFG_DEF_WARFOG_COLOR_G);
  config.file->ReadByteRange(config.warfog_color+2, const_cast<char*>("warfog_color"), 0, 255, CFG_DEF_WARFOG_COLOR_B);
  config.file->ReadByteRange(&config.warfog_intensity, const_cast<char*>("warfog_intensity"), 0, 100, CFG_DEF_WARFOG_ALPHA);

  config.file->ReadBool(&config.show_fps, const_cast<char*>("show_fps"), CFG_DEF_SHOW_FPS);
  config.file->ReadBool(&config.show_process_fps, const_cast<char*>("show_process_fps"), CFG_DEF_SHOW_PROCESS_FPS);
  config.file->ReadBool(&config.show_disconnect_warning, const_cast<char*>("show_disconnect_warning"), CFG_DEF_SHOW_DISCONNECT_WARNING);
  config.file->ReadIntRange(&config.max_frame_rate, const_cast<char*>("max_frame_rate"), 1, 1000, CFG_DEF_MAX_FRAME_RATE);
  config.file->ReadByteRange(&config.map_move_speed, const_cast<char*>("map_move_speed"), 1, 100, CFG_DEF_MAP_MOVE_SPEED);
  config.file->ReadByteRange(&config.map_zoom_speed, const_cast<char*>("map_zoom_speed"), 1, 100, CFG_DEF_MAP_ZOOM_SPEED);

  // audio
  config.file->ReadByteRange(&config.snd_master_volume, const_cast<char*>("snd_master_volume"), 0, 100, CFG_DEF_SND_MASTER_VOLUME);
  config.file->ReadByteRange(&config.snd_game_music_volume, const_cast<char*>("snd_game_music_volume"), 0, 100, CFG_DEF_SND_GAME_MUSIC_VOLUME);
  config.file->ReadByteRange(&config.snd_game_sound_volume, const_cast<char*>("snd_game_sound_volume"), 0, 100, CFG_DEF_SND_GAME_SOUND_VOLUME);
  config.file->ReadByteRange(&config.snd_menu_music_volume, const_cast<char*>("snd_menu_music_volume"), 0, 100, CFG_DEF_SND_MENU_MUSIC_VOLUME);
  config.file->ReadByteRange(&config.snd_menu_sound_volume, const_cast<char*>("snd_menu_sound_volume"), 0, 100, CFG_DEF_SND_MENU_SOUND_VOLUME);
  
  config.file->ReadBool(&config.snd_unit_speech, const_cast<char*>("snd_unit_speech"), CFG_DEF_SND_MENU_MUSIC);
  config.file->ReadBool(&config.snd_menu_music, const_cast<char*>("snd_menu_music"), CFG_DEF_SND_GAME_MUSIC);
  config.file->ReadBool(&config.snd_game_music, const_cast<char*>("snd_game_music"), CFG_DEF_SND_UNIT_SPEECH);

  // customize
  config.file->ReadStr(config.player_name, const_cast<char*>("player_name"), const_cast<char*>(CFG_DEF_PLAYER_NAME), true);
  config.file->ReadStr(config.address, const_cast<char*>("address"), const_cast<char*>(CFG_DEF_ADDRESS), true);
  config.file->ReadByteRange(&config.sensitivity, const_cast<char*>("sensitivity"), 0, 100, CFG_DEF_SENSITIVITY);

  // Networking options.
  config.file->ReadIntRange(&config.net_server_port, const_cast<char*>("net_server_port"), 1024, 65535, CFG_DEF_NET_SERVER_PORT);
  
  ComputePrecompiled();

  return true;
}


/**
 *  Saves and closes configuration file.
 */
void SaveConfig(void)
{
  if (!config.file) return;

  Info(LogMsg("%s%s%s", "Saving configuration to '", CFG_FILE_NAME, "'"));

  CloseConfFile(config.file);
}

//=========================================================================
// END
//=========================================================================
// vim:ts=2:sw=2:et:

