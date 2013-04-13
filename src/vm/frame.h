#ifndef VM_FRAME_H
#define VM_FRAME_H

#include <thread.h>
#include <list.h>
#include "threads/synch.h"
#include "vm/swap.h"

struct frame
{
  void* v_addr;
  void* p_addr;
  struct thread *owner;
  struct list_elem frame_elem;
  char *name; //name of thread owner for debugging
};

static struct list frame_list;
static struct lock frame_lock;

void frame_init ();
void* new_frame ();
void* evict ();
bool remove_frame (void*);

#endif
