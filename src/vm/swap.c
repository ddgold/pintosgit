#include "swap.h"
#include "threads/vaddr.h"
// Initializes swap_block, swap_bitmap, and swap_lock
void swap_init ()
{
  swap_block = block_get_role (BLOCK_SWAP);
  swap_bitmap = bitmap_create (block_size (swap_block) / 8);
  lock_init (&swap_lock);
}

// Returns first free sector
int get_sector ()
{
  lock_acquire (&swap_lock);
  int index = BITMAP_ERROR;  
  
  // Loop through swap_bitmap to find first free sector
  int i;
  for (i = 0; i < bitmap_size (swap_bitmap); ++i)
  {
    if (!bitmap_test (swap_bitmap, i))
    {
      index = i;
      break;
    }
  }
  
  // Panic if no free sector found
  if (index == BITMAP_ERROR)
  {
    print_page ();
    PANIC ("Swap Block Full!");
  }
  
  lock_release (&swap_lock);
  return index;  
}

// Writes from page to the sector at index
void swap_out (int index, void* page)
{
  lock_acquire (&swap_lock);
  ASSERT (!bitmap_test (swap_bitmap, index));
  
  int i;
  for (i = 0; i < 8; ++i)
  {
    block_write (swap_block, (index * 8) + i, page + (i * BLOCK_SECTOR_SIZE));
  }
  
  bitmap_set (swap_bitmap, index, 1);
  lock_release (&swap_lock);
}

// Writes from the sector at index to page
void swap_in (int index, void* page)
{
  lock_acquire (&swap_lock);
  ASSERT (bitmap_test (swap_bitmap, index));
  
  int i;
  for (i = 0 ; i < 8; ++i)
  {
    block_read (swap_block, (index * 8) + i, page + (i * BLOCK_SECTOR_SIZE));
  }
  
  bitmap_set (swap_bitmap, index, 0);
  lock_release (&swap_lock);
}

// Clears the sector at index
void swap_remove (int index)
{
  bitmap_set (swap_bitmap, index, 0);
}
