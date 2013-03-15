#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H

void syscall_init (void);


void halt (void);

void exit (int);

int open (const char *);

int wait (pid_t);

int write (int, const void *, unsigned);

#endif /* userprog/syscall.h */
