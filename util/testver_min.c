#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include "umsdos_gen.h"


                     
int main(int argc, char *argv[])
{
  int fd;
  struct umsdos_ioctl data;
  char *dir;
  
  if (argc != 2) { printf ("Usage: testver <dir>\n"); exit (99); }
  dir = argv[1];
  
  if ((fd=open(dir, O_RDONLY)) < 0) { perror ("can't open directory"); exit (1); }
  memset (&data, 0, sizeof (struct umsdos_ioctl));
  if (ioctl (fd, UMSDOS_GETVERSION, &data) < 0) { perror ("ioctl failed"); exit (2); }
  close (fd);
  printf ("version=%d, release=%d\n", (int)data.version, (int)data.release);
  return 0;
}