#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"


int main(int argc, char *argv[])
{
    // Initializing the array of numbers:
    int size = 65536;   //  65536 = 2^16 integers
    int *array_of_numbers = (int *) malloc(size * sizeof(int));  // Allocate memory for 65536 integers = 2^16 integers
    for(int i = 0; i < size; i++)
    {
        array_of_numbers[i] = i;
    }


    int n = 4; // Number of children to create    <<---  like stated in the assignment, forkn will check if the number of children is 1...16 or not   <<----
    int *number_of_children_waited_for = (int*)malloc(sizeof(int)); // Allocate memory for 1 integer
    *number_of_children_waited_for = 0; // Initialize to 0

    int *pids = (int*) malloc(n*sizeof(int)); // Allocate memory for 4 integers
    memset(pids, 0, n * sizeof(int)); // Initialize all elements to 0

    int *statuses = (int*) malloc(n*sizeof(int)); // Allocate memory for 4 integers
    memset(statuses, 0, n * sizeof(int)); // Initialize all elements to 0


    int proc_num = forkn(n, pids);            //* forkn here

    if (proc_num == -1)  // check that the call to forkn was successful by checking the return value
    {
        printf("Error: forkn failed\n");  //   If the fork operation fails, the parent process should print an error message and exit
        exit(1,"");
    }
    
    int size_per_kid = size/n;

    if (proc_num == 0)    //  parent's code
    {
        if (waitall(number_of_children_waited_for, statuses) < 0)                 //*  waitall here
        {
            printf("waitall failed\n");
            exit(1, "");
        }

        if(*number_of_children_waited_for != n)
        {
            printf("Warning: number of children waited for is not equal to the number of children created\n");
        }
        printf("number_of_children_waited_for is %d, number of children created is %d\n", *number_of_children_waited_for, n);
        
        

        for (int i = 0; i < n; i++)  // In the parent, print the PIDs of the child processes
        {
            printf("Child #%d PID: %d\n",i+1 ,pids[i]);
        }

        
        int sum_of_sums = 0;
        for (int i = 0; i < n; i++)
        {
            sum_of_sums += statuses[i];
        }
        
        printf("Parent: waited for %d children and their sums, the ---->> SUM OF SUMS <<---- is: %d\n", *number_of_children_waited_for, sum_of_sums);

        free(array_of_numbers);  // Free the memory allocated for the array of numbers
        free(pids);  // Free the memory allocated for the pids
        free(statuses);  // Free the memory allocated for the statuses
        free(number_of_children_waited_for);  // Free the memory allocated for the number of children waited for

        exit(0, "exiting from parent\n");  // exit with the sum of the parent
    }
    else       // children's code
    {
        int temp = 0;
        if(proc_num == n)    // last child
        {
            temp += size % n;
        }

        int sum_of_child = 0;


        for (int i = (size_per_kid*(proc_num-1)) ; i < (size_per_kid*(proc_num) + temp) ; i++)
        {
            sum_of_child += array_of_numbers[i];
        }

        printf("Child #%d (with subarray #%d): got sum = %d\n", proc_num, proc_num, sum_of_child); // Each child will calculate the sum in its part/quarter of the array and print it

        free(array_of_numbers);  // Free the memory allocated for the array of numbers
        free(pids);  // Free the memory allocated for the pids
        free(statuses);  // Free the memory allocated for the statuses
        free(number_of_children_waited_for);  // Free the memory allocated for the number of children waited for

        exit(sum_of_child, "");   // exit with the sum of the child
    }



    free(array_of_numbers);  // Free the memory allocated for the array of numbers
    free(pids);  // Free the memory allocated for the pids
    free(statuses);  // Free the memory allocated for the statuses
    free(number_of_children_waited_for);  // Free the memory allocated for the number of children waited for
    
    exit(0, "exiting");
}
