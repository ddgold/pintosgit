#include "swap.h"
#include <stdio.h>

void swap_init ()
{
  struct block *swap_block = block_get_role (BLOCK_SWAP);
  swap_bitmap = bitmap_create ((block_size (swap_block))/8);
  lock_init (&swap_lock);
}

int swap_index ()
{
  lock_acquire (&swap_lock);
  int index = BITMAP_ERROR;  
  int i;

  for (i = 0; i < bitmap_size (swap_bitmap); ++i)
  {
    if (bitmap_test (swap_bitmap, i) == 0)
    {
      index = i;
      break;
    }
  }
  
  if (index == BITMAP_ERROR)
  {
    PANIC ("Swap Block Full! %d", index);
  }
  
  bitmap_set (swap_bitmap, index, 1);
  lock_release (&swap_lock);
  return index;  
}

struct bitmap* get_bitmap ()
{
  return swap_bitmap;
}
