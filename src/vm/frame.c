#include "vm/frame.h"
#include "vm/swap.h"
#include "vm/page.h"
#include "threads/vaddr.h"

// Initializes frame_list and frame_lock
void frame_init ()
{
  list_init(&frame_list);
  lock_init(&frame_lock);
}

// Pallocs a new user page and installs upage, or if full evitcts a frame
void* add_frame (void* upage)
{
  lock_acquire(&frame_lock);
  
  void* kpage = palloc_get_page (PAL_USER | PAL_ZERO);
  
  if (kpage == NULL)    // No free frames, so evict oldest frame
  {
    kpage = evict ();
  }
  
  struct frame* f = (struct frame*) (malloc (sizeof (struct frame)));
  f->owner = thread_current ();

  f->v_addr = upage;
  f->p_addr = kpage;

  list_push_back (&frame_list, &f->frame_elem);
  lock_release (&frame_lock);
  
  return kpage;
}

// Removes page from frame list and frees page
void remove_frame (void* kpage)
{
  lock_acquire(&frame_lock);
  struct frame* f;
  struct list_elem* e;
  for (e = list_begin (&frame_list); e != list_end (&frame_list); e = list_next (e))
  {
    f = list_entry (e, struct frame, frame_elem);
    
    if (f->p_addr == kpage)
    {
      //printf("Remove Frame, Owner = %s, p_addr = %x, v_addr = %x\n", f->owner->name, f->v_addr, f->p_addr);
      
      pagedir_clear_page (f->owner->pagedir, f->v_addr);
      list_remove (e);
      break;
    }
  }
  lock_release (&frame_lock);
}

// Finds oldest frame and evicts it
void* evict ()
{
  struct list_elem* e = list_pop_front (&frame_list);
  struct frame *f = list_entry (e, struct frame, frame_elem);
  struct page *p = get_page (f->v_addr);
  
  void* kpage = p->p_addr;
  
  // Set onDisk and unmap p_addr
  p->onDisk = 1;
  p->p_addr = NULL;
  p->swap_index = get_sector ();
  
  // Write page to disk
  swap_out (p->swap_index, p->v_addr);
  
  // Free frame
  palloc_free_page(kpage);
  pagedir_clear_page (f->owner->pagedir, f->v_addr);
  list_remove (e);
  
  return palloc_get_page (PAL_USER | PAL_ZERO);
}

























