#include <stdio.h>
#include "umsdos_gen.h"

/* #Specification: utilities / usize
	prints the sizes of umsdos_dirent struct.

	for development only
*/

static int count = 0;

void calc (char *name, void *element, void *start) 
{
	int offs = element - start;
	int size = element - (start + count);
	printf (", size=%d\n%s is offset %d", size, name, offs);
	count += size;
}


int main ()
{
	struct umsdos_dirent d;

	calc ("name_len", &d.name_len, &d);
	calc ("flags", &d.flags, &d);
	calc ("nlink", &d.nlink, &d);
	calc ("uid", &d.uid, &d);
	calc ("gid", &d.gid, &d);
	calc ("atime", &d.atime, &d);
	calc ("mtime", &d.mtime, &d);
	calc ("ctime", &d.ctime, &d);
	calc ("rdev", &d.rdev, &d);
	calc ("mode", &d.mode, &d);
	calc ("spare", &d.spare, &d);
	calc ("name", &d.name, &d);
	printf (" , size=%d\n\n sizeof(umsdos_dirent)=%d\n", sizeof(d) - count, sizeof(d) );
	return 0;
}


