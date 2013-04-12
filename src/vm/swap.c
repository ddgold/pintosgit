#include "swap.h"

void swap_init ()
{
  lock_init(&swap_lock);
  list_init(&swap_list);
}


