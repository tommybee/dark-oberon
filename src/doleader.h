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
 *  @file doserver.h
 *
 *  Server.
 *
 *  @author Marian Cerny
 *
 *  @date 2003
 */

#ifndef __doserver_h__
#define __doserver_h__


//=========================================================================
// Included files
//=========================================================================

#include "cfg.h"
#include "doalloc.h"

#include <vector>
#include <string>

#include "dohost.h"


//=========================================================================
// Constants
//=========================================================================

const int leader_in_queue_size = 10024;
const int leader_out_queue_size = 10024;


//=========================================================================
// TLEADER
//=========================================================================

/**
 *  Leader is a type of host used in menu. There is one leader which sets up
 *  the game. Other hosts are connected to leader as followers (#TFOLLOWER).
 */
class TLEADER : public THOST {
public:
  TLEADER (int incoming_message_queue_size, in_port_t listen_port,
           int outgoing_message_queue_size);

  void ConnectFollower (in_addr remote_address, in_port_t remote_port, int fd);

  void SendPingReply (double request_time);

  void SendPlayerArray (std::string map_name, bool start_game);

  void SendAllowProcessFunction ();

  void FillRemoteAddresses ();
};


#endif

//=========================================================================
// END
//=========================================================================
// vim:ts=2:sw=2:et:

