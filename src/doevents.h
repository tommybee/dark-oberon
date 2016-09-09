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
 *  @file doevents.h
 *
 *  Event structures.
 *
 *  @author Martin Kosalko
 *  @author Valeria Sventova
 *
 *  @date 2004
 */

#ifndef __doevents_h__
#define __doevents_h__


//========================================================================
// Forward declarations
//========================================================================

class TEVENT;
class TQUEUE_EVENTS;

//========================================================================
// Definitions & typedefs
//========================================================================
//event type
#define ET_NONE                         3000  //initial state  
#define ET_NEXTMINE_SOURCE_OK           3001  //unit is in state US_NEXT_MINE, new source found ,OK = 1
#define ET_NEXTMINE_SOURCE_NOK          3002  //unit is in state US_NEXT_MINE, new source found ,NOK = 1
#define ET_ATTACK_TOO_FAR_AWAY          3003  //unit is in state US_NEXT_ATTACK, but is too far away from attacking goal
#define ET_TARGET_MOVING                3004  //unit has target, which is moving
#define ET_HIDER_MOVING                 3005  //unit has hider, which is moving and change position
#define ET_CONSTRUCTED_OBJECT_MOVING    3006  //unit has build_or_repaired_unit, which is moving and change position
#define ET_NOTNEXTPOS_LANDING           3007  //next position in path is not moveable, but last -> landing
#define ET_UNIT_AGAINST_UNIT            3008  //next position is not empty -> worker unit against the another unit 
#define ET_WORKER_ACTIVITY              3009  //unit is one of following states: US_START_MINE, US_START_REPAIR,US_START_HIDING,US_START_UNLOAD
#define ET_NEXT_HIDING                  3010  //unit is in state US_NEXT_HIDING  
#define ET_NEXT_UNLOADING               3011  //worker will unload material if possible
#define ET_MINED_HAS_ACCEPTOR           3012  //worker mined maximal amount of material and has acceptable building
#define ET_CANTMINE_CANLEAVE_SOURCE     3013  //worker cannot mine any more,can leave source unit and found new source, where can continue in mining
#define ET_NOTPATH_LAND                 3014  //path is NULL -> stop or land unit
#define ET_NEXTPOS_NOTMOVABLE           3015  //next position in path is not moveable
#define ET_FORCE_UNIT_HIDING            3016  //special state for force unit (part of ET_WORKER_ACTIVITY) 
#define ET_ATTACK_TO_OTHER_SEG          3017  //unit is in state US_NEXT_ATTACK;attack is possible only in the other segment (upper or lower)
#define ET_UNLOAD_DEST_RUINED           3018  //unit has material to unload; acceptable building ruined while unit walks towards it
#define ET_UNLOAD_DEST_BUILD_NOT_HELD   3019  //unit is in state US_NEXT_UNLOADING,acceptor is in state US_IS_BEING_BUILT and unit was not held in acceptor
#define ET_UNLOAD_DEST_NOT_NEIGHBOUR    3020  //unit is in state US_NEXT_UNLOADING,acceptor is not neighbour->unit has to find its way to acceptor          
#define ET_NEXTMINE_SOURCE_DESTR        3021  //unit is in state US_NEXT_MINE, it is in the neighbourhood of the source, but source is in one of states US_DYING or US_ZOMBIE or US_DELETE  
#define ET_NEXT_CR_NOT_NEIGHBOUR        3022  //unit is in state US_NEXT_CONSTRUCTING or US_NEXT_REPAIRING, but build/repaired unit is not in the heighbourhood , so unit has to walk towards it
#define ET_UNLOAD_HELD                  3023  //unit has material to unload;it is in some held list;acceptable building was found  
#define ET_NEXTMINE_NOT_HEIGHBOUR       3024  //unit is in state US_NEXT_MINE, doesnt have max.amount of material and unit is not neighbour of source    
#define ET_CANTMINE_LEAVE_NOSOURCE_OK   3025  //unit has some unmined material, new source wasnt found, needs to go to the acceptable building to unload at least what has, ok = 1.
#define ET_CANTMINE_LEAVE_NOSOURCE_NOK  3026  //unit has some unmined material, new source wasnt found, needs to go to the acceptable building to unload at least what has, ok = 0.


//searching for nearest building
#define ET_MINED_SEARCH_ACCEPTOR        3027  //unit already unmined maximal amount of material and will search for nearest acceptable building     
#define ET_ACC_BUILD_DESTROYED          3028  //unit unmined material, but acceptable building was ruined while unit walks towards it
#define ET_UNLOAD_HELD_SEARCH_ACC       3029  //unit has material to unload;it is in some held list;acceptable building must be found now
#define ET_UNLOAD_DEST_BUILD_SEARCH_ACC 3030  //unit is in state US_NEXT_UNLOADING,acceptor is in stae US_BEING_BUILD,unit was not held, accept.building searched. 
#define ET_UNLOAD_NO_SOURCE             3031  //unit unloaded material, but not maximal amount, source has collapsed, new source was not found, unit searches acceptable building

//group moving
#define ET_GROUP_MOVING                 3032  //unit is moving as a part of bigger group of units.

//time stamps
#define TS_MIN_EVENTS_DIFF              0.000001f //time between two events which are send through network (with same TS) (TS of secon will be TS + TS_MIN_EVENTS_DIFF)

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

#include "dosimpletypes.h"
#include "dopool.h"

//========================================================================
// class TEVENT
//========================================================================

