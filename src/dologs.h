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
 *  @file dologs.h
 *
 *  Log files declarations and methods.
 *
 *  @author Marian Cerny
 *  @author Peter Knut
 *
 *  @date 2003, 2004
 */

#ifndef __dologs_h__
#define __dologs_h__

//========================================================================
// Definitions
//========================================================================

#if LOG_TO_LOGFILES
/**
 *   Directory, where log files are created.
 *   @note Macro is only available if #LOG_TO_LOGFILES is specified.
 */
# define LOG_PATH        (user_dir + DATA_DIR "logs/").c_str()
/**
 *   Filename of the error log #err_log.
 *   @note Macro is only available if #LOG_TO_LOGFILES is specified.
 */
# define LOG_ERR_NAME    (std::string(LOG_PATH) + "error.log").c_str()
# define LOG_FULL_NAME   (std::string(LOG_PATH) + "full.log").c_str()
#endif

/** @def LOG_DEBUG
 *  Debug level of log message. */
/** @def LOG_INFO
 *  Info level of log message. */
/** @def LOG_WARNING
 *  Warning level of log message. */
/** @def LOG_ERROR
 *  Error level of log message. */
/** @def LOG_CRITICAL
 *  Critical level of log message. */
#define LOG_DEBUG    (1 << 0)
#define LOG_INFO     (1 << 1)
#define LOG_WARNING  (1 << 2)
#define LOG_ERROR    (1 << 3)
#define LOG_CRITICAL (1 << 4)

/** @def ERR_LOG_LEVELS
 *  Levels of log messages which are logged into #err_log. */
/** @def FULL_LOG_LEVELS
 *  Levels of log messages which are logged into #full_log. */
/** @def STDERR_LOG_LEVELS
 *  Levels of log messages which are logged into stderr. */
/** @def EXTERNAL_LOG_LEVELS
 *  Levels of log messages which are logged with external callback function
 *  specified by RegisterLogCallback(). */
#define ERR_LOG_LEVELS      (LOG_WARNING | LOG_ERROR | LOG_CRITICAL)
#define FULL_LOG_LEVELS     (LOG_DEBUG | LOG_INFO | LOG_WARNING | LOG_ERROR | LOG_CRITICAL)
#define STDERR_LOG_LEVELS   (LOG_DEBUG | LOG_INFO | LOG_WARNING | LOG_ERROR | LOG_CRITICAL)
#define EXTERNAL_LOG_LEVELS (LOG_INFO | LOG_WARNING | LOG_ERROR | LOG_CRITICAL)

/** @def TEXT_DEBUG
 *  Header of the log in macro Debug().
 *  @note On @c UNIX it is defined in more condensed format. */
/** @def TEXT_INFO
 *  Header of the log in macro Info().
 *  @note On @c UNIX it is defined in more condensed format. */
/** @def TEXT_WARNING
 *  Header of the log in macro Warning().
 *  @note On @c UNIX it is defined in more condensed format. */
/** @def TEXT_ERROR
 *  Header of the log in macro Error().
 *  @note On @c UNIX it is defined in more condensed format. */
/** @def TEXT_CRITICAL
 *  Header of the log in macro Critical().
 *  @note On @c UNIX it is defined in more condensed format. */
#ifdef WINDOWS  // on Windows
# define TEXT_DEBUG      "[Debug]   "
# define TEXT_INFO       "[Info]    "
# define TEXT_WARNING    "[Warning] "
# define TEXT_ERROR      "[Error]   "
# define TEXT_CRITICAL   "[Critical]"

#else // on UNIX
# define TEXT_DEBUG      " DBG:"
# define TEXT_INFO       "Info:"
# define TEXT_WARNING    "Warn:"
# define TEXT_ERROR      " Err:"
# define TEXT_CRITICAL   "Crit:"
#endif

/**
 *  Maximum size of the log message.
 */
#define MAX_LOG_MESSAGE_SIZE 1024


//========================================================================
// Included files
//========================================================================

#include "cfg.h"
#include "doalloc.h"

#include <stdio.h>
#include <string.h>

#ifdef NEW_GLFW3
#include <glfw3.h>
#include <tinycthread.h>
#else
#include <glfw.h>
#endif


//========================================================================
// Typedefs
//========================================================================

/**
 *   Type of the log message string.
 */
typedef char TLOG_MESSAGE[MAX_LOG_MESSAGE_SIZE];

//========================================================================
// Variables
//========================================================================
#ifdef NEW_GLFW3
extern mtx_t log_mutex;
#else
extern GLFWmutex log_mutex;
#endif



//========================================================================
// Macros
//========================================================================

/**
 *  Write information about source file and line number, where one of the
 *  macros Debug(), Info(), Warning(), Error(), Critical() was used.
 *
 *  @param logfile Log file (FILE *).
 *
 *  @note Macro #DEBUG must be defined to enable this.
 */
#if DEBUG
# ifdef WINDOWS
// On Windows we change (for example) '\Projects\dark-oberon\src\dodata.cpp' to
// 'dodata.cpp'.
// For some Windows compilers (for example Dev C++) the same thing is done when
// forward slashes are used instead of backward slashes in pathnames. For
// example 'src/dodata.cpp' is changed to 'dodata.cpp'.
#  define LogFileInfo(logfile) \
     do { \
       char *file; \
       file = strrchr(__FILE__, '\\'); \
       if (!file) file = strrchr(__FILE__, '/'); \
       if (file) file++; \
       else file = __FILE__; \
       fprintf(logfile, "[%13s:%4d] ", file, __LINE__); \
       fflush (logfile); \
     } while (0)

# else // on UNIX

#  define LogFileInfo(logfile) \
     do { \
       fprintf(logfile, "[%13s:%4d] ", __FILE__, __LINE__); \
       fflush (logfile); \
     } while (0)

# endif // #ifdef WINDOWS

#else // not DEBUG
# define LogFileInfo(logfile)
#endif // #if DEBUG


/**
 *  Log log message to logfile. If #DEBUG is defined, information about
 *  source file and line number, where the Log() was used, is printed using
 *  LogFileInfo().
 *
 *  @param logfile Log file (FILE *).
 *  @param header  Header to be printed (char *).
 *  @param msg     Log message (char *).
 *
 *  @note Macro #LOG_TO_LOGFILES must be defined, to enable logging to
 *        logfiles.
 */
#define Log_logfile(logfile, header, msg) \
do { \
  if (logfile != NULL) { \
    LogFileInfo(logfile); \
    fprintf(logfile, "%s ", header); \
    fprintf(logfile, "%s\n", msg); \
    fflush(logfile); \
  } \
} while (0)

/**
 *  Log log message to logfiles.
 *
 *  @param level   Log level (#LOG_DEBUG, #LOG_INFO, #LOG_WARNING, #LOG_ERROR, #LOG_CRITICAL).
 *  @param header  Header to be printed (char *).
 *  @param msg     Log message (char *).
 *
 *  @note Macro #LOG_TO_LOGFILES must be defined, to enable logging to
 *        logfiles.
 */
#if LOG_TO_LOGFILES
# define Log_logfiles(level, header, msg) \
do { \
  if (level & ERR_LOG_LEVELS) \
    Log_logfile(err_log, header, msg); \
  if (level & FULL_LOG_LEVELS) \
    Log_logfile(full_log, header, msg); \
} while (0)
#else
# define Log_logfiles(level, header, msg)
#endif

/**
 *  Log log message to stderr. If #DEBUG is defined, information about
 *  source file and line number, where the Log() was used, is printed using
 *  LogFileInfo().
 *
 *  @param level  Log level (#LOG_DEBUG, #LOG_INFO, #LOG_WARNING, #LOG_ERROR, #LOG_CRITICAL).
 *  @param header Header to be printed (char *).
 *  @param msg    Log message (char *).
 *
 *  @note Macro #LOG_TO_STDERR must be defined, to enable logging to stderr.
 */
#if LOG_TO_STDERR
# define Log_stderr(level, header, msg) \
do { \
  if (level & STDERR_LOG_LEVELS) { \
    LogFileInfo(stderr); \
    fprintf(stderr, "%s ", header); \
    fprintf(stderr, "%s\n", msg); \
  } \
} while (0)
#else
# define Log_stderr(level, header, msg)
#endif

/**
 *  Log a messages with external callback registered with
 *  RegisterLogCallback().
 *
 *  @param level   Log level (#LOG_DEBUG, #LOG_INFO, #LOG_WARNING, #LOG_ERROR, #LOG_CRITICAL).
 *  @param header  Header to be printed before message (char *).
 *  @param msg     Log message (char *).
 *
 *  @note Macro #LOG_TO_EXTERNAL_CALLBACK must be defined, to enable logging
 *  with external callback.
 */
#if LOG_TO_EXTERNAL_CALLBACK
# define Log_callback(level, header, msg) \
do { \
  if (level & EXTERNAL_LOG_LEVELS) { \
    if (log_callback) log_callback (level, header, msg); \
  } \
} while (0)
#else
# define Log_callback(level, header, msg)
#endif

/**
 *  Log a messages. It will be logged to logfiles using Log_logfiles(), @c
 *  stderr using Log_stderr() and with external callback function using
 *  Log_callback().
 *
 *  @param level   Log level (#LOG_DEBUG, #LOG_INFO, #LOG_WARNING, #LOG_ERROR, #LOG_CRITICAL).
 *  @param header  Header to be printed before message (char *).
 *  @param msg     Log message (char *).
 *
 *  @note Macro #LOG_TO_LOGFILES must be defined, to enable logging to
 *        logfiles.
 *  @note Macro #LOG_TO_STDERR must be defined, to enable logging to stderr.
 *  @note Macro #LOG_TO_EXTERNAL_CALLBACK must be defined, to enable logging
 *        to external callback function.
 *
 *  @see Debug(), Info(), Warning(), Error(), Critical()
 */
#ifdef NEW_GLFW3

#define Log(level, header, msg) \
do { \
  mtx_lock(&log_mutex); \
  \
  Log_stderr(level, header, msg); \
  Log_logfiles(level, header, msg); \
  Log_callback(level, header, msg); \
  \
  mtx_unlock(&log_mutex); \
} while (0)

#else

#define Log(level, header, msg) \
do { \
  if (log_mutex) glfwLockMutex(log_mutex); \
  \
  Log_stderr(level, header, msg); \
  Log_logfiles(level, header, msg); \
  Log_callback(level, header, msg); \
  \
  if (log_mutex) glfwUnlockMutex(log_mutex); \
} while (0)

#endif



/** Log info messages. It is implemented by the macro Log(). */
#define Info(msg)       Log(LOG_INFO,     TEXT_INFO,     (msg))
/** Log warning messages. It is implemented by the macro Log(). */
#define Warning(msg)    Log(LOG_WARNING,  TEXT_WARNING,  (msg))
/** Log error messages. It is implemented by the macro Log(). */
#define Error(msg)      Log(LOG_ERROR,    TEXT_ERROR,    (msg))
/** Log critical messages. It is implemented by the macro Log(). */
#define Critical(msg)   Log(LOG_CRITICAL, TEXT_CRITICAL, (msg))

/**
 *  Log debug messages. It is implemented by the macro Log().
 *  @note If #DEBUG is not defined, Debug() is defined empty.
 */
#if DEBUG
# define Debug(msg)     Log(LOG_DEBUG,    TEXT_DEBUG,    (msg))
#else
# define Debug(msg)
#endif


//=========================================================================
// Variables
//=========================================================================

extern FILE *err_log;
extern FILE *full_log;
extern void (*log_callback)(int, const char *, const char *);

//========================================================================
// Functions
//========================================================================

char *LogMsg(const char *msg, ...);

bool CreateLogMutex(void);
void DestroyLogMutex(void);

bool OpenLogFiles(void);
void CloseLogFiles(void);

void RegisterLogCallback (void (*)(int, const char *, const char *));

#endif  // __dologs_h__

//=========================================================================
// END
//=========================================================================
// vim:ts=2:sw=2:et:

