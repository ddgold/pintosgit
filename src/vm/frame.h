#ifndef VM_FRAME_H
#define VM_FRAME_H

#include <thread.h>
#include <list.h>
#include "threads/synch.h"
#include "threads/palloc.h"

struct frame
{
  struct thread *owner;         /* The frames current owner */
  
  void* v_addr;                 /* The virtual address of the frame */
  void* p_addr;                 /* The physical address of the frame */
  
  struct list_elem frame_elem;  /* List element for frame_list */
};

struct list frame_list;
struct lock frame_lock;

void frame_init ();
void* add_frame (void*);
void remove_frame (void*);
void* evict ();
#endif
