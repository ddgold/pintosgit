#include "vm/frame.h"
#include "threads/loader.h"
#include "threads/palloc.h"
#include <stdio.h>
#include <list.h>
#include <vm/page.h>




void frame_init (int user_page_limit)
{
    frame_left = user_page_limit;
    list_init(&frame_list);
    lock_init(&frame_lock);
}

void* frame_add (void *pg)
{
    lock_acquire(&frame_lock);
    void *fm = palloc_get_page (PAL_USER);
    if (fm == NULL)   // Frame table full, swap frame
    {
        PANIC ("NEED SWAPING!");
    }
    else              // Frane table not full, add frame
    {
        struct frame* temp;
        temp = (struct frame*) malloc (sizeof (struct frame));
        temp->v_addr = pg;
        temp->p_addr = fm;
        temp->owner = thread_current ();
        list_push_back (&frame_list, &temp->frame_elem);
        --frame_left;
    }
    lock_release(&frame_lock);
    return 0;
}
