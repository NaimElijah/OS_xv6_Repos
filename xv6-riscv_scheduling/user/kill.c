#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char **argv)
{
  int i;

  if(argc < 2){
    fprintf(2, "usage: kill pid...\n");
    exit(1,"exiting - error");
  }
  for(i=1; i<argc; i++)
    kill(atoi(argv[i]));
  exit(0, "exiting");
}
