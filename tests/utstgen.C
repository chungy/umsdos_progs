#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include "umsdos_test.h"

/* #Specification: umsdos / automated test / general
	utstgen.c is a sequence of test for the UMSDOS file system.
	These test are not really specific to the UMSDOS file system.
	You will find extensive testing of some stuff which are specific
	to the UMSDOS file system. There is a long section on hard link
	which could hardly fail on a normal UNIX file system and were
	a nightmare to implement in UMSDOS.
*/

int main (int argc, char *argv[])
{
	int ret = -1;
	int arg1 = 1;
	extern int register_verbose;
	int count = 1;	// Number of test iteration
	const char *basepath = "/mnt";
	while (arg1 < argc){
		const char *pt = argv[arg1];
		if (strcmp(pt,"-s")==0){
			register_verbose = 0;
			arg1++;
		}else if (strncmp(argv[arg1],"-n",2)==0){
			count = atoi (pt+2);
			arg1++;
		}else if (strncmp(argv[arg1],"-p",2)==0){
			basepath = pt + 2;
			arg1++;
		}else{
			break;
		}
	}
	UMSDOS_REGISTER_GEN::sort();
	if (argc == arg1){
		printf ("%s [ -nCOUNT ] [ -pBASEPATH ] [-s] all | cases path\n",argv[0]);
		printf ("Default BASEPATH is /mnt\n");
		printf ("\n");
		printf ("List of available tests\n");
		UMSDOS_REGISTER_GEN *pt = UMSDOS_REGISTER_GEN::first;
		while (pt != NULL){
			printf ("%s: %s\n",pt->path,pt->desc);
			pt = pt->getnext();
		}
		printf (
			"\n"
			"You may test all those cases or a sample\n"
			"by specifying some argument. Those arguments\n"
			"must be taken from the preceding list. You may\n"
			"used partial path to test all cases under that path\n"
			"\n"
			"Have fun!\n"
			);
	}else{
		ret = 0;
		umask (0);
		for (int c=0; c<count && ret == 0; c++){
			if (argc == arg1+1 && strcmp(argv[arg1],"all")==0){
				UMSDOS_REGISTER_GEN *pt = UMSDOS_REGISTER_GEN::first;
				ret = 0;
				while (pt != NULL){
					if (register_verbose) pt->intro();
					ret |= pt->call(basepath);
					if (ret != 0) break;
					pt = pt->getnext();
				}
			}else{
				ret = 0;
				for (int i=arg1; i<argc && ret == 0; i++){
					char *path = argv[i];
					int lenpath = strlen(path);
					UMSDOS_REGISTER_GEN *pt
						= UMSDOS_REGISTER_GEN::first;
					ret = 0;
					while (pt != NULL){
						if (strncmp(pt->path,path,lenpath)==0){
							if (register_verbose) pt->intro();
							ret |= pt->call(basepath);
							if (ret != 0) break;
						}
						pt = pt->getnext();
					}
				}
			}
		}
		if (ret != 0){
			printf ("***** There has been at least one error.\n"
				"      Don't forget to clean %s\n",basepath);
		}else{
			printf ("***** SUCCESS *****\n");
		}
	}
	return ret;
}


