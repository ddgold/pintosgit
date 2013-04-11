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

void* sup_page_add (struct page *pg)
{
    
    //pg = (struct page*) malloc (sizeof (struct page));
    pg->owner = thread_current ();
    //printf("%s is adding: %d\n", (thread_current())->name, pg->pte_num);
    list_push_back (&page_list, &pg->page_elem);
}

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
/*
void* sup_page_search (void *fault_addr)
{
   struct list_elem *e;
   for (e = list_begin (&page_list); e != list_end (&page_list);
       e = list_next (e))
    {
      struct page *p = list_entry (e, struct page, page_elem);

      if(p->v_addr - fault_addr < PGSIZE)
      {
        printf("phy: %x - vir: %x - fa: %x\n", p->p_addr, p->v_addr, fault_addr);
        void *phys = frame_add(p->v_addr);
        printf("PHYS: %x\n", phys);
        printf("pte: %d\n", p->pte_num);
        int *temp = pte_get_page(p->pte_num);
        printf("TEMP: %x\n", temp);
        temp = phys;
        printf("PTE_GET_PAGE: %x\n", pte_get_page(p->pte_num));
        return temp;
      }
      
    }
}
*/
