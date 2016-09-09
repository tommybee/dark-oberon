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
 *  Sounds functions.
 *
 *  @author Peter Knut
 *
 *  @date 2004
 */


#include "cfg.h"
#include "doalloc.h"
#include "dologs.h"
#include "dosound.h"

#if SOUND
# include <fmod_errors.h>
#endif

#if SOUND


//=========================================================================
// InitSound
//=========================================================================

bool InitSound() {
  if (FSOUND_GetVersion() < FMOD_VERSION) {
    Error("Wrong FMOD version");
    return false;
  }

  if (!FSOUND_Init(SND_MIXRATE, SND_MAX_CHANNELS, FSOUND_INIT_USEDEFAULTMIDISYNTH)) {
    Error(LogMsg("Can not initialize FMOD - %s", FMOD_ErrorString(FSOUND_GetError())));
    return false;
  }

  return true;
}


//=========================================================================
// TCHANNEL
//=========================================================================

void TCHANNEL::_SetVolume()
{
  if (channel >= 0 && vol_type != VT_NONE) {
    if (vol_type == VT_NORMAL) FSOUND_SetVolume(channel, volume);
    else FSOUND_SetVolumeAbsolute(channel, volume);
  }
}


void TCHANNEL::SetVolume(T_BYTE vol)
{ 
  volume = vol;
  vol_type = VT_NORMAL;

  _SetVolume();
}


void TCHANNEL::SetVolumeAbsolute(T_BYTE vol)
{ 
  volume = vol;
  vol_type = VT_ABSOLUTE;

  _SetVolume();
}


bool TCHANNEL::IsPlaying()
{ 
  return channel >= 0 && FSOUND_IsPlaying(channel);
}


//=========================================================================
// TSAMPLE
//=========================================================================

TSAMPLE::~TSAMPLE()
{ 
  if (sample) FSOUND_Sample_Free(sample);
}


bool TSAMPLE::Load(char *data, int size)
{
  if (sample) {
    FSOUND_Sample_Free(sample);
    channel = -1;
  }

  sample = FSOUND_Sample_Load(FSOUND_FREE, data, FSOUND_NORMAL | FSOUND_LOADMEMORY, 0, size);

  return (sample != NULL);
}


void TSAMPLE::SetMaxPlaybacks(int max)
{
  if (sample) FSOUND_Sample_SetMaxPlaybacks(sample, max);
}


void TSAMPLE::Play()
{ 
  if (sample) channel = FSOUND_PlaySound(FSOUND_FREE, sample);

  // set properties
  _SetVolume();
}


void TSAMPLE::Stop()
{ 
  if (channel >= 0) FSOUND_StopSound(channel);
  channel = -1;
}


void TSAMPLE::SetLoop(bool lp)
{
  if (loop == lp) return;

  loop = lp;

  if (sample) {
    if (loop)
      FSOUND_Sample_SetMode(sample, FSOUND_LOOP_NORMAL);
    else
      FSOUND_Sample_SetMode(sample, FSOUND_LOOP_OFF);
  }
}


//=========================================================================
// TSTREAM
//=========================================================================


TSTREAM::~TSTREAM()
{ 
  Stop();
  if (stream) FSOUND_Stream_Close(stream);
}


bool TSTREAM::Load(const char *file_name, int seek, int size)
{
  if (stream) {
    FSOUND_Stream_Close(stream);
    channel = -1;
  }

  stream = FSOUND_Stream_Open(file_name, FSOUND_SIGNED | FSOUND_2D, seek, size);
  return (stream != NULL); 
}


void TSTREAM::Play()
{
  if (stream) channel = FSOUND_Stream_Play(FSOUND_FREE, stream);

  // set properties
  _SetVolume();
}


void TSTREAM::Stop()
{ 
  if (stream && channel >= 0) FSOUND_Stream_Stop(stream);
  channel = -1;
}


void TSTREAM::SetLoop(bool lp)
{
  if (loop == lp) return;

  loop = lp;

  signed char res;

  if (stream) {
    if (loop)
      res = FSOUND_Stream_SetMode(stream, FSOUND_LOOP_NORMAL);
    else
      res = FSOUND_Stream_SetMode(stream, FSOUND_LOOP_OFF);
  }
}


//=========================================================================
// TMODULE
//=========================================================================

TMODULE::~TMODULE()
{ 
  if (mod) FMUSIC_FreeSong(mod);
}


bool TMODULE::Load(char *data, int size)
{
  if (mod) FMUSIC_FreeSong(mod);

  mod = FMUSIC_LoadSongEx(data, 0, size, FSOUND_LOADMEMORY | FSOUND_LOOP_OFF, NULL, 0);
  if (mod) FMUSIC_SetLooping(mod, false);
  return (mod != NULL);
}


void TMODULE::Play()
{ 
  if (mod) FMUSIC_PlaySong(mod);
}


void TMODULE::Stop()
{ 
  if (mod) FMUSIC_StopSong(mod);
}


void TMODULE::SetVolume(T_BYTE vol)
{ 
  if (mod) FMUSIC_SetMasterVolume(mod, vol);
}


void TMODULE::SetVolumeAbsolute(T_BYTE vol)
{ 
  SetVolume(vol);
}


void TMODULE::SetLoop(bool lp)
{
  if (loop == lp) return;

  loop = lp;

  if (mod) FMUSIC_SetLooping(mod, lp);
}


bool TMODULE::IsPlaying()
{ 
  return mod && FMUSIC_IsPlaying(mod);
}

#endif  // #if SOUND

//=========================================================================
// END
//=========================================================================
// vim:ts=2:sw=2:et:

