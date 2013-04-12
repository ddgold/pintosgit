#include "devices/block.h"
#include "lib/kernel/bitmap.h"
#include <list.h>
#include "threads/synch.h"

static struct bitmap *swap_bitmap;
static struct lock swap_lock;
//static struct block *swap_block;
