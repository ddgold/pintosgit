#ifndef VM_FRAME_H
#define VM_FRAME_H

#include <vm/page.h>
#include <thread.h>
#include <list.h>

struct frame
{
  uint32_t v_addr;
  uint32_t p_addr;
  int isMapped;
  struct thread *owner;
  struct list_elem *frame_elem;
};

static struct list *frame_table;
static int frame_table_left;

void frame_table_init (void);
uint32_t* frame_table_add (struct page *);

#endif
