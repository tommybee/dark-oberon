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
 *  @file dohost.h
 *
 *  Common functionality for all host types (client, server, leader, follower).
 *
 *  @author Marian Cerny
 *
 *  @date 2004
 */

#ifndef __dohost_h__
#define __dohost_h__


//=========================================================================
// Included files
//=========================================================================

#include "cfg.h"
#include "doalloc.h"

#include <string>

#include "donet.h"


//=========================================================================
// Enumerations
//=========================================================================

/**
 *  All types of messages for network communication used by all host types.
 */
enum NetworkingProtocolMessage {
  net_protocol_chat_message = 0x01,   //!< Chat message.
  net_protocol_connect      = 0x02,   //!< Connect request.
  net_protocol_player_array = 0x03,   //!< Full info about players.
  net_protocol_hello        = 0x04,   //!< Hello message.
  net_protocol_change_race  = 0x05,   //!< Change race of a player.
  net_protocol_event        = 0x06,   //!< Event of unit.
  net_protocol_synchronise  = 0x07,   //!< Synchronisation.
  net_protocol_disconnect   = 0x08,   //!< Disconnect message.
  net_protocol_ping         = 0x09,   //!< Ping request.
  net_protocol_end
};


//=========================================================================
// THOST
//=========================================================================

const int all_players = 255;

/**
 *  Base class for a network communication participant (client, server). It
 *  does not contain any talker (TNET_TALKER), because generic host does not
 *  know how many hosts will be connected to it.
 */
class THOST {
public:
  /** Type of the host. */
  enum HostType {
    ht_leader,    //!< Leader (TLEADER).
    ht_follower,  //!< Follower (TFOLLOWER).
  };

public:
  THOST (int incoming_message_queue_size, in_port_t listen_port,
         int outgoing_message_queue_size);

  THOST (int incoming_message_queue_size, in_port_t listen_port,
         int outgoing_message_queue_size, in_addr remote_address,
         in_port_t remote_port);

  virtual ~THOST ();

  void SendChatMessage (std::string chat_message);

  void SendDisconnect (T_BYTE player_id);

  /** Sends networking message to all connected hosts for which the message is
   *  destined for. */
  void SendMessage (TNET_MESSAGE *msg) {
    talker->GetMessageQueue ()->PutMessage (msg);
  }

  /** Sends networking message to specified destination. If destination is
   *  XXX:255, the message is send to all connected hosts. */
  void SendMessage (TNET_MESSAGE *msg, T_BYTE destination) {
    msg->SetDest (destination);

    SendMessage (msg);
  }

  /** Returns type of the host. */
  HostType GetType ()
  { return type; }

  /** Returns listener. */
  TNET_LISTENER *GetListener ()
  { return listener; }

  /** Returns talker. */
  TNET_TALKER *GetTalker ()
  { return talker; }

  in_addr GetRemoteAddress () { return talker->GetRemoteAddress (0); }
  in_port_t GetRemotePort () { return talker->GetRemotePort (0); }

  void RemoveAllAddresses () {
    talker->RemoveAllAddresses ();
  }

  T_BYTE AddRemoteAddress (in_addr address, in_port_t port) {
    return talker->AddAddress (address, port);
  }

  T_BYTE AddRemoteAddress (in_addr address, in_port_t port, int fd) {
    return talker->AddAddress (address, port, fd);
  }

  T_BYTE AddEmptyAddress () {
    return talker->AddEmptyAddress ();
  }

  void RegisterExtendedFunction (T_BYTE type, void (*f)(TNET_MESSAGE *msg)) {
    handler->RegisterExtendedFunction (type, f);
  }

  void DisconnectAddress (int id) {
    talker->DisconnectAddress (id);
  }

  void DisconnectAddress (in_addr address, in_port_t port) {
    talker->DisconnectAddress (address, port);
  }

  void RemoveAddress (int id) {
    talker->RemoveAddress (id);
  }

  void RegisterOnDisconnect (void (* f)(in_addr, in_port_t)) {
    listener->RegisterOnDisconnect (f);
  }

private:
  TNET_LISTENER *listener;        //!< Listener.
  TNET_TALKER *talker;
  TNET_MESSAGE_HANDLER *handler;  //!< Handler.
  TNET_DISPATCHER *dispatcher;    //!< Dispatcher.

protected:
  HostType type;    //!< Type of the host.
};


#endif

//=========================================================================
// END
//=========================================================================
// vim:ts=2:sw=2:et:

