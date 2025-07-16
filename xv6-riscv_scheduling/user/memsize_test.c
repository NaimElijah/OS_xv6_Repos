#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"


int main(int argc, char *argv[])
{
    printf("%d\n", memsize());
    void* mem_addition = malloc(20000);
    printf("%d\n", memsize());

    free(mem_addition);
    printf("%d\n", memsize());

    exit(0, "exiting");
}
