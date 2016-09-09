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
 *  @file doalloc.cpp
 *
 *  
 *
 *  @author Peter Knut
 *
 *  @date 2005
 */

#include "doalloc.h"

#if DEBUG_MEMORY

#include "dologs.h"
#include <glfw.h>
#include <crtdbg.h>
#include <malloc.h>


bool alloc_logging = false;


//========================================================================
// Structures
//========================================================================

/**
 *  Stores information about one allocated block
 */
struct TMEMORY_BLOCK {
  void *address;          //!< Block address in memory.
  size_t size;            //!< Size of allocated block.
  const char *file;       //!< File from which was block allocated.
  int line;               //!< Line from which was block allocated.

  TMEMORY_BLOCK *next;    //!< Pointer to next block.
  TMEMORY_BLOCK *prev;    //!< Pointer to previous block.
};


//========================================================================
// Variables
//========================================================================

TMEMORY_BLOCK *memory_list = NULL;  //!< Head of blocks list.
TMEMORY_BLOCK *memory_back = NULL;  //!< Back of block list.

bool check_blocks = false;          //!< Whether checking is initialized.
unsigned total_count = 0;           //!< Total count of allocated blocks.
size_t total_size = 0;              //!< Total size of allocated blocks.

GLFWmutex mutex = NULL;


//========================================================================
// Methods
//========================================================================

/**
 *  Check undeleted blocks and write info to log files.
 */
void CheckMemory(void)
{
  TMEMORY_BLOCK *iter;
  unsigned count = 0;
  size_t size = 0;

  glfwLockMutex(mutex);

  Debug("*** MEMORY BEGIN ***");

  for (iter = memory_list; iter; iter = iter->next) {
    if (iter->file)
      Debug(LogMsg("Undeleted block: %8d   %s:%d", iter->size, iter->file, iter->line));
    else
      Debug(LogMsg("Undeleted block: %8d", iter->size));

    count++;
    size += iter->size;
  }

  while (memory_list) {
    iter = memory_list;
    memory_list = memory_list->next;
    free(iter);
  }

  memory_list = memory_back = NULL;

  Debug("*** MEMORY END ***");
  Debug(LogMsg("*** Total blocks:     %d", total_count));
  Debug(LogMsg("*** Undeleted blocks: %d (%.2lf%%)", count, ((double)count / total_count) * 100));
  Debug(LogMsg("*** Total size:       %d", total_size));
  Debug(LogMsg("*** Undeleted size:   %d (%.2lf%%)", size, ((double)size / total_size) * 100));

  glfwUnlockMutex(mutex);
}


/**
 *  Initialize memory system.
 */
void InitMemorySestem(void)
{
  mutex = glfwCreateMutex();

  check_blocks = true;
}


/**
 *  Finishes memory system and checks undeleted blocks.
 */
void DoneMemorySystem(void)
{
  check_blocks = false;

  CheckMemory();
  glfwDestroyMutex(mutex);
}


/**
 *  Set logging of memory allocating.
 */
void SetAllocLogging(bool log)
{
  alloc_logging = log;
}


//========================================================================
// Allocating memory blocks
//========================================================================

/**
 *  Allocates new memory block.
 */
inline
void *NewBlock(size_t size, const char *file = NULL, int line = 0)
{
  void* res;

  if (file)
    res = _malloc_dbg(size, _NORMAL_BLOCK, file, line);
  else
    res = _malloc_dbg(size, _NORMAL_BLOCK, __FILE__, __LINE__);

  if (alloc_logging) {
    if (file)
      Debug(LogMsg("New block: %8d   %s:%d", size, file, line));
    else
      Debug(LogMsg("New block: %8d", size));
  }

  if (!check_blocks) return res;

  TMEMORY_BLOCK *block = (TMEMORY_BLOCK *)malloc(sizeof TMEMORY_BLOCK);

  block->address = res;
  block->size = size;
  block->file = file;
  block->line = line;
  block->next = block->prev = NULL;

  glfwLockMutex(mutex);

  total_count++;
  total_size += size;

  if (memory_back) {
    memory_back->next = block;
    block->prev = memory_back;
    memory_back = block;
  }

  else {
    memory_list = memory_back = block;
  }

  glfwUnlockMutex(mutex);

  return res;
}


/**
 *  Deletes memory block.
 */
inline
void DeleteBlock(void *address, const char* file = NULL, int line = 0)
{
  if (address == NULL) {
    if (file) Debug(LogMsg("Deleting NULL pointer: %s:%d", file, line));
    else Debug("Deleting NULL pointer");

    return;
  }

  _free_dbg(address, _NORMAL_BLOCK);

  if (!check_blocks) return;

  TMEMORY_BLOCK *iter;

  glfwLockMutex(mutex);

  for (iter = memory_list; iter; iter = iter->next) {
    if (iter->address == address) break;
  }

  if (iter) {
    if (iter == memory_list) memory_list = iter->next;
    if (iter == memory_back) memory_back = iter->prev;

    if (iter->next) iter->next->prev = iter->prev;
    if (iter->prev) iter->prev->next = iter->next;

    //memset(iter->address, 0, iter->size);

    free(iter);
  }

  else {
    if (file) Debug(LogMsg("Missing block: %8lx   %s:%d", address, file, line));
    else Debug(LogMsg("Missing block: %lx", address));
  }

  glfwUnlockMutex(mutex);
}


//========================================================================
// Replacements of global new & delete
//========================================================================

/**
 *  Replacement global new operator without location reporting.
 *  This catches calls which don't use NEW for some reason.
 */
void* operator new(size_t size)
{
    return NewBlock(size);
}


/**
 *  from n_new()).
 */
void* operator new(size_t size, const char* file, int line)
{
    return NewBlock(size, file, line);
}


/**
 *  Replacement global new[] operator without location reporting.
 */
void* operator new[](size_t size)
{
    return NewBlock(size);
}


/**
 *  Replacement global new[] operator with location reporting.
 */
void* operator new[](size_t size, const char* file, int line)
{
    return NewBlock(size, file, line);
}


/**
 *  Replacement global delete operator.
 */
void operator delete(void* p)
{
  DeleteBlock(p);
}


/**
 *  Replacement global delete operator to match the new with location reporting.
 */
void operator delete(void* p, const char* file, int line)
{
  DeleteBlock(p, file, line);
}


/**
 *  Replacement global delete[] operator.
 */
void operator delete[](void* p)
{
  DeleteBlock(p);
}


/**
 * Replacement global delete[] operator to match the new with location reporting.
 */
void operator delete[](void* p, const char* file, int line)
{
  DeleteBlock(p, file, line);
}


#endif

//=========================================================================
// END
//=========================================================================
// vim:ts=2:sw=2:et:

