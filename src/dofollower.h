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
 *  @file doclient.h
 *
 *  Client side of the client-server communication.
 *
 *  @author Marian Cerny
 *
 *  @date 2004
 */

#ifndef __doclient_h__
#define __doclient_h__


//=========================================================================
// Included files
//=========================================================================

#include "cfg.h"
#include "doalloc.h"

#include <string>

#include "dohost.h"
#include "donet.h"


//=========================================================================
// Constants.
//=========================================================================

/** Timeout for resolve to fail. */
const double resolve_timeout = 30;
/** Timeout for connect to fail. */
const double connect_timeout = 10;

/** Size of incoming message queue for follower. */
const int follower_in_queue_size = 10024;
/** Size of outgoing message queue for follower. */
const int follower_out_queue_size = 10024;


//=========================================================================
// TFOLLOWER
//=========================================================================

/**
 *  Follower is a type of host used in menu. Follower is connected to a leader
 *  (TLEADER).
 */
class TFOLLOWER : public THOST {
public:
  TFOLLOWER (int incoming_message_queue_size, in_port_t listen_port,
             int outgoing_message_queue_size, in_addr remote_address,
             in_port_t remote_port);

  void Connect (std::string);
  void ChangeRace (int index, std::string player_name, std::string new_race);

  void SetMyAddress (in_addr address, in_port_t port);

  in_addr GetMyAddress () { /* XXX: have_my_address */ return my_address; }
  in_port_t GetMyPort () { /* XXX: have_my_address */ return my_port; }

  double GetPingRequestTime () { return ping_request_time; }
  double GetMinimalTimeshift () { return minimal_timeshift; }
  void SetMinimalTimeshift (double timeshift) { minimal_timeshift = timeshift; }

  void SendPingRequest ();
  void SendSynchronise ();


private:
  bool have_my_address;
  in_addr my_address;
  in_port_t my_port;

  double ping_request_time;   //!< Time when the connect request was sent.
  double minimal_timeshift;   //!< Minimal timeshift yet received.
};


#endif

//=========================================================================
// END
//=========================================================================
// vim:ts=2:sw=2:et:

