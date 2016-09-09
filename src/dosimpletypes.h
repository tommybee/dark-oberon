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
 *  @file dosimpletypes.h
 *
 *  Definitions of simple types.
 *
 *  @author Valeria Sventova
 *
 *  @date 2003
 */

#ifndef __dosimpletypes_h__
#define __dosimpletypes_h__


//=========================================================================
// Included Files
//=========================================================================

#include "cfg.h"
#include "doalloc.h"

#ifdef NEW_GLFW3
#include <glfw3.h>
#else
#include <glfw.h>
#endif


//=========================================================================
// Type definitions
//=========================================================================

#ifndef NULL
#define NULL 0
#endif

typedef unsigned char T_BYTE;          //!< Smaller type that int. Saves more memory.

typedef T_BYTE TTERRAIN_ID;            //!< Type used only for terrain identifier.
typedef TTERRAIN_ID *PTERRAIN_ID;      //!< Pointer to terrain identifier.
typedef TTERRAIN_ID **TTERRAIN_FIELD;  //!< Terrain field type - 2D array of TTERRAIN_ID. 


/**
 *  Type used for map metrics (map size, unit size, positions in the map...).
 *
 *  @warning This type must always remain unsigned!
 */
typedef T_BYTE T_SIMPLE;


//=========================================================================
// Macros
//=========================================================================

/**
 *  Max value of T_SIMPLE type.
 */
#define MAX_T_SIMPLE 255


/**
 *  Max value of T_BYTE type.
 */
#define MAX_T_BYTE 255


/**
 *  Returns second power of @p a.
 */
#define Sqr(a)  ((a) * (a))
 
/**
 *  Returns maximum of parameters @p a and @p b.
 */
#define MAX(a, b)   ((a) <= (b) ? (b) : (a))

/**
 *  Returns minimum of parameters @p a and @p b.
 */
#define MIN(a, b)   ((a) <= (b) ? (a) : (b))

#define PI        3.141592653589  //!< Value of Pi with enough precision.
#define SQRT_2    1.414213562373  //!< Value of sqare root of two.
#define SQRT_22   0.707106781187  //!< Half of the square root of two.

/**
 *  Returns angle in radians of @p deg in degrees.
 */

#define ToRadian(deg) (deg * PI/180)


//=========================================================================
// Template TINTERVAL
//=========================================================================

/**
 *  Implements interval as template for using with different types.
 *  
 *  @note Requires definitions of operators >= and <= for type @p T.
 */
template <class T>
struct TINTERVAL {
  T min;                            //!< Bottom border.
  T max;                            //!< Top border.

  //! Test whether @p x is in the interval.
  bool IsMember(T x) const
    {return ((x >= min) && (x <= max));};
};


/**
 *  Returns second power.
 *
 *  @param number Number which is powered.
 */
template<class T> T sqr(T number)
{
  return (number*number);
}


//=========================================================================
// Template TLIST
//=========================================================================

/**
 *  Help template for lists.
 */
template <class T> 
class TLIST {
public:
  //! Auxiliary template of the node in the help template TLIST.
  template <class T2> class TNODE
  {
    friend class TLIST<T2>;     //!< Class TLIST uses direct access to members.
    private:
      T * pitem;                //!< Pointer to item.
      TNODE<T2> *next;          //!< Pointer to next node in the list.

    public:

      TNODE() { pitem = NULL; next = NULL; }
      TNODE(T *itm) { pitem = itm; next = NULL; }

      //! Returns pointer to next node in the list.
      TNODE<T2> * GetNext() { return next;};
      //! Returns pointer to item.
      T2 * GetPitem() { return pitem;};
  };

  TLIST() { first = NULL; last = NULL; length = 0; }
  ~TLIST() { DestroyList(); }

  void AddNode(T * const new_pitem);                  //!< Adds new node at the beginning of the list.
  void AddNodeToEnd(T * const new_pitem);             //!< Adds new node at the end of the list.
  bool AddNonDuplicitNode(T * const new_pitem);       //!< Adds new node at the beginning of the list only if it isn't in list.
  void RemoveNode(T* delete_node);                    //!< The method removes node from the list.

  TNODE<T> *GetFirst() { return first; };            //!< Returns pointer to the first node in the list.
  TNODE<T> *GetLast() { return last; };              //!< Returns pointer to the last node in the list.

  void DestroyList();
  void ApplyFunction(void (T::*fcion)(void *), void *);    //!< Aplicate function given as parameter on the all nodes of the list with given parameter.
  void ApplyFunction(void (T::*fcion)(void));              //!< Aplicate function given as parameter on the all nodes of the list.
  /** The methods tests whether parameter is in the list.
   *  @param tested Tested pointer. */
  bool IsMember(T * const tested) {
    for (TNODE<T> *aux = first; aux; aux = aux->next)
      if (aux->pitem == tested) return true;
    return false;
  };

  /** Take out first node from the list and return pointer to item from it. If the list is empty return NULL.*/
  T* TakeFirstOut() {
    if (!length) return NULL;
    T* result = first->GetPitem();
    TNODE<T> *aux = first; first = aux->GetNext(); delete aux;
    length--;
    if (length == 0) last = NULL;
    return result;
  };

  /** @return The method returns length of the list. */
  unsigned int GetLength() const
    { return length; }

  /** The method tests if list is empty or not. 
   *  @return The method returns true if list is empty. */
  bool IsEmpty() const
    { return (length == 0); }

private:  
  TNODE<T> *first;              //!< First node in the list.
  TNODE<T> *last;               //!< Last node in the list.
  unsigned int length;          //!< Length of the list.
};


/**
 *  Adds new node at the beginning of the list.
 *
 *  @param new_pitem  Pointer to added item to the list.
 */
template <class T>
void TLIST<T>::AddNode(T * const new_pitem) 
{ 
  TNODE<T> * help_node;

  help_node = NEW TNODE<T>;
  help_node->next = first;
  help_node->pitem = new_pitem;
  first = help_node;

  if (length == 0) last = first;
  length++;
}


/**
 *  Adds new node at the beginning of the list.
 *
 *  @param new_pitem  Pointer to added item to the list.
 */
template <class T>
void TLIST<T>::AddNodeToEnd(T * const new_pitem) 
{ 
  TNODE<T> * help_node = NEW TNODE<T>(new_pitem);
  if (last) last->next = help_node;
  else first = help_node;
  last = help_node;
  length++;
}


/**
 *  Adds new node at the beginning of the list.
 *
 *  @param new_pitem  Pointer to added item to the list.
 */
template <class T>
bool TLIST<T>::AddNonDuplicitNode(T * const new_pitem) 
{ 
  TNODE<T> * hlp;
  bool exists = false;

  for (hlp = first; hlp != NULL; hlp = hlp->next){
    if (hlp->pitem == new_pitem){
      exists = true;
      break;
    }
  }
  
  if (!(exists)) AddNode(new_pitem);

  return (!exists);
}


/**
 *  The method removes node from the list. It doesn't delete a object which pointer
 *  is stored in the node. 
 *
 *  @param delete_node  Pitem of the node to delete.
 */
template <class T>
void TLIST<T>::RemoveNode(T* delete_node)
{
  TNODE<T> *node = first;
  TNODE<T> *prev = NULL;

  while (node)
  {
    if (delete_node == node->pitem)
    {
      if (node == first) first = node->next;
      if (node == last)  last = prev;
      if (prev) prev->next = node->next;

      delete node;
      length--;

      return;
    }

    prev = node;
    node = node->next;
  }
}


/**
 *  Destroys the list. Does not delete pointed items.
 */
template <class T>
void TLIST<T>::DestroyList()
{
  TNODE<T> *p_delete = first;

  while (p_delete)
  {
    first = p_delete->GetNext();
    delete p_delete;
    p_delete = first;
  }
  first = NULL;
  last = NULL;
  length = 0;
}


/**
 *  Aplicate function given as parameter on the all nodes of the list. Function is method in class T which is parameter of
 *  the template TLIST. As paramater of the method is used second parameter of the Iterator function.
 *
 *  @param fcion  Pointer to member function in the class T.
 *  @param param  Pointer to parameters of the member function from the first parameter.
 */
template <class T>
void TLIST<T>::ApplyFunction(void (T::*fcion)(void *), void *param)
{
  TNODE<T> *node = first;

  while (node)
  {
    (node->pitem->*fcion)(param);
    node = node->GetNext();
  }
}

/**
 *  Aplicate function given as parameter on the all nodes of the list. Function is method in class T which is parameter of
 *  the template TLIST.
 *
 *  @param fcion  Pointer to member function in the class T.
 */
template <class T>
void TLIST<T>::ApplyFunction(void (T::*fcion)(void))
{
  TNODE<T> *node = first;

  while (node)
  {
    (node->pitem->*fcion)();
    node = node->GetNext();
  }
}


//=========================================================================
// Terrain field
//=========================================================================

void DeleteTerrainField(TTERRAIN_FIELD terrain_field, int width);
TTERRAIN_FIELD CreateTerrainField(int width, int height);

//=========================================================================
// Common functions
//=========================================================================

int GetRandomInt(int max);
float GetRandomFloat();

#endif

//=========================================================================
// END
//=========================================================================
// vim:ts=2:sw=2:et:

