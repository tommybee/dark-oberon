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
 *  @file dopool.h
 *
 *  Methods for work with events.
 *
 *  @author Martin Kosalko
 *  @author Valeria Sventova
 *
 *  @date 2003, 2004
 */

#ifndef __dopool_h__
#define __dopool_h__


//========================================================================
// Forward declarations
//========================================================================

class TPOOL_ELEMENT;


//========================================================================
// Definitions & typedefs
//========================================================================

#define EV_MIN_POOL_ELEMENTS  500   //!< Minimal count of events in pool.


//========================================================================
// Included files
//========================================================================

#ifdef NEW_GLFW3
#include <glfw3.h>
#include <tinycthread.h>
#else
#include <glfw.h>
#endif

#include <string.h>
#include "dologs.h"
#include "dosimpletypes.h"


//========================================================================
// class TPOOL_ELEMENT
//========================================================================


class TPOOL_ELEMENT {
protected:
  TPOOL_ELEMENT *pool_next;
public:

  TPOOL_ELEMENT() {pool_next = NULL;};
  virtual ~TPOOL_ELEMENT() {};  

  TPOOL_ELEMENT * GetNext() { return pool_next;};
  void SetNext(TPOOL_ELEMENT *next) { pool_next = next;};
  virtual void Clear(bool all) {if (all) pool_next = NULL;};
};


//========================================================================
// class TPOOL
//========================================================================

template <class T>
class TPOOL {

private:
  T * list_begin;                    //!< List of clear elements in pool.
  int count;                         //!< Count of elements in pool.
  int critical_count;                //!< Minimal count of elements in pool.
  int increment_count;               //!< Count of elements which are added to pool when count <= crutical_count.
#ifdef NEW_GLFW3
	mtx_t mutex;
#else
	GLFWmutex mutex;                   //!< Pool mutex.
#endif
  

public:
  T * GetFromPool(void);             //!< Returns pointer to any clear elements.
  void PutToPool(T * event);         //!< Puts element to pool.

  TPOOL(int events_count, int count_critical = 0, int count_increment = EV_MIN_POOL_ELEMENTS);  //!< Constructor.
  ~TPOOL(void);                               //!< Destructor.

  /** The method returns new pool.*/
  static TPOOL<T>* CreateNewPool(int events_count, int count_critical, int count_increment);

private:
  bool AllocateNewElements(int events_count); //!< Allocate new elements.

  /** The constructor creates new empty pool.*/
  TPOOL()
    { list_begin = NULL; count = 0; critical_count = 0; increment_count = 0; mutex = NULL;};
};


//========================================================================
// class TPOOL - methods definition
//========================================================================

/**
 *  Constructor. Prepare @param events_count events.
 */
template <class T>
TPOOL<T>::TPOOL(int elements_count, int count_critical, int count_increment)
{
#ifdef NEW_GLFW3
	// create mutex
  if ((mtx_init(&mutex, mtx_plain)) == thrd_error) 
    Critical ("Could not create mutex");
#else
  // create mutex
  if ((mutex = glfwCreateMutex ()) == NULL) 
    Critical ("Could not create mutex");
#endif

  count = 0;
  critical_count = count_critical;
  increment_count = count_increment;
  list_begin = NULL;
  
  if (!(AllocateNewElements(elements_count)))
    Critical("Can not allocate memory for pool of events");
};

/**
 *  Destructor.
 */
template <class T>
TPOOL<T>::~TPOOL(void)
{
  T * hlp, * all;
  int i;

  if (list_begin)
  {
    i = 0;
    all = list_begin;
    hlp = static_cast<T*>(all->GetNext()); 

    while (hlp != NULL)
    {
      delete all;
      all = hlp;
      hlp = static_cast<T*>(hlp->GetNext());
      i++;
    }
    delete all;

    list_begin = NULL;
    count = 0;
  }
  
#ifdef NEW_GLFW3
	// free memory
	mtx_destroy(&mutex);
#else
  // free memory
  if (mutex != NULL)
    glfwDestroyMutex(mutex);
#endif

  
};

