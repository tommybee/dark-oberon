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
 *  @file dologs.cpp
 *
 *  Log files methods.
 *
 *  @author Peter Knut
 *  @author Marian Cerny
 *
 *  @date 2003, 2004
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "cfg.h"
#include "doalloc.h"
#include "dologs.h"
#include "dologs.h"
#include "doengine.h"
#include "utils.h"


//=========================================================================
// Variables
//=========================================================================


/**
 *  String for function LogMsg().
 */
TLOG_MESSAGE log_msg;
GLFWmutex log_mutex = NULL;            //!< Mutex for LogMsg function.

/**
 *  Error log file. By default, only errors come here. This can be changed by
 *  #ERR_LOG_LEVELS.
 */
FILE *err_log;

/**
 *  Full log file. By default, all logs go here. This can be changed by
 *  #FULL_LOG_LEVELS.
 */
FILE *full_log;

/**
 *  Variable holding pointer to callback function which is called every time a
 *  log message is created. It is set by RegisterLogCallback().
 */
void (*log_callback)(int, const char *, const char *) = NULL;


//=========================================================================
// LogMsg
//=========================================================================

/**
 *  Construct the log message. This function is similar to sprintf, but the
 *  output is always in @c log_msg, so you don't need to care about memory
 *  allocation.
 *
 *  @param msg Format of the message
 *  @param ... Arguments
 *
 *  @return Pointer to message in memory.
 */
char *LogMsg(const char *msg, ...)
{
  va_list arg;

  va_start(arg, msg);

#ifdef WINDOWS  // on Windows
  _vsnprintf(log_msg, MAX_LOG_MESSAGE_SIZE, msg, arg);
#else // on UNIX
  vsnprintf(log_msg, MAX_LOG_MESSAGE_SIZE, msg, arg);
#endif
  
  va_end(arg);

  return log_msg;
}


//=========================================================================
// LogFiles
//=========================================================================

/**
 *  Creates mutex for LogMsg() method.
 */
bool CreateLogMutex(void)
{
  // create mutex
  if ((log_mutex = glfwCreateMutex ()) == NULL) {
    Critical ("Could not create log mutex");
    return false;
  }

  return true;
}


/**
 *  Destroys mutex for LogMsg() method.
 */
void DestroyLogMutex(void)
{
  glfwDestroyMutex(log_mutex);
  log_mutex = NULL;
}


/**
 *  Opens a log file for writing.
 *
 *  @param name Filename of the log file.
 *
 *  @note Macro #LOG_TO_LOGFILES must be set to 1 to enable logging to log
 *        files.
 *
 *  @return FILE pointer to newly opened file or NULL on error.
 */
FILE * OpenLogFile(const char *name)
{
  FILE *ret;

  if ((ret = fopen(name, "w")) == NULL)
    Error(LogMsg("Can not open '%s' file. Logging is disabled", name));

  return ret;
}


/**
 *  Opens all log files. Also creates directory for log files, if not yet
 *  created. It calls OpenLogFile() for every log file.
 *
 *  @note Macro @c LOG_TO_LOGFILES must be set to 1 to enable logging to log
 *        files.
 *
 *  @return @c true on success, otherwise @c false.
 */
bool OpenLogFiles(void)
{
#if LOG_TO_LOGFILES
  do_mkdir (LOG_PATH);
  err_log =  OpenLogFile (LOG_ERR_NAME);
  full_log = OpenLogFile (LOG_FULL_NAME);

  return err_log != NULL && full_log != NULL;
#else
  return false;
#endif
}


/**
 *  Closes all log files.
 *
 *  @note Macro #LOG_TO_LOGFILES must be set to 1 to enable logging to log
 *        files.
 */
void CloseLogFiles(void)
{
#if LOG_TO_LOGFILES
  if (err_log)  fclose (err_log);
  if (full_log) fclose (full_log);
#endif

  glfwDestroyMutex(log_mutex);
}

/**
 *  Registers a callback function which should be called every time a log
 *  message is created. This is used for logging to some external place. We use
 *  it to log to screen using #ost.
 *
 *  @param f  Pointer to callback function. The function should have the
 *            following prototype:
 *
 *            @code
 *            void function_name (int level, const char *header, const char *message)
 *            @endcode
 *
 *            Where @a level is level of the log message (one of #LOG_DEBUG,
 *            #LOG_INFO, #LOG_WARNING, #LOG_ERROR or #LOG_CRITICAL), @a header
 *            is text description of the log level and @a message is the log
 *            message itself.
 *
 *  @note Only one function can be registered. Every time a new callback
 *        function is registered, the previous is forgeted.
 */
void RegisterLogCallback (void (*f)(int, const char *, const char *))
{
  log_callback = f;
}

//=========================================================================
// END
//=========================================================================
// vim:ts=2:sw=2:et:

