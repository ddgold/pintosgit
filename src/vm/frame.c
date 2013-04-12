#include "vm/frame.h"
#include "threads/loader.h"
#include "threads/palloc.h"
#include <stdio.h>
#include <list.h>
#include <vm/page.h>
#include "threads/vaddr.h"
#include "threads/thread.h"
#include "devices/block.h"


void frame_init ()
{
  list_init(&frame_list);
  lock_init(&frame_lock);
}

void* new_frame ()
{
  lock_acquire(&frame_lock);
  void* new_frame = palloc_get_page (PAL_USER | PAL_ZERO);
  
  if (new_frame != NULL)
  {
    sup_page_add (new_frame);
    
    struct frame* f;
    f->v_addr = new_frame;
    f->p_addr = new_frame;
    f->owner = thread_current ();
    list_push_back (&frame_list, &f->frame_elem);
    
    //printf("Add Frame, Owner = %s, p_addr = %x, v_addr = %x\n", f->owner->name, f->v_addr, f->p_addr);
    
    lock_release(&frame_lock);
    return f->p_addr;
  }
  else
  {
    evict ();
    PANIC ("Evict frame");
    
  }
  
  lock_release(&frame_lock);
  return ;  // NEEDS TO BE CHANGED!!!!!!!!
}

bool evict ()
{
  struct list_elem *e = list_back (&frame_list);
  struct frame *oldest_frame = list_entry (e, struct frame, frame_elem);
  
   
  struct page *pg = sup_page_search (oldest_frame->v_addr);
  
  struct block *bk = block_get_role (BLOCK_SWAP); // Change to get from swap table
  block_write (bk, 0, pg->v_addr);
  
  list_remove (e);
  
  palloc_free_page (pg->v_addr);
  pg->onDisk = 1;
  pg->v_addr = NULL;
  pg->p_addr = NULL;
  
  return 1;
}

bool remove_frame (void* pg)
{
  struct list_elem* e;
  struct frame* f;

  for (e = list_begin (&frame_list); e != list_end (&frame_list); e = list_next (e))
  {
    f = list_entry (e, struct frame, frame_elem);
    
    
    if (f->v_addr == pg)
    {
      list_remove (e);
      remove_sup_table (pg);
      palloc_free_page (f->v_addr);
      break;
    }
  }
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
