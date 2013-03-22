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


static void syscall_handler (struct intr_frame *);

void
syscall_init (void) 
{
  lock_init(&write_lock);
  lock_init(&exec_lock);
  lock_init(&process_lock);
  sema_init(&list_sema, 0);
  
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f UNUSED) 
{
  
  //printf("Name: %s, Tid: %d\n", thread_current()->name, thread_current()->tid);
  //hex_dump((int) f->esp, f->esp, (char *) PHYS_BASE - (char *) f->esp, 1);
  
  int call_number = *(int *) f->esp;
  void *arg0 = f->esp + 4;
  void *arg1 = f->esp + 8;
  void *arg2 = f->esp + 12;
  
  if ( (*(int *) arg0 > f->esp) && 
       (*(int *) arg0 > f->esp) && 
       (*(int *) arg0 > f->esp) )
  {
    exit (-1);
  }
  
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
        f->eax = exec ((char *) arg0);
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
        f->eax = create ((char *) arg0, *(unsigned *) arg1);
      }
      else
      {
        exit (-1);
      }
      break;
    case SYS_REMOVE:
      remove ((char *) arg0);
      break;
    case SYS_OPEN:
      if (valid_pointer (arg0))
      {
        f->eax = open (*(int *) arg0);
      }
      else
      {
        //printf("bad pointer!!!!!!\n");
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
    default:
      //printf("Invalid sys_call\n");
      exit(-1);
      break;      
  }
}

struct file* find_file (int fd)
{
  struct thread *t = thread_current();
  struct list_elem *e;
  struct file *f;
  
  for (e = list_begin (&t->open_files); e != list_end (&t->open_files); e = list_next (e))
  {
    f = list_entry (e, struct file, open_file);
    if (&f->fd == fd)
    {
      return f;
    }
  }
  
  return NULL;
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
  tid_t tid = process_execute (*(int *) cmd_line);  
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
bool create (const char *file, unsigned initial_size)
{
  return filesys_create (*(int *) file, (off_t) initial_size);
}

// ------
// remove
// ------
bool remove (const char *file)
{
  
  return filesys_remove(* (int *) file);
}

// ----
// open
// ----
int open (const char *file)
{
  struct thread *t = thread_current();
  struct file *f = filesys_open(file);
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

// --------
// filesize
// --------
int filesize (int fd)
{ 

  struct file *f = find_file (fd);
  return (int) file_length (&f);
}

// ----
// read
// ----
int read (int fd, void *buffer, unsigned size) 
{
  struct file *f = find_file(fd);
  if (f == NULL)
  {
    return -1;
  }
  return (int) file_read (&f, &buffer, (off_t)size); 
}

// -----
// write
// -----
int write (int fd, const void *buffer, unsigned size)
{
  if (fd == 0) // Write to stdin
  {
    return 0;
  }
  else if (fd == 1) // Write to stdout
  {
    putbuf(*(int *)buffer, size);
    return (int) size;
  }
  else // Write to file
  {
    lock_acquire(&write_lock);
    struct thread *t = thread_current();
    struct file *f = list_entry (list_begin (&t->open_files), struct file, open_file);
  
    do
    {
      // Is open_files empty?
      if( &f->open_file == list_tail(&t->open_files) )
      {
        lock_release(&write_lock);
        return -1;
      }
      
      // Is f correct file?
      if ( *(int *) &f->fd == fd )
      {
        lock_release(&write_lock);
        return (int) file_write(f, buffer, size);
      }
      f = list_entry (list_next(&f->open_file), struct file, open_file);    
    } while ((int) &f->fd != fd);
  }
  
  return -1;
}

// ----
// seek
// ----
void seek (int fd, unsigned position)
{
  struct file *f = find_file (fd);
  return file_seek (&f, (off_t) position);
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
  printf("close!\n");
  return;
}














