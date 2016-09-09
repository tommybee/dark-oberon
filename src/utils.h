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
 *  @file utils.h
 *
 *  Wrapped up versions of importable functions.
 *
 *  Functions that are not portable under both Unix and Windows are wrapped up
 *  here under different names to concentrate importable code into one place.
 *  There is usually no overhead, because those functions are declared inline
 *  where possible.
 *
 *  @author Marian Cerny
 *
 *  @date 2004, 2005
 */

#ifndef __utils_h__
#define __utils_h__


//=========================================================================
// Included files
//=========================================================================

#include "cfg.h"

#include <string>

#ifdef WINDOWS
#  include <winsock.h>
#  include <direct.h>
#else
#  include <unistd.h>
#  include <sys/types.h>
#  include <sys/stat.h>
#endif


//=========================================================================
// Functions
//=========================================================================

inline int do_close (int fd) {
#ifdef WINDOWS
  return closesocket (fd);
#else
  return close (fd);
#endif
}

inline int do_mkdir (std::string path)
{
#ifdef WINDOWS
  return _mkdir(path.c_str());
#else
  return mkdir(path.c_str(), 0777);
#endif
}


#endif

//=========================================================================
// END
//=========================================================================
// vim:ts=2:sw=2:et:

