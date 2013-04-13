#ifndef VM_SWAP_H
#define VM_SWAP_H

#include <stdio.h>
#include <bitmap.h>
#include "devices/block.h"
#include "threads/synch.h"

struct block *swap_block;
struct bitmap *swap_bitmap;
struct lock swap_lock;

void swap_init ();
int get_sector ();
void swap_out (int, void*);
void swap_in (int, void*);
void swap_remove (int);

#endif
