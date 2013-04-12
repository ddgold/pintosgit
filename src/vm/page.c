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
    list_init(&page_list);
    lock_init(&page_lock);
}

void* sup_page_add (void* pg)
{
    struct page *new_page = (struct page*) (malloc (sizeof(struct page)));
    new_page->owner = thread_current ();
    new_page->onDisk = 0;
    new_page->v_addr = pg;
    new_page->p_addr = pg;
    
    //printf("Add Page, Owner = %s, p_addr = %x, v_addr = %x\n", new_page->owner->name, new_page->v_addr, new_page->p_addr);
    
    list_push_back (&page_list, &new_page->page_elem);
}

bool remove_sup_table (void* pg)
{
  struct list_elem* e;
  struct page* p;
  
  
  for (e = list_begin (&page_list); e != list_end (&page_list); e = list_next (e))
  {
    p = list_entry (e, struct page, page_elem);
    
    if (p->v_addr == pg)
    {
      list_remove (e);
      break;
    }
  }
}

struct page* sup_page_search (void *v_addr)
{
  struct list_elem *e;
  struct page *p;
  for (e = list_begin (&page_list); e != list_end (&page_list); e = list_next (e))
  {
    p = list_entry (e, struct page, page_elem);
    //printf("Sup_Page: onDisk: %d, v_addr: %x, p_addr: %x\n", p->onDisk, p->v_addr, p->p_addr);
    
    if(p->v_addr == v_addr)
      {
        return p;
      }
  }
  return NULL;
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
