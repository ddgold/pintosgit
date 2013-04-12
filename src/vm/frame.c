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
    
    struct frame* f = (struct frame*) (malloc (sizeof(struct frame)));
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
    PANIC ("Evict this shit");
  }
  
  lock_release(&frame_lock);
  return ;  // NEEDS TO BE CHANGED!!!!!!!!
}

bool evict ()
{
  struct list_elem *e = list_front (&frame_list);
  struct frame *oldest_frame = list_entry (e, struct frame, frame_elem);
  struct page *pg = sup_page_search (oldest_frame->v_addr);
  
  sup_page_search (0);
  printf("\n\n\n\n\n\n");
  
  int index = swap_index ();
  struct block *swap_block = block_get_role (BLOCK_SWAP);
  
  int i;
  void* temp_addr;
  for (i = 0; i < 8; ++i)
  {
    temp_addr = pg->v_addr;
    block_write (swap_block, (index * 8) + i, temp_addr);
    temp_addr += (PGSIZE / 8);
  }
  
  list_remove (e);
  
  palloc_free_page (pg->v_addr);
  pg->onDisk = 1;
  pg->sector_index = index;
  pg->v_addr = NULL;
  pg->p_addr = NULL;
  
  sup_page_search (0);
  printf("\n\n\n\n\n\n");
  
  return 1;
}

bool remove_frame (void* pg)
{
  lock_acquire (&frame_lock);
  
  struct list_elem* e;
  struct frame* f;
  for (e = list_begin (&frame_list); e != list_end (&frame_list); e = list_next (e))
  {
    f = list_entry (e, struct frame, frame_elem);
    
    if (f->v_addr == pg)
    {
      //printf("Remove Frame, Owner = %s, p_addr = %x, v_addr = %x\n", f->owner->name, f->v_addr, f->p_addr);
      list_remove (e);
      lock_release (&frame_lock);
      return 1;
    }
  }
  lock_release (&frame_lock);
}
