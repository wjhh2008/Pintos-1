#include "lib/kernel/list.h"
#include <stdlib.h>
#include <stdbool.h>
#include "threads/thread.h"
#include <string.h>
#include <limits.h>
#include <stdio.h>
#include "threads/priority.h"
#include "synch.h"



/************************TO BE DELETED*****************************************/

struct thread* pick_highest_priority(void)
{

   ASSERT (intr_get_level () == INTR_OFF);
   struct thread *next, *next1;
   struct list_elem *e;
   e = list_begin (&ready_list);

   next = list_entry (e, struct thread, elem);



   if (list_size(&ready_list)>1)
   {
       for (e = list_next(e); e != list_end (&ready_list); e = list_next (e))
       {
          next1 = list_entry (e, struct thread, elem);
          if((next1->priority) > (next->priority))
              next = next1;
       }
    }

   list_remove(&next->elem);
   return next;
}
/*****************************************************************************/


/* Returns true if priority A is less than priority B, false
   otherwise. */
bool
priority_more (struct list_elem *a_, struct list_elem *b_,
            void *aux UNUSED)
{
   struct thread *a = list_entry (a_, struct thread, elem);
   struct thread *b = list_entry (b_, struct thread, elem);
   return a->priority > b->priority;
}


/* Returns true if priority A is less than priority B, false
   otherwise. */
bool
priority_more_condvar (struct list_elem *a_, struct list_elem *b_,
            void *aux UNUSED)
{
   struct semaphore_elem *a = list_entry (a_, struct semaphore_elem, elem);
   struct semaphore_elem *b = list_entry (b_, struct semaphore_elem, elem);
   return a->priority > b->priority;
}





bool check_preemption()
{
 struct thread *cur = thread_current ();
 struct list_elem* top=list_begin(&ready_list);
 return (priority_more(top, &cur->elem, NULL));
}



/* Our implementation of list_insert_ordered() */
void
my_list_insert_ordered (struct list *list, struct list_elem *elem,
                     list_less_func *more, void *aux)
{
  struct list_elem *e;

  ASSERT (list != NULL);
  ASSERT (elem != NULL);
  ASSERT (more != NULL);

  for (e = list_begin (list); e != list_end (list); e = list_next (e))
    if (more (elem, e, aux))
      break;
  return list_insert (e, elem);
}


/* Sets the priority of threads waiting in ready_list to NEW_PRIORITY. */
void
thread_set_ready_priority (struct thread* t, int new_priority)
{
 ASSERT(new_priority >= PRI_MIN && new_priority<= PRI_MAX);
 if(new_priority>(t->priority))
 {
    t->priority = new_priority;
    t->under_donation=true;

    list_sort(&ready_list,&priority_more,NULL);

    /*Should we really check this here*/
    //if(check_preemption())
    //    thread_yield();
  }
}


/* Sets the priority of this thread t equal to the maximum priority of a thread that has blocked upon a lock acquired by t  */
void
thread_set_true_priority (struct thread* t)
{
 
    t->under_donation=false;
    //First find the maximal priority blocked thread.

    struct list_elem *e;
    struct list_elem *te;
    struct thread *topthread;
    struct lock *l;
    int max_priority=t->true_priority;

     if (list_size( &(t->locks_list) )>0)
    {
       for (e = list_begin( &(t->locks_list) ); e != list_end (&t->locks_list); e = list_next (e))
       {
          l = list_entry (e, struct lock, locks_elem);

          if( ( list_empty( &((l->semaphore).waiters) ) ) )
              continue;
          else{
              te=list_begin( &((l->semaphore).waiters) );
              topthread=list_entry(te,struct thread,elem);
              if(topthread->priority > max_priority)
                  max_priority=topthread->priority;
           }
       }
    }
 
    t->priority = max_priority;
    list_sort(&ready_list,&priority_more,NULL);

    /*Should we really check this here NO*/
    //if(check_preemption())
    //    thread_yield();
}
