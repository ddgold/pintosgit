#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H

void syscall_init (void);



int wait (pid_t);

int write (int, const void *, unsigned);

#endif /* userprog/syscall.h */
