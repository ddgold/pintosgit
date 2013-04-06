#include "vm/frame.h"
#include "threads/loader.h"
#include "threads/palloc.h"
#include <stdio.h>
#include <list.h>
#include <vm/page.h>
#include "threads/vaddr.h"
#include "threads/thread.h"



void frame_init (int num_frames)
{
  frame_left = num_frames;
  list_init(&frame_list);
  lock_init(&frame_lock);
  
  struct frame* f;
  int i;
  for (i = 0; i < num_frames; ++i)
  {
    f = (struct frame*) malloc (sizeof (struct frame));
    f->v_addr = NULL;
    f->p_addr = palloc_get_page (PAL_ZERO | PAL_USER);
    f->owner = NULL;
    list_push_back (&frame_list, &f->frame_elem);

  }
}

void* frame_add (void *pg)
{
  struct thread *t = thread_current();
  lock_acquire(&frame_lock);
    
  if (frame_left > 0)  // Frane table not full, add frame
  {
    struct list_elem* e;
    struct frame* f;
  
    for (e = list_begin (&frame_list); e != list_end (&frame_list); e = list_next (e))
    {
      f = list_entry (e, struct frame, frame_elem);
      if (f->v_addr == NULL)
      {
        f->v_addr = pg;
        f->owner = t;
        --frame_left;
        lock_release(&frame_lock);
        return f->p_addr;
      }
    }
  }
  else                  // Frame table full, swap frame
  {
    PANIC("OH SHIT!");
  }
  lock_release(&frame_lock);
  return ;  // NEEDS TO BE CHANGED!!!!!!!!
}
