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
 *  @file dodraw.cpp
 *
 *  Drawing functions.
 *
 *  @author Peter Knut
 *  @author Marian Cerny
 *
 *  @date 2003, 2004
 */

#ifdef NEW_GLFW3
#include <glfw3.h>
#include <tinycthread.h>
#include <GL/glu.h>
#else
#include <glfw.h>
#endif

#include <stdio.h>
#include <math.h>


#include "doconfig.h"
#include "dodraw.h"
#include "domap.h"
#include "doplayers.h"
#include "doraces.h"
#include "domouse.h"
#include "doselection.h"


//=========================================================================
// Global Variables
//=========================================================================

TFPS fps;             //!< Variable to compute count of frames per second.
TFPS fps_of_update;   //!< Variable to compute count of frames per second of UpdateFunction().

/** On screen text. Shows text messages in the left corner of the screen. If
 *  #LOG_TO_OST is @c 1, log messages are also displayed. */
TOST* ost = NULL;

TGUI* gui = NULL; //!< Main gui node.

T_BYTE view_segment = DRW_ALL_SEGMENTS;    //!< Which segment is actually seen.

TPROJECTION projection;         //!< Actual projection.
bool reduced_drawing = false;   //!< Reduced drawing for higher FPS. Usefull for testing network features on slow computers.
bool show_all = false;          //!< Displays whole map without warfog and unvisible areas.


//=========================================================================
// TPROJECTION
//=========================================================================

/**
 *  Changes projection type.
 *
 *  @param projection  Either #PRO_GAME or #PRO_MENU.
 */
void TPROJECTION::SetProjection(TPROJECTION_TYPE projection)
{
  GLfloat zoom = 1.0f;
  GLfloat dx;

  type = projection;

  switch (type) {
  case PRO_MENU:
    right = (GLfloat)config.scr_width;
    left = 0.0;

    top = (GLfloat)config.scr_height;
    bottom = 0.0;

    width = right;
    height = top;
    break;

  case PRO_GAME:
    zoom = map.GetZoom();

    right = PRO_DEF_WIDTH * zoom / 2;
    left = -right;

    top = PRO_DEF_HEIGHT * zoom / 2;
    bottom = -top;

    width = PRO_DEF_WIDTH * zoom;
    height = PRO_DEF_HEIGHT * zoom;
    break;

  case PRO_RENDER_RADAR:
    dx = ((GLfloat)(map.height) * DRW_RADAR_TEX_SIZE) / (map.height + map.width);
    zoom = DAT_MAPEL_DIAGONAL_SIZE * map.height / dx;

    left = -dx * zoom;
    right = (config.scr_width) * zoom + left;

    top = config.scr_height * zoom / 2;
    bottom = 0;

    width = config.scr_width * zoom;
    height = config.scr_height * zoom / 2;
    break;
  }

  Update();

  // set projection
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  gluOrtho2D(left, right, bottom, top);  // orthographic projection
  glViewport(0, 0, config.scr_width, config.scr_height);

  if (type == PRO_GAME) {
    glTranslated(map.dx, map.dy, 0.0);
  }

  // reset model settings
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
}


/**
 *  Update projection.
 */
void TPROJECTION::Update()
{
  // update game coefs
  game_h_coef = (PRO_DEF_WIDTH * map.GetZoom()) / config.scr_width;
  game_v_coef = (PRO_DEF_HEIGHT * map.GetZoom()) / config.scr_height;
}

//=========================================================================
// TFPS
//=========================================================================

/**
 *  Computes new fps value.
 *  This computation is done only after standart time shift stored in DRW_FPS_DELAY.
 *
 *  @param time_shift  Time shift from the last update.
 */
void TFPS::Update(double time_shift)
{
  frames_count++;
  shift_time += time_shift;

  if (shift_time >=  DRW_FPS_DELAY) {       // fps is drawed every DRW_FPS_DELAY seconds
    // calculate FPS
    fps = (int)ceil(frames_count / shift_time - 0.49);

    // reset counters
    shift_time = 0.0;
    frames_count = 0;
  }
}


/**
 *  Resets fps information.
 */
void TFPS::Reset(void)
{
  fps = 0;
  shift_time = 0.0;
  frames_count = 0;
}


//=========================================================================
// Drawing
//=========================================================================


/**
 *  Inits basic OpenGL settings.
 */
