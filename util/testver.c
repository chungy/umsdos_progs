#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include "umsdos_progs.h"
#include "umsdos_gen.h"

                     
int main(int argc, char *argv[])
{
  char *dir;
  int fd;
  
  if (argc != 2) { printf ("Usage: testver <dir>\n"); exit (99); }
  dir = argv[1];

  fd = UM_open (dir, 1, 1);
  UM_close (&fd);
  
  printf ("Testver passed!\n");
  return 0;
}
