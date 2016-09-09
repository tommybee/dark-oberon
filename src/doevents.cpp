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
 *  @file doevents.cpp
 *
 *  Methods for work with events.
 *
 *  @author Martin Kosalko
 *  @author Valeria Sventova
 *
 *  @date 2004
 */


//========================================================================
// Included files
//========================================================================
#include <stdio.h>

#include "doevents.h"
#include "dologs.h"
#include "doplayers.h"
#include "dopool.h"

//=========================================================================
// Global variables and functions
//=========================================================================

TPOOL<TEVENT> * pool_events;
TPOOL<TPATH_INFO> *pool_path_info;
TPOOL<TSEL_NODE> * pool_sel_node;
TPOOL<TNEAREST_INFO> *pool_nearest_info;
TQUEUE_EVENTS * queue_events;

/**
 *  Mutex to assure safe data sharing between graphic thread and update thread.
 */
GLFWmutex delete_mutex = 0;
GLFWmutex path_mutex =0;

//=========================================================================
// class TEVENT
//=========================================================================

/**
 *  Constructor. Only zeroize the values.
 */
TEVENT::TEVENT(void)
{
  // information values
  player_id = 0;
  unit_id = 0;
  priority = false;
  event = US_NONE;
  last_event = US_NONE;
  time_stamp = 0;
  request_id = 0;
  simple1 = simple2 = simple3 = simple4 = simple5 = simple6 = 0;
  int1 = int2 = 0;

  // queue
  queue_left = NULL;
  queue_right = NULL;
};


/**
 *  Clears all values of event.
 *  @param all clears pointers too.
 */
void TEVENT::Clear(bool all)
{
  TPOOL_ELEMENT::Clear(all);
  player_id = 0;
  unit_id = 0;
  priority = false;
  event = US_NONE;
  last_event = US_NONE;
  time_stamp = 0;
  request_id = 0;
  simple1 = simple2 = simple3 = simple4 = simple5 = simple6=0;
  int1 = int2 =0;
  
  if (all){
    queue_left = queue_right = NULL;
  }
}

// Sets  player_id in event.
void TEVENT::SetPlayerID(int new_player_id) {
  if (new_player_id < player_array.GetCount()) player_id = new_player_id;
  else player_id = 0;
};


/** Sets request_id. */
void TEVENT::SetRequestID(int new_request_id, int player_id)
{ 
  if (new_request_id != 0) // set given request_id
    request_id = new_request_id;
  else  // generating new request_id
    request_id = players[player_id]->IncrementRequestCounter();
};

/**
 *  Sets all properties of event.
 *  If n_request_id is 0, autogenerates new request id.
 *  If player with identificator n_player_id does not exists, request_id is set to 0
 */
void TEVENT::SetEventProps(int n_player_id, int n_unit_id, bool n_priority, double n_time_stamp, int n_event, int n_last_event, int n_request_id, T_SIMPLE n_simple1, T_SIMPLE n_simple2, T_SIMPLE n_simple3, T_SIMPLE n_simple4, T_SIMPLE n_simple5, T_SIMPLE n_simple6, int n_int1,int n_int2)
{
  // identification values
  player_id = n_player_id;
  unit_id = n_unit_id;
  
  // if event is in queue and priority is chnged -> event will be stored to another priority queue
  if ((queue_left || queue_right) && (priority != n_priority)) {
    queue_events->GetEvent(this);
    priority = n_priority;
    queue_events->PutEvent(this);
  }
  else
    priority = n_priority;

  #if DEBUG_EVENTS
    if ((queue_left || queue_right) && (time_stamp != n_time_stamp))
        Error(LogMsg("Chnged timestamp of event in queue!"));
    else
  #endif
      time_stamp = n_time_stamp;
  event = n_event;
  last_event = n_last_event;
  
  if (n_request_id != 0) // set given request_id
    request_id = n_request_id;
  else 
  {  // generating new request_id
    if (n_player_id >= player_array.GetCount()) request_id = 0;
    else request_id = players[n_player_id]->IncrementRequestCounter();
  }

  // data values
  simple1 = n_simple1;
  simple2 = n_simple2;
  simple3 = n_simple3;
  simple4 = n_simple4;
  simple5 = n_simple5;
  simple6 = n_simple6;
  
  int1 = n_int1;
  int2 = n_int2;
}

