#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <pwd.h>
#include <grp.h>
#include "umsdos_progs.h"
#include "umsdos_gen.h"

static void usage()
{
	PROG_ID("umssetup");
	fprintf (stderr,
"umssetup [ options ] dir_path\n"
"\n"
"\tSet the default permissions and owner of files in DOS\n"
"\tdirectories. UMSDOS directory are not affected.\n"
"\n"
" -ggroup : group id for entry creation\n"
" -mmode  : Permission bits (in octal)\n"
"           (Set the default mode, not the umask)\n"
" -uuser  : user id for entry creation\n"
"\n"
"dir_path is normally a mount point. It does not matter.Note that the change\n"
"apply to all DOS directories (and files in those) of the same partition\n"
"as dir_path.\n"
"\n"
"This utility is normally used on the root partition at boot time, since\n"
"it is not possible to pass mount option. When mounting other umsdos\n"
"partitions, you can use all msdos option (see mount(8)) such as\n"
"uid=,gid=,umask=\n"
"\n"
"*** This utility may be used at any time but its effect may be delayed\n"
"    because of the some kernel internal caching (inode). Better use it\n"
"    immediatly after the mount. If you need to play often with permissions\n"
"    and owner, use umssync to promote directory to full Unix capabilities.\n"
);
}

int umssetup_main (int argc, char *argv[])
{
	int ret = -1;
	if (argc == 1){
		usage();
	}else{
		int uid = 0;
		int gid = 0;
		mode_t mode = 0666;
		int i;
		ret = 0;
		for (i=1; i<argc; i++){
			char *arg = argv[i];
			if (arg[0] != '-'){
				break;
			}else{
				const char *parm = arg+2;
				if (arg[2] == '\0'){
					i++;
					if (i==argc){
						fprintf(stderr,"Incomplete option %s\n",arg);
						ret = -1;
						break;
					}
					parm = argv[i];
				}
				if (arg[1] == 'g'){
					if (isdigit(parm[0])){
						gid = xtoi(parm,&ret);
					}else{
						struct group *pt = getgrnam (parm);
						if (pt == NULL){
							fprintf (stderr,"Invalid option -g : "
								"Group %s unknown\n",parm);
							ret = -1;
						}else{
							gid = pt->gr_gid;
						}
					}
				}else if (arg[1] == 'm'){
					mode = xtoi(parm,&ret);
				}else if (arg[1] == 'u'){
					if (isdigit(parm[0])){
						uid = xtoi(parm,&ret);
					}else{
						struct passwd *pt = getpwnam (parm);
						if (pt == NULL){
							fprintf (stderr,"Invalid option -u : "
								"User %s unknown\n",parm);
							ret = -1;
						}else{
							uid = pt->pw_uid;
						}
					}
				}else{
					fprintf (stderr,"Invalid option %s\n",arg);
					ret = -1;
				}
			}
		}
		if (ret != -1){
			if (i == argc){
				fprintf (stderr,"You must specify a mount point\n");
				ret = -1;
			}else{
				int fd;
				fd = UM_open (argv[i],1,1);
				ret = UM_dossetup (fd, uid,gid,mode);
				UM_close (&fd);
			}
		}
	}
	return ret;
}

