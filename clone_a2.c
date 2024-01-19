#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sched.h>
#include <sys/wait.h>
#include <stdint.h>

int global_sum = 0; // global variable to store the sum of squares

// Function to calculate the sum of squares from 0 to n
static int calcNSumOfSquare(void *arg)
{
    int n = *(int *)arg; // cast void pointer to int pointer and dereference it
    int local_sum = 0;   // local variable to store the sum of squares

    for (int i = 0; i <= n; i++) // calculate the sum of squares
    {
        local_sum += i * i; // add i * i to local_sum
    }

    sleep(2);               // sleep for 2 seconds
    global_sum = local_sum; // save local_sum in global_sum
    return 0;
}

int main()
{
    int n = 100; // as parameter for function

    // Allocate stack for child task.
    // const int STACK_SIZE = 256; // stack size is 8K because of ulimit -s
    const int STACK_SIZE = 13*16; // min size is 13*16 = 208 because malloc operates in buckets of 16 bytes 
    // if i just use the 200 bytes needed for the stack, it will override the header of the malloc in front 
    // of the stack at position stack - 8 and a size of uint64_t (8 bytes)

    // so basically malloc allocates one word more than requested and saves the size of the allocation in the first word
    // https://stackoverflow.com/a/3479496

    // "A 64 bit processor will have a 64 bit "word" size (and pointer size) so the extra word will be 8 bytes."

    unsigned char *stack;    // Start of stack buffer
    unsigned char *stackTop; // End of stack buffer

    stack = malloc(STACK_SIZE); // malloc() allocates memory in bytes
    if (!stack)                 // if malloc() failed
    {
        perror("malloc"); // print error message
        exit(1);          // exit with error code 1
    }

    // pointer of header of malloc() is 8 byte (stack - 8)
    // uint64_t *header = (uint64_t *)(stack - 8); // cast stack - 8 to uint64_t pointer 

    // fill stack with 0xAA for watermarking (with a for loop)
    for (int i = 0; i < STACK_SIZE; i++)
    {
        stack[i] = 0xAA;
    }

    unsigned long flags = 0; // flags for clone()
    flags |= CLONE_VM;       // The child task will execute in the same memory space as the parent task. (behave like a thread)
    flags |= SIGCHLD;        // The parent will be notified when the child terminates.

    stackTop = stack + STACK_SIZE; // stack grows downward

    // The child task will execute calcNSumOfSquare() and save the id of the child task in pid
    int pid = clone(calcNSumOfSquare, stackTop, flags, (void *)&n); // stack + STACK_SIZE is the top of the stack
    if (pid == -1)
    {
        perror("clone"); // print error message
        free(stack);     // free the stack
        exit(1);         // exit with error code 1
    }

    printf("pid of child: %d \n", pid); // print pid of child

    // wait with waitpid() to get the return value of the child task.
    int status;
    if (waitpid(pid, &status, 0) == -1) // wait for child fails
    {
        perror("waitpid");  // print error message
        kill(pid, SIGTERM); // handling of SIGTERM not necessary since its not handling any important data
        free(stack);        // free the stack
        exit(1);            // exit with error code 1
    }

    printf("(n = %d) sum of squares = %d\n", n, global_sum); // print the result

    // search for the first element that is not 0xAA
    // not going from last to first element because the stack is growing downwards
    // and the first element that is not 0xAA is the top of the stack
    int unused_stack = 0;
    for (int i = 0; i < STACK_SIZE; i++)
    {
        if (stack[i] != 0xAA) // if the element is not 0xAA (the watermark)
        {
            unused_stack = i; // save the index of the first element that is not 0xAA
            break;            // break the loop
        }
    }

    int used_stack_size = STACK_SIZE - unused_stack;   // calculate the used stack size (STACK_SIZE - i)
    printf("used stack: %d bytes\n", used_stack_size); // print the used stack size (STACK_SIZE - i)

    // calc used_stack_size / 16 to get the number of 16 byte words
    // calc used_stack_size % 16 to get the number of bytes that are not a 16 byte word
    // if used_stack_size % 16 != 0 then add 1 to the number of 16 byte words
    int number_of_16_byte_words = used_stack_size / 16 + (used_stack_size % 16 != 0);
    printf("number of 16 byte words: %d\n", number_of_16_byte_words); // print the number of 16 byte words

    free(stack); // free the stack
    return 0;
}