/**
 *  Allocate new elements for pool
 *
 *  @param el_count count of elements which will be allocated and added to pool.
 *  Returns false on error, true else
 */
template <class T>
bool TPOOL<T>::AllocateNewElements(int el_count)
{
  T * hlp, * all, * last;
  int i;
  bool ok;
  
  // zeroize all values
  hlp = all = last = NULL;
  ok = true;

  // allocate new TEVENT in for cycle
  for (i = 0; (ok) && (i < el_count); i++){
    hlp = NEW (T);
    if (i == 0) 
      last = hlp;
    if (hlp) // if allocate is OK, put new TEVENT to begin of list
    {
      hlp->SetNext(all);
      all = hlp;
    }
    else  // allocate returns NULL
      ok = false;
  }

  if (ok){  // if everything is OK returns list of 'events_count' TEVENTs
#ifdef NEW_GLFW3
	mtx_lock(&mutex);
#else
  	glfwLockMutex (mutex);
#endif
    
      last->SetNext(list_begin);
      list_begin = all;
      count += el_count;
#ifdef NEW_GLFW3
	mtx_unlock(&mutex);
#else
  	glfwUnlockMutex (mutex);
#endif      
    
    return true;
  }
  else
  {
    if (all){ // if any TEVENTS was created, delete it
      for (hlp = static_cast<T*>(all->GetNext()); hlp != NULL; hlp = static_cast<T*>(hlp->GetNext())){
        delete all;
        all = hlp;
      }
      delete all;
    }

    return false;
  }
};

/**
 *  Returns pointer to first clear element from pool.
 */
template <class T>
T * TPOOL<T>::GetFromPool(void)
{
  T * first = NULL;
#ifdef NEW_GLFW3
	mtx_lock(&mutex);
#else
  	glfwLockMutex (mutex);
#endif
  

    if (list_begin) { // if exists any event in pool, returns first
      first = list_begin;
      list_begin = static_cast<T*>(list_begin->GetNext()); // remove first from pool
      count --; // decrease count of events in pool
    }

  if (count <= critical_count) // if count of events in pool is critical
    if (!(AllocateNewElements(increment_count))) // allocate new events
      Critical("Can not allocate memory for pool of events");
  
  if (first) 
    first->Clear(true); // clear event
    
#ifdef NEW_GLFW3
	mtx_unlock(&mutex);
#else
  	glfwUnlockMutex (mutex);
#endif 
  
    
  return first;
};

/**
 *  Puts event given in parameter to pool.
 */
template <class T>
void TPOOL<T>::PutToPool(T * element)
{
  if (element){
#ifdef NEW_GLFW3
	mtx_lock(&mutex);
#else
  	glfwLockMutex (mutex);
#endif
    
      element->SetNext(list_begin); // add event to pool
      list_begin = element;
      count ++; // increase count of events in pool
      element->Clear(false);
#ifdef NEW_GLFW3
	mtx_unlock(&mutex);
#else
  	glfwUnlockMutex (mutex);
#endif
    
  }
};


/** 
 *  The method returns new pool.
 *  If the count of elements is lower then new elements are allocate and add
 *  to the pool.
 *
 *  @param elements_count Count of the elements which will be in the pool after 
 *  successful creating.
 *  @param count_critical Min amount of elements in pool.
 *  @param count_increment  Increment count.
 *  @return The method returns pointer to the new pool if is successful. 
 *  Otherwise returns NULL.
 */
template <class T>
TPOOL<T>* TPOOL<T>::CreateNewPool(int elements_count, int count_critical, int count_increment)
{
  //create new empty pool
  TPOOL<T> *new_pool = NEW TPOOL<T>(elements_count, count_critical, count_increment);

  return new_pool;
}


#endif // __dopool_h__


//=========================================================================
// END
//=========================================================================
// vim:ts=2:sw=2:et:
