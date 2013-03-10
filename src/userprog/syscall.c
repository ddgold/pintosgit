#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include <filesys/file.h>
#include "threads/vaddr.h"

static void syscall_handler (struct intr_frame *);

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f UNUSED) 
{
  hex_dump((int) f->esp, f->esp, (char *) PHYS_BASE - (char *) f->esp, 1);
  int call_number = *(int *) f->esp;
  void *arg0 = f->esp + 4;
  void *arg1 = f->esp + 8;
  void *arg2 = f->esp + 12;
  
  switch (call_number)
  {
    case SYS_WRITE:
      write (*(int *) arg0, arg1, *(unsigned *) arg2);
      break;
    default:
      printf("system call!\n");
      thread_exit();
      break;      
  }
  
  thread_exit(); 
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
  if (fd == 0) // stdin
  {
    return 0;
  }
  else if (fd == 1) // stdout
  {
    putbuf(*(int *)buffer, size);
    return (int) size;
  }
  else // file
  {
    while ((int) &f->fd != fd);
    { 
      printf("this is looping: %d\n", &f->fd);
      if( &f->open_file == list_tail(&t->open_files) )
      {
        return -1;
      }
      f = list_entry (list_next(&f->open_file), struct file, open_file);
    }
  }
  //printf("testing: %x\n", *(int *)buffer);
  
  //return (int) file_write(f, buffer, size);
}
