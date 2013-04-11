#ifndef VM_PAGE_H
#define VM_PAGE_H

#include <thread.h>
#include <list.h>
#include "threads/synch.h"

struct page
{
  char *name;               /*holds name for debuggin purposes*/
  struct thread *owner;     /*the thread that owns the page */
  void* v_addr;             /*holds the virtual address of the page */
  void* p_addr;             /*the page's physical address*/
  struct list_elem page_elem;
  uint32_t pte_num;         /*the pte for the page */
  struct file *file;        /* holds the executable file for that page */
};

static struct list page_list;
static struct lock page_lock;

void frame_init ();
void* sup_page_add (struct page*);
#endif
