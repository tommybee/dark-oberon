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
 *  @file doipc.h
 *
 *  IPC - inter process communication (mutexes etc.).
 *
 *  @author Marian Cerny
 *
 *  @date 2005
 */

#ifndef __doipc_h__
#define __doipc_h__

//=========================================================================
// Included files
//=========================================================================

#include "cfg.h"

#include <string>
#include <list>

#ifdef NEW_GLFW3
#include <glfw3.h>
#include <tinycthread.h>
#else
#include <glfw.h>
#endif


//=========================================================================
// TLOCK
//=========================================================================

struct TLOCK {
public:
  class MutexException {};

  TLOCK ();
  ~TLOCK ();

#if DEBUG
  void Lock ();
  void Unlock ();
#else
  void Lock () {
#ifdef NEW_GLFW3
	mtx_lock(&mutex);
#else
  	glfwLockMutex (mutex);
#endif
    
  }

  void Unlock () {
#ifdef NEW_GLFW3
	mtx_unlock(&mutex);
#else
  	glfwUnlockMutex (mutex);
#endif
    
  }
#endif

private:
#if DEBUG

#ifdef NEW_GLFW3
	thrd_t locked_by;
	cnd_t unlocked;
#else
	GLFWthread locked_by;
  	GLFWcond unlocked;
#endif
  
#endif

#ifdef NEW_GLFW3
	mtx_t mutex;
#else
	GLFWmutex mutex;        //!< Mutex for atomicity of operations.
#endif
  
};


//=========================================================================
// TRECURSIVE_LOCK
//=========================================================================

struct TRECURSIVE_LOCK {
public:
  class MutexException {};

  TRECURSIVE_LOCK ();
  ~TRECURSIVE_LOCK ();

  void Lock ();
  void Unlock ();

private:
  
  int locked_count;
#ifdef NEW_GLFW3
	thrd_t locked_by; 
	mtx_t mutex;
#else
	GLFWthread locked_by;
	GLFWmutex mutex;        //!< Mutex for atomicity of operations.
#endif
};


//=========================================================================
// template TSAVE_LIST
//=========================================================================

template <class T>
class TSAVE_LIST {
public:
  class MutexException {};

  TSAVE_LIST () {
  	
#ifdef NEW_GLFW3
	if(mtx_init(&mutex, mtx_plain) == thrd_error)
		throw MutexException ();
#else
	mutex = glfwCreateMutex ();
	if (!mutex)
      throw MutexException ();
#endif
    
    
  }

  ~TSAVE_LIST () {
#ifdef NEW_GLFW3
	// free memory
	mtx_destroy(&mutex);
#else
  // free memory
  if (mutex != NULL)
    glfwDestroyMutex(mutex);
#endif
    
  }

  void PushBack (T node) {
  	
#ifdef NEW_GLFW3
	mtx_lock(&mutex);
#else
  	glfwLockMutex (mutex);
#endif 	
    
    
    list.push_back(node);
    
#ifdef NEW_GLFW3
	mtx_unlock(&mutex);
#else
  	glfwUnlockMutex (mutex);
#endif     
    
  }

  bool PopFront (T &node) {
    bool ret;
#ifdef NEW_GLFW3
	mtx_lock(&mutex);
#else
  	glfwLockMutex (mutex);
#endif 
    

    ret = !list.empty();
    if (ret) {
      node = *list.begin();
      list.pop_front();
    }
#ifdef NEW_GLFW3
	mtx_unlock(&mutex);
#else
  	glfwUnlockMutex (mutex);
#endif  
    

    return ret;
  }

private:
#ifdef NEW_GLFW3
  mtx_t mutex;
#else
  GLFWmutex mutex;        //!< Mutex for atomicity of operations.
#endif

  std::list<T> list;
};


//=========================================================================
// Global variables
//=========================================================================

extern TRECURSIVE_LOCK *giant;
extern TRECURSIVE_LOCK *process_mutex;


//=========================================================================
// Global functions
//=========================================================================

void init_giant ();


#endif  // __doplayers_h__

//=========================================================================
// END
//=========================================================================
// vim:ts=2:sw=2:et:

