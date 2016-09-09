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
 *  @file cfg.h
 *
 *  Configuration options for pre-processing.
 *
 *  @author Marian Cerny
 *
 *  @date 2003, 2004
 */

#ifndef __cfg_h__
#define __cfg_h__

/**
 *  This macro is defined if compiling on Windows. On Unix the the macro @c
 *  UNIX is defined, but is passed to compiler with the @c -DUNIX option.
 */
#ifndef UNIX
#define WINDOWS
#endif

/**
 *  Specifies the base directory of all the data files. On Unix it is passed to
 *  compiler with the @c -DDATADIR="directory" option.
 */
#ifndef DATA_DIR
#define DATA_DIR ""
#endif

/**
 *  Specifies the debugging mode. In this mode some debugging information is
 *  also printed to log files, and the lines in the logs will have a header
 *  with source file and line number to easily find the position in the source
 *  files, where the message was printed.
 *
 *  If NDEBUG macro is defined (MS Visual Studio), DEBUG is set to 0.
 */
#ifndef DEBUG
  #ifdef NDEBUG
  #define DEBUG 0
  #else
  #define DEBUG 1
  #endif
#endif

/**
 *  Specifies the debugging mode for threads methods.
 */
#ifndef DEBUG_MEMORY
  #ifdef WINDOWS
  #define DEBUG_MEMORY   (DEBUG && 0)
  #else
  #define DEBUG_MEMORY   (0)
  #endif
#endif

/**
 *  Specifies the debugging mode for threads methods.
 */
#ifndef DEBUG_THREADS
#define DEBUG_THREADS   (DEBUG && 0)
#endif

/**
 *  Specifies the debugging mode for events methods.
 */
#ifndef DEBUG_EVENTS
#define DEBUG_EVENTS    (DEBUG && 0)
#endif

/**
 *  Specifies the debugging mode for sorting methods.
 */
#ifndef DEBUG_SORTING
#define DEBUG_SORTING   (DEBUG && 0)
#endif

/**
 *  Specifies the debugging mode for path finding.
 */
#ifndef DEBUG_PATHFINDING
#define DEBUG_PATHFINDING   (DEBUG && 0)
#endif

/**
 *  Specifies if should print log messages to stderr.
 */
#ifndef LOG_TO_STDERR
#define LOG_TO_STDERR  1
#endif

/**
 *  Specifies if should print log messages to log files. Log files are created
 *  in #LOG_PATH.
 */
#ifndef LOG_TO_LOGFILES
#define LOG_TO_LOGFILES  1
#endif

/**
 *  Specifies if should print log messages to #ost.
 */
#ifndef LOG_TO_OST
#define LOG_TO_OST  1
#endif

/**
 *  Specifies if should call external log callback.
 */
#ifndef LOG_TO_EXTERNAL_CALLBACK
#define LOG_TO_EXTERNAL_CALLBACK  1
#endif

/**
 *  Specifies if sound support should be compiled in.
 */
#ifndef SOUND
# ifdef UNIX
#  define SOUND  0
# else
#  define SOUND  1
# endif
#endif

#if defined(_WIN32) && defined(GLFW_BUILD_DLL)

 // We are building a Win32 DLL
 #define GLFWCALL     __stdcall

#elif defined(_WIN32) && defined(GLFW_DLL)

 #define GLFWCALL     __stdcall

#else

 // We are either building/calling a static lib or we are non-win32
#define GLFWCALL

#endif

#ifdef NEW_GLFW3
#define OBERON_WINDOW               0x00010001
#define OBERON_FULLSCREEN           0x00010002
#endif


#endif // __cfg_h__

//========================================================================
// End
//========================================================================
// vim:ts=2:sw=2:et:

