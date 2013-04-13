#ifndef VM_PAGE_H
#define VM_PAGE_H

#include <stdio.h>
#include <list.h>
#include "threads/thread.h"
#include "threads/synch.h"

#include "vm/swap.h"

struct page
{
  struct thread *owner;         /* The thread that owns the page */
  
  bool onDisk;                  /* TRUE if on disk, FALSE else */  
  
  void* v_addr;                 /* The virtual address of the page */
  void* p_addr;                 /* The physical address of the page */
  
  unsigned int swap_index;      /* The index for the sector containing the page */
  
  struct list_elem page_elem;   /* List element for page_list */
};

struct list page_list;
struct lock page_lock;

void page_init ();
void add_page (void*, void*);
void remove_page (void*);
struct page* get_page (void*);
void print_page ();

#endif
