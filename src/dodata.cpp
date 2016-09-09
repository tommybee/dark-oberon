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
 *  @file dodata.cpp
 *
 *  Resources functions.
 *
 *  @author Peter Knut
 *
 *  @date 2003, 2004
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __MINGW32__
#include <GL/glu.h>
#endif

#include "doconfig.h"
#include "dodata.h"
#include "doengine.h"
#include "domouse.h"
#include "tga.h"


//=========================================================================
// Defines
//=========================================================================

// sound types
#define DAT_ST_MODULE   -1
#define DAT_ST_SAMPLE   0
#define DAT_ST_STREAM   1


//=========================================================================
// Global variables
//=========================================================================

TTEX_TABLE fonts_table;       //!< Textures used by fonts.
TTEX_TABLE gui_table;         //!< Common texures used for gui objects.
#if SOUND
TSND_TABLE sounds_table;      //!< Common sounds used in menu and game.
#endif


GLFfont  *font0 = NULL;       //!< Basic font.


//=========================================================================
// Useful Methods.
//=========================================================================

bool fReadString(char **txt, FILE *fr)
{
  unsigned char len = 0;

  fread(&len, sizeof(len), 1, fr);

  if (len) {
    *txt = NEW char[len+1];
    fread(*txt, sizeof(char), len, fr);
    (*txt)[len] = 0;
  }

  return (len > 0);
}


//=========================================================================
// TTEX_TABLE
//=========================================================================

/**
 *  Load textures from *.dat.
 *
 *  @param file_name  Filename of the dat file.
 *
 *  @return @c true on success, @c false otherwise.
 */
bool TTEX_TABLE::Load(const char *file_name, int mag_filter, int min_filter)
{
  FILE *fr;
  
  Info(LogMsg("Loading textures from '%s'", file_name));

  if (!(fr = fopen(file_name, "rb"))) {
    Error(LogMsg("Can not open file '%s'", file_name));
    return false;
  }

  TGA_INFO tga;
  int format, iformat;

  char   header[257];
  T_BYTE version;
  long   textures_seek = 0;

  int   tid, gid;   // texture id, group id
  TGUI_TEXTURE *tex;

  int    atime;                            // animation time [miliseconds]
  T_BYTE ttype;                            // texture type
  T_BYTE hcount;                           // horizontal frames count
  T_BYTE vcount;                           // vertical frames count
  int   pointx, pointy;
  unsigned int dsize;                     // data size

  // file header
  fread(header, sizeof(char), strlen(DAT_FILE_HEADER), fr);
  header[strlen(DAT_FILE_HEADER)] = 0;
  if (strcmp(header, DAT_FILE_HEADER)) {
    fclose(fr);
    return false;
  }

  // file version
  fread(&version, sizeof(version), 1, fr);

  if (version > DAT_MAX_VERSION || version < DAT_MIN_VERSION) {
    Error(LogMsg("Version of data file '%s' is not allowed", file_name));
    fclose(fr);
    return false;
  }

  // seeks
  fread(&textures_seek, sizeof(textures_seek), 1, fr);

  if (!textures_seek) {
    Error(LogMsg("Data file '%s' does not contain any textures", file_name));
    fclose(fr);
    return false;
  }

  glEnable(GL_TEXTURE_2D);

  // texture groups table
  fseek(fr, textures_seek, SEEK_SET);
  fread(&count, sizeof(count), 1, fr);

  if (!(groups = NEW TTEX_GROUP[count])) {
    Critical(LogMsg("Can not allocate memory for texture groups table from '%s'", file_name));
    fclose(fr);
    return false;
  }

  // texture groups
  for (gid = 0; gid < count; gid++) {

    fReadString(&groups[gid].name, fr);
    fread(&groups[gid].count, sizeof(groups[gid].count), 1, fr);
    
    if (!(groups[gid].textures = NEW TGUI_TEXTURE[groups[gid].count])) {
      Critical(LogMsg("Can not allocate memory for texture table from '%s'", file_name));
      fclose(fr);
      return false;
    }

    // textures
    for (tid = 0; tid < groups[gid].count; tid++) {
      tex = groups[gid].textures + tid;

      // read values from file
      fReadString(&tex->id, fr);
      fread(&hcount, sizeof(hcount), 1, fr);
      fread(&vcount, sizeof(vcount), 1, fr);     
      fread(&atime, sizeof(atime), 1, fr);
      fread(&pointx, sizeof(pointx), 1, fr);
      fread(&pointy, sizeof(pointy), 1, fr);
      fread(&ttype, sizeof(ttype), 1, fr);
      fread(&dsize, sizeof(dsize), 1, fr);

      // generate id for texture
      glGenTextures(1, &tex->gl_id);

      // read TGA image
      if (!tgaRead(fr, &tga, TGA_RESCALE)) {
        Error(LogMsg("Error reading TGA data from '%s'", file_name));
        return false;
      }

      if (tga.bytesperpixel == 3) format = iformat = GL_RGB;
      else format = iformat = GL_RGBA;

      // generate texture
      glBindTexture(GL_TEXTURE_2D, tex->gl_id);
    
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag_filter);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filter);

      // upload to memory
      if (min_filter == GL_NEAREST || min_filter == GL_LINEAR)
        glTexImage2D(GL_TEXTURE_2D, 0, iformat, tga.width, tga.height, 0, format, GL_UNSIGNED_BYTE, (void *)tga.data);
      else gluBuild2DMipmaps(GL_TEXTURE_2D, iformat, tga.width, tga.height, format, GL_UNSIGNED_BYTE, (void *)tga.data);

      // fill texture
      tex->type = (TGUI_TEX_TYPE)ttype;
      tex->point_x = -(GLfloat)pointx;
      tex->point_y = -(GLfloat)pointy;

      tex->h_count = hcount;
      tex->v_count = vcount;
      tex->frames_count = hcount * vcount;

      tex->frame_width = tga.original_width / hcount;
      tex->frame_height = tga.original_height / vcount;
      tex->width = tga.width;
      tex->height = tga.height;

      tex->frame_time = (double)atime / (1000 * tex->frames_count);

      // free memory
      free(tga.data);
      tga.data = NULL;

    } // for tid
  } // for gid

  fclose(fr);

  return true;
}


//=========================================================================
// TSND_TABLE
//=========================================================================

#if SOUND
bool TSND_TABLE::Load(const char *file_name)
{
  if (!file_name) return false;

  Info(LogMsg("Loading sounds from '%s'", file_name));

  char   header[257];
  T_BYTE version;
  int    sounds_seek = 0;
  FILE *fr;
  TMODULE *music = NULL;
  TSAMPLE *sample = NULL;
  TSTREAM *stream = NULL;
  TSOUND *snd;
  int sid;
  char sformat;
  char stype;
  int dsize;
  long dseek;
  char *data;
  char *id;

  Clear();

  if (!(fr = fopen(file_name, "rb"))) {
    Error(LogMsg("Can not open file '%s'", file_name));
    return false;
  }

  // file header
  fread(header, sizeof(char), strlen(DAT_FILE_HEADER), fr);
  header[strlen(DAT_FILE_HEADER)] = 0;
  if (strcmp(header, DAT_FILE_HEADER)) {
    fclose(fr);
    return false;
  }

  // file version
  fread(&version, sizeof(version), 1, fr);

  if (version > DAT_MAX_VERSION || version < DAT_MIN_VERSION) {
    Error(LogMsg("Version of data file '%s' is not allowed", file_name));
    fclose(fr);
    return false;
  }

  // seeks
  fread(&sounds_seek, sizeof(sounds_seek), 1, fr); // textures
  if (version > 2) fread(&sounds_seek, sizeof(sounds_seek), 1, fr); // sounds
  else sounds_seek = 0;

  if (!sounds_seek) {
    Error(LogMsg("Data file '%s' does not contain any sounds", file_name));
    fclose(fr);
    return false;
  }

  // sounds table
  fseek(fr, sounds_seek, SEEK_SET);
  fread(&count, sizeof(count), 1, fr);

  if (!(sounds = NEW TSOUND *[count])) {
    Critical(LogMsg("Can not allocate memory for sounds table from '%s'", file_name));
    fclose(fr);
    return false;
  }

  for (sid = 0; sid < count; sid++) sounds[sid] = NULL;

  // sounds
  for (sid = 0; sid < count; sid++) {
    music = NULL;
    sample = NULL;
    data = NULL;

    // read values from file
    fReadString(&id, fr);
    fread(&sformat, sizeof(sformat), 1, fr);
    fread(&stype, sizeof(stype), 1, fr);
    fread(&dsize, sizeof(dsize), 1, fr);
    dseek = ftell(fr);

    switch (stype) {
    case DAT_ST_MODULE:
      music = NEW TMODULE();
      sounds[sid] = music;
      break;

    case DAT_ST_STREAM:
      stream = NEW TSTREAM();
      sounds[sid] = stream;
      break;

    case DAT_ST_SAMPLE:
    default:
      sample = NEW TSAMPLE();
      sounds[sid] = sample;
      break;
    }

    snd = sounds[sid];

    // fill sound
    snd->id = id;
    snd->format = (TSOUND_FORMAT)sformat;

    // load data
    if (stype != DAT_ST_STREAM) {
      if (!(data = NEW char[dsize])) {
        Critical(LogMsg("Can not allocate memory for sound data from '%s'", file_name));
        Clear();
        return false;
      }
      fread(data, sizeof(char), dsize, fr);
    }
    else fseek(fr, dsize, SEEK_CUR);

    // load sound
    if (sample) 
      sample->Load(data, dsize);
    else if (music) 
      music->Load(data, dsize);
    else if (stream) 
      stream->Load(file_name, dseek, dsize);

    // free data
    if (data) delete[] data;
  }

  fclose(fr);

  return true;
}


void TSND_TABLE::Clear(void) { 
  int i;

  if (sounds) {
    for (i = 0; i < count; i++)
      if (sounds[i]) delete sounds[i];

    delete[] sounds;
    sounds = NULL;
  }

  count = 0;
};

TSOUND * TSND_TABLE::GetSound(char * id) {
  int i;

  for (i = 0; i < count; i++)
    if (!(strcmp(id, sounds[i]->id))) return sounds[i];
  
  return NULL;
}


#endif

//=========================================================================
// Data
//=========================================================================

/**
 *  Load textures.
 */
bool LoadData(void)
{
  Info("Loading data");

  if (fonts_table.Load(DAT_FONTS_NAME, config.tex_mag_filter, config.tex_min_filter) &&
      gui_table.Load(DAT_GUI_NAME, GL_LINEAR, GL_LINEAR) &&
      mouse.LoadData(DAT_CURSORS_NAME)
#if SOUND
      && sounds_table.Load(DAT_GUI_NAME)
#endif
      )
  {
#if SOUND
    PrepareSounds();
#endif

    return true;
  }

  else {
    DeleteData();
    Critical("Can not load data.");
    return false;
  }
}

/**
 *  Delete textures sets.
 */
void DeleteData(void)
{
  fonts_table.Clear();
  mouse.DeleteData();
  gui_table.Clear();

#if SOUND
  sounds_table.Clear();
#endif
}



//=========================================================================
// Fonts data
//=========================================================================

/**
 *  Initializes the fonts. Creates fonts.
 *
 *  @return @c true on success, @c false otherwise.
 */
bool CreateFonts(void)
{
  // common font
  if ((font0 = glfNewFont(fonts_table.groups[DAT_TGID_BASIC_FONT].textures[0].gl_id,
                          128, 256,
                          16, 12,
                          8, 16, 7,
                          config.scr_width, config.scr_height)))
  {
    glfSetFontBase(font0, 32, 32 - 96);
    return true;
  }
  else {
    Critical("Can not create fonts");
    return false;
  }
}


/**
 *   Destroys fonts structures.
 */
void DestroyFonts(void)
{
  glfDeleteFont(font0);
}


//=========================================================================
// END
//=========================================================================
// vim:ts=2:sw=2:et:

