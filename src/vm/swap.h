#include "devices/block.h"
#include <list.h>
#include "threads/synch.h"

static struct list *swap_list;
static struct lock *swap_lock;