void InitOpenGL(void)
{
  glShadeModel(GL_FLAT);                  // enable flat shading
  glDisable(GL_DEPTH_TEST);               // disable depth testing  
  glEnable(GL_TEXTURE_2D);                // enable textures

  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);  // type of blending
  glEnable(GL_BLEND);                     // enable blending

  glReadBuffer(GL_BACK);                  // reading is set to read from back buffer

  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);   // black background
  glClearDepth(1.0f);                     // default depth
}


/**
 *  Draws actual frames per second in left corner of the screen if @c show_fps
 *  is set in config file.
 */
void DrawFps(void)
{
  char txt[257];

  if (config.show_fps) {
    glColor3f(1.0f, 1.0f, 1.0f);
    sprintf(txt, "FPS: %d", fps.fps);
    glfPrint(font0, 10.0f, config.scr_height - 23.0f, txt, true);
  }
}

/**
 *  Draw game.
 */
void DrawGame(void)
{
  char txt[257];
  
  // clear buffers
  glClear(GL_COLOR_BUFFER_BIT);

  projection.SetProjection(PRO_GAME);

  if (!reduced_drawing) {
    // map
    map.Draw();
    selection->DrawUnitsLines();
    mouse.DrawBuildMap();
  }

  // menu projection
  projection.SetProjection(PRO_MENU);

  // draw on screen text
  ost->Draw();

  DrawFps();

  if (config.show_process_fps) {
    glColor3f(1.0f, 1.0f, 1.0f);
    sprintf(txt, "Process FPS: %d", fps_of_update.fps);
    glfPrint(font0, 10.0f, config.scr_height - 36.0f, txt, true);
  }

  mouse.DrawSelection();

#if DEBUG
  // mouse info
  glColor3f(0.7f, 0.7f, 0.7f);
  sprintf(txt, "Mouse: x: %d, y: %d", mouse.map_pos.x, mouse.map_pos.y);
  glfPrint(font0, 10.0f, config.scr_height - 70.0f, txt, true);

  // units count
  glColor3f(0.7f, 0.7f, 0.7f);
  sprintf(txt, "Units count: %d", myself->GetPlayerUnitsCount());
  glfPrint(font0, 10.0f, config.scr_height - 85.0f, txt, true);
#endif

  if (reduced_drawing) {
    glColor3f(1.0f, 0.93f, 0.82f);
    glfPrint(font0, GLfloat(config.scr_width / 2 - 50), GLfloat(config.scr_height / 2), const_cast<char*>("Low CPU mode"), false);
    glColor3f(0.7f, 0.7f, 0.7f);
    glfPrint(font0, GLfloat(config.scr_width / 2 - 70), GLfloat(config.scr_height / 2 - 15), const_cast<char*>("press 'G' for exit"), false);
    
#ifdef NEW_GLFW3
	struct timespec interval;
	interval.tv_sec = 0;
  	interval.tv_nsec = 0.05;
	thrd_sleep(&interval, NULL); 
#else
	glfwSleep(0.05);
#endif

	

    
  }

  // draw gui
  gui->Draw();

  // mouse cursor
  mouse.Draw();
}


/**
 *  Logs a log message to #ost.
 *
 *  @param level  Log level.
 *  @param header Text description of the log level.
 *  @param msg    Log message itself.
 */
void LogToOst (int level, const char *header, const char *msg) {
  if (!ost)
    return;

  switch (level) {
  case LOG_DEBUG:
    ost->AddText (msg,  DBG_COLOR_R,  DBG_COLOR_G,  DBG_COLOR_B);
    break;

  case LOG_INFO:
    ost->AddText (msg, INFO_COLOR_R, INFO_COLOR_G, INFO_COLOR_B);
    break;

  case LOG_WARNING:
    ost->AddText (msg, WARN_COLOR_R, WARN_COLOR_G, WARN_COLOR_B);
    break;

  case LOG_ERROR:
    ost->AddText (msg,  ERR_COLOR_R,  ERR_COLOR_G,  ERR_COLOR_B);
    break;

  case LOG_CRITICAL:
    ost->AddText (msg, CRIT_COLOR_R, CRIT_COLOR_G, CRIT_COLOR_B);
    break;
  }
}


//=========================================================================
// TOST
//=========================================================================


/**
 *  Constructor.
 *
 *  @warning Mutex is only created when constructor is called after glfwInit().
 */
TOST::TOST ()
{
  count = 0;
  for (int i = 0; i < OST_MAX_LINES; i++) {
    unused_texts_stack[i] = i;
  }
  first = last = OST_NULL;

#ifdef NEW_GLFW3
	if(mtx_init(&mutex, mtx_plain) == thrd_error) 
	{
          Critical ("Mutex could not be created");
	}
#else
	mutex = glfwCreateMutex ();

  	if (mutex == NULL)
    	Critical ("Mutex could not be created");
#endif
 
}


