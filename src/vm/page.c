#include "vm/page.h"

// Initializes page_list and page_lock
void page_init ()
{
  list_init (&page_list);
  lock_init (&page_lock);
}

// Add page to page list mapping upage to kpage
void add_page (void* kpage, void* upage)
{
  lock_acquire(&page_lock);
  struct page* p = (struct page*) (malloc (sizeof (struct page)));
  p->owner = thread_current ();
  
  p->onDisk = 0;
  
  p->v_addr = upage;
  p->p_addr = kpage;
  
  p->swap_index = -1;
  
  list_push_back (&page_list, &p->page_elem);
  lock_release (&page_lock);
}

// Removes page from page list
void remove_page (void* upage)
{
  lock_acquire(&page_lock);
  struct page* p;
  struct list_elem* e;
  for (e = list_begin (&page_list); e != list_end (&page_list); e = list_next (e))
  {
    p = list_entry (e, struct page, page_elem);
    
    if (p->v_addr == upage)
    {
      list_remove (e);
      if (p->onDisk)
      {
        swap_remove (p->swap_index);
      }
      break;
    }
  }
  lock_release (&page_lock);
}

// Returns the page that contains upage
struct page* get_page (void* upage)
{
  lock_acquire(&page_lock);
  struct page* p;
  struct list_elem* e;
  for (e = list_begin (&page_list); e != list_end (&page_list); e = list_next (e))
  {
    p = list_entry (e, struct page, page_elem);
    
    if (p->v_addr == upage)
    {
      lock_release (&page_lock);
      return p;
    }
  }
  lock_release (&page_lock);
  return NULL;
}

void print_page ()
{
  lock_acquire(&page_lock);
  struct page* p;
  struct list_elem* e;
  int count = 0;
  int mem = 0;
  int disk = 0;
  for (e = list_begin (&page_list); e != list_end (&page_list); e = list_next (e))
  {
    
    p = list_entry (e, struct page, page_elem);
    printf ("Owner: %s, onDisk %d, v_addr: %x, p_addr %x, swap_index %d\n", 
          p->owner->name, p->onDisk, p->v_addr, p->p_addr, p->swap_index );
    ++count;
    if (p->onDisk)
      ++disk;
    else
      ++mem;
    
  }
  printf ("Page Count: %d, Memory: %d, Disk: %d\n", count, mem, disk);
  lock_release (&page_lock);
}
