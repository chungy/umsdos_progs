#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "umsdos_progs.h"
#include "umsdos_gen.h"


int main (int argc, char *argv[])
{
	int ret = -1;
	char *progname;
	if ((progname = strrchr(argv[0], '/'))){
		++progname;
	}else{
		progname = argv[0];
	}

	if (strcmp(progname,SYNC_NAME)==0){
		ret = umssync_main(argc,argv);
	}else{
		/* #Specification: umsdos utils / setuid root
			umssync is designed to be run (optionnally) setuid root.
			The other utilities (umssetup and udosctl) must be run
			by root. There is no interest in providing this
			kind of capabilities to normal users.

			To save some disk space, the 3 apps are merge into
			a single one and 2 symlinks are done at install time.
			The argv[0] is tested to find which personnality
			the application should have. If it is not umssync, the
			setuid root is automaticly void like this.

			seteuid(getuid());
		*/
		seteuid(getuid());
		if (strcmp(progname,DOSCTL_NAME)==0){
			ret = udosctl_main(argc,argv);
		}else if (strcmp(progname,SETUP_NAME)==0){
			ret = umssetup_main(argc,argv);
		}else{
			fprintf (stderr,"%s ???\n"
				"Who am I. The program name must be\n"
				SYNC_NAME", "SETUP_NAME" or "DOSCTL_NAME".\n"
				"Please don't rename it.\n",argv[0]);
		}
	}
	return ret;
}

