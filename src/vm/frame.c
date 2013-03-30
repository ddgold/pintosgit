#include <frame.h>
#include "threads/loader.h"
#include "threads/palloc.h"
#include <stdio.h>

uint32_t* frame_table;

void frame_table_init (void)
{
  printf("frame table init\n");
  
  uint32_t *table = palloc_get_page (PAL_USER);
  
  uint32_t *index;
  struct frame *temp = NULL;
  
  int i;
  for(i = 0; i < init_ram_pages; ++i)
  {
    memcpy(&index, &temp, sizeof (temp));
    ++index;
  }
  
  frame_table = table;
}


uint32_t* frame_table_add (struct frame* fm)
{
  uint32_t *index = frame_table;
  int i;

  for (i = 0; i < init_ram_pages; ++i)
  {

    if (*index == NULL)
    {
      index = fm;
      break;
    }
    ++index;
  }

  if (i == init_ram_pages)
  {
    // Frame Table FULL!!
    ASSERT(0);
    
  }
  
  return index;
  
}



