#include <stdio.h>
#include "umsdos_gen.h"

/* #Specification: utilities / udump
	udump display the content of a --linux-.--- file (EMD file).
	Simply type:

	udump file

	This utility was mainly used to debug the UMSDOS file systems.
*/

int main (int argc, char *argv[])
{
	int recsize;
	int ret = -1;
	if (argc != 2){
		fprintf (stderr,"UMSDOS EMD file dump\nudump file\n");
	}else{
		FILE *fin = fopen (argv[1],"r");
		if (fin == NULL){
			fprintf (stderr,"Can't open %s\n",argv[1]);
		}else{
			struct umsdos_dirent entry;	
			int no = 1;
			while (fread(&entry, UMSDOS_REC_SIZE, 1, fin) == 1){
				recsize = umsdos_evalrecsize (entry.name_len);
				/* Variable size record. Maybe, we have to read some more */
				if (recsize > UMSDOS_REC_SIZE) {
					fread (((char *) &entry) + UMSDOS_REC_SIZE, recsize - UMSDOS_REC_SIZE, 1, fin);
				}
			
				if (entry.name_len != 0){
					printf ("%3d - uid(%d) gid(%d) mode(%o) flags(%d) "
						"rdev(%d,%d)\n",no
						,entry.uid,entry.gid,entry.mode,entry.flags
						,entry.rdev>>8,entry.rdev&0xff);
					printf ("      atime(%lu) mtime(%lu) ctime(%lu) nlink(%d)\n"
						, (unsigned long) entry.atime, (unsigned long) entry.mtime, (unsigned long) entry.ctime
						,entry.nlink);
					printf ("      %*.*s\n",entry.name_len,entry.name_len
						,entry.name);
				}
				no++;
			}
			ret = 0;
			fclose (fin);
		}
	}
	return ret;
}



