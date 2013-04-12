#ifndef VM_PAGE_H
#define VM_PAGE_H

#include <thread.h>
#include <list.h>
#include "threads/synch.h"

struct page
{
  char *name;               /*holds name for debuggin purposes*/
  struct thread *owner;     /*the thread that owns the page */
  
  bool onDisk;              /* TRUE if on disl, FALSE else */  
  
  void* v_addr;             /*holds the virtual address of the page */
  void* p_addr;             /*the page's physical address*/
  uint32_t pte_num;         /*the pte for the page */
  
  struct list_elem page_elem;
  struct file *file;        /* holds the executable file for that page */
};

static struct list page_list;
static struct lock page_lock;

void frame_init ();
void* sup_page_add (void*);
bool remove_sup_table (void*);
struct page* sup_page_search (void*);
#endif
