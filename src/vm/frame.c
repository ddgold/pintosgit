#include "vm/frame.h"
#include "threads/loader.h"
#include "threads/palloc.h"
#include <stdio.h>
#include <list.h>
#include <vm/page.h>
#include "threads/vaddr.h"
#include "threads/thread.h"


void frame_init ()
{
  list_init(&frame_list);
  lock_init(&frame_lock);
}

void* new_frame ()
{
  lock_acquire(&frame_lock);
  void* new_frame = palloc_get_page (PAL_USER);
  
  if (new_frame != NULL)
  {
    sup_page_add (new_frame);
    
    struct frame* f;
    f->v_addr = new_frame;
    f->p_addr = new_frame;
    f->owner = thread_current ();
    list_push_back (&frame_list, &f->frame_elem);
    
    lock_release(&frame_lock);
    return f->p_addr;
  }
  else
  {
    PANIC ("Evict frame");
    
  }
  
  lock_release(&frame_lock);
  return ;  // NEEDS TO BE CHANGED!!!!!!!!
}

/*
void* get_frame (void *pg)
{
  lock_acquire(&frame_lock);
  void* new_frame = palloc_get_page (PAL_USER);
  
  if (new_frame != NULL)
  {
    struct frame* f;
    f->v_addr = pg;
    f->p_addr = new_frame;
    f->owner = thread_current ();
    list_push_back (&frame_list, &f->frame_elem);
    
    lock_release(&frame_lock);
    return f->p_addr;
  }
  else
  {
    // Evict frame
    
    get_frame (pg);
  }
  lock_release(&frame_lock);
  return ;  // NEEDS TO BE CHANGED!!!!!!!!
}


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
    f->p_addr = palloc_get_page (PAL_USER);
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
        memcpy(f->p_addr, f->v_addr, PGSIZE);
        lock_release(&frame_lock);
        printf(" f->p_addr: %x\n",  f->p_addr);
        return f->p_addr;
      }
    }
  }
  else // Frame table full, swap frame
  {
    //Swap here.
  }
  lock_release(&frame_lock);
  return ;  // NEEDS TO BE CHANGED!!!!!!!!
}

*/
