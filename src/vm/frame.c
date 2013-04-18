#include "vm/frame.h"
#include "threads/loader.h"
#include "threads/palloc.h"
#include <stdio.h>
#include <list.h>
#include <vm/page.h>
//uint32_t* frame_table;



void frame_table_init (void)
{
  list_init(&frame_table);
  frame_table_left = init_ram_pages;
  int i;
  for(i = 0; i < ram_init_pages; ++i)
  {
    struct frame *fm;
    void *index = palloc_get_page(PAL_USER);//allocating a page in PHY_MEM
    fm->p_addr = &index; //Sets the physical address of the frame in the table
    fm->isMapped = 0;    //initially the frame is unmapped
    list_push_back(&frame_table, &fm->frame_elem);
  }
}

uint32_t* frame_table_add (struct page *pg)
{
  if (frame_table_left > 0)
  {
    struct frame *fm;
    fm->p_addr = memcpy(&index, &pg, sizeof(struct page));
    fm->v_addr = pg;
    list_push_back(&frame_table, &fm->frame_elem);
    frame_table_left--;
  }
  else
  {
    PANIC("frame table full.");
  }
  return 0;
}
