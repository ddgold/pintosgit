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

static void syscall_handler (struct intr_frame *);

void
syscall_init (void) 
{
  lock_init(&write_lock);
  lock_init(&exec_lock);
  list_init(&process_list);
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
  
  switch (call_number)
  {
    
    case SYS_HALT:
      halt ();
      break;
    case SYS_EXIT:
      exit (*(int *) arg0);
      break;
    case SYS_EXEC:
      f->eax = exec ((char *) arg0);
      break;
    case SYS_WAIT:
      f->eax = wait (*(pid_t *) arg0);
      break;
    case SYS_CREATE:
      create ((char *) arg0, *(unsigned *) arg1);
      break;
    case SYS_REMOVE:
      remove ((char *) arg0);
      break;
    case SYS_OPEN:
      f->eax = open (*(int *) arg0);
      break;
    case SYS_FILESIZE:
      f->eax = filesize (*(int *) arg0);
      break;
    case SYS_READ:
      f->eax = read (*(int *) arg0, arg1, *(unsigned *) arg2);
      break;
    case SYS_WRITE:
      f->eax = write (*(int *) arg0, arg1, *(unsigned *) arg2);
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
      printf("Invalid sys_call\n");
      thread_exit();
      break;      
  }
}


// ----
// halt
// ----
void halt (void)
{
  shutdown_power_off ();
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
  lock_acquire(&exec_lock);  
  tid_t tid = process_execute (cmd_line);
  lock_release(&exec_lock);
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
  printf("Create!\n");
  return 0;
}

// ------
// remove
// ------
bool remove (const char *file)
{
  printf("Remove!\n");
  return 0;
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

int filesize (int fd)
{
  printf("filesize!\n");
  return 0;
}

int read (int fd, void *buffer, unsigned size) 
{
  printf("read!\n");
  return 0;
}

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

void seek (int fd, unsigned position)
{
  printf("Seek!\n");
  return;
}

unsigned tell (int fd) 
{
  printf("tell!\n");
  return 0u;
}

void close (int fd) 
{
  printf("close!\n");
  return;
}














