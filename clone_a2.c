#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sched.h>
#include <sys/wait.h>

int global_sum = 0; // global variable to store the sum of squares 

// Function to calculate the sum of squares from 0 to n
static int calcNSumOfSquare(void *arg)
{
    int n = *(int *)arg;

    int local_sum = 0;

    for (int i = 0; i <= n; i++)
    {
        local_sum += i * i;
    }

    sleep(2);

    global_sum = local_sum;

    return 0;
}

int main()
{
    int n = 100; // as parameter for function

    // Allocate stack for child task
    const int STACK_SIZE = 8192; // stack size is 8K because of ulimit -s

    unsigned char *stack = malloc(STACK_SIZE); // malloc() allocates memory in bytes
    if (!stack) // if malloc() failed
    {
        perror("malloc");
        exit(1);
    }

    // fill stack with 0xAA for watermarking (with a for loop)
    for (int i = 0; i < STACK_SIZE; i++)
    {
        stack[i] = 0xAA;
    }

    unsigned long flags = 0; // flags for clone()
    flags |= CLONE_VM; // The child task will execute in the same memory space as the parent task. (behave like a thread)
    flags |= SIGCHLD;  // The parent will be notified when the child terminates.

    // The child task will execute calcNSumOfSquare() and save the id of the child task in pid
    int pid;
    if ((pid = clone(calcNSumOfSquare, stack + STACK_SIZE, flags, (void *)&n)) == -1) //stack + STACK_SIZE is the top of the stack
    {
        perror("clone");
        free(stack); // free the stack
        exit(1);
    }

    printf("pid of child: %d \n", pid); //print pid of child

    // wait with waitpid() and WEXITSTATUS() to get the return value of the child task.
    int status;
    if (waitpid(pid, &status, 0) == -1)
    {
        perror("waitpid");
        kill(pid, SIGTERM); // handling of SIGTERM not necessary since its not handling any important data
        free(stack);        // free the stack
        exit(1);
    }

    printf("(n = %d) sum of squares = %d\n", n, global_sum); // print the result

    //search for the first element that is not 0xAA
    //not going from last to first element because the stack is growing downwards
    //and the first element that is not 0xAA is the top of the stack
    int unused_stack = 0;
    for (int i = 0; i < STACK_SIZE; i++)
    {
        if (stack[i] != 0xAA)
        {
            unused_stack = i;
            break;
        }
    }

    printf("used stack: %d\n", STACK_SIZE - unused_stack);  // print the used stack size (STACK_SIZE - i) 

    free(stack); // free the stack
    
    return 0;
}
