#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/petersonlock.h"

// THIS IS THE LIBRARY OF THE TOURNAMENT TREE FUNCTIONS

int *petelocksIDsArray;       // Global Array for the Peterson Locks IDs    //  not really needed because the lock at place i is with lock_id i, but we can still use this array.
int *forkedProcsIndexesPIDs;  // Global Array for the Forked Processes Indexes/Tournament IDs & PIDs(PIDs inside the indexes).

int processesAmount = 0;      // Number of processes in the tournament tree.
int numOfPeteLocks = 0;       // Number of locks needed for the tournament tree.  (= processesAmount - 1)
int levelsAmount = 0;         //  Number of levels in the tournament tree. (for example: 3 levels = 0, 1, 2)


int tournament_create(int processes)
{
    if (processes == 1)
    {
        processesAmount = 1;  //  Number of processes in the tournament tree.
        return 0;   //  No need to create a tournament tree for only one process.
    }
    if (processes != 2 && processes != 4 && processes != 8 && processes != 16)
    {
        fprintf(2, "Error: Invalid number of processes. Must be 2, 4, 8, or 16.\n");
        return -1;
    }

    processesAmount = processes;  //  Number of processes in the tournament tree.
    levelsAmount = processes == 2 ? 1 : processes == 4 ? 2 : processes == 8 ? 3 : 4;   //  Number of levels in the tournament tree. (for example: 3 levels = 0, 1, 2)

    numOfPeteLocks = processes - 1;  // Number of locks needed for the tournament tree
    petelocksIDsArray = malloc(numOfPeteLocks * sizeof(int));    //  for the IDs of the locks we will create
    if (petelocksIDsArray == 0)
    {
        fprintf(2, "Error: Memory allocation failed for peterson locks IDs array.\n");
        return -1;
    }
    for (int i = 0; i < numOfPeteLocks; i++)
    {
        petelocksIDsArray[i] = peterson_create();   //  create the locks and save their IDs in the array
        if (petelocksIDsArray[i] < 0)
        {
            fprintf(2, "Error: Failed to create a Peterson lock.\n");
            free(petelocksIDsArray);  // Free the previously allocated memory if lock creation fails
            return -1;
        }
    }
    //* ---->>  Now we have created the peterson locks needed for the tournament tree, and the IDs of the locks are saved in the petelocksIDsArray.


    forkedProcsIndexesPIDs = malloc(processes * sizeof(int));  //  for the forked processes indexes
    if (forkedProcsIndexesPIDs == 0)
    {
        fprintf(2, "Error: Memory allocation failed.\n");
        free(petelocksIDsArray);  // Free the previously allocated memory if this second malloc fails
        return -1;
    }

    // Create the processes               HERE EACH PROCESS IS CREATED AND WILL GO ITS OWN WAY
    for (int i = 0; i < processes; i++)
    {
        int pid = fork();
        if (pid == 0)
        {   //  in child process i
            forkedProcsIndexesPIDs[i] = getpid();  // Save the process ID in the forkedProcsIndexesPIDs array, that index is now that process's index in the tournament tree.
            return i;  // Child process returns its tournament index/ID.
        }
        else if (pid < 0)
        {
            fprintf(2, "Error: Fork failed.\n");
            free(forkedProcsIndexesPIDs);  // Free the previously allocated memory if fork fails.
            free(petelocksIDsArray);  // Free the previously allocated memory for locks IDs if fork fails.
            return -1;
        }
    }
    //* ---->>  Now we have the forked processes PIDs in the forkedProcsIndexesPIDs array, and also, each process has now got its own index in the tournament tree(=index in PID array).
    return 99;   // Successfully created the tournament tree and forked the processes, the parent gets 99, the children get their Tournament indexes/IDs.
}









int tournament_acquire()
{
    if (processesAmount == 1) { return 0; }   //  No need to acquire a lock for only one process, that lock wasn't even created because only 1 process.
    // We are now process number i in the tournament tree.
    int currProcessTournamentIDIndex = -1;
    for (int i = 0; i < processesAmount; i++)
    {
        if (forkedProcsIndexesPIDs[i] == getpid())
        {
            currProcessTournamentIDIndex = i;   //  get the index of the process in the tournament tree.
            break;
        }
    }
    if (currProcessTournamentIDIndex == -1)
    {
        fprintf(2, "Error(in tournament_acquire): Process ID not found in the tournament tree.\n");
        return -1;
    }
    
    int role = currProcessTournamentIDIndex % 2;   //  get the role of the process in the starting lock.
    int lock_id = (numOfPeteLocks - 1) - (currProcessTournamentIDIndex / 2);   //  get the initial lock id in which the current process will partake.

    for (int i = 0; i < levelsAmount; i++)
    {
        if (peterson_acquire(lock_id, role) < 0)   //  acquire the lock for the current process from the current lock.
        {
            fprintf(2, "Error(in tournament_acquire in peterson_acquire): Failed to acquire Peterson lock.\n");
            return -1;
        }

        //* Here we have acquired the lock for the current lock in this level, for the current process.
        
        //*  end of loop/level, moving up a level in the tournament tree.
        currProcessTournamentIDIndex = currProcessTournamentIDIndex / 2;   //  get the index of the process for the next level/iteration of the tournament tree.
        role = currProcessTournamentIDIndex % 2;   //  get the role of the process for the next level/iteration of the tournament tree.
        lock_id = (lock_id - 1) / 2 ;   //  get the lock id in which the current process will partake for the next level/iteration of the tournament tree.
    }
    
    //* Here we have acquired the root lock, for the current process, and the current process will now enter the Critical Section.
    return 0;   // Successfully acquired the root lock.
}











 int tournament_release()
{
    if (processesAmount == 1) { return 0; }   //  No need to release a lock for only one process, that lock wasn't even created because only 1 process.
    // We are now process number i in the tournament tree.
    int currProcessTournamentIDIndex = -1;
    for (int i = 0; i < processesAmount; i++)
    {
        if (forkedProcsIndexesPIDs[i] == getpid())
        {
            currProcessTournamentIDIndex = i;   //  get the index of the process in the tournament tree.
            break;
        }
    }
    if (currProcessTournamentIDIndex == -1)
    {
        fprintf(2, "Error(in tournament_release): Process ID not found in the tournament tree.\n");
        return -1;
    }
    
    //*  HERE WE ARE IN THE PROCESS THAT HAS THE ROOT LOCK AND OTHER LOCKS BENEATH IT AND WANT TO RELEASE THEM ALL IN THE OPPOSITE ORDER OF ACQUISITION.    <<<---------------

    int role = currProcessTournamentIDIndex % 2;   //  get the role of the process in the starting lock.
    int lock_id = (numOfPeteLocks - 1) - (currProcessTournamentIDIndex / 2);   //  get the initial lock id in which the current process will partake.

    // put all this processe's lock IDs he went through in an array and release in the correct order.(from lock_id = 0 onwards...)
    int currPetelocksIDsArray[levelsAmount];    //  for the IDs of the locks this process went through

    // put all this processe's roles he went through in an array and release in the correct order. (0s and 1s he was throughout the locks he acquired)
    int roleInThesePeteLocksArray[levelsAmount];   //  for the roles of the process in these locks (each element here is 0 or 1)

    for (int i = 0; i < levelsAmount; i++)
    {
        currPetelocksIDsArray[i] = lock_id;   //  save the current lock id in the array
        roleInThesePeteLocksArray[i] = role;   //  save the role in the current lock id in the array

        //  end of loop/level, moving up a level in the tournament tree.
        currProcessTournamentIDIndex = currProcessTournamentIDIndex / 2;   //  get the index of the process for the next level/iteration of the tournament tree.
        role = currProcessTournamentIDIndex % 2;   //  get the role of the process for the next level/iteration of the tournament tree.
        lock_id = (lock_id - 1) / 2 ;   //  get the lock id in which the current process will partake for the next level/iteration of the tournament tree.
    }
    
    for (int i = levelsAmount; i > 0; i--)     //  now releasing the locks this process acquired, in reversed order.
    {
        if (peterson_release(currPetelocksIDsArray[i-1], roleInThesePeteLocksArray[i-1]) < 0)   //  release the lock for the current process from the current lock.
        {
            fprintf(2, "Error(in tournament_release in peterson_release): Failed to release Peterson lock.\n");
            return -1;
        }
        //* Here we have released the lock for the current lock in this level, for the current process.
    }

    //* Here we have released all the locks that the current process acquired in the tournament tree, and the current process has now exited the Critical Section.
    return 0;   // Successfully released all locks.
}





