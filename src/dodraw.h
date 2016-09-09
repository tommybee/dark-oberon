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
 *  @file dodraw.h
 *
 *  Drawing declarations and methods.
 *
 *  @author Peter Knut
 *  @author Marian Cerny
 *
 *  @date 2003, 2004
 */

#ifndef __dodraw_h__
#define __dodraw_h__

#include "cfg.h"
#include "doalloc.h"

//=========================================================================
// Forward declarations
//=========================================================================

struct TPROJECTION;
struct TFPS;
struct TOST_TEXT;
class TOST;
struct TPANEL_INFO;
class TTIME;


//=========================================================================
// Definitions
//=========================================================================

// drawing settings
#define DRW_ALL_SEGMENTS      3       //!< Combination of all segments will be drawn.
#define DRW_FPS_DELAY         0.1     //!< Delay time for drawing fps. [seconds]
#define DRW_MIN_RADAR_SIZE    1.0f    //!< Minimal size of units squares on radar.
#define DRW_RADAR_SIZE        170     //!< Size of radar window. [pixels]
#define DRW_RADAR_TEX_SIZE    256     //!< Size of radar texture. [pixels]
#define DRW_BUILD_MAP_ALPHA   0.3f    //!< Alpha channel for drawing build map.
#define DRW_CURSOR_HEIGHT     24      //!< Height of mouse cursor. [pixels]


// projections
enum TPROJECTION_TYPE {
  PRO_MENU,                           //!< Projection used for menu and for panels in game.
  PRO_GAME,                           //!< Projection used for drawing game (map).
  PRO_RENDER_RADAR                    //!< Projection used for rendering radar textures (map).
};

#define PRO_DEF_WIDTH     1024.0f     //!< Default projection width.
#define PRO_DEF_HEIGHT    768.0f      //!< Default projection height.

// on screen text
#define OST_MAX_TEXT_LENGTH         80   //!< Maximum length of texts in TOST.
#define OST_MAX_LINES               8    //!< Maximum count of lines in TOST.
#define OST_DEFAULT_LAST_FOR_TIME   5.0  //!< Default time for messages to remain on the screen.
#define OST_DEFAULT_TEXT_RED        0.3f //!< Default red component of messages in TOST.
#define OST_DEFAULT_TEXT_GREEN      0.7f //!< Default green component of messages in TOST.
#define OST_DEFAULT_TEXT_BLUE       0.7f //!< Default blue component of messages in TOST.
#define OST_NULL                    -1   //!< "Null" pointer used in TOST.

// Colors for log messages.
#define DBG_COLOR_R   0.5f  //!< Red component of color for debug log messages on #ost.
#define DBG_COLOR_G   0.0f  //!< Green component of color for debug log messages on #ost.
#define DBG_COLOR_B   0.5f  //!< Blue component of color for debug log messages on #ost.

#define INFO_COLOR_R  0.6f  //!< Red component of color for info log messages on #ost.
#define INFO_COLOR_G  0.6f  //!< Green component of color for info log messages on #ost.
#define INFO_COLOR_B  0.6f  //!< Blue component of color for info log messages on #ost.

#define WARN_COLOR_R  0.8f  //!< Red component of color for warning log messages on #ost.
#define WARN_COLOR_G  0.0f  //!< Green component of color for warning log messages on #ost.
#define WARN_COLOR_B  0.0f  //!< Blue component of color for warning log messages on #ost.

#define ERR_COLOR_R   0.9f  //!< Red component of color for error log messages on #ost.
#define ERR_COLOR_G   0.0f  //!< Green component of color for error log messages on #ost.
#define ERR_COLOR_B   0.0f  //!< Blue component of color for error log messages on #ost.

#define CRIT_COLOR_R  1.0f  //!< Red component of color for critical log messages on #ost.
#define CRIT_COLOR_G  0.0f  //!< Green component of color for critical log messages on #ost.
#define CRIT_COLOR_B  0.0f  //!< Blue component of color for critical log messages on #ost.


//=========================================================================
// Included files
//=========================================================================

#include "glgui.h"
#include "dodata.h"


//=========================================================================
// Classes
//=========================================================================

/**
 *  Structure containing inforation about actual used projection.
 */
struct TPROJECTION {
  TPROJECTION_TYPE type;  //!< Type of actual projection.

  GLfloat left;     //!< Left co-ordinate of projection.
  GLfloat right;    //!< Right co-ordinate of projection.
  GLfloat top;      //!< Top co-ordinate of projection.
  GLfloat bottom;   //!< Bottom co-ordinate of projection.

  GLfloat width;    //!< Projection width.
  GLfloat height;   //!< Projection height.

  /** @def game_h_coef
   *  Horizontal projection coeficient.
   *  Prepocitava horizontalne vzdialenosti z okna aplikacie na interne vzdialenosti 
   *  vzhladom na projekciu hry.
   */
  /** @def game_v_coef
   *  Vertical projection coeficient.
   *  Prepocitava vertikalne vzdialenosti z okna aplikacie na interne vzdialenosti 
   *  vzhladom na projekciu hry.
   */
  GLfloat game_h_coef;
  GLfloat game_v_coef;

  void SetProjection(TPROJECTION_TYPE projection);
  void Update();

#ifdef __GNUC__
	TPROJECTION() { SetProjection(PRO_MENU); };
#else
  	TPROJECTION::TPROJECTION() { SetProjection(PRO_MENU); };
#endif
};


/**
 *  Structure to compute count of drawn frames per second.
 */
struct TFPS {
  int frames_count;    //!< Actual count of frames.
  double shift_time;   //!< Time shift.

  int fps;             //!< Actual frames per second.

  void Update(double time_shift);
  void Reset(void);
};

/**
 *  One text item (text line) in TOST.
 */
struct TOST_TEXT {
  TOST_TEXT () {
    /* Initialise last char of string to '\0', so we can use strncpy of
     * OST_MAX_TEXT_LENGTH and the string will always be finished. */
    string[OST_MAX_TEXT_LENGTH] = '\0';
  }

  /** String, where the text message will be copied. */
  char string[OST_MAX_TEXT_LENGTH + 1];
  /** "Pointer" to next message. */
  int next;
  /** Time, when the message should disappear. When equal to #OST_NULL, it was
   *  not yet set. */
  double disappear_time;
  /** Time in seconds how long should the text message remain on the screen. */
  double last_for;
  /** Red component of the color for the text message. */
  float red;
  /** Green component of the color for the text message. */
  float green;
  /** Blue component of the color for the text message. */
  float blue;
};

/**
 *  On screen text. This allows maximum of #OST_MAX_LINES lines of text
 *  messages to be displayed on the screen.
 *
 *  @note    This class is thread safe.
 *  @warning Mutex is only created when constructor is called after glfwInit().
 */
class TOST {
public:
  TOST ();
  ~TOST ();

  void AddText (const char *s, double last_for, float red, float green, float blue);
 /**
  *  Adds a text with specified #last_for time and default color.
  *
  *  @param s        Text to add.
  *  @param last_for Time in seconds how long should the text remain on the screen.
  *
  *  @note This function is thread safe.
  */
  void AddText (const char *s, double last_for)
    { AddText (s, last_for, OST_DEFAULT_TEXT_RED, OST_DEFAULT_TEXT_GREEN, OST_DEFAULT_TEXT_BLUE); }
 /**
  *  Adds a text with specified color and default #last_for time.
  *
  *  @param s     Text to add.
  *  @param red   Color of the text (red component).
  *  @param green Color of the text (green component).
  *  @param blue  Color of the text (blue component).
  *
  *  @note This function is thread safe.
  */
  void AddText (const char *s, float red, float green, float blue)
    { AddText (s, OST_DEFAULT_LAST_FOR_TIME, red, green, blue); }
 /**
  *  Adds a text with all properties to be default.
  *  @param s Text to add.
  *
  *  @note This function is thread safe.
  */
  void AddText (const char *s)
    { AddText (s, OST_DEFAULT_LAST_FOR_TIME); }

  bool Update (double actual_time);
  void Draw ();

private:
  TOST_TEXT text[OST_MAX_LINES];         //!< Array of texts.
  int first;        //!< "Pointer" to first item.
  int last;         //!< "Pointer" to last item.
  int unused_texts_stack[OST_MAX_LINES]; //!< Stack of unused texts.
  int count;        //!< Count of active texts. Also used as a stack top.
#ifdef NEW_GLFW3
	mtx_t mutex;
#else
	GLFWmutex mutex;  //!< Mutex to avoid race conditions.
#endif
  
};


//=========================================================================
// Class TTIME
//=========================================================================

/**
 *  Class to easily find out time shift from last_update.
 */
class TTIME {
  public:
    double GetActual () { return time_actual; }   //!< Returns actual time = time of last update. [seconds]
    double GetShift () { return time_shift; }     //!< Returns time shift. [seconds]

    /** Sleeps the necessare time to get expected frame duration @p
     *  expected_frame_duration. */
    void SleepToGetExpectedFrameDuration (double expected_frame_duration)
    {
      double sleep_time = expected_frame_duration - (glfwGetTime() - time_actual);
      /* the smallest time for sleep is 0.01 (1 ms), we are ignoring sleep times
       * smaller than that */
#ifdef NEW_GLFW3
		if (sleep_time > 0.009) 
		{
			struct timespec interval;
		
			interval.tv_sec = 0;
	  		interval.tv_nsec = sleep_time / 10;
  		
			thrd_sleep (&interval, NULL);
		}
#else
		if (sleep_time > 0.009) glfwSleep (sleep_time);
#endif       
      
    }

    //! Updates #time_actual and #time_shift.
    void Update ()
    {
      double last_update = time_actual;
      time_actual = glfwGetTime();
      time_shift = time_actual - last_update;
    }

    //! Constructor. Initializes #time_actual and #time_shift.
    TTIME ()
    { time_actual = glfwGetTime(); time_shift = 0.0; }

  private:
    double time_actual;   //!< Actual time. [seconds]
    double time_shift;    //!< Time shift. [seconds]
};


//=========================================================================
// Variables
//=========================================================================

extern TFPS fps;
extern TFPS fps_of_update;
extern TOST* ost;
extern TGUI* gui;

extern T_BYTE view_segment;      // which segment is actually seen

extern TPROJECTION projection;   // Actual projection.
extern bool reduced_drawing;
extern bool show_all;


//=========================================================================
// Functions
//=========================================================================

void InitOpenGL(void);
void DrawFps(void);
void DrawGame(void);
void LogToOst (int level, const char *header, const char *msg);


#endif  // __dodraw_h__

//=========================================================================
// END
//=========================================================================
// vim:ts=2:sw=2:et:

