#include "vm/frame.h"
#include "threads/loader.h"
#include "threads/palloc.h"
#include <stdio.h>
#include <list.h>
#include <vm/page.h>




void frame_table_init (void)
{
  frame_table_left = 20;
  int i;
  struct frame fm;
  for(i = 0; i < 20; ++i)
  {
    void *index = palloc_get_page(PAL_USER);//allocating a page in PHY_MEM
    fm.p_addr = &index; //Sets the physical address of the frame in the table
    fm.isMapped = 0;    //initially the frame is unmapped
    struct frame *fm_ptr = &fm; //sets the virtual address in the frame table
    list_push_back(&frame_table, &fm_ptr->frame_elem); 
  }
}

/*

*/

uint32_t* frame_table_add (struct page *pg)
{
  if (frame_table_left > 0)
  {
    struct list_elem *e;
    struct frame *fm;
    
    for(e = list_begin(&frame_table); e != list_end(&frame_table);
        e = list_next(e))
    {
      //Get a frame from the frame table
      fm = list_entry(e, struct frame, frame_elem);
      //finds the first unmapped frame in the table
      if(fm->isMapped == 0)
      {
        //Copy the page into physical memory
        memcpy(&fm->p_addr, &pg, sizeof(struct page));
        //Set the mapped bit
        fm->isMapped = 1;
        //Decrement the amount of open pages left count
        --frame_table_left;
      }
    }
  }
  else
  {
    //PANIC("frame table full.");
  }
  return 0;
}