/**
 *  Destructor.
 */
TOST::~TOST ()
{
/* The mutex is destroyed, if it was created successfully. */
#ifdef NEW_GLFW3
	mtx_destroy(&mutex);
#else
	if (mutex)
    	glfwDestroyMutex (mutex);
#endif
}


/**
 *  Adds a text with the time in seconds, how long should the text remain on
 *  the screen and the text color.
 *
 *  @param s        Text string to add.
 *  @param last_for Time in seconds this text should remain on the screen.
 *  @param red      Color of the text (red component).
 *  @param green    Color of the text (green component).
 *  @param blue     Color of the text (blue component).
 *
 *  @note This function is thread safe.
 */
void TOST::AddText (const char *s, double last_for, float red, float green, float blue)
{
  int new_text;

  /* Only one thread can access data. */
#ifdef NEW_GLFW3
	mtx_lock(&mutex);
#else
	glfwLockMutex (mutex);
#endif
  

  // Finds the position in stack for the text.
  if (count == OST_MAX_LINES) {
    new_text = first;
    first = text[first].next;
    if (first == OST_NULL)
      last = OST_NULL;
  } else
    new_text = unused_texts_stack[count++];

  // Adds the text.
  text[new_text].next = OST_NULL;
  text[new_text].disappear_time = OST_NULL;
  text[new_text].last_for = last_for;
  text[new_text].red = red;
  text[new_text].green = green;
  text[new_text].blue = blue;

  /* Makes a copy of the string s. The string will be always finished by '\0',
   * because text[].string[OST_MAX_TEXT_LENGTH] is initialized to '\0' in
   * constructor. */
  strncpy (text[new_text].string, s, OST_MAX_TEXT_LENGTH);

  // Adds the new text to linked list
  if (last == OST_NULL)
    first = new_text;
  else
    text[last].next = new_text;

  last = new_text;
#ifdef NEW_GLFW3
	mtx_unlock(&mutex);
#else
	glfwUnlockMutex (mutex);
#endif
  
}


/**
 *  Removes texts, which are already on the screen for #last_for seconds.
 *
 *  @param actual_time  Time of last update.
 *
 *  @return @c true if some text was removed or new text added recently.
 *          If no change has been made, returns @c false.
 *
 *  @note This function is thread safe.
 */
bool TOST::Update (double actual_time)
{
  int p;
  int previous = OST_NULL;

  bool any_change_made = false;

  /* Only one thread can access data. */
#ifdef NEW_GLFW3
	mtx_lock(&mutex);
#else
	glfwLockMutex (mutex);
#endif
  

  /* Texts, which are displayed for too long are removed. */
  p = first;
  while (p != OST_NULL) {
    if (text[p].disappear_time == OST_NULL) {
      // If it is the first time this text p will be displayed, we count
      // disappear_time for it.
      text[p].disappear_time = actual_time + text[p].last_for;

      any_change_made = true;
    } else if (text[p].disappear_time <= actual_time) {
      // If the text is too long on the screen, it is removed.
      unused_texts_stack[--count] = p;

      // Removes p from linked list
      if (p == first)
        first = text[p].next;
      else 
        text[previous].next = text[p].next;

      if (p == last)
        last = previous;

      if (previous != OST_NULL)
        text[previous].next = text[p].next;

      any_change_made = true;
    } else
      previous = p;

    p = text[p].next;
  }
#ifdef NEW_GLFW3
	mtx_unlock(&mutex);
#else
	glfwUnlockMutex (mutex);
#endif
  

  return any_change_made;
}


/**
 *  Draws the all texts to the screen.
 *
 *  @note This function is thread safe.
 */
void TOST::Draw ()
{
  int p;

  /* Only one thread can access data. */
#ifdef NEW_GLFW3
	mtx_lock(&mutex);
#else
	glfwLockMutex (mutex);
#endif
  

  /* Displays all texts. */
  int i;
  for (p = first, i = 0; p != OST_NULL; p = text[p].next, i++) {
    glColor3f(text[p].red, text[p].green, text[p].blue);
    glfPrint(font0, 10.0f, ((GLfloat)OST_MAX_LINES - i + 1) * 13, text[p].string, true);
  }
#ifdef NEW_GLFW3
	mtx_unlock(&mutex);
#else
	glfwUnlockMutex (mutex);
#endif
  
}


//=========================================================================
// END
//=========================================================================
// vim:ts=2:sw=2:et:

