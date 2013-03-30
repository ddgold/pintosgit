#include <page.h>


struct frame
{
  struct page *page;            // Holds pointer to frame's page in memory, else null
};


void frame_table_init (void);
uint32_t* frame_table_add (struct frame*);
