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
 *  @file dohost.cpp
 *
 *  Implementation of functions for THOST.
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

#include "dohost.h"
#include "donet.h"

using std::string;


//=========================================================================
// THOST
//=========================================================================

THOST::THOST (int incoming_message_queue_size, in_port_t listen_port,
              int outgoing_message_queue_size)
{
  listener = NEW TNET_LISTENER (incoming_message_queue_size, listen_port);
  talker = NEW TNET_TALKER (outgoing_message_queue_size);
  handler = NEW TNET_MESSAGE_HANDLER ();
  dispatcher = NEW TNET_DISPATCHER (listener->GetMessageQueue (), handler);

  listener->ConsumerIsAttached (dispatcher->GetThread ());
}

/**
 *  Constructor.
 *
 *  @param incoming_message_queue_size Size of queue for incoming messages.
 *  @param listen_port                 Port on which #listener should listen.
 */
THOST::THOST (int incoming_message_queue_size, in_port_t listen_port,
              int outgoing_message_queue_size, in_addr remote_address,
              in_port_t remote_port)
{
  talker = NEW TNET_TALKER (outgoing_message_queue_size, remote_address,
                            remote_port);

  listener = NEW TNET_LISTENER (incoming_message_queue_size, listen_port);
  handler = NEW TNET_MESSAGE_HANDLER ();
  dispatcher = NEW TNET_DISPATCHER (listener->GetMessageQueue (), handler);

  listener->ConsumerIsAttached (dispatcher->GetThread ());
  listener->AddListenerByFileDescriptor (remote_address, remote_port, talker->GetRemoteFiledescriptor (0));
}

THOST::~THOST ()
{
  delete talker;

  /* Order in which objects are deleted must be: listener, dispatcher, handler!
   */
  delete listener;
  delete dispatcher;
  delete handler;
}

/**
 *  Sends chat message to all connected hosts.
 *
 *  @param chat_message Message content.
 */
void THOST::SendChatMessage (string chat_message) {
  TNET_MESSAGE *m = pool_net_messages->GetFromPool();
  m->Init_send(net_protocol_chat_message, 0);
  m->PackString (chat_message);

  SendMessage (m);
}

/**
 *  Sends chat message to all connected hosts.
 *
 *  @param chat_message Message content.
 */
void THOST::SendDisconnect (T_BYTE player_id) {
  TNET_MESSAGE *m = pool_net_messages->GetFromPool();
  m->Init_send(net_protocol_disconnect, 0);
  m->PackByte (player_id);

  SendMessage (m);
}


//=========================================================================
// END
//=========================================================================
// vim:ts=2:sw=2:et:

