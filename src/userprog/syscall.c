#include "userprog/syscall.h"
#include "lib/user/syscall.h"
#include "lib/kernel/list.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include <filesys/file.h>
#include "threads/vaddr.h"
#include "threads/synch.h"
#include "userprog/pagedir.h"
#include "filesys/inode.h"
#include "filesys/directory.h"

static void syscall_handler (struct intr_frame *);

void
syscall_init (void) 
{
  lock_init(&write_lock);
  lock_init(&exec_lock);
  lock_init(&process_lock);
  sema_init(&list_sema, 0);
  sema_init(&close_sema, 1);
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f UNUSED) 
{
  if (!valid_pointer (&f->esp))
  {
    exit (-1);
  }
  // Get syscall number off stack
  int call_number = *(int *) f->esp;
 

  // Get arguments off stack
  void *arg0 = f->esp + 4;
  void *arg1 = f->esp + 8;
  void *arg2 = f->esp + 12;
 

  // Varify that all the arguments are above the stack pointer
  if ( (*(int *) arg0 > f->esp) &&
       (*(int *) arg1 > f->esp) && 
       (*(int *) arg2 > f->esp) )
  {
    exit (-1);
  }
   
  // Switch for the syscalls
  switch (call_number)
  {
    case SYS_HALT:
      halt ();
      break;
    case SYS_EXIT:
      exit (*(int *) arg0);
      break;
    case SYS_EXEC:
      if (valid_pointer (arg0))
      {
        f->eax = exec (*(const char **) arg0);
      }
      else
      {
        exit (-1); 
      }
      break;
    case SYS_WAIT:
      f->eax = wait (*(pid_t *) arg0);
      break;
    case SYS_CREATE:
      if (valid_pointer (arg0))
      {
        f->eax = create (*(const char **) arg0, *(unsigned *) arg1);
      }
      else
      {
        exit (-1);
      }
      break;
    case SYS_REMOVE:
      remove (*(const char **) arg0);
      break;
    case SYS_OPEN:
      if (valid_pointer (arg0))
      {
        f->eax = open (*(const char **) arg0);
      }
      else
      {
        exit (-1);
      }
      break;
    case SYS_FILESIZE:
      f->eax = filesize (*(int *) arg0);
      break;
    case SYS_READ:
      if (valid_pointer (arg1))
      {
        f->eax = read (*(int *) arg0, arg1, *(unsigned *) arg2);
      }
      else
      {
        exit (-1);
      }
      break;
    case SYS_WRITE:
      if (valid_pointer (arg1))
      {
        f->eax = write (*(int *) arg0, arg1, *(unsigned *) arg2);
      }
      else
      {
        exit (-1);
      }
      break;
    case SYS_SEEK:
      seek (*(int *) arg0, *(unsigned *) arg1);
      break;
    case SYS_TELL:
      f->eax = tell (*(int *) arg0);
      break;
    case SYS_CLOSE:
      close (*(int *) arg0);
      break;
    
    // Project 4
    case SYS_CHDIR:
      f->eax = chdir(*(const char **) arg0);
      break;
    case SYS_MKDIR:
      f->eax = mkdir(*(const char **) arg0);
      break;
    case SYS_READDIR:
      f->eax = readdir(*(int *) arg0, *(const char **) arg1);
      break;
    case SYS_ISDIR:
      f->eax = isdir(*(int *) arg0);
      break;
    case SYS_INUMBER:
      f->eax = inumber(*(int *) arg0);

    // Invalid Syscall
    default:
      exit(-1);
      break;      
  }
}

// Look up file from current thread's open_files list from fd
struct file* find_file (int fd)
{
  struct thread *t = thread_current();
  struct list_elem *e;
  struct file *f;
  
  for (e = list_begin (&t->open_files); e != list_end (&t->open_files); e = list_next (e))
  {
    f = list_entry (e, struct file, open_file);
    if (f->fd == fd)
    {
      return f;
    }
  }
  return NULL;
}


struct dir* find_dir (int fd)
{
  struct thread *t = thread_current();
  struct list_elem *e;
  struct dir *d;

  for (e = list_begin (&t->open_dirs); e != list_end (&t->open_dirs); e = list_next (e))
  {
    d = list_entry (e, struct dir, open_dir);
    if (d->fd == fd)
    {
      return d;
    }
  }
  return NULL;
}


char* parse (const char *path)
{
  char *token, *save_ptr;
  char *temp = palloc_get_page(0);
  strlcpy (temp, path, PGSIZE);
  for(token = strtok_r (temp, "/", &save_ptr); token != NULL;
        token = strtok_r (NULL, "/", &save_ptr))
  {
    if(!strcmp(save_ptr,""))
    {
      break;
    }
  }
  return token;
}


struct dir *
follow_path(const char *path, struct dir *dir)
{
  char *token, *save_ptr;
  char *temp = palloc_get_page(0);
  strlcpy (temp, path, PGSIZE);
  for(token = strtok_r (temp, "/", &save_ptr); token != NULL;
        token = strtok_r (NULL, "/", &save_ptr))
  {
      
    // Last entry, break;
    if(!strcmp(save_ptr,""))
    {
      break;
    }
    
    // Current directory
    else if(!strcmp(save_ptr,"."))
    {
      continue;
    }
    
    // Previous directory
    else if(!strcmp(save_ptr,".."))
    {
      if(dir->parent_dir != NULL)
      {
        dir = dir->parent_dir;
      }
    }
    else
    {
      struct dir_entry e;
      size_t ofs;
      bool didFind = false;
      for (ofs = 0; inode_read_at (dir->inode, &e, sizeof e, ofs) == sizeof e; ofs += sizeof e) 
      {
        if(!strcmp(e.name, token) && e.isSubDir)
        {
          dir = dir_open(inode_open(e.inode_sector));
          didFind = true;
          break;
        }
      }
      if(didFind == false)
      {
        dir = NULL;
        break;
      }
    }
  }
  palloc_free_page(temp);
  return dir;
}



// ----
// halt
// ----
void halt (void)
{
  shutdown_power_off ();
  return;
}

// ----
// exit
// ----
void exit (int status)
{
  struct thread *t = thread_current();
  t->exit_status = status;
  thread_exit ();
}

// ----
// exec
// ----
pid_t exec (const char *cmd_line) 
{
  tid_t tid = process_execute (cmd_line);
  return tid;
}

// ----
// wait
// ----
int wait (pid_t pid)
{
  return process_wait(pid);
}

// ------
// create
// ------
bool create (const char *path, unsigned initial_size)
{
  if (!strcmp(path, ""))
  {
    return false;
  }

  struct dir *directory = follow_path (path, thread_current()->cwd);
  char *file = parse (path);
  return filesys_create (file, (off_t) initial_size, directory);
}

// ------
// remove
// ------
bool remove (const char *path)
{
  struct dir *directory = follow_path (path, thread_current()->cwd);
  char *file = parse (path);
  
  struct dir_entry entry;
  size_t ofs;
  for (ofs = 0; inode_read_at (directory->inode, &entry, sizeof entry, ofs) == sizeof entry; ofs += sizeof entry)
  {
    if (!strcmp(entry.name, file))
    {
      if (entry.isSubDir)
      {
        /*
        struct dir *d = dir_open(inode_open(e.inode_sector));
        for (ofs = 0; inode_read_at (d->inode, &e, sizeof e, ofs) == sizeof e; ofs += sizeof e)
        {
          if (e.in_use)
          {
            return false;
          }
        }
        
        */
        return true;
      }
      else
      {
        return filesys_remove(file, directory);
      }
    }
  }
  
  PANIC ("open: File Not Found");
  return false;
}

// ----
// open
// ----
int open (const char *path)
{
  if (!strcmp(path, ""))
  {
    return -1;
  }
  
  struct dir *directory = follow_path (path, thread_current()->cwd);
  char *file = parse (path);
  struct thread *t = thread_current();

  struct dir_entry e;
  size_t ofs;
  for (ofs = 0; inode_read_at (directory->inode, &e, sizeof e, ofs) == sizeof e; ofs += sizeof e)
  {
    if (!strcmp(e.name, file))
    {
      if (e.isSubDir)
      {
        struct dir *d = dir_open(inode_open(e.inode_sector));
        list_push_back(&t->open_dirs, &d->open_dir);
        d->fd = t->fd_count;
        t->fd_count++;
        return d->fd;
      }
      else
      {
        struct file *f = file_open (inode_open(e.inode_sector));
        list_push_back(&t->open_files, &f->open_file);
        f->fd = t->fd_count;
        t->fd_count++;
        return f->fd;
      }
    }
  }

  return -1;
}

// --------
// filesize
// --------
int filesize (int fd)
{ 
  struct file *f = find_file (fd);
  int length = file_length (f);
  return length;
}

// ----
// read
// ----
int read (int fd, void *buffer, unsigned size) 
{
  struct file *f = find_file(fd);
  // If valid fd read, else return -1
  if (f == NULL)
  {
    return -1;
  }
  else
  {
    int read = file_read(f, *(int *)buffer, (off_t) size);
    return read;
  }
}

// -----
// write
// -----
int write (int fd, const void *buffer, unsigned size)
{
  // Write to stdin
  if (fd == 0)
  {
    return 0;
  }
  
  // Write to stdout
  else if (fd == 1) 
  {
    putbuf(*(int *)buffer, size);
    return (int) size;
  }
  
  // Write to file
  else
  {
    lock_acquire(&write_lock);
    
    struct file *f = find_file (fd);
        
    if(f == NULL)
    {
      lock_release(&write_lock);
      return -1;
    }
    else
    {
      struct inode *in = *(int *) &f->inode;
      
      // Check if file needs a new sector
      if((size + *(int *)&f->pos) > (*(unsigned int *)&in->data.sectors * BLOCK_SECTOR_SIZE))
      {
        add_sector(&in->data);
      }
      
      // Increase the length of file
      if( in->data.length < (size + *(int *)&f->pos) )
      {
        in->data.length = *(int *)&f->pos + size;
      }
      
      lock_release(&write_lock);
      int test = file_write(f, *(int *)buffer, size);
      return test;
    }
  }
  
  return -1;
}

// ----
// seek
// ----
void seek (int fd, unsigned position)
{
  struct file *f = find_file (fd);
  return file_seek (f, (off_t) position);
}

// ----
// tell
// ----
unsigned tell (int fd) 
{
  struct file *f = find_file (fd);
  return (unsigned) file_tell (&f);
}

// -----
// close
// -----
void close (int fd)
{
  sema_down(&close_sema);
  struct file *f = find_file (fd);
  if(f == NULL)
  {
    sema_up(&close_sema);
    exit(-1);
    return;
  }
  list_remove(&f->open_file);
  sema_up(&close_sema);
  return file_close(f);
}

// -----
// chdir
// -----
bool chdir (const char *dir)
{
  struct dir *directory = follow_path (dir, thread_current()->cwd);

  if (directory == NULL)
  {
    PANIC ("chdir: Invalid Path");
    return false;
  }
 
  char *file = parse (dir);
  struct dir_entry e;
  size_t ofs;
  for (ofs = 0; inode_read_at (directory->inode, &e, sizeof e, ofs) == sizeof e; ofs += sizeof e) 
  {
    if(!strcmp(e.name, file))
    {
      thread_current()->cwd = dir_open(inode_open(e.inode_sector));
      return true;
    }
  }
  
  PANIC ("chdir: SubDir Not Found");
  return false;
}

// -----
// mkdir
// -----
bool mkdir (const char *dir)
{
  if (!strcmp(dir, ""))
  {
    return false;
  }
  
  struct dir *directory = follow_path (dir, thread_current()->cwd);
  
  if (directory == NULL)
  {
    PANIC ("mkdir: Invalid Path");
    return false;
  }
  
  block_sector_t sector;
  if(!free_map_allocate(1, &sector))
  {
    PANIC ("mkdir: Filesys Full");
    return false;
  }
  
  dir_create(sector, 1);
  return dir_add (directory, parse (dir), sector, true); 
  return true;
}

// -------
// readdir
// -------
bool readdir (int fd, char *name)
{
  PANIC ("readdir not implemented");
  return false;
}

// -----
// isdir
// -----
bool isdir (int fd) 
{
  struct thread *t = thread_current();
  struct list_elem *e;
  struct dir *d;
  
  for (e = list_begin (&t->open_dirs); e != list_end (&t->open_dirs); e = list_next (e))
  {
    d = list_entry (e, struct dir, open_dir);
    if (d->fd == fd)
    {
      return true;
    }
  }
  
  return false;
}

// -------
// inumber
// -------
int inumber (int fd)
{
  PANIC ("inumber not implemented");
  return -1;
}
