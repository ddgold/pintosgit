#include "vm/page.h"



void page_init ()
{
    list_init(&page_list);
    lock_init(&page_lock);
}

void* sup_page_add (void* v_addr)
{
    struct page* temp;
    temp = (struct page*) malloc (sizeof (struct page));
    temp->v_addr = v_addr;
    temp->owner = thread_current ();
    list_push_back (&page_list, &temp->page_elem);
}
