#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/stat.h>
#include "umsdos_test.h"

static int utstspc_doone (
	UMSDOS_REGISTER *reg,
	const char *drive,
	const char *mountpt,
	const char *moption,
	int mount_umsdos,	// mount -t umsdos or mount -t msdos
	int no_mount)
{
	int ret = -1;
	if (no_mount){
		ret = reg->call(mountpt);
	}else if (utilspc_format (drive,*reg)==0
		&& utilspc_mount (drive,mountpt,mount_umsdos,moption,*reg)==0){
		ret = reg->call(mountpt);
		if (ret == 0) ret = utilspc_umount (mountpt,*reg);
	}
	return ret;
}

/* #Specification: umsdos / automated test / specific
	utstspc.c is a sequence of test for the UMSDOS file system.
	These tests are specific to the UMSDOS file system.
*/

int main (int argc, char *argv[])
{
	int ret = -1;
	int arg1 = 1;
	extern int register_verbose;
	int count = 1;	// Number of test iteration
	/* #Specification: utstspc / default environnement

		/mn/ FIXME 990701: outdated ! new mtools.conf is much more complex! 

		utstspc needs to start from a fresh partition (it reformats it).
		So we normally use it on a floppy. utstspc do mount and umount
		of that floppy. The default mount point is /mnt and the default
		drive (for mformat) is F:.

		This value F: looks very odd. This comes from my own setup.
		Here is my definition for /etc/mtools.conf

		A /dev/fd0 12 0 0 0 
		B /dev/fd1 12 0 0 0
		E /dev/fd0h1200 12 80 2 15	# A: 5 1/4
		F /dev/fd1H1440 12 80 2 18	# B: 3 1/2

		Using /dev/fd0 and /dev/fd1 on A and B, this gives me flexibility
		for normal operation. I can read any type of floppy without much
		question. I can't do a mformat A: or B:. I always get an error.
		I guess /dev/fd0 are flexible driver, so do not impose a format.

		The entries E and F replicate A and B but this time use the specific
		device. So  I can do a "mformat f:" and expect to format a 1.44
		3 1/2 floppy correctly.

		Of course if you know better, please tell me!
	*/
	const char *mountpt = "/mnt";
	const char *drive  = "a:";
	//const char *moption = "";
	int no_mount = 0;
	while (arg1 < argc){
		const char *pt = argv[arg1];
		if (strcmp(pt,"-s")==0){
			register_verbose = 0;
			arg1++;
		#if 0
			}else if (strcmp(argv[arg1],"-a")==0){
				moption = "-o conv=auto";
				arg1++;
		#endif
		}else if (strncmp(argv[arg1],"-n",2)==0){
			count = atoi (pt+2);
			arg1++;
		}else if (strncmp(argv[arg1],"-m",2)==0){
			no_mount = 1;
			arg1++;
		}else if (strncmp(argv[arg1],"-d",2)==0){
			drive = pt + 2;
			int lendrv = strlen(drive);
			if (   (lendrv != 2 && lendrv != 1)
				|| !isalpha(drive[0])
				|| (lendrv ==2 && drive[1] != ':')){
				fprintf (stderr,"Invalid DOS drive specification : %s\n"
					,drive);
				exit(-1);
			}
			arg1++;
		}else if (strncmp(argv[arg1],"-m",2)==0){
			mountpt = pt + 2;
			arg1++;
		}else{
			break;
		}
	}
	UMSDOS_REGISTER_SPC::sort();
	if (argc == arg1){
		printf ("%s [-a] [-dDOSDRIVE ] [-m] [ -nCOUNT ] [ -mMOUNTPOINT] [-s] all | cases path\n",argv[0]);
		printf ("Default MOUNTPOINT is /mnt\n");
		printf ("Default DRIVE is A:\n");
		// printf ("-a : Select conv=auto as mount option\n");
		printf ("-m : Do not mount, use MOUNTPOINT directly\n");
		printf ("     (do not format either)\n");
		printf ("\n");
		printf ("List of available tests\n");
		UMSDOS_REGISTER_SPC *pt = UMSDOS_REGISTER_SPC::first;
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
			"Each test start with a reformat (mformat DRIVE).\n"
			"A test is made to ensure the DRIVE IS a floppy\n"
			"\n"
			"Have fun!\n"
			);
	}else{
		/* #Specification: utstspc / floppy only
			To avoid desaster, utstspc will only work on floppy.
			A test is done before everything to ensure that
			the drive is indeed a floppy.

			It use /etc/mtools.conf to locate the proper device. It also
			assume a floppy device have a path starting with
			"/dev/fd". This is not fool proof!
		*/
		if (!utilspc_isfloppy (drive)){
			fprintf (stderr
				,"Drive %s is not a floppy. To avoid any\n"
				 "desaster, the test won't be done.\n"
				,drive
				);
		}else{
			ret = 0;
			umask (0);
			for (int c=0; c<count && ret == 0; c++){
				if (argc == arg1+1 && strcmp(argv[arg1],"all")==0){
					UMSDOS_REGISTER_SPC *pt = UMSDOS_REGISTER_SPC::first;
					ret = 0;
					while (pt != NULL){
						if (register_verbose) pt->intro();
						ret |= utstspc_doone(pt,drive
							,mountpt
							,pt->mount_option,pt->mount_umsdos
							,no_mount);
						if (ret != 0) break;
						pt = pt->getnext();
					}
				}else{
					ret = 0;
					for (int i=arg1; i<argc && ret == 0; i++){
						char *path = argv[i];
						int lenpath = strlen(path);
						UMSDOS_REGISTER_SPC *pt = UMSDOS_REGISTER_SPC::first;
						ret = 0;
						while (pt != NULL){
							if (strncmp(pt->path,path,lenpath)==0){
								if (register_verbose) pt->intro();
								ret |= utstspc_doone(pt,drive,mountpt
									,pt->mount_option
									,pt->mount_umsdos
									,no_mount);
								if (ret != 0) break;
							}
							pt = pt->getnext();
						}
					}
				}
			}
		}
		if (ret != 0){
			printf ("***** There has been at least one error.\n"
				"      Don't forget to clean %s\n",mountpt);
		}else{
			printf ("***** SUCCESS *****\n");
		}
	}
	return ret;
}


