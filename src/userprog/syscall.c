#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include <filesys/file.h>

static void syscall_handler (struct intr_frame *);

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f UNUSED) 
{
  printf ("system call!\n");
  thread_exit ();
}

/*
int open (const char *file)
{
  struct thread *t = thread_current();
  struct file *f = open_file(file);
  if (f != NULL)
  {
    list_push_back(&t->open_files, &f->open_file);
    f->fd = t->fd_count;
    t->fd_count++;
    return f->fd;
  }
  else
  {
    return -1;
  }
}
*/
/*
int wait (pid_t pid)
{
  return process_wait(pid);  
}
*/

int write (int fd, const void *buffer, unsigned size)
{
  struct thread *t = thread_current();
  struct file *f = list_entry (list_begin (&t->open_files), struct file, open_file);
  
  while ((int) &f->fd != fd);
  {
    if( &f->open_file == list_tail(&t->open_files) )
    {
      return -1;
    }
    f = list_entry (list_next(&f->open_file), struct file, open_file);
  }
  
  
  
  return 0;
}





















