class TEVENT: public TPOOL_ELEMENT 
{
private:
  // information stored in event
  int player_id;              //!< Player identificator.
  int unit_id;                //!< Unit identificator.
  bool priority;              //!< Specified to which queue will be event stored.
  double time_stamp;          //!< Time stamp of event.
  int event;                  //!< Type of event. (states US_MOVE, US_ATTACK ....)
  int last_event;             //!< Type of event that was replaned. (if event was not replaned, US_NONE is set)
  int request_id;             //!< Unique identificator of request.
  
  // Pointers needed for event queue structure.
  TEVENT * queue_left;    //!< Pointer to next event in TQUEUE_EVENTS.
  TEVENT * queue_right;   //!< Pointer to previous event in TQUEUE_EVENTS.


public:
  T_SIMPLE simple1, simple2, simple3, simple4, simple5, simple6;
  int int1, int2;
  
  //!< Returns identificator of player.
  int GetPlayerID(void) {return player_id;};
  //!< Sets player identificator. If new_player_id >= player_array.GetCount(), sets 0.
  void SetPlayerID(int new_player_id);
  
  //!< Returns identificator of unit.
  int GetUnitID(void) {return unit_id;};
  //!< Sets unit identificator.
  void SetUnitID(int new_unit_id) {unit_id = new_unit_id;};

  //!< Returns priority of event.
  bool GetPriority(void) {return priority;};
  //!< Sets priority of event.
  void SetPriority(bool new_priority) {priority = new_priority;};

  //!< Returns time stamp of event.
  double GetTimeStamp(void) {return time_stamp;};
  //!< Sets time stamp of unit.
  void SetTimeStamp(double new_time_stamp) {time_stamp = new_time_stamp;};

  //!< Tests if event stored in event is equal to @param n_event.
  bool TestEvent(int n_event) {return n_event == event;};
  //!< Returns event.
  int GetEvent(void) {return event;};
  //!< Sets event.
  void SetEvent(int new_event) {event = new_event;};

  //!< Tests if event stored in last_event is equal to @param n_last_event.
  bool TestLastEvent(int n_last_event) {return n_last_event == last_event;};
  //!< Returns event.
  int GetLastEvent(void) {return last_event;};
  //!< Sets last_event.
  void SetLastEvent(int new_last_event) {last_event = new_last_event;};

  //!< Returns request_id.
  int GetRequestID(void) {return request_id;};
  //!< Sets request_id.
  void SetRequestID(int new_request_id, int player_id);

  //!< Sets next pool event.
  void SetNextPoolEvent(TEVENT * new_pool_next) { pool_next = new_pool_next;};

  //!< Sets next queue event.
  void SetNextQueueEvent(TEVENT *queue_event) { queue_right = queue_event;};

  //!< Sets previous queue event.
  void SetPrevQueueEvent(TEVENT *queue_event) { queue_left = queue_event;};

  //!< Returns next queue event.
  TEVENT *GetNextQueueEvent(void) {return queue_right;};

  //!< Returns previous queue event.
  TEVENT *GetPrevQueueEvent(void) {return queue_left;};
  
  void Clear(bool all); // Clears TEVENT values. If all == true, clears pointers too
  void SetEventProps(int n_player_id, int n_unit_id, bool n_priority, double n_time_stamp, int n_event, int n_last_event, int n_request_id, T_SIMPLE n_simple1, T_SIMPLE n_simple2, T_SIMPLE n_simple3, T_SIMPLE n_simple4, T_SIMPLE n_simple5, T_SIMPLE n_simple6, int n_int1,int n_int2=0); // sets event's properties
  
  int LinearizeEvent(char * char_event); // Linearize event to array of chars (prepare event for net).
  void DelinearizeEvent(char * char_event, int lin_event_len); // Delinearize event from array of chars (fill event with data in char).
  
  TEVENT(void);   // Constructor only zeroize all data.
};

//========================================================================
// class TQUEUE_EVENTS
//========================================================================

class TQUEUE_EVENTS {
private:
  TEVENT *events;         //!< List of events in the queue.
  TEVENT *prior_events;   //!< List of events in the queue.
  int count;              //!< Count of events in the queue.
#ifdef NEW_GLFW3
	mtx_t mutex;
#else
	GLFWmutex mutex;        //!< Queue mutex.
#endif
  

public:
  TEVENT * GetFirstEvent(void);         // Returns pointer to event with lowest timestamp and deletes it from queue.
  double GetFirstEventTimeStamp(void);  // Returns time stamp of event with lowest timestamp. If queue is empty, return -1.
  void PutEvent(TEVENT *event);         // Puts event to queue according to ist timestamp.
  void GetEvent(TEVENT *event); // Gets event from queue  according to given pointer.

  //!< Returns queue length.
  int GetQueueLength() { return count; };  

  void Clear();

#if DEBUG
  void LogQueue(void);
#endif

  TQUEUE_EVENTS();        // Constructor.
  ~TQUEUE_EVENTS();       // Destructor.
};

//========================================================================
// Global variables
//========================================================================

extern TPOOL<TEVENT> * pool_events;
extern TQUEUE_EVENTS * queue_events;

#ifdef NEW_GLFW3
extern mtx_t delete_mutex;
#else
extern GLFWmutex delete_mutex;
#endif


//========================================================================
// Global functions
//========================================================================

#endif // __doevents_h__

//========================================================================
// End
//========================================================================
// vim:ts=2:sw=2:et:

