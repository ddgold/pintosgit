#ifndef VM_PAGE_H
#define VM_PAGE_H

struct page
{
  char *name;               /*holds name for debuggin purposes*/
  struct thread *owner;     /*the thread that owns the page */
};

#endif
