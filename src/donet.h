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
 *  @file donet.h
 *
 *  Header file for networking.
 *
 *  @author Marian Cerny
 *
 *  @date 2003, 2004
 */

#ifndef __donet_h__
#define __donet_h__


//=========================================================================
// Included files
//=========================================================================

#include "cfg.h"
#include "doalloc.h"

#ifdef WINDOWS
 #undef APIENTRY
 #undef WINGDIAPI
 #include <winsock.h>
#else
/* #include <arpa/inet.h>*/
 #include <netdb.h>
 #include <netinet/in.h>
 #include <sys/socket.h>
 #include <sys/types.h>
 #include <unistd.h>
#endif

#include <string>
#include <vector>

#include "dologs.h"
#include "dosimpletypes.h"
#include "dopool.h"
#include "doipc.h"


//=========================================================================
// Typedefs
//=========================================================================

/*
 * On Windows we need some typedefs for POSIX networking types used in project.
 */
#ifdef WINDOWS
typedef int socklen_t;
typedef unsigned short in_port_t;
#endif


//=========================================================================
// Constants
//=========================================================================

/** Maximum size of networking message including headers. */
const int max_net_message_size = 255;


//=========================================================================
// Define macros
//=========================================================================

/**
 *  @def SOCKET_ERROR_MESSAGE(msg)
 *  Macro returning string describing last error with sockets.
 *
 *  @param msg String which will be prepended to the log message.
 *
 *  @note This has got different implementation on #UNIX and #WINDOWS. UNIX
 *        uses standard mechanism with errno. Windows uses function
 *        WSAGetLastError() to get last socket error code.
 */
#ifdef WINDOWS
# define SOCKET_ERROR_MESSAGE(msg) \
  (_snprintf (socket_error_message, MAX_LOG_MESSAGE_SIZE, "%s: WSAGetLastError = %d", (msg), WSAGetLastError()), \
   socket_error_message)
#else
# include <errno.h>
# define SOCKET_ERROR_MESSAGE(msg) \
  (snprintf (socket_error_message, MAX_LOG_MESSAGE_SIZE, "%s: %s", (msg), strerror (errno)), \
   socket_error_message)
#endif


//=========================================================================
// Variables
//=========================================================================

extern TLOG_MESSAGE socket_error_message;

//=========================================================================
// TNET_ADDRESS
//=========================================================================

struct TNET_ADDRESS {
public:
  class ConnectingErrorException {};

  TNET_ADDRESS (in_addr address, in_port_t port);
  TNET_ADDRESS (in_addr address, in_port_t port, int fd);
  TNET_ADDRESS ();
  ~TNET_ADDRESS ();

  in_addr GetAddress () { return address; }
  in_port_t GetPort ()  { return port; }

  int GetFileDescriptor ()
  { return fd; }

  void Disconnect ();

  bool Connected ()
  { return connected; }

  bool IsEmpty ()
  { return empty; }

private:
  in_addr     address;
  in_port_t   port;
  int         fd;

  bool        connected;
  bool        empty;
};


//=========================================================================
// TNET_MESSAGE
//=========================================================================

/**
 *  Network message containing one event. One network packet can contain more
 *  messages.
 */
struct TNET_MESSAGE : public TPOOL_ELEMENT {
public:
  class SendError {};

  // XXX: 255
  TNET_MESSAGE();
  
  void Init_send (T_BYTE type, T_BYTE subtype, T_BYTE dest = 255);
  void Init_receive (in_addr address, in_port_t port, int fd, T_BYTE *data);

  T_BYTE GetSize ()     { return size; }
  T_BYTE GetType ()     { return type; }
  T_BYTE GetSubtype ()  { return subtype; }
  T_BYTE GetDest ()     { return dest; }

  void SetDest (T_BYTE dest)  { this->dest = dest; }

  const T_BYTE *GetBuf () { return buf; }

  /** Puts @p size of bytes from memory at address @p data into the message. */
  void Pack (const void *data, int size)
  {
    if (this->size + size > max_net_message_size) {
      Critical ("Size of the message too big");
      throw 0;
    }
    memcpy (&buf[this->size - GetHeaderSize ()], data, size);
    this->size += size;
  }

  /** Puts a byte into the message. */
  void PackByte (T_BYTE byte)
  { 
    if (this->size + 1 > max_net_message_size) {
      Critical ("Size of the message too big");
      throw 0;
    }
    buf[this->size++ - GetHeaderSize ()] = byte;
  }

  /** Puts a string @p str into the message. */
  void PackString (std::string str)
  { Pack (str.c_str (), str.length () + 1); }

  /** Gets @p size of bytes from message into memory at address @p data. */
  void Extract (void *data, int size)
  {
    memcpy (data, &buf[extract_p], size);
    extract_p += size;
  }

  /** Gets a byte from the message. */
  T_BYTE ExtractByte ()
  { return buf[extract_p++]; }

  /** Gets a string from a message. */
  std::string ExtractString ()
  { 
    int old_p = extract_p;
    const char *char_p = reinterpret_cast<const char *>(&buf[old_p]);
    extract_p += strlen (char_p) + 1;
    return char_p;
  }

  /** Finds out the header size of the message. */
  static int GetHeaderSize ()
  { return 4 * sizeof (T_BYTE); /* size, type, subtype, dest */ }

  /** Returns pointer to start of the header. */
  const char *GetHeaderStart () { return (const char *)&size; }

  in_addr GetAddress () { return address; }
  in_port_t GetPort ()  { return port; }
  int GetFileDescriptor () { return fd; }

  void Send (int fd);

private:
  /*
   * The order of the next variables MUST be as follows -
   *   size, type, subtype, dest, buf
   *
   * 'size' MUST be first variable!
   */
  T_BYTE size;     //!< Size of the message without headers.
  T_BYTE type;     //!< Type of the message.
  T_BYTE subtype;  //!< Subtype. Might not be used.

  T_BYTE dest;     //!< Player id or all_players.

  T_BYTE buf[max_net_message_size];   //!< The message itself.

  in_addr address;
  in_port_t port;
  int fd;

  int extract_p;
};


//=========================================================================
// TNET_MESSAGE_QUEUE
//=========================================================================

/**
 *  Message queue containing network messages. This queue is used in
 *  TNET_LISTENER, TNET_TALKER, TNET_DISPATCHER.
 *
 *  @note This class is thread safe.
 */
class TNET_MESSAGE_QUEUE {
public:
  /** Exception that is thrown when there is an error creating variables needed
   *  for multi-threading support. */
  class MutexException {};

  TNET_MESSAGE_QUEUE (int size);
  ~TNET_MESSAGE_QUEUE ();

  TNET_MESSAGE *GetMessage ();
  void PutMessage (TNET_MESSAGE *message);

  void Die ();

private:
#ifdef NEW_GLFW3
	mtx_t mutex;
	cnd_t is_not_empty;
	cnd_t is_not_full;
#else
	GLFWmutex mutex;        //!< Mutex used for locking the queue.
  	GLFWcond is_not_empty;  //!< Conditional variable for signaling that the
                          //!  queue is not empty.
  	GLFWcond is_not_full;   //!< Conditional variable for signaling that the
                          //!  queue is not full.
#endif
  

  TNET_MESSAGE **message; //!< Dynamicaly created array of networking messages.
  int size;     //!< Size of the queue.
  int count;    //!< Actual count of messages in the queue.
  int head;     //!< Actual position of the head in the queue.

  bool dead;
};


//=========================================================================
// TNET_LISTENER
//=========================================================================

/**
 *  Class for listener, which is a thread receiving packets from network,
 *  unpacking them into messages and putting them into incoming message queue.
 */
class TNET_LISTENER {
public:
  class MutexException {};

  struct ACCEPT_DATA {
    ACCEPT_DATA (int fd, sockaddr_in address, TNET_LISTENER *self) {
      this->fd = fd;
      this->address = address;
      this->self = self;
    }

    int fd;
    sockaddr_in address;
    TNET_LISTENER *self;
  };

  TNET_LISTENER (int queue_size, in_port_t port);
  ~TNET_LISTENER ();

  void AddListenerByFileDescriptor (in_addr address, in_port_t port, int fd);

  /** Returns pointer to incoming message queue. */
  TNET_MESSAGE_QUEUE *GetMessageQueue ()
  { return incoming_messages; }

  /** Returns the port on which the listener listen. */
  in_port_t GetPort ()
  { return port; }

#ifdef NEW_GLFW3
	void ConsumerIsAttached (thrd_t thread);
#else
	void ConsumerIsAttached (GLFWthread thread);
#endif
  
  int GetListenersFileDescriptor (in_addr address);

  void RegisterOnDisconnect (void (* f)(in_addr, in_port_t));

private:
  static void GLFWCALL listener_thread_function (void *listener_class);
  static void GLFWCALL listener_accept (void *d);

#ifdef NEW_GLFW3
	mtx_t mutex;
	thrd_t thread;
	thrd_t consumer_thread;
#else
	GLFWthread thread;    //!< Thread id for listener's thread.
  	GLFWthread consumer_thread;
#endif
  

  std::vector<int> subthread_fd;
  std::vector<in_addr> subthread_address;
  	
#ifdef NEW_GLFW3
	std::vector<thrd_t> subthread_thread;
#else
	std::vector<GLFWthread> subthread_thread;
#endif
  

  int fd;   //!< File descriptior.
  in_port_t port;  //!< Port on which the listener is listening.
  TNET_MESSAGE_QUEUE *incoming_messages;  //!< Message queue for incoming
                                          //!  network messages.

  void (* on_disconnect)(in_addr, in_port_t);
};


//=========================================================================
// TNET_TALKER
//=========================================================================

/**
 *  Class for talker, which is a thread waiting for new messages in outgoing
 *  message queue, packing them into packets and sending them through network
 *  to remote side.
 */
class TNET_TALKER {
public:
  class MutexException {};

  TNET_TALKER (int queue_size);
  TNET_TALKER (int queue_size, in_addr remote_address, in_port_t remote_port);
  ~TNET_TALKER ();

  void RemoveAllAddresses ();

  T_BYTE AddAddress (in_addr address, in_port_t port, int fd = -1);
  T_BYTE AddEmptyAddress ();

  void DisconnectAddress (int id);
  void DisconnectAddress (in_addr address, in_port_t port);
  void RemoveAddress (int id);

  /** Returns pointer to outgoing message queue. */
  TNET_MESSAGE_QUEUE *GetMessageQueue ()
  { return outgoing_messages; }

  in_addr GetRemoteAddress (int index)
  { return remote_address[index]->GetAddress (); }

  in_port_t GetRemotePort (int index)
  { return remote_address[index]->GetPort (); }

  int GetRemoteFiledescriptor (int index)
  { return remote_address[index]->GetFileDescriptor (); }

private:
  void Initialise (int queue_size);

  static void GLFWCALL talker_thread_function (void *talker_class);

#ifdef NEW_GLFW3
	thrd_t thread;
#else
	GLFWthread thread;    //!< Thread id for talker's thread.
#endif
  

  TNET_MESSAGE_QUEUE *outgoing_messages;  //!< Message queue for outgoing
                                          //!  network messages.

  std::vector<TNET_ADDRESS *> remote_address;
  std::vector<TNET_ADDRESS *> distinct_remote_addresses;

  void RemoveAddressFromDisctinctAddresses (in_addr address, in_port_t port);
};


//=========================================================================
// TNET_MESSAGE_HANDLER
//=========================================================================

/**
 *  Message handler calls for every message the right function, which is
 *  associated with that type of the message.
 */
class TNET_MESSAGE_HANDLER {
public:
  TNET_MESSAGE_HANDLER ();

  void HandleMessage (TNET_MESSAGE *msg);

  void RegisterExtendedFunction (T_BYTE type, void (*f)(TNET_MESSAGE *msg));

private:
  /** Array of pointers to callback functions. */
  void (*extended_function[MAX_T_BYTE + 1])(TNET_MESSAGE *);
}; 


//=========================================================================
// TNET_DISPATCHER
//=========================================================================

/**
 *  Class for dispatcher, which is a separate thread grabbing messages from
 *  incoming message queue and dispatching them to mailboxes (handlers)
 *  according to type of networking messages.
 */
class TNET_DISPATCHER {
public:
  TNET_DISPATCHER (TNET_MESSAGE_QUEUE *incoming_messages,
                   TNET_MESSAGE_HANDLER *handler);
  ~TNET_DISPATCHER ();

  /** Returns pointer to incoming message queue. */
  TNET_MESSAGE_QUEUE *GetMessageQueue ()
  { return incoming_messages; }

  /** Returns pointer to message handler. */
  TNET_MESSAGE_HANDLER *GetHandler ()
  { return handler; }
/** Returns dispatcher's thread id. */
#ifdef NEW_GLFW3
	thrd_t GetThread () {
    	return thread;
  	}
#else
	GLFWthread GetThread () {
    	return thread;
  	}
#endif
  

private:
  static void GLFWCALL dispatcher_thread_function (void *dispatcher_class);

#ifdef NEW_GLFW3
	thrd_t thread;
#else
	GLFWthread thread;    //!< Thread id for dispatcher's thread.
#endif
  

  TNET_MESSAGE_QUEUE *incoming_messages;  //!< Message queue for incoming
                                          //!  network messages.
  TNET_MESSAGE_HANDLER *handler;  //!< Message handler.
};


//=========================================================================
// TNET_RESOLVER
//=========================================================================

/**
 *  Class containing methods to resolve hostnames, find out local hostname,
 *  converting network address to ASCII representation, etc.
 *
 *  @note This class is NOT thread safe.
 */
class TNET_RESOLVER {
public:
  /** Exception throwed when there was an error with resolving hostname. */
  class ResolveException {};

  TNET_RESOLVER ();

  static std::string GetHostName ();

  in_addr Resolve (std::string hostname);

  static std::string NetworkToAscii (in_addr address);
  static in_addr AsciiToNetwork (std::string address);

private:
  TLOCK *mutex;
};


//=========================================================================
// Functions
//=========================================================================

bool init_sockets (void);
void end_sockets (void);

extern TPOOL<TNET_MESSAGE> * pool_net_messages;
#endif

//=========================================================================
// END
//=========================================================================
// vim:ts=2:sw=2:et:

