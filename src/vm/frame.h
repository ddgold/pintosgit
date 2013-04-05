#ifndef VM_FRAME_H
#define VM_FRAME_H

#include <thread.h>
#include <list.h>

struct frame
{
  void* v_addr;
  void* p_addr;
  struct thread *owner;
  struct list_elem frame_elem;
};

static struct list *frame_list;
static struct lock *frame_lock;
static int frame_left;

void frame_init (int);
void* frame_add (void*);

#endif
