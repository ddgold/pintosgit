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
  /*
    void *fm = palloc_get_page (PAL_USER);
    if (fm == NULL)   // Frame table full, swap frame
    {
        
    }
    else              // Frane table not full, add frame
    {
        struct frame* temp;
        temp = (struct frame*) malloc (sizeof (struct frame));
        temp->v_addr = pg;
        temp->p_addr = fm;
        temp->owner = thread_current ();
        
        --frame_left;
    }
    */
    return 0;
}
