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
 *  @file doberon.cpp
 *
 *  Main module.
 *
 *  @author Peter Knut
 *  @author Marian Cerny
 *
 *  @date 2002, 2003, 2004
 */

/**
 *  @mainpage
 *
 *  An advanced strategy game.
 *
 *  @date Copyright 2002, 2003, 2004, 2005
 *
 *  @author Valeria Sventova    <liberty@matfyz.cz>
 *  @author Jiri Krejsa         <crazych@matfyz.cz>
 *  @author Peter Knut          <peterpp@matfyz.cz>
 *  @author Martin Kosalko      <cauchy@matfyz.cz>
 *  @author Marian Cerny        <jojo@matfyz.cz>
 *  @author Michal Kral         <index@matfyz.cz>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License (see docs/gpl.txt) as
 *  published by the Free Software Foundation; either version 2 of the License,
 *  or (at your option) any later version.
 *
 *  Powered by GLFW - an OpenGL framework <http://glfw.sourceforge.net/>
 */

#include "cfg.h"
#include "doalloc.h"

#include <ctime>
#include <stdlib.h>
#include <string>

#include "donet.h"

#ifdef NEW_GLFW3
#include <glfw3.h>
#else
#include <glfw.h>
#endif


#include "doconfig.h"
#include "doengine.h"
#include "doipc.h"
#include "domouse.h"
#include "dosound.h"
#include "utils.h"

using std::string;

#ifdef NEW_GLFW3
static GLFWwindow* mBeronWindow;
#endif

//========================================================================
// Initializing & Destroying
//========================================================================

/**
 *   Inits basic IO handling.
 */