/**
 *  Linearize event to array of chars (prepare event for sending through
 *  network).
 *
 *  @param char_event Input paramenter where linearized event will be stored
 *
 *  @return Count of characters written to char_event (size of linearized
 *          event).
 */
int TEVENT::LinearizeEvent(char *char_event)
{
  int size = 0;
#define pack(variable) \
  memcpy (char_event + size, &variable, sizeof (variable)); \
  size += sizeof (variable);

  pack (player_id);
  pack (unit_id);
  pack (time_stamp);
  pack (event);
  pack (last_event);
  pack (request_id);
  pack (simple1);
  pack (simple2);
  pack (simple3);
  pack (simple4);
  pack (simple5);
  pack (simple6);
  pack (int1);
  pack (int2);

#undef pack
 
  return size;
}

/**
 *  Delinearize event - fill it with data from parameter @p char_event.
 *
 *  @param char_event    Linearized event where data are taken from.
 *  @param lin_event_len Size of linearized event.
 */
void TEVENT::DelinearizeEvent(char *char_event, int lin_event_len)
{
  int pos = 0;

#define unpack(variable) \
  memcpy (&variable, char_event + pos, sizeof (variable)); \
  pos += sizeof (variable);

  unpack (player_id);
  unpack (unit_id);
  unpack (time_stamp);
  unpack (event);
  unpack (last_event);
  unpack (request_id);
  unpack (simple1);
  unpack (simple2);
  unpack (simple3);
  unpack (simple4);
  unpack (simple5);
  unpack (simple6);
  unpack (int1);
  unpack (int2);

#undef unpack

  priority = false;
}


//========================================================================
// class QUEUE_EVENTS
//========================================================================

/**
 *  Constructor. Zeroize the values and create mutex for later use.
 */
TQUEUE_EVENTS::TQUEUE_EVENTS()
{
  // create mutex
  if ((mutex = glfwCreateMutex ()) == NULL) 
    Critical ("Could not create mutex");

  count = 0;    //initialization
  events = prior_events = NULL;  
}


/**
 *  Destructor. Clears and destroys mutex.
 */
TQUEUE_EVENTS::~TQUEUE_EVENTS()
{ 
  Clear ();

  glfwDestroyMutex(mutex);
}


/**
 *  Clear structure.
 */
void TQUEUE_EVENTS::Clear()
{
  TEVENT *act_event, *delete_event;

  glfwLockMutex (mutex);

  // delete priority gueue
  act_event = delete_event = prior_events;
  while(act_event)
  {
    act_event = act_event->GetNextQueueEvent();
    delete delete_event;    
    delete_event = act_event;
  }
  prior_events = NULL;

  // delete basic queue
  act_event = delete_event = events;
  while(act_event)
  {
    act_event = act_event->GetNextQueueEvent();
    delete delete_event;    
    delete_event = act_event;
  }
  events = NULL;

  count = 0;

  glfwUnlockMutex (mutex);
}


/**
 *  Puts event given in parameter to queue according to its timestamp.
 *  Queue is implemented as both-directional linked list.
 */
void TQUEUE_EVENTS::PutEvent(TEVENT *event)
{  
  if (!event) return;

  TEVENT **list_begin, *act_event, *act_event_prev = NULL;

  glfwLockMutex (mutex);

  // for sure
#if DEBUG_EVENTS
  Debug(LogMsg("To Q: P:%d U:%d E:%s RQ:%d X:%d Y:%d Z:%d R:%d TS:%f COUNT:%d", event->GetPlayerID(), event->GetUnitID(), EventToString(event->GetEvent()), event->GetRequestID(), event->simple1, event->simple2, event->simple3, event->simple4, event->GetTimeStamp(), count));

  if (event->GetNextQueueEvent() || event->GetPrevQueueEvent())
    Error("Putting event to queue, that is already in queue!");
#endif

  // set list
  if (event->GetPriority())
    list_begin = &prior_events;
  else
    list_begin = &events;

  // queue is empty
  if (!(*list_begin))
  {
    // add event to begin
    (*list_begin) = event;
  }

  // queue is not empty
  else
  {
    // go to right place in queue
    act_event = *list_begin;
    while (act_event && event->GetTimeStamp() > act_event->GetTimeStamp())
    {
      act_event_prev = act_event;
      act_event = act_event->GetNextQueueEvent();
    }

    // add event at the end of the queue
    if (!act_event)
    {
      act_event_prev->SetNextQueueEvent(event);
      event->SetPrevQueueEvent(act_event_prev); 
    }

    // add event before the first
    else if (act_event == *list_begin)
    {
      event->SetNextQueueEvent(act_event);
      act_event->SetPrevQueueEvent(event); 
      (*list_begin) = event;
    }

    // add event before act_event
    else
    {
      event->SetPrevQueueEvent(act_event_prev);
      event->SetNextQueueEvent(act_event);
      act_event->SetPrevQueueEvent(event);
      act_event_prev->SetNextQueueEvent(event);
    }    
  }

  // inc count
  count++;

#if DEBUG_EVENTS
  int c = 0;
  for (act_event = prior_events; act_event; act_event = act_event->GetNextQueueEvent())
    c++;
  for (act_event = events; act_event; act_event = act_event->GetNextQueueEvent())
    c++;

  if (c != count)
  {
    Error("Broken queue! - PutEvent()");
    LogQueue();
  }
#endif

  glfwUnlockMutex (mutex);
}


/**
 *  Gets event from queue  according to given pointer.
 */
void TQUEUE_EVENTS::GetEvent(TEVENT *event)
{
  if (!event) return;

  TEVENT **list_begin;

  glfwLockMutex (mutex);

  #if DEBUG_EVENTS
  Debug(LogMsg("Fr Q: P:%d U:%d E:%s RQ:%d X:%d Y:%d Z:%d R:%d TS:%f COUNT:%d", event->GetPlayerID(), event->GetUnitID(), EventToString(event->GetEvent()), event->GetRequestID(), event->simple1, event->simple2, event->simple3, event->simple4, event->GetTimeStamp(), count));
  #endif

  // set list
  if (event->GetPriority())
    list_begin = &prior_events;
  else
    list_begin = &events;
 
  // remove event from list
  if (event->GetNextQueueEvent()) 
    event->GetNextQueueEvent()->SetPrevQueueEvent(event->GetPrevQueueEvent());
  if (event->GetPrevQueueEvent())
    event->GetPrevQueueEvent()->SetNextQueueEvent(event->GetNextQueueEvent());   

  // if event is first
  if (event == *list_begin)
    *list_begin = (*list_begin)->GetNextQueueEvent();

  // clear event
  event->SetNextQueueEvent(NULL);
  event->SetPrevQueueEvent(NULL);

  // decrease events count
  count--;

#if DEBUG_EVENTS
  TEVENT *act_event;
  int c = 0;
  for (act_event = prior_events; act_event; act_event = act_event->GetNextQueueEvent())
    c++;
  for (act_event = events; act_event; act_event = act_event->GetNextQueueEvent())
    c++;

  if (c != count)
  {
    Error("Broken queue! - GetEvent()");
    LogQueue();
  }
#endif

  glfwUnlockMutex (mutex);
}


/**
 *  Returns pointer to the event with lowest timestamp.
 */
TEVENT *TQUEUE_EVENTS::GetFirstEvent()
{
  TEVENT **list_begin, *first;

  glfwLockMutex (mutex);

  // set list
  if (prior_events)
    list_begin = &prior_events;
  else if (events)
    list_begin = &events;
  else {
    glfwUnlockMutex (mutex);
    return NULL;
  }

  first = *list_begin;
  (*list_begin) = first->GetNextQueueEvent();
  if (*list_begin)
    (*list_begin)->SetPrevQueueEvent(NULL); 
  
  first->SetNextQueueEvent(NULL);
  first->SetPrevQueueEvent(NULL);

#if DEBUG_EVENTS
  Debug(LogMsg("Fr Q: P:%d U:%d E:%s RQ:%d X:%d Y:%d Z:%d R:%d TS:%f COUNT:%d", first->GetPlayerID(), first->GetUnitID(), EventToString(first->GetEvent()), first->GetRequestID(), first->simple1, first->simple2, first->simple3, first->simple4, first->GetTimeStamp(), count));
#endif


  // decrease events count
  count--;


#if DEBUG_EVENTS
  TEVENT *act_event;
  int c = 0;
  for (act_event = prior_events; act_event; act_event = act_event->GetNextQueueEvent())
    c++;
  for (act_event = events; act_event; act_event = act_event->GetNextQueueEvent())
    c++;

  if (c != count)
  {
    Error("Broken queue! - GetFirstEvent()");
    LogQueue();
  }
#endif


  glfwUnlockMutex (mutex);

  return first;
}


/**
 *  Returns timestamp of event with lowest timestamp. If there is no event in queue returns -1.
 */
double TQUEUE_EVENTS::GetFirstEventTimeStamp()
{ 
  double ts;
  TEVENT **list_begin;

  glfwLockMutex (mutex);

  // set list
  if (prior_events)
    list_begin = &prior_events;
  else if (events)
    list_begin = &events;
  else {
    glfwUnlockMutex (mutex);
    return -1;
  }

  ts = (*list_begin)->GetTimeStamp();

  glfwUnlockMutex (mutex);
  
  return ts;
}


#if DEBUG
/**
 *  Writes whole message queue to log file.
 */
void TQUEUE_EVENTS::LogQueue(void)
{
  TEVENT *act_event;
  int c;

  glfwLockMutex (mutex);

  Debug(LogMsg("*** TQUEUE_EVENTS - count: %d", count));

  // write priority queue
  Debug("*** Priority message queue ***");

  c = 0;
  for (act_event = prior_events; act_event; act_event = act_event->GetNextQueueEvent())
  {
    Debug(LogMsg("*** P:%d U:%d E:%s RQ:%d X:%d Y:%d Z:%d R:%d TS:%f", act_event->GetPlayerID(), act_event->GetUnitID(), EventToString(act_event->GetEvent()), act_event->GetRequestID(), act_event->simple1, act_event->simple2, act_event->simple3, act_event->simple4, act_event->GetTimeStamp()));
    c++;
  }
  Debug(LogMsg("*** count: %d", c));

  // write basic queue
  Debug("*** Basic message queue ******");

  c = 0;
  for (act_event = events; act_event; act_event = act_event->GetNextQueueEvent())
  {
    Debug(LogMsg("*** P:%d U:%d E:%s RQ:%d X:%d Y:%d Z:%d R:%d TS:%f", act_event->GetPlayerID(), act_event->GetUnitID(), EventToString(act_event->GetEvent()), act_event->GetRequestID(), act_event->simple1, act_event->simple2, act_event->simple3, act_event->simple4, act_event->GetTimeStamp()));
    c++;
  }
  Debug(LogMsg("*** count: %d", c));

  Debug("*** END **********************");

  glfwUnlockMutex (mutex);
}
#endif


//=========================================================================
// END
//=========================================================================
// vim:ts=2:sw=2:et:

