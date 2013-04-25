#include "filesys/inode.h"
#include <list.h>
#include <debug.h>
#include <round.h>
#include <string.h>
#include "filesys/filesys.h"
#include "filesys/free-map.h"
#include "threads/malloc.h"

/* Identifies an inode. */
#define INODE_MAGIC 0x494e4f44




/* Returns the number of sectors to allocate for an inode SIZE
   bytes long. */
static inline size_t
bytes_to_sectors (off_t size)
{
  return DIV_ROUND_UP (size, BLOCK_SECTOR_SIZE);
}



/* Returns the block device sector that contains byte offset POS
   within INODE.
   Returns -1 if INODE does not contain data for a byte at offset
   POS. */
static block_sector_t
byte_to_sector (const struct inode *inode, off_t pos) 
{
  ASSERT (inode != NULL);
  
  block_sector_t sector_num = (pos / BLOCK_SECTOR_SIZE);
 
  if (sector_num < 5)
  {
    return inode->data.data_blocks[sector_num];
  }
  else if (sector_num < 6005)
  {
    block_sector_t block_num = (sector_num - 5) / 100;
    block_sector_t block_index =  (sector_num - 5) % 100;
    
    if (inode->data.indirect_blocks[block_num] == -1)
    {
      return -1;
    }
    else
    {
      block_sector_t* buffer = calloc (1, sizeof (struct indirect_block));
      block_read (fs_device, inode->data.indirect_blocks[block_num], buffer);
      return buffer[block_index];          
    }
  }
  else if (sector_num < 16005)
  {
    PANIC ("Need to implement double indirect block");
    return -1;
  }
  else
  {
    return -1;
  }
}

/* List of open inodes, so that opening a single inode twice
   returns the same `struct inode'. */
static struct list open_inodes;

/* Initializes the inode module. */
void
inode_init (void) 
{
  list_init (&open_inodes);
  lock_init (&inode_lock);
}


bool add_sector(struct inode_disk *disk_inode)
  {
    lock_acquire (&inode_lock);
    static char zeros [BLOCK_SECTOR_SIZE];
    block_sector_t sector;
    if (!free_map_allocate(1, &sector))
    {
      lock_release (&inode_lock);
      return false;
    }
    
     //printf ("add_sector: %d\n", sector);

    block_write (fs_device, sector, zeros);
    
    if (disk_inode->sectors < 5)
    {
      disk_inode->data_blocks[disk_inode->sectors] = sector;
    }
    else if (disk_inode->sectors < 6005)
    {
      block_sector_t block_num = (disk_inode->sectors - 5) / 100;
      block_sector_t block_index =  (disk_inode->sectors - 5) % 100;
      
      if (block_index == 0)
      {
        // New Indirect Block
        struct indirect_block *i_block = calloc (1, sizeof (struct indirect_block));
        
        block_sector_t i_block_sector;
        if (!free_map_allocate(1, &i_block_sector))
        {
          lock_release (&inode_lock);
          return false;
        }
        
        int i;
        for (i = 0; i < 100; ++i)
        {
          i_block->data_blocks[i] = -1;
        }

        block_write (fs_device, i_block_sector, i_block);
        disk_inode->indirect_blocks[block_num] = i_block_sector;
      }
      
      // Follow Indirect Block
      block_sector_t* buffer = calloc (1, sizeof (struct indirect_block));
         
      block_read (fs_device, disk_inode->indirect_blocks[block_num], buffer);
      buffer[block_index] = sector;    
      block_write (fs_device, disk_inode->indirect_blocks[block_num], buffer);

    }
    else if (disk_inode->sectors < 16005)
    {
      PANIC ("NEEDS DOUBLE INDIRECT BLOCK");
    }
    else
    {
      PANIC ("File too large...");
      lock_release (&inode_lock);
      return false;
    }
    
    ++disk_inode->sectors;
    
    lock_release (&inode_lock);
    return true;
  }


bool remove_sectors (struct inode_disk *disk_inode)
  {
    lock_acquire(&inode_lock);
    if(disk_inode->sectors > 6005)
      {
        PANIC("Too big...");
      }
    if (disk_inode->sectors > 5)
      {
        block_sector_t block_num = (disk_inode->sectors - 5) / 100;
        block_sector_t block_index =  (disk_inode->sectors - 5) % 100;
        
        int n = block_num - 1;
        for(n; n >= 0; --n)
        {
          block_sector_t* buffer;
          block_read (fs_device, disk_inode->indirect_blocks[block_num], buffer);
         
          int i = block_index - 1;
          for(i; i >= 0; --i)
          {
            --disk_inode->sectors;
            free_map_release(buffer[i], 1);
          }
          
          block_index = 100;
          free_map_release(disk_inode->indirect_blocks[n], 1);
        }
      
      }
    if (disk_inode->sectors >= 0)
    {
      int i = disk_inode->sectors - 1;
      for(i; i >= 0; --i)
      {
        --disk_inode->sectors;
        free_map_release(disk_inode->data_blocks[i], 1);
      }
    }
    
    lock_release(&inode_lock);
    ASSERT(disk_inode->sectors == 0);
  }


/* Initializes an inode with LENGTH bytes of data and
   writes the new inode to sector SECTOR on the file system
   device.
   Returns true if successful.
   Returns false if memory or disk allocation fails. */
   
bool
inode_create (block_sector_t sector, off_t length)
{
  struct inode_disk *disk_inode = NULL;
  bool success = false;

  ASSERT (length >= 0);

  /* If this assertion fails, the inode structure is not exactly
     one sector in size, and you should fix that. */
  ASSERT (sizeof *disk_inode == BLOCK_SECTOR_SIZE);

  disk_inode = calloc (1, sizeof *disk_inode);
  if (disk_inode != NULL)
    {
      //size_t sectors = bytes_to_sectors (length);
      disk_inode->sectors = 0; 
      size_t sectors = bytes_to_sectors (length);
      disk_inode->length = length;
      disk_inode->magic = INODE_MAGIC;
      
      int i;
      for (i = 0; i < 66; ++i)
      {
        if (i < 5)
        {
          disk_inode->data_blocks[i] = -1;
        }
        else if (i < 65)
        {
          disk_inode->indirect_blocks[i - 5] = -1;
        }
        else
        {
          disk_inode->db_block = -1;
        }
      }
      
      //printf ("Num Sectors: %d\n", sectors);
      for(i = 0; i < sectors; i++)
      {
        if (!add_sector(disk_inode))
        {
            PANIC("ALLOCATE FAILED");
        }
        //printf ("Sector: %d\n", i + 1);
      }
      
      block_write (fs_device, sector, disk_inode);
      
      success = true;
      free (disk_inode);
    }
  return success;
}


/* Reads an inode from SECTOR
   and returns a `struct inode' that contains it.
   Returns a null pointer if memory allocation fails. */
struct inode *
inode_open (block_sector_t sector)
{
  struct list_elem *e;
  struct inode *inode;

  /* Check whether this inode is already open. */
  for (e = list_begin (&open_inodes); e != list_end (&open_inodes);
       e = list_next (e)) 
    {
      inode = list_entry (e, struct inode, elem);
      if (inode->sector == sector) 
        {
          inode_reopen (inode);
          return inode; 
        }
    }

  /* Allocate memory. */
  inode = malloc (sizeof *inode);
  if (inode == NULL)
    return NULL;

  /* Initialize. */
  list_push_front (&open_inodes, &inode->elem);
  inode->sector = sector;
  inode->open_cnt = 1;
  inode->deny_write_cnt = 0;
  inode->removed = false;
  //printf("length before for sector %d: %d\n", inode->sector,inode->data.length);
  block_read (fs_device, inode->sector, &inode->data);
  //printf("length after for sector %d: %d\n", inode->sector, inode->data.length);
  return inode;
}

/* Reopens and returns INODE. */
struct inode *
inode_reopen (struct inode *inode)
{
  if (inode != NULL)
    inode->open_cnt++;
  return inode;
}

/* Returns INODE's inode number. */
block_sector_t
inode_get_inumber (const struct inode *inode)
{
  return inode->sector;
}

/* Closes INODE and writes it to disk.
   If this was the last reference to INODE, frees its memory.
   If INODE was also a removed inode, frees its blocks. */
void
inode_close (struct inode *inode) 
{
  /* Ignore null pointer. */
  if (inode == NULL)
    return;

  /* Release resources if this was the last opener. */
  if (--inode->open_cnt == 0)
    {
      /* Remove from inode list and release lock. */
      list_remove (&inode->elem);
 
      /* Deallocate blocks if removed. */
      if (inode->removed) 
        {

          remove_sectors(&inode->data);   
          free_map_release (inode->sector, 1);
          //free_map_release (inode->data.start,
         //                   bytes_to_sectors (inode->data.length)); 
        }

      free (inode); 
    }
}

/* Marks INODE to be deleted when it is closed by the last caller who
   has it open. */
void
inode_remove (struct inode *inode) 
{
  ASSERT (inode != NULL);
  inode->removed = true;
}

/* Reads SIZE bytes from INODE into BUFFER, starting at position OFFSET.
   Returns the number of bytes actually read, which may be less
   than SIZE if an error occurs or end of file is reached. */
off_t
inode_read_at (struct inode *inode, void *buffer_, off_t size, off_t offset) 
{
  uint8_t *buffer = buffer_;
  off_t bytes_read = 0;
  uint8_t *bounce = NULL;

  while (size > 0) 
    {
      /* Disk sector to read, starting byte offset within sector. */
      block_sector_t sector_idx = byte_to_sector (inode, offset);
      int sector_ofs = offset % BLOCK_SECTOR_SIZE;
      
      //printf("inode_read_at: %d\n", sector_idx);
      
      /* Bytes left in inode, bytes left in sector, lesser of the two. */
      off_t inode_left = inode_length (inode) - offset;
      int sector_left = BLOCK_SECTOR_SIZE - sector_ofs;
      int min_left = inode_left < sector_left ? inode_left : sector_left;

      /* Number of bytes to actually copy out of this sector. */
      int chunk_size = size < min_left ? size : min_left;
      if (chunk_size <= 0)
        break;

      if (sector_ofs == 0 && chunk_size == BLOCK_SECTOR_SIZE)
        {
          /* Read full sector directly into caller's buffer. */
         
          block_read (fs_device, sector_idx, buffer + bytes_read);
        }
      else 
        {
          /* Read sector into bounce buffer, then partially copy
             into caller's buffer. */
          if (bounce == NULL) 
            {
              bounce = malloc (BLOCK_SECTOR_SIZE);
              if (bounce == NULL)
                break;
            }
          block_read (fs_device, sector_idx, bounce);
          memcpy (buffer + bytes_read, bounce + sector_ofs, chunk_size);
        }
      
      /* Advance. */
      size -= chunk_size;
      offset += chunk_size;
      bytes_read += chunk_size;
    }
  free (bounce);

  return bytes_read;
}

/* Writes SIZE bytes from BUFFER into INODE, starting at OFFSET.
   Returns the number of bytes actually written, which may be
   less than SIZE if end of file is reached or an error occurs.
   (Normally a write at end of file would extend the inode, but
   growth is not yet implemented.) */
off_t
inode_write_at (struct inode *inode, const void *buffer_, off_t size,
                off_t offset) 
{
  const uint8_t *buffer = buffer_;
  off_t bytes_written = 0;
  uint8_t *bounce = NULL;

  if (inode->deny_write_cnt)
  {
    return 0;
  }

  while (size > 0) 
    {
      /* Sector to write, starting byte offset within sector. */
      block_sector_t sector_idx = byte_to_sector (inode, offset);
      int sector_ofs = offset % BLOCK_SECTOR_SIZE;
      
      //printf("inode_write_at: %d\n", sector_idx);
      
      /* Bytes left in inode, bytes left in sector, lesser of the two. */
      off_t inode_left = inode_length (inode) - offset;
      int sector_left = BLOCK_SECTOR_SIZE - sector_ofs;
      int min_left = inode_left < sector_left ? inode_left : sector_left;
      
      /* Number of bytes to actually write into this sector. */
      int chunk_size = size < min_left ? size : min_left;
      //printf("sector left: %d\n", sector_left);
      //printf("inode_left: %d\n", inode_left);
      //printf("chunk size: %d\n", chunk_size);
      
      if (chunk_size <= 0)
        break;

      if (sector_ofs == 0 && chunk_size == BLOCK_SECTOR_SIZE)
        {
          /* Write full sector directly to disk. */

          block_write (fs_device, sector_idx, buffer + bytes_written);
        }
      else 
        {
          /* We need a bounce buffer. */
          if (bounce == NULL) 
            {
              bounce = malloc (BLOCK_SECTOR_SIZE);
              if (bounce == NULL)
                break;
            }

          /* If the sector contains data before or after the chunk
             we're writing, then we need to read in the sector
             first.  Otherwise we start with a sector of all zeros. */
          if (sector_ofs > 0 || chunk_size < sector_left) 
            block_read (fs_device, sector_idx, bounce);
          else
            memset (bounce, 0, BLOCK_SECTOR_SIZE);
          memcpy (bounce + sector_ofs, buffer + bytes_written, chunk_size);

          block_write (fs_device, sector_idx, bounce);
        }

      /* Advance. */
      size -= chunk_size;
      offset += chunk_size;
      bytes_written += chunk_size;
    }
  free (bounce);

  block_write (fs_device, inode->sector, &inode->data);

  return bytes_written;
}

/* Disables writes to INODE.
   May be called at most once per inode opener. */
void
inode_deny_write (struct inode *inode) 
{
  inode->deny_write_cnt++;
  ASSERT (inode->deny_write_cnt <= inode->open_cnt);
}

/* Re-enables writes to INODE.
   Must be called once by each inode opener who has called
   inode_deny_write() on the inode, before closing the inode. */
void
inode_allow_write (struct inode *inode) 
{
  ASSERT (inode->deny_write_cnt > 0);
  ASSERT (inode->deny_write_cnt <= inode->open_cnt);
  inode->deny_write_cnt--;
}

/* Returns the length, in bytes, of INODE's data. */
off_t
inode_length (const struct inode *inode)
{
  //printf ("length = %d sector: %d\n", inode->data.length, inode->sector);
  return inode->data.length;
}