void InitIO(void)
{
  // synchronize with monitor refresh rate
  glfwSwapInterval(config.vert_sync ? 1 : 0);

	// disable sticky input
#ifdef NEW_GLFW3
	if(!mBeronWindow)
	{
		glfwSetInputMode(mBeronWindow, GLFW_STICKY_KEYS, GLFW_FALSE);
		glfwSetInputMode(mBeronWindow, GLFW_STICKY_MOUSE_BUTTONS, GLFW_FALSE);
	}
#else
	glfwDisable(GLFW_STICKY_KEYS);
  	glfwDisable(GLFW_STICKY_MOUSE_BUTTONS);
#endif
	
	// set callback functions
#ifdef NEW_GLFW3
	if(!mBeronWindow)
	{
		glfwSetWindowSizeCallback(mBeronWindow, SizeCallback);
		glfwSetKeyCallback(mBeronWindow, KeyCallback);
		glfwSetMouseButtonCallback(mBeronWindow, MouseButtonCallback);
		glfwSetCursorPosCallback(mBeronWindow, MousePosCallback);
		glfwSetScrollCallback(mBeronWindow, MouseWheelCallback);
		glfwSetWindowRefreshCallback (mBeronWindow, WindowRefreshCallback);
	}
#else
	glfwSetWindowSizeCallback(SizeCallback);
	glfwSetKeyCallback(KeyCallback);
	glfwSetMouseButtonCallback(MouseButtonCallback);
	glfwSetMousePosCallback(MousePosCallback);
	glfwSetMouseWheelCallback(MouseWheelCallback);
	glfwSetWindowRefreshCallback (WindowRefreshCallback);
#endif
  

  // disable mouse cursor in window mode
#ifdef NEW_GLFW3
	if(!mBeronWindow)
	{
		//if (!config.fullscreen) glfwSetInputMode(mBeronWindow, GLFW_MOUSE_CURSOR, GLFW_FALSE);
		if (!config.fullscreen) glfwSetInputMode(mBeronWindow, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
	}
#else
	if (!config.fullscreen) glfwDisable(GLFW_MOUSE_CURSOR);
#endif
  
}


/**
 *   Inits everything on game start. Returns true if successful.
 */
bool InitAll(void)
{
  srand((unsigned)time(NULL));    //initializing of random generator

  // initialize log files
  if (!OpenLogFiles())
    Warning("Log files were not opened");

   
  // initialize GLFW
  if (!glfwInit()) {
    Critical("Can not initialize GLFW library");
    return false;
  }

  // Initialize network on Windows.
  init_sockets ();

  pool_net_messages = NEW TPOOL<TNET_MESSAGE>(1000, 0, 100);

  // initialize memory checking system
  // must be called after initializing log files and GLWF
#if DEBUG_MEMORY
  InitMemorySestem();
#endif

  // initialize FMOD
#if SOUND
  if (!InitSound()) Error("Can not initialize sound");
#endif

  // configuration
  LoadConfig();                               // load configuration from file

#ifdef NEW_GLFW3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	
	switch(config.pr_wnd_mode)
	{
		case OBERON_WINDOW:
			mBeronWindow = glfwCreateWindow(config.scr_width, config.scr_height, "Dark Oberon", NULL, NULL);
			break;
		case OBERON_FULLSCREEN:
			mBeronWindow = glfwCreateWindow(config.scr_width, config.scr_height, "Dark Oberon", glfwGetPrimaryMonitor(), NULL);
			break;
		default:
			mBeronWindow = glfwCreateWindow(config.scr_width, config.scr_height, "Dark Oberon", NULL, NULL);
	}
	
	if (!mBeronWindow)
	{
	    // Window or OpenGL context creation failed
	    Critical("Can not open OpenGL window");
	    glfwTerminate();
	    return false;
	}
#else
	// open OpenGL window
  if (!glfwOpenWindow(config.scr_width, config.scr_height,  // width, height
                      8, 8, 8,                // bit per red, green, blue
                      0, 8, 0,                // alfa, depth, stencil
                      config.pr_wnd_mode)) {  // window mode
    Critical("Can not open OpenGL window");
    glfwTerminate();
    return false;
  }
  glfwSetWindowTitle("Dark Oberon");          // set window title
#endif 
  

  // load data
  if (!LoadData()) {
    glfwTerminate();
    return false;
  }

  // create fonts
  if (!CreateFonts()) {
    glfwTerminate();
    DeleteData();
    return false;
  }

  gui = NEW TGUI();
  gui->SetCursorHeight(DRW_CURSOR_HEIGHT);

  /* This section MUST be called after glfwInit(). */
  ost = NEW TOST;               // Initialise On Screen Text.
  // Registers function LogToOst() as log callback.
  RegisterLogCallback (LogToOst);

  // initialize player array
  player_array.Initialise ();

  /* Init giant mutex. */
  init_giant ();
  process_mutex = NEW TRECURSIVE_LOCK();

  CreateLogMutex();

  /* MUST be called after glfwInit(), but before InitIO(). */
  need_redraw = NEW TSAFE_BOOL_SWITCH (true);

  InitOpenGL();
  InitIO();
  InitPositionChanges();
  map.InitPools();

  queue_events = NEW TQUEUE_EVENTS;
  if (!queue_events)
  {
    Critical ("Could not create game structure queue_events");
    return false;
  }

  mouse.Center();
  return true;
}


/**
 *   Destroys everything and free memory on game end.
 */
void DestroyAll(void)
{
  if (queue_events) delete queue_events;

  // save configuration
  SaveConfig();

  // destroy structures
  DestroyFonts();
  DeleteData();

  // destroy OST
  if (ost) delete ost;
  RegisterLogCallback (NULL);
  delete need_redraw;

  // destroy gui
  if (gui) delete gui;

  DestroyLogMutex();
  delete process_mutex;

#if SOUND
  FSOUND_Close();
#endif

  // stop memory checking system
  // must be called before closing log files
#if DEBUG_MEMORY
  DoneMemorySystem();
#endif
    
  // close OpenGL window
  glfwTerminate();

  // Initialize network on Windows.
  end_sockets ();

  // end logging
  CloseLogFiles();
}


//========================================================================
// Main Function
//========================================================================

/**
 *   Program's main().
 *
 *   @returns @c EXIT_SUCCESS on successful end, otherwise @c EXIT_FAILURE.
 */
int main(int argc, char *argv[])
{
#ifdef WINDOWS
  app_path = argv[0];
  app_path = app_path.substr(0, app_path.rfind('\\') + 1);

  user_dir = app_path;
#else
  char *home = getenv("HOME");
  if (!home)
    home = "/tmp";

  user_dir = string (home) + "/.dark-oberon/";
  do_mkdir (user_dir);
#endif

  if (!InitAll()) return EXIT_FAILURE;

  // main loop
  while (state != ST_QUIT) 
  {
    switch (state) 
    {
    case ST_MAIN_MENU:
    case ST_VIDEO_MENU:      
    case ST_PLAY_MENU:  Menu(); break;
    case ST_GAME:       Game(); break;

    default: break;
    }

  }

  DestroyAll();

  return EXIT_SUCCESS;
}


//=========================================================================
// END
//=========================================================================
// vim:ts=2:sw=2:et:


