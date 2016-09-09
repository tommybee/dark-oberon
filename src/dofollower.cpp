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
 *  @file doclient.cpp
 *
 *  Client side of the client-server communication.
 *
 *  @author Marian Cerny
 *
 *  @date 2004
 */


//=========================================================================
// Included files
//=========================================================================

#include "cfg.h"
#include "doalloc.h"

#include <string>

#include "dofollower.h"
#include "donet.h"

using std::string;


//=========================================================================
// TFOLLOWER
//=========================================================================

/**
 *  Constructor.
 *
 *  @param incoming_message_queue_size Size of the incoming message queue.
 *  @param listen_port                 The port on which #listener should
 *                                     listen.
 *  @param outgoing_message_queue_size Size of the outgoing message queue.
 *  @param remote_address              Address of the leader.
 *  @param remote_port                 Port of the leader.
 */
TFOLLOWER::TFOLLOWER (int incoming_message_queue_size, in_port_t listen_port,
                      int outgoing_message_queue_size, in_addr remote_address,
                      in_port_t remote_port)
:THOST (incoming_message_queue_size, listen_port, outgoing_message_queue_size,
        remote_address, remote_port)
{
  type = ht_follower;
  have_my_address = false;
}

/**
 *  Connects the follower to the leader.
 *
 *  @param player_name Name of the player to connect.
 */
void TFOLLOWER::Connect (string player_name) {
  in_port_t listener_port = GetListener ()-> GetPort ();

  TNET_MESSAGE *m = pool_net_messages->GetFromPool();
  m->Init_send(net_protocol_connect, 0);
  m->Pack (&listener_port, sizeof (in_port_t));
  m->PackString (player_name);

  ping_request_time = glfwGetTime ();

  /* Send message to leader. */
  SendMessage (m, 0);
}

/**
 *  Sends a message to change race of a player.
 *
 *  @param index       Index of the player.
 *  @param player_name Name of the player to connect.
 *  @param new_race    Id name of the new race.
 */
void TFOLLOWER::ChangeRace (int index, string player_name, string new_race) {
  TNET_MESSAGE *m = pool_net_messages->GetFromPool();
  m->Init_send(net_protocol_change_race, 0);
  m->PackByte (static_cast<T_BYTE>(index));
  m->PackString (player_name);
  m->PackString (new_race);

  /* Send message to leader. */
  SendMessage (m, 0);
}

void TFOLLOWER::SetMyAddress (in_addr address, in_port_t port) {
  my_address = address;
  my_port = port;

  have_my_address = true;
}

void TFOLLOWER::SendPingRequest () {
  TNET_MESSAGE *msg = pool_net_messages->GetFromPool();
  msg->Init_send(net_protocol_ping, 0);

  ping_request_time = glfwGetTime ();
  msg->Pack (&ping_request_time, sizeof ping_request_time);

  SendMessage (msg, 0);
}

void TFOLLOWER::SendSynchronise () {
  TNET_MESSAGE *msg = pool_net_messages->GetFromPool();
  msg->Init_send(net_protocol_synchronise, 0);
  SendMessage (msg, 0);
}

//=========================================================================
// END
//=========================================================================
// vim:ts=2:sw=2:et:

