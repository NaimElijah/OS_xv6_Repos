// Peterson locks
#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
// #include "memlayout.h"   //  not needed here
#include "spinlock.h"
#include "proc.h"
#include "petersonlock.h"

struct petersonlock petlocks[NPETLOCKS];   //*  The peterson locks array, for the NPETLOCKS(now 15) processes.   <<------------------

// initialize the peterson locks array.
void petersoninit(void)
{
  struct petersonlock *pl;
  for(pl = petlocks; pl < &petlocks[NPETLOCKS]; pl++) {
    pl->locked = -1;   //  initialized to -1 to indicate -1 = unused lock.
    pl->turn = -1;     // also initialized to -1 Even Though the -1 in locked already tells us its unused.
    pl->interested[0] = 0;
    pl->interested[1] = 0;

    // For debugging:
    pl->twospids[0] = -1;
    pl->twospids[1] = -1;
    pl->holdingPid = -1;
  }
}





int peterson_create()
{
    // Find an unused lock in the petlocks array.
    int lock_idOfFreeLock = -1;
    struct petersonlock *pl;
    for(pl = petlocks; pl < &petlocks[NPETLOCKS]; pl++) {
      __sync_synchronize();
      if (pl->locked == -1) {   //  -1 = unused lock.
          __sync_lock_test_and_set(&pl->locked, 0);    //  set the lock to 0 already, indicating that we want to use it, saving it already.
          __sync_synchronize();
          lock_idOfFreeLock = pl - petlocks;   //  get the index of the lock in the petlocks array.
          break;
      }
    }

    if (lock_idOfFreeLock != -1) {
        __sync_synchronize();
        __sync_lock_test_and_set(&pl->locked, 0);   //  set the lock to 0, indicating it is now in use, already done in the loop but still.
        __sync_synchronize();
        __sync_lock_test_and_set(&pl->turn, -1);    //  set the turn to -1, indicating that no process set the turn yet, because no one is trying to hold it yet.
        __sync_synchronize();
        __sync_lock_test_and_set(&pl->interested[0], 0);  //  indicating no process is interested in the lock yet.
        __sync_synchronize();
        __sync_lock_test_and_set(&pl->interested[1], 0);  //  indicating no process is interested in the lock yet.
        __sync_synchronize();

        // For debugging:
        __sync_lock_test_and_set(&pl->twospids[0], -1);  //  when a process of role(0/1) is interested in the lock, it will set its pid in this array in the peterson_acquire() function.
        __sync_synchronize();
        __sync_lock_test_and_set(&pl->twospids[1], -1);  //  when a process of role(0/1) is interested in the lock, it will set its pid in this array in the peterson_acquire() function.
        __sync_synchronize();
        __sync_lock_test_and_set(&pl->holdingPid, -1);   //  when a process of role(0/1) is interested in the lock, it will set its pid in this variable in the peterson_acquire() function.
        __sync_synchronize();
    }

    __sync_synchronize();
    return lock_idOfFreeLock;   // Return the index of the lock in the petlocks array.
}









int peterson_acquire(int lock_id, int role)
{
    // Check if the lock_id is valid.
    if (lock_id < 0 || lock_id >= NPETLOCKS || role < 0 || role > 1) {
        return -1;   // Invalid lock ID
    }
    struct petersonlock *pl = &petlocks[lock_id];
    __sync_synchronize();
    if (pl->locked == -1) {
        return -1;    // Lock is unused and wasn't created, can't acquire it.
    }
    __sync_synchronize();
    if (pl->holdingPid == myproc()->pid)
    {
        return -1;  //  because the current process already has the lock.
    }
    

    // The Algorithm:
    __sync_synchronize();
    __sync_lock_test_and_set(&pl->interested[role], 1);   //  Set the interested flag for the current process.
    __sync_synchronize();
    __sync_lock_test_and_set(&pl->turn, role);   //  Set the turn to the current process's role.
    __sync_synchronize();
    while(pl->interested[1 - role] == 1 && pl->turn == role){
        __sync_synchronize();
        if (pl->locked == -1) {
            return -1;    // Lock was destroyed, can't acquire it.
        }
        yield();   //  yield() until the other process is not interested or it's not their turn.
    }
    
    //  At this point, the current process is about to acquire the lock and enter the CS afterwards.

    __sync_synchronize();
    __sync_lock_test_and_set(&pl->locked, 1);   //  Set the lock to 1, indicating that it is now held.
    __sync_synchronize();
    __sync_lock_test_and_set(&pl->holdingPid, myproc()->pid);   //  Set the holdingPid to the current process's pid.
    __sync_synchronize();
    __sync_lock_test_and_set(&pl->twospids[role], myproc()->pid);   //  Set the twospids[role] to the current process's pid.
    __sync_synchronize();
    return 0;   // Successfully acquired the lock.
}









int peterson_release(int lock_id, int role)
{
    // Check if the lock_id is valid.
    if (lock_id < 0 || lock_id >= NPETLOCKS || role < 0 || role > 1) {
        return -1;   // Invalid lock ID
    }
    struct petersonlock *pl = &petlocks[lock_id];
    __sync_synchronize();
    if (pl->locked == -1) {
        return -1;    // Lock is unused, can't release it.
    }
    __sync_synchronize();
    if (pl->holdingPid != myproc()->pid)
    {
        return -1;  //  because the current process doesnt have the lock so it cannot release it.
    }
    
    // the Algorithm
    __sync_synchronize();
    __sync_lock_test_and_set(&pl->holdingPid, -1);   //  Set the holdingPid to -1, indicating that no process is holding the lock.
    __sync_synchronize();
    __sync_lock_test_and_set(&pl->twospids[role], -1);   //  Set the twospids[role] to -1, indicating that no process is interested in the lock.
    __sync_synchronize();
    __sync_lock_test_and_set(&pl->locked, 0);   //  Set the lock to 0, indicating that it is now unused.
    __sync_synchronize();
    __sync_lock_test_and_set(&pl->interested[role], 0);   //  Set the interested flag for the current process to 0.
    __sync_synchronize();   //  just synchronize to make sure all the operations are done before returning.
    return 0;   // Successfully released the lock.
}








int peterson_destroy(int lock_id)
{
    // Check if the lock_id is valid.
    if (lock_id < 0 || lock_id >= NPETLOCKS) {
        return -1;   // Invalid lock ID
    }
    struct petersonlock *pl = &petlocks[lock_id];
    __sync_synchronize();
    if (pl->locked == 1 || pl->locked == -1) {
        return -1;    // Lock is held by a process or destoryed, can't destroy it.
    }
    
    // Reset the lock.  set its locked value to -1(unused) and reset to -1 the other variables in the struct as well.
    __sync_synchronize();
    __sync_lock_test_and_set(&pl->locked, -1);   //  set the lock to -1, indicating that it is unused.
    __sync_synchronize();
    __sync_lock_test_and_set(&pl->turn, -1);    //  set the turn to -1, indicating that no process set the turn yet, because no one is trying to hold it yet.
    __sync_synchronize();
    __sync_lock_test_and_set(&pl->interested[0], 0);  //  indicating no process is interested in the lock yet.
    __sync_synchronize();
    __sync_lock_test_and_set(&pl->interested[1], 0);  //  indicating no process is interested in the lock yet.
    __sync_synchronize();
    __sync_lock_test_and_set(&pl->twospids[0], -1);   //  when a process of role(0/1) is interested in the lock, it will set its pid in this array in the peterson_acquire() function.
    __sync_synchronize();
    __sync_lock_test_and_set(&pl->twospids[1], -1);   //  when a process of role(0/1) is interested in the lock, it will set its pid in this array in the peterson_acquire() function.
    __sync_synchronize();
    __sync_lock_test_and_set(&pl->holdingPid, -1);   //  when a process of role(0/1) is interested in the lock, it will set its pid in this variable in the peterson_acquire() function.
    __sync_synchronize();
    return 0;
}



