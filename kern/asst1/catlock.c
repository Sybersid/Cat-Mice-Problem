/*
 * catlock.c
 *
 * 30-1-2003 : GWA : Stub functions created for CS161 Asst1.
 *
 * NB: Please use LOCKS/CV'S to solve the cat syncronization problem in 
 * this file.
 */


/*
 * 
 * Includes
 *
 */

#include <types.h>
#include <lib.h>
#include <test.h>
#include <thread.h>
#include <synch.h>

/*
 * 
 * Constants
 *
 */

/*
 * Number of food bowls.
 */

#define NFOODBOWLS 2

/*
 * Number of cats.
 */

#define NCATS 6
static const char *const catnames[NCATS] = {"Meow1", "Meow2", "Meow3", "Meow4", "Meow5", "Meow6"};

/*
 * Number of mice.
 */

#define NMICE 2
static const char *const mousenames[NMICE] = {"Mickey", "Minnie"};

typedef int bool;
#define TRUE 1
#define FALSE 0


static volatile bool all_dishes_available;
static volatile bool dish1_busy;
static volatile bool dish2_busy;
static volatile int mydish;

static volatile int cats_wait_count;
static volatile int cats_in_this_turn;
static volatile int cats_eat_count;
static volatile int num_cats_done;

static volatile int mice_wait_count;
static volatile int mice_in_this_turn;
static volatile int mice_eat_count;
static volatile int num_mice_done;

static volatile int turn_type;
static struct lock *mutex;
static struct cv *turn_cv;
static struct cv *done_cv;


static void turn_switch() {
       if (mice_wait_count > 0) {
        turn_type = 2;
        mice_in_this_turn = 2;
        kprintf("It is mice turn now.\n");
        } else if (cats_wait_count > 0) {
                turn_type = 1;
                cats_in_this_turn = 2;
                kprintf("It is cats turn now.\n");
       } else {
                turn_type = 0;
       }
       cv_broadcast(turn_cv, mutex);    
}


/*
 * 
 * Function Definitions
 * 
 */


/*
 * catlock()
 *
 * Arguments:
 *      void * unusedpointer: currently unused.
 *      unsigned long catnumber: holds the cat identifier from 0 to NCATS -
 *      1.
 *
 * Returns:
 *      nothing.
 *
 * Notes:
 *      Write and comment this function using locks/cv's.
 *
 */

static
void
catlock(void * unusedpointer, 
        unsigned long catnumber)
{
        /*
         * Avoid unused variable warnings.
         */

        (void) unusedpointer;
        (void) catnumber;
        int i;
        int mydish = 0;

// iterate 5 times
for (i=0; i < 5; i++) {
        lock_acquire(mutex);    // critical section 1
        cats_wait_count++;
        
        // initialize cats enter kitchen
        if (turn_type == 0) {
                turn_type = 1;
                cats_in_this_turn = 2;
        }
        
        // wait sequence while not cats turn
        while (turn_type != 1 || cats_in_this_turn == 0) {
                cv_wait(turn_cv, mutex);
        }
        // cats enter kitchen
        cats_in_this_turn--;
        cats_eat_count++;
        lock_release(mutex);    // critical section 1

        kprintf("%s enters the kitchen.\n", catnames[catnumber]);
        clocksleep(1);  // time to eat

        lock_acquire(mutex);    // critical section 2
        // aquire dish
        if (dish1_busy == FALSE) {
                dish1_busy = TRUE;
                mydish = 1;
        } else {
                dish2_busy = TRUE;
                mydish = 2;
        }
        lock_release(mutex);    // critical section 2
        kprintf("%s is eating at dish %d.\n", catnames[catnumber], mydish);
        
        clocksleep(1);
        kprintf("%s finishes eating at dish %d.\n", catnames[catnumber], mydish);
        
        lock_acquire(mutex);    // critical section 3
        
        //return dish
        if (mydish == 1) {
                dish1_busy = FALSE;
        } else {
                dish2_busy = FALSE;
        }
        // cats leave kitchen 
        cats_eat_count--;
        cats_wait_count--;
        num_cats_done++;
        lock_release(mutex);    // critical section 3
        clocksleep(1);

        lock_acquire(mutex);    // critical section 4
        // siwtch turn sequence
        if (mice_wait_count == 0 && (cats_wait_count - cats_eat_count > 0)) {
                cats_in_this_turn++;    // different set of cats enter
                cv_signal(turn_cv, mutex);

        } else {
                turn_switch();          // switch to waiting mice
        }
        lock_release(mutex);    // critical section 4
        kprintf("%s leaves kitchen.\n", catnames[catnumber]);
        kprintf("%d cats done\n", num_cats_done);       // total cats entered count
        }
        // after looping exit program
        if (cats_wait_count == 0 && mice_wait_count == 0) {
                cv_broadcast(done_cv, mutex);
        }
}
	

/*
 * mouselock()
 *
 * Arguments:
 *      void * unusedpointer: currently unused.
 *      unsigned long mousenumber: holds the mouse identifier from 0 to 
 *              NMICE - 1.
 *
 * Returns:
 *      nothing.
 *
 * Notes:
 *      Write and comment this function using locks/cv's.
 *
 */

static
void
mouselock(void * unusedpointer,
          unsigned long mousenumber)
{
        /*
         * Avoid unused variable warnings.
         */
        
        (void) unusedpointer;
        (void) mousenumber;
        int i;
        int mydish = 0;

for (i=0; i < 5; i++) {
        lock_acquire(mutex);    // critical section 1
        mice_wait_count++;
        
        // initialize mice enter kitchen
        if (turn_type == 0) {
                turn_type = 2;
                mice_in_this_turn = 2;
        }
        
        // wait sequence while not mice turn
        while (turn_type != 2 || mice_in_this_turn == 0) {
                cv_wait(turn_cv, mutex);
        }
        // mice enter kitchen
        mice_in_this_turn--;
        mice_eat_count++;
        lock_release(mutex);    // critical section 1

        kprintf("%s enters the kitchen.\n", mousenames[mousenumber]);
        clocksleep(1);  // time to eat

        lock_acquire(mutex);    // critical section 2
        // aquire dish
        if (dish1_busy == FALSE) {
                dish1_busy = TRUE;
                mydish = 1;
        } else {
                dish2_busy = TRUE;
                mydish = 2;
        }
        lock_release(mutex);    // critical section 2
        kprintf("%s is eating at dish %d.\n", mousenames[mousenumber], mydish);
        
        clocksleep(1);
        kprintf("%s finishes eating at dish %d.\n", mousenames[mousenumber], mydish);
        
        lock_acquire(mutex);    // critical section 3
        
        //return dish
        if (mydish == 1) {
                dish1_busy = FALSE;
        } else {
                dish2_busy = FALSE;
        }
        // mice leave kitchen 
        mice_eat_count--;
        mice_wait_count--;
        num_mice_done++;
        lock_release(mutex);    // critical section 3
        clocksleep(1);

        lock_acquire(mutex);    // critical section 4
        // siwtch turn sequence
        if (cats_wait_count == 0 && (mice_wait_count - mice_eat_count > 0)) {
                mice_in_this_turn++;    // mice again
                cv_signal(turn_cv, mutex);

        } else {
                turn_switch();          // switch to waiting cats
        }
        lock_release(mutex);    // critical section 4
        kprintf("%s leaves kitchen.\n", mousenames[mousenumber]);
        kprintf("%d mice done\n", num_mice_done);       // total mice entered count
        }
        // after looping exit program
        if (cats_wait_count == 0 && mice_wait_count == 0) {
                cv_broadcast(done_cv, mutex);
        }
}


/*
 * catmouselock()
 *
 * Arguments:
 *      int nargs: unused.
 *      char ** args: unused.
 *
 * Returns:
 *      0 on success.
 *
 * Notes:
 *      Driver code to start up catlock() and mouselock() threads.  Change
 *      this code as necessary for your solution.
 */

int
catmouselock(int nargs,
             char ** args)
{
        int index, error;
   
        /*
         * Avoid unused variable warnings.
         */

        (void) nargs;
        (void) args;

        // init variables
        all_dishes_available = TRUE;
        dish1_busy = FALSE;
        dish2_busy = FALSE;
        mydish = 0;

        cats_wait_count = 0;
        cats_in_this_turn = 0;
        cats_eat_count = 0;
        num_cats_done = 0;

        mice_wait_count = 0;
        mice_in_this_turn = 0;
        mice_eat_count = 0;
        num_mice_done = 0;

        turn_type = 0;
        mutex = lock_create("catlock mutex");
        turn_cv = cv_create("catlock turncv");
        done_cv = cv_create("catlock donecv");

        /*
         * Start NCATS catlock() threads.
         */

        for (index = 0; index < NCATS; index++) {
           
                error = thread_fork("catlock thread", 
                                    NULL, 
                                    index, 
                                    catlock, 
                                    NULL
                                    );
                
                /*
                 * panic() on error.
                 */

                if (error) {
                 
                        panic("catlock: thread_fork failed: %s\n", 
                              strerror(error)
                              );
                }
        }

        /*
         * Start NMICE mouselock() threads.
         */

        for (index = 0; index < NMICE; index++) {
   
                error = thread_fork("mouselock thread", 
                                    NULL, 
                                    index, 
                                    mouselock, 
                                    NULL
                                    );
      
                /*
                 * panic() on error.
                 */

                if (error) {
         
                        panic("mouselock: thread_fork failed: %s\n", 
                              strerror(error)
                              );
                }
        }

        lock_acquire(mutex);
        while (num_cats_done < 6 || num_mice_done < 2) {
                cv_wait(done_cv, mutex);
        }
        lock_release(mutex);
        
        // cleanup
        cv_destroy(turn_cv);
        cv_destroy(done_cv);
        lock_destroy(mutex);

        return 0;
}

/*
 * End of catlock.c
 */
