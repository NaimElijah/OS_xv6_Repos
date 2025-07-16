#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char **argv)
{
    if (argc != 2)
    {
        fprintf(2, "Error: Invalid number of arguments. Usage: %s <number_of_processes>\n", argv[0]);
        exit(1);
    }
    int tournamentID = tournament_create(atoi(argv[1]));
    if (tournamentID < 0)
    {
        fprintf(2, "Error: Failed to initiate the Tournament.\n");
        exit(1);
    }

    if (tournamentID == 99)   //  the parent process will get 99 and we only care about the his children.
    {
        exit(0);   //  Parent process exits after creating the tournament tree.     after the parent exits, the children will reparent so all good.
    }

    if (tournament_acquire() < 0)   //  each process that was created/forked will call this function to acquire the lock.
    {
        exit(1);   //  Failed to acquire the lock.
        //  prints are done in the tournament_acquire function.
    }

    //* This is the Critical Section.   <<------
    //* This is the Critical Section.   <<------
    printf("Process with PID: %d & Tournament ID: %d, acquired the lock and is in the Critical Section.\n", getpid(), tournamentID);  //* This is the Critical Section.   <<------
    //* This is the Critical Section.   <<------
    //* This is the Critical Section.   <<------

    if(tournament_release() < 0)   //  each process that was created/forked will call this function to release the lock.
    {
        exit(1);
        //  prints are done in the tournament_release function.
    }

    exit(0);
}
