#include "vm/page.h"
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include "threads/init.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "threads/pte.h"

void page_init ()
{
  list_init (&page_list);
  lock_init (&page_lock);
}

void* sup_page_add (void* pg)
{
  struct page *new_page = (struct page*) (malloc (sizeof(struct page)));
  new_page->owner = thread_current ();
  new_page->onDisk = 0;
  new_page->v_addr = pg;
  new_page->p_addr = pg;
  new_page->sector_index = -1;
  
  //printf("Add Page, Owner = %s, p_addr = %x, v_addr = %x\n", new_page->owner->name, new_page->v_addr, new_page->p_addr);
  
  list_push_back (&page_list, &new_page->page_elem);
}

bool remove_sup_table (void* pg, bool inExit)
{
  if (!inExit)
  {
    lock_acquire (&page_lock);
  }
  struct list_elem* e;
  struct page* p;
  
  
  for (e = list_begin (&page_list); e != list_end (&page_list); e = list_next (e))
  {
    p = list_entry (e, struct page, page_elem);
    
    if (p->v_addr == pg)
    {
      //printf("Remove spe, Owner = %s, p_addr = %x, sector_index = %d", p->owner->name, p->p_addr, p->sector_index);
      if (p->onDisk)
      {
        PANIC ("REMOVE FROM DISK");
      }
      else
      {
        remove_frame (p->v_addr);
        palloc_free_page (p->v_addr);
      }
      list_remove (e);
      if (!inExit)
      {
        lock_release (&page_lock);
      }
      return 1;
    }
  }
  if (!inExit)
  {
    lock_release (&page_lock);
  }
}

struct page* sup_page_search (void *v_addr)
{
  lock_acquire (&page_lock);
  struct list_elem *e;
  struct page *p;
  for (e = list_begin (&page_list); e != list_end (&page_list); e = list_next (e))
  {
    p = list_entry (e, struct page, page_elem);
    
    printf ("Owner: %s, OnDisk %d, v_addr %x, sector_index %d\n", p->owner->name, p->onDisk, p->v_addr, p->sector_index);
    if(p->v_addr == v_addr)
    {
      lock_release (&page_lock);
      return p;
    }
  }
  lock_release (&page_lock);
  return NULL;
}

void exit_sup_table ()
{
  lock_acquire (&page_lock);
  
  struct list_elem *e;
  struct page *p;
  char * thread_name = thread_current()->name;
  
  for (e = list_front (&page_list); e != list_end (&page_list); e = list_next (e))
  {
    p = list_entry (e, struct page, page_elem);
    
    if (p->owner->name == thread_name)
    {
      if (p->onDisk)
      {
        struct bitmap *swap_bitmap = get_bitmap ();
        //printf("Size: %d  Index %d\n", bitmap_size (swap_bitmap), p->sector_index);
        bitmap_set_multiple (swap_bitmap, p->sector_index, 7, 0);
      }
      else
      {
        remove_frame (p->v_addr);
      }
      list_remove (e);
    }
  }
  
  lock_release (&page_lock);
}


/*
void print_list ()
{
    struct list_elem *e;
    int i = 1;
    for (e = list_begin (&page_list); e != list_end (&page_list);
       e = list_next (e))
    {
      struct page *p = list_entry (e, struct page, page_elem);
      printf("pte %d: v:%x - p:%x\n", i, p->v_addr, p->p_addr);
      i++;
    }
}


*/
