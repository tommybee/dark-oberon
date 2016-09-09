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
 *  @file doserver.cpp
 *
 *  Server.
 *
 *  @author Marian Cerny
 *
 *  @date 2003, 2004
 */


//=========================================================================
// Included files
//=========================================================================

#include "cfg.h"
#include "doalloc.h"

#include <string>

#include "doleader.h"
#include "dohost.h"
#include "doplayers.h"

using std::string;


//=========================================================================
// TLEADER
//=========================================================================

/**
 *  Constructor.
 *
 *  @param incoming_message_queue_size Size of the incoming message queue.
 *  @param listen_port                 Port on which listener should listen.
 */
TLEADER::TLEADER (int incoming_message_queue_size, in_port_t listen_port,
                  int outgoing_message_queue_size)
:THOST (incoming_message_queue_size, listen_port, outgoing_message_queue_size)
{
  type = ht_leader;
}

void TLEADER::ConnectFollower (in_addr remote_address, in_port_t remote_port, int fd)
{
  T_BYTE dest = AddRemoteAddress (remote_address, remote_port, fd);

  TNET_MESSAGE *m = pool_net_messages->GetFromPool();
  m->Init_send(net_protocol_hello, 0);

  m->Pack (&remote_address, sizeof remote_address);
  m->Pack (&remote_port, sizeof remote_port);

  double time = glfwGetTime ();

  m->Pack (&time, sizeof time);

  SendMessage (m, dest);
}

void TLEADER::SendPingReply (double request_time) {
  TNET_MESSAGE *m = pool_net_messages->GetFromPool();
  m->Init_send(net_protocol_ping, 1);

  double time = glfwGetTime ();

  m->Pack (&request_time, sizeof request_time);
  m->Pack (&time, sizeof time);

  SendMessage (m);
}

void TLEADER::SendPlayerArray (string map_name, bool start_game) {
  TNET_MESSAGE *m = pool_net_messages->GetFromPool();
  m->Init_send(net_protocol_player_array, start_game);

  player_array.Lock ();

  double time = glfwGetTime ();

  m->Pack (&time, sizeof time);

  m->PackString (map_name);

  /* Count of players. */
  m->PackByte (static_cast<T_BYTE>(player_array.GetCount ()));

  for (int i = 0; i < player_array.GetCount(); i++) {
    m->PackString (player_array.GetPlayerName (i));
    m->PackString (player_array.GetRaceIdName (i));

    m->PackByte (static_cast<T_BYTE>(player_array.IsComputer (i)));
    m->PackByte (static_cast<T_BYTE>(player_array.IsRemote (i)));

    m->PackByte (static_cast<T_BYTE>(player_array.GetStartPoint (i)));

    if (player_array.IsRemote (i)) {
      in_addr addr = player_array.GetAddress (i);
      in_port_t port = player_array.GetPort (i);

      m->Pack (&addr, sizeof (in_addr));
      m->Pack (&port, sizeof (in_port_t));
    }
  }

  player_array.Unlock ();

  SendMessage (m);
}

void TLEADER::SendAllowProcessFunction () {
  TNET_MESSAGE *m = pool_net_messages->GetFromPool();
  m->Init_send(net_protocol_synchronise, 1);
  SendMessage (m);
}


//=========================================================================
// END
//=========================================================================
// vim:ts=2:sw=2:et:

