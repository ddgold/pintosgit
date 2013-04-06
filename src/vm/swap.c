

void swap_init ()
{
  &swap_block->BLOCK_SWAP;
  lock_init(&swap_lock);
  list_init(&swap_list);
}

bool evict ()
{
  
}
