/* #Specification: utility / synchroniser
	The UMSDOS synchroniser (umssync) make sure that the EMD file
	is in sync with the MSDOS directory. File created during a DOS
	session should be add to the EMD. File removed should erased
	from the EMD.

	The UMSDOS file system will operate normally even if the
	system is out of sync. However, files will be missing from
	directory search, creating an annoying feeling.

	There is no easy way this kind of update may be achieved by
	UMSDOS transparently. Here are the reason:

	This process take some time for each directory. If there were some
	access time in MSDOS for directories, then, based on boot time, it would
	be possible to do it once per directory. It is not the case.

	When a file is discover in MSDOS which does not exist in the EMD, we
	need some directives to properly map the file. At least the owner must
	be known.

	A set of ioctl are available (wrapper interface in
	umsdos_progs/umsdosio.c) to allow independant manipulation of the EMD
	and the DOS directory.

	A utility is provided. It should be run from /etc/rc.
	A man page (umssync.8) describe its options.
*/
#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <pwd.h>
#include <grp.h>
#include <sys/stat.h>
#include <limits.h>
#include <errno.h>
#include "ums_config.h"
#include "umsdos_progs.h"
#include "umsdos_gen.h"

extern int UM_verbose;

struct UMSSYNC_OPT {
	int uid;
	int gid;
	unsigned dir_mode;
	unsigned fil_mode;
	int inherit;
	int verbose;
	int condit;
	int full_recurse;
	int debug;
};

struct UMSSYNC_INFO{
	struct umsdos_dirent uent;	/* Entry in EMD */
	struct dirent ment;		/* Corresponding mangle name */
};


static void path_make (
	const char *dir_path,
	const char *fil_name,
	char *fpath)
{
	if (dir_path[0] == '/' && dir_path[1] == '\0'){
		sprintf (fpath,"/%s",fil_name);
	}else{
		sprintf (fpath,"%s/%s",dir_path,fil_name);
	}
}


/*
	Create a file in the EMD.
	The file already exist in DOS.

	Return -1 if error.
*/
static int umssync_create (
	int UM_fd,
	const char *path,
	struct UMSSYNC_OPT *opt,
	char *name,			/* If name is a mangled name */
					/* it will be renamed */
	int *largest)			/* Key to sync mangled file name */
{
	char fpath[PATH_MAX];
	struct umsdos_info info;
	struct stat fstat;
	int ret = -1;

			
	path_make (path,name,fpath);
	umsdos_parse (name,strlen(name),&info);

	if (opt->debug > 2) printf (" /mn/ parse %s fake=%s (%d/%d) entry=%s (%d/%d)\n", fpath, info.fake.fname, strlen(info.fake.fname), info.fake.len, info.entry.name, strlen(info.entry.name), info.entry.name_len);

#if 1
	/* special case: VFAT allows longer entries than UVFAT ! (259 against 219) */
	if (strlen(name) >= UMSDOS_MAXNAME) {
		printf ("File %s too long (%d >= %d), mangling\n", name, strlen(name), UMSDOS_MAXNAME);

		if (opt->debug > 2) printf ("umssync_create toolong: %s (%d)\n", info.entry.name, info.entry.name_len);
		ret = UM_dosstat (UM_fd, name, &fstat);
//printf ("   /mn/ dosstat ret=%d\n", ret);
		if (ret != -1){
			mode_t mode;
			const char *msgtype;
			if (S_ISDIR(fstat.st_mode)){
				mode = opt->dir_mode|S_IFDIR;
				msgtype = "dir";
			}else{
				mode = opt->fil_mode|S_IFREG;
				msgtype = "file";
			}
			if (opt->verbose) printf ("Creating %s %s (%d)\n",msgtype, info.entry.name, info.entry.name_len);
//			sleep (40);
			ret = -EIO;
			if (UM_dosrename (UM_fd, name, info.entry.name)!=-1){
				ret = UM_create (UM_fd, info.entry.name,mode,fstat.st_mtime,fstat.st_mtime
					,fstat.st_mtime,opt->uid,opt->gid,0);
			}
		}

		return ret;

	}
#endif	

#if BE_UVFAT
#ifdef FS_UVFAT_MANGLE_ALL
	info.msdos_reject = 0;
#endif
	name = info.entry.name;
	if (0){
#else
	if (strchr(name,' ')!=NULL){
#endif
		/* #Specification: umssync / special system file
			One glitch in the msdos fs (the linux driver) is that it
			shows system and hidden file. This is useless. OS/2 have
			some special hidden file for extended attribute. These
			file are hidden and have a space in the name. This
			cause some problem to the msdos fs (they appear but
			can't be access !). umssync try to promote these
			file by renaming them. bad!!!

			To avoid this, umssync will silently ignore those file.
			Hopefully the msdos fs will be fixed to hide to special file
			anyway.
		*/
		ret = 0;
	}else if (info.msdos_reject == 1){
		/* #Specification: umssync / mangled name
			If a DOS file is missing from the EMD, it is added. If
			the file has an extension with the first character
			being a member of the restricted set for mangling,
			the operation can't be done. See below (why all this ?)

			So the file will be renamed both in DOS and in
			the EMD to a valid DOS name. The base name will be kept
			but the extension will be changed to something valid, but
			less common. Umssync will try $01. If it already exist, then
			it will try $02 and so on. Normally, the user will have
			to give that file the original name (lost in a crash I guess).

			Why all this ?

			To synchronise back into the EMD, the file must be renamed.
			If one try to create a such a file with umsdos, it is
			automaticly mangled, producing a different file name in DOS.

			This is always done to avoid the following problem

			Unix command		MsDOS file name created
			============		=======================
			mkdir DIR			dir.{__
			mkdir dir.{__		dir.{_1
			...
			mkdir dir.{_1		dir.{10

			Now, suppose that dir.{__ does not exist in the EMD. dir.{_1
			do exist in DOS. If we try to create it in Umsdos, this
			will create a mangled name. Mangling is based on the
			entry offset in the EMD.
		*/
		/* Don't care about hidden hard link */
		/* A special utility (umsdos.fsck) will be needed to */
		/* make sure all hidden link are referenced. */
		if (strncmp(name,"__link",6)!=0){
			char newname[13];
			char *pt;
			umsdos_manglename (&info);
			strcpy (newname,info.fake.fname);
			pt = strchr(newname,'.');
			if (pt == NULL){
				sprintf (newname,"%s.$%02d",info.fake.fname,*largest);
			}else{
				sprintf (pt+1,"$%02d",*largest);
			}
			(*largest)++;
			fprintf (stderr
				,"Can't synchonise file %s\n"
				 "This look like a mangled name. It has been renamed to %s.\n"
				,fpath,newname);
		
printf ("  /mn/ ide rename %s -> %s\n", name, newname);		
			if (UM_dosrename (UM_fd, name,newname)!=-1){
printf ("  /mn/ ide create path=%s newname=%s largest=%d\n", path, newname, *largest);
				ret = umssync_create (UM_fd, path, opt, newname, largest);
				if (ret == 0) strcpy (name,newname);
			}
		}else{
			ret = 0;
		}
	}else{
		struct stat fstat;
		if (opt->debug > 2) printf ("umssync_create: msdos_reject=%d for %s (%d)\n", info.msdos_reject, name, strlen(name));
		ret = UM_dosstat (UM_fd, name, &fstat);
//printf ("   /mn/ dosstat ret=%d\n", ret);
		if (ret != -1){
			mode_t mode;
			const char *msgtype;
			if (S_ISDIR(fstat.st_mode)){
				mode = opt->dir_mode|S_IFDIR;
				msgtype = "dir";
			}else{
				mode = opt->fil_mode|S_IFREG;
				msgtype = "file";
			}
			if (opt->verbose) printf ("Creating %s %s\n",msgtype,fpath);
//			sleep (400);
			ret = UM_create (UM_fd, name,mode,fstat.st_mtime,fstat.st_mtime
				,fstat.st_mtime,opt->uid,opt->gid,0);
		}
	}
	return ret;
}

/*
	Do a unlink or rmdir on an EMD file only
*/
static int umssync_uunlink (
	int UM_fd,
	const char *path,
	struct UMSSYNC_OPT *opt,
	struct umsdos_dirent *ent)
{
	const char *name = ent->name;
	int is_dir = S_ISDIR(ent->mode);
	if (opt->verbose){
		printf ("%s from EMD %s/%s\n"
			,is_dir ? "Rmdir" : "Unlink"
			,path,name);
	}
	return is_dir ? UM_urmdir (UM_fd, name) : UM_uunlink (UM_fd, name);
}	

/*
	Sort function for qsort.
	Sort in ascending directory using the corresponding MsDOS name of
	an entry.
*/
static int cmp_udir (const void *p1, const void *p2)
{
	struct UMSSYNC_INFO *i1 = (struct UMSSYNC_INFO *)p1;
	struct UMSSYNC_INFO *i2 = (struct UMSSYNC_INFO *)p2;
#if BE_UVFAT
	return strcasecmp (i1->uent.name, i2->uent.name);	/* use EMD long name, to compare with VFAT LFN long name ! */
#else
	return strcmp (i1->ment.d_name, i2->ment.d_name);
#endif
}
/*
	Sort function for qsort.
	Sort in ascending directory using the MsDOS name.
*/
static int cmp_ddir (const void *p1, const void *p2)
{
	struct dirent *i1 = (struct dirent  *)p1;
	struct dirent *i2 = (struct dirent  *)p2;
#if BE_UVFAT
	return strcasecmp (i1->d_name,i2->d_name);
#else
	return strcmp (i1->d_name,i2->d_name);
#endif
}

struct UMS_DIRINFO {
	char name[222+1];
	int gid;
	int uid;
};
/*
	Add a sub-directory to tbsub[].
	Check if name is a directory.

	Return -1 if room is exhausted in tbsub.
*/
static int umssync_addsdir(
	int UM_fd,
	const char *ums_name,
	const char *dos_name,
	int uid,
	int gid,
	struct UMS_DIRINFO tbsub[],
	int maxsub,
	int *nbsub)
{
	int ret = -1;
	struct stat fstat;

#if BE_UVFAT
	int cur_verbose = UM_verbose;
	UM_verbose = 0;
	/* try first dos [mangled] name, and if that fails LFN name */
	if ((UM_dosstat(UM_fd, dos_name, &fstat)==-1) && (UM_dosstat(UM_fd, ums_name, &fstat)==-1)){
		fprintf (stderr,"dosstat failed for entry %s (%d) / %s (%d)\n", dos_name, strlen(dos_name), ums_name, strlen(ums_name));
	UM_verbose = cur_verbose;
#else
	if (UM_dosstat(UM_fd, dos_name, &fstat)==-1){
		fprintf (stderr,"dosstat failed for entry %s\n",ums_name);
#endif		
		ret = 0;
	}else if (S_ISDIR(fstat.st_mode)){
		if (*nbsub < maxsub){
			struct UMS_DIRINFO *ptsub = tbsub + (*nbsub)++;
			ret = 0;
			strcpy (ptsub->name,ums_name);
			ptsub->uid = uid;
			ptsub->gid = gid;
		}
	}else{
		/* This is a file , so we don't care */
		ret = 0;
	}
	return ret;
}

#if BE_UVFAT
	struct UMSSYNC_LIST {
		void *p;			/* Pointer to tb_ddir[] or tb_udir[] element */
		struct UMSSYNC_LIST *next;	/* next in list, or NULL if last */
	};
	struct UMSSYNC_LIST *first_udir, *first_ddir, *prev_udir, *prev_ddir, *cur_udir, *cur_ddir, *next_udir;
	
/*
	compares udir and ddir (for FAKE of LFN entries, depending on argument)
*/
static int check_names (
	struct UMSSYNC_OPT *opt, 
	int UM_fd,
	const char *path,
	struct UMS_DIRINFO tbsub[],
	int maxsub,
	int *nbsub,
	int lfn)
{
	struct UMSSYNC_INFO *pt_udir;
	struct dirent *pt_ddir;
	int toomanysub=0;
	
	prev_udir = NULL;
	for (cur_udir = first_udir; cur_udir;) {
		int cmp=-1;
		pt_udir = (struct UMSSYNC_INFO *) cur_udir->p;
		prev_ddir = NULL;
		
		for (cur_ddir = first_ddir; cur_ddir; cur_ddir = cur_ddir->next){
			pt_ddir = (struct dirent *) cur_ddir->p;
			if (lfn) {
				if (opt->debug > 1)
					printf (" -- be_uvfat  cmp_lfn %s (%d/%d) <--> %s (%d/%d) == ", pt_udir->uent.name, strlen(pt_udir->uent.name), pt_udir->uent.name_len, pt_ddir->d_name, strlen(pt_ddir->d_name), pt_ddir->d_reclen);
				cmp = strncasecmp(pt_udir->uent.name,pt_ddir->d_name, UMSDOS_MAXNAME); /* case isensitive: for example, "CVS" in EMD will be created as "cvs" in LFN, since 8.3 conformant names do not preserve case ? */
			} else {
				if (opt->debug > 1)
					printf (" -- be_uvfat  cmp_fake %s <--> %s == ", pt_udir->ment.d_name, pt_ddir->d_name);
				cmp = strncmp(pt_udir->ment.d_name, pt_ddir->d_name, UMSDOS_MAXNAME);
			}
			if (cmp == 0) {
				if (lfn) {
					if (opt->debug > 0) printf ("BREAKdd: ** found LFN ** %s\n", pt_udir->uent.name);
				} else {
					if (opt->debug > 0) printf ("BREAKdd: ** found FAKE ** %s\n", pt_udir->ment.d_name);
				}

				/* found match, remove the element */
				if (opt->debug > 2) printf ("    remove from LIST ddir %s\n", ((struct dirent *)(cur_ddir->p))->d_name);
				if (prev_ddir == NULL) first_ddir = cur_ddir->next; 
					else prev_ddir->next = cur_ddir->next;
				free(cur_ddir);

				/* Handle a special case (bug) when the EMD entry is */
				/* a directory but FAT entry is a file               */
				if (S_ISDIR(pt_udir->uent.mode))
                               	{
					struct stat fstat;

					UM_dosstat( UM_fd, pt_ddir->d_name, &fstat );
					if (opt->debug > 3) { printf ("UENT %s DIR mode=%o, DOSSTAT mode=%o\n", pt_ddir->d_name, pt_udir->uent.mode, fstat.st_mode); }
					if( !S_ISDIR(fstat.st_mode) ) {
						printf ("FILE %s represented as DIR ? Changing back to file!\n", pt_ddir->d_name);
						/* remove the wrong entry from the EMD file */
						umssync_uunlink( UM_fd, path, opt, &pt_udir->uent );
						/* create a correct entry */
						UM_create( UM_fd,
							pt_udir->uent.name,
							fstat.st_mode,
							fstat.st_mtime,
							fstat.st_mtime,
							fstat.st_mtime,opt->uid,opt->gid,0);
					} else {
						toomanysub |= umssync_addsdir (UM_fd
							,pt_udir->uent.name,pt_udir->ment.d_name
							,pt_udir->uent.uid,pt_udir->uent.gid
							,tbsub,maxsub,nbsub);
					}
				}
				break;
			}
			if (cmp < 0) {
				if (lfn) {
					if (opt->debug > 1) printf ("BREAKdd:  uu<dd %s < %s\n", pt_udir->uent.name, pt_ddir->d_name);
				} else {
					if (opt->debug > 1) printf ("BREAKdd:  uu<dd %s < %s\n", pt_udir->ment.d_name, pt_ddir->d_name);
				}
				break;
			}
			if (opt->debug > 1) printf ("miss\n");
			prev_ddir = cur_ddir;
		}

		if (cmp == 0) { /* match was found above, remove udir element as well */
			next_udir = cur_udir->next; 
			if (opt->debug > 2) printf ("    remove from LIST udir %s\n", ((struct UMSSYNC_INFO *)(cur_udir->p))->uent.name);
			free(cur_udir);
			if (prev_udir == NULL) first_udir = next_udir;
				else prev_udir->next = next_udir;
			cur_udir = next_udir;
		} else {	/* No match, normal advance forward */
			prev_udir = cur_udir;
			cur_udir = cur_udir->next;
		}
	}
	return toomanysub;
}


#endif

/*
	returns true if string is "." or ".."
*/		
int isdotdir(const char *s)
{
	if (s[0] == '.') {
		if (s[1] == '\0') return 1;			/* this is "." dir */
		if ((s[1] == '.') && (s[2] == '\0')) return 2; 	/* this is ".." dir */
	}
	return 0;
}

/*
	Synchronise one directory.
*/
static int umssync_dodir (
	const char *path,
	struct UMSSYNC_OPT *opt,
	struct UMS_DIRINFO tbsub[],
	int maxsub,		/* Maximum room in tbsub[] */
	int *nbsub,		/* Will hold the number of subdirectory entered in */
	int *hasemd)		/* Will be set if this directory contains a EMD file. */
				/* tbsub[] */
{

	int UM_fd;
	int ret = 0;
	int toomanysub = 0;
	struct UMSSYNC_INFO tb_udir[4000];
	int nb_udir = 0;
	struct dirent tb_ddir[4000];		/* Name in DOS */
	int nb_ddir = 0;
	int largest = 0;
	struct UMSSYNC_INFO *pt_udir;
	struct dirent *pt_ddir;
	struct UMSSYNC_INFO  *last_udir;
	struct dirent *last_ddir;
	
	UM_fd = UM_open (path, 1, 0);
	*nbsub = 0;

	if (!UM_isok(UM_fd)) {
		if (opt->verbose) printf ("%s failed\n",path);
		UM_close (&UM_fd);
		return -1;
	}


	if (!opt->condit || UM_ispromoted(UM_fd)){
		*hasemd = -1;
		if (opt->verbose) printf ("%s\n",path);

		UM_initemd(UM_fd);
		
		nb_udir = 0;
		/* First read the EMD file */
		pt_udir = tb_udir;
		while (UM_ureaddir(UM_fd, &pt_udir->uent, &pt_udir->ment)!=-1){
			if (opt->debug > 3) printf (" /mn/ uent=%s (%d) ment=%s (%d)\n", pt_udir->uent.name, strlen(pt_udir->uent.name), pt_udir->ment.d_name, strlen(pt_udir->ment.d_name));
			if (pt_udir->uent.name_len >= UMSDOS_MAXNAME) {
				printf ("Entry too long -- uhoh -- fixme in kernel\n");
			}
			if ((pt_udir->uent.flags & UMSDOS_HIDDEN)==0){
				nb_udir++;
				pt_udir++;
				if (nb_udir == 4000){
					fprintf (stderr,"Too many entry in the directory: "
						"max 4000\n");
					exit (-1);
				}
			}
		}
		nb_ddir = 0;
		/* Next read the DOS directory */
		pt_ddir = tb_ddir;
		UM_rewind(UM_fd);
		/* See umssync_create. Some mangled file name can't be */
		/* sync in the EMD, so we must rename it first to something */
		/* acceptable both to the EMD and DOS without mangling. */
		largest = 0;
		while (UM_readdir(UM_fd, pt_ddir)!=-1){
			if (opt->debug > 3) printf (" /mn/ ddir=%s (%d)\n", pt_ddir->d_name, strlen(pt_ddir->d_name));
			if (!isdotdir(pt_ddir->d_name)
				&& strcmp(pt_ddir->d_name,UMSDOS_EMD_FILE)!=0){
				char *ptpt = strchr(pt_ddir->d_name,'.');
				if (ptpt != NULL
					&& ptpt[1] == '$'
					&& isdigit(ptpt[2])
					&& isdigit(ptpt[3])){
					int val = atoi(ptpt+2);
					if (val >= largest) largest = val+1;
				}
				nb_ddir++;
				pt_ddir++;
				if (nb_ddir == 4000){
					fprintf (stderr,"Too many entry in the directory: "
						"max 4000\n");
					exit (-1);
				}
			}
		}
		qsort (tb_udir,nb_udir,sizeof(tb_udir[0]),cmp_udir);
		qsort (tb_ddir,nb_ddir,sizeof(tb_ddir[0]),cmp_ddir);

#if BE_UVFAT
		/* create lists for easier & _faster_ managment! */

		last_udir = tb_udir + nb_udir;
		cur_udir = NULL;
		for (pt_udir = last_udir-1; pt_udir >= tb_udir; pt_udir--){
			prev_udir = cur_udir;
			cur_udir = malloc(sizeof(struct UMSSYNC_LIST)); if (!cur_udir) { perror ("malloc"); exit (1); }
			cur_udir->p = (void *) pt_udir; cur_udir->next = prev_udir;
		}
		first_udir = cur_udir;

		last_ddir = tb_ddir + nb_ddir;
		cur_ddir = NULL;
		for (pt_ddir = last_ddir-1; pt_ddir >= tb_ddir; pt_ddir--){
			prev_ddir = cur_ddir;
			cur_ddir = malloc(sizeof(struct UMSSYNC_LIST)); if (!cur_ddir) { perror ("malloc"); exit (1); }
			cur_ddir->p = (void *) pt_ddir; cur_ddir->next = prev_ddir;
		}
		first_ddir = cur_ddir;

		if (opt->debug > 0) {
			int cnt;

			printf (".........................\n"); cnt=0;
			for (cur_udir = first_udir; cur_udir; cur_udir = cur_udir->next){
				pt_udir = (struct UMSSYNC_INFO *) cur_udir->p;
				printf ("sorted list udir %d: %s (%s)\n", ++cnt, pt_udir->uent.name, pt_udir->ment.d_name);
			}
			printf (".........................\n"); cnt=0;
			for (cur_ddir = first_ddir; cur_ddir; cur_ddir = cur_ddir->next){
				pt_ddir = (struct dirent *) cur_ddir->p;
				printf ("sorted list ddir %d: %s\n", ++cnt, pt_ddir->d_name);
			}

			printf ("\n-------------------------\n\n");
			sleep (2);
		}
		
		/* first, walk the udir and remove all FAKE-name matches */
		if (opt->debug > 0) printf ("------------ CHECK FAKE -------------\n");
		toomanysub |= check_names(opt, UM_fd, path, tbsub, maxsub, nbsub, 0);

		/* second, walk the udir and remove all LFN-name matches */
		if (opt->debug > 0) printf ("------------ CHECK LFN -------------\n");
		toomanysub |= check_names(opt, UM_fd, path, tbsub, maxsub, nbsub, 1);

		if (opt->debug > 0) printf ("------------ END CHECKS -------------\n");

		if (opt->debug > 0) {
			int cnt;

			printf (".........................\n"); cnt=0;
			for (cur_udir = first_udir; cur_udir; cur_udir = cur_udir->next){
				pt_udir = (struct UMSSYNC_INFO *) cur_udir->p;
				printf ("final list udir %d: %s (%s)\n", ++cnt, pt_udir->uent.name, pt_udir->ment.d_name);
			}
			printf (".........................\n"); cnt=0;
			for (cur_ddir = first_ddir; cur_ddir; cur_ddir = cur_ddir->next){
				pt_ddir = (struct dirent *) cur_ddir->p;
				printf ("final list ddir %d: %s (%d/%d)\n", ++cnt, pt_ddir->d_name, strlen(pt_ddir->d_name), pt_ddir->d_reclen);
			}

			printf ("\n-------------------------\n\n");
			sleep (10);

		}
		
		if (opt->debug > 0) printf ("------------ REMOVE REMAINING UDIRs -------------\n");
		for (cur_udir = first_udir; cur_udir; cur_udir = cur_udir->next){
			pt_udir = (struct UMSSYNC_INFO *) cur_udir->p;
			if (opt->debug > 0) printf ("remove EMD %s\n", pt_udir->uent.name);
			umssync_uunlink( UM_fd, path, opt, &pt_udir->uent );
		}

		if (opt->debug > 0) printf ("------------ ADD MISSING DDIRs -------------\n");
		for (cur_ddir = first_ddir; cur_ddir; cur_ddir = cur_ddir->next){
			pt_ddir = (struct dirent *) cur_ddir->p;
			if (opt->debug > 0) printf ("add EMD for %s\n", pt_ddir->d_name);
			
			ret = umssync_create (UM_fd,path,opt,pt_ddir->d_name,&largest);
			if (ret != -1){
				toomanysub |= umssync_addsdir (UM_fd
					,pt_ddir->d_name,pt_ddir->d_name
					,opt->uid,opt->gid,tbsub,maxsub,nbsub);
			}
		}
#else
		if (opt->debug > 0) {
			int cnt;
			pt_udir = tb_udir;
			pt_ddir = tb_ddir;
			last_udir = tb_udir + nb_udir;
			last_ddir = tb_ddir + nb_ddir;

			printf (".........................\n"); cnt=0;
			for (; pt_udir < last_udir; pt_udir++){
				printf ("sorted udir %d: %s (%s)\n", ++cnt, pt_udir->uent.name, pt_udir->ment.d_name);
			}
			printf (".........................\n"); cnt=0;
			for (; pt_ddir < last_ddir; pt_ddir++){
				printf ("sorted ddir %d: %s\n", ++cnt, pt_ddir->d_name);
			}

			printf ("\n-------------------------\n\n");
			sleep (2);
		}

		pt_udir = tb_udir;
		pt_ddir = tb_ddir;
		last_udir = tb_udir + nb_udir;
		last_ddir = tb_ddir + nb_ddir;
		ret = 0;
		/* Even if tbsub overflow, process this directory completly */
		/* toomanysub will act as a flag telling us the overflow */
		/* has happen, so we will print a single message. */
		for (; pt_udir < last_udir && ret != -1; pt_udir++){
			if (pt_ddir == last_ddir){
				ret = umssync_uunlink (UM_fd,path,opt, &pt_udir->uent);
			}else{
				int cmp;
				cmp = strcmp(pt_udir->ment.d_name, pt_ddir->d_name);
			
				if (cmp == 0){
					/* Handle a special case (bug) when the EMD entry is */
					/* a directory but FAT entry is a file               */
					if (S_ISDIR(pt_udir->uent.mode))
                                	{
                              			struct stat fstat;

						UM_dosstat( UM_fd, pt_ddir->d_name, &fstat );
                                              	if( !S_ISDIR(fstat.st_mode) )
                                                {
                                                  printf ("FILE %s represented as DIR ? Changing back to file!\n", pt_ddir->d_name);
                                                  /* remove the wrong entry from the EMD file */
                                                  umssync_uunlink( UM_fd, path, opt, &pt_udir->uent );
						  /* create a correct entry */
						  UM_create( UM_fd,
							     pt_udir->uent.name,
							     fstat.st_mode,
								fstat.st_mtime,
								fstat.st_mtime,
								fstat.st_mtime,opt->uid,opt->gid,0);
                                                }
					        else
						{
							toomanysub |= umssync_addsdir (UM_fd
								,pt_udir->uent.name,pt_udir->ment.d_name
								,pt_udir->uent.uid,pt_udir->uent.gid
								,tbsub,maxsub,nbsub);
						}
					}
					pt_ddir++;
				}else if (cmp < 0){
//					printf ("    ne valja: %s, %s\n", pt_udir->uent.name, pt_ddir->d_name);
					ret = umssync_uunlink (UM_fd,path,opt, &pt_udir->uent);
				}else{
		
					ret = umssync_create (UM_fd,path,opt,pt_ddir->d_name
						,&largest);
					if (ret != -1){
						toomanysub |= umssync_addsdir (UM_fd
							,pt_ddir->d_name,pt_ddir->d_name
							,opt->uid,opt->gid,tbsub,maxsub,nbsub);
					}
					pt_ddir++;
					pt_udir--;
				}
			}
		}
		for (; pt_ddir < last_ddir && ret != -1; pt_ddir++){
			ret = umssync_create (UM_fd,path,opt,pt_ddir->d_name,&largest);
			if (ret != -1){
				toomanysub |= umssync_addsdir (UM_fd
					,pt_ddir->d_name,pt_ddir->d_name
					,opt->uid,opt->gid,tbsub,maxsub,nbsub);
			}
		}

#endif
	} else {
		/* Added recursing into unpromoted directories,
		   Michael Nonweiler 28 March '96 */
		*hasemd = 0;

		if (opt->full_recurse > 0) {
			struct dirent cf;
			UM_rewind(UM_fd);

			while (UM_readdir(UM_fd, &cf)!=-1 && !toomanysub) {
				if (!isdotdir(cf.d_name)){
					toomanysub |= umssync_addsdir (UM_fd
						,cf.d_name,cf.d_name
						,opt->uid,opt->gid
						,tbsub,maxsub,nbsub);
				}
			}
		}
	}

	if (toomanysub){
		fprintf (stderr,"Too many subdirectories in %s\n",path);
	}

	UM_close (&UM_fd);
	return ret;
}

/*
	Synchronise a directory and its subdirectories.
	Recursive function.

	Return -1 if any error.
*/
static int umssync_dodirs (
	const char *path,
	struct stat * dir_stat,
	struct UMSSYNC_OPT * opt,
	int depth,
	struct UMS_DIRINFO tbsub[],	/* Accumulator for subdirectories */
					/* This table is shared by all instance */
					/* of this function. */
	int maxsub)
{
	int nbsub;
	int hasemd;
	struct UMSSYNC_OPT new_opt;
	int ret, i;
	
	memcpy (&new_opt, opt, sizeof (struct UMSSYNC_OPT));
	if (opt->inherit){
		new_opt.uid = dir_stat->st_uid;
		new_opt.gid = dir_stat->st_gid;
	}
	ret = umssync_dodir (path,&new_opt,tbsub,maxsub,&nbsub,&hasemd);
	if (ret != -1 && depth > 0){
		struct UMS_DIRINFO *endsub = tbsub + nbsub;
		int new_maxsub = maxsub - nbsub;
		struct UMS_DIRINFO *ptsub = tbsub;
		depth--;
		if (!hasemd)
			new_opt.full_recurse--;

		/* Recursion of this function will use the unused part of */
		/* tbsub[]. */
		for (i=0; i<nbsub && ret != -1; i++,ptsub++){
			char spath[PATH_MAX];
			struct stat sstat;
			if (strlen(path)+strlen(ptsub->name) >= PATH_MAX) {
				fprintf (stderr, "\n%s/%s\nPath too long (%u)!\n", path, ptsub->name, PATH_MAX);
				return -1;
			}
			path_make (path,ptsub->name,spath);
			if (stat(spath,&sstat)==-1){
				fprintf (stderr,"\nCan't stat directory %s\n",spath);
				ret = -1;
			}else if (dir_stat->st_dev == sstat.st_dev){
				/* #Specification: umssync / mount point
					umssync won't cross mount point. It means
					you must specify each mount point separatly.
				*/
				ret = umssync_dodirs (spath,&sstat,&new_opt,depth
					,endsub,new_maxsub);
			}
		}
	}
	return ret;
}

/*
	Validate an option + or -. Print an error message if anything else.
*/
static int umssync_checkoptplus (
	const char *parm,
	char optletter,
	int *err)		/* Will contain -1 if any error detected */
{
	int ret = 0;
	if (strcmp(parm,"+")==0){
		ret = 1;
	}else if (strcmp(parm,"-")==0){
		ret = 0;
	}else{
		fprintf (stderr,"Invalid option -%c\n"
			 "Use -%c+ or -%c-\n",optletter,optletter,optletter);
		*err = -1;
	}
	return ret;
}

/*
	Synchronise a directory and its subdirectories.
	Entry point for a recursive function.

	Return -1 if any error.
*/
static int UMSSYNC_DODIRS (
	const char *path,
	struct UMSSYNC_OPT *opt,
	int depth,
	struct UMS_DIRINFO tbsub[],	/* Accumulator for subdirectories */
					/* This table is shared by all instance */
					/* of this function. */
	int maxsub)
{
	struct stat dir_stat;
	int ret = stat(path,&dir_stat);
	if (ret == -1){
		fprintf (stderr,"Can't stat directory %s\n",path);
	}else{
		ret = umssync_dodirs (path,&dir_stat,opt,depth,tbsub,maxsub);
	}
	return ret;
}

int umssync_main (int argc, char *argv[])
{
	int ret = -1;
	if (argc < 2 || geteuid()!=0){
		PROG_ID(SYNC_NAME);
		fprintf (stderr,
			"umssync [ options ] dir_path [ [ options ] dir_path ... ]\n"
			"\n"
			" -c      : Conditionnal mode\n"
			"           sync a directory only if it contain\n"
			"           a --linux-.--- file.\n"
			" -dmode  : directory creation mode\n"
			" -fmode  : file creation mode\n"
			" -ggroup : group id for entry creation\n"
			" -i-     : Do not inherit ownership from parent directory\n"
			" -i+     : Inherit from parent\n"
			" -rN     : Recurse N level of subdirectory\n"
			" -RN     : Allow recursion through N directories that\n"
			"           do not contain a --linux-.--- file.\n"
			" -uuser  : user id for entry creation\n"
			" -v-     : Silent operation\n"
			" -v+     : Verbose operation\n"
			" -dN     : Debug level N\n"
			);
		if (geteuid()!=0){
			fprintf (stderr,"\n*** Only root can run umssync\n");
		}
	}else{
		/* #Specification: umssync / default creation mode
			Unless override with command line option, file
			and directory created by umssync will be owned
			by root with mode 755 for directories and mode
			644 for files.
		*/
		int depth=0;
		int usermode = getuid() != 0;
		struct UMSSYNC_OPT opt;
		int i;
		opt.uid = 0;
		opt.gid = 0;
		opt.verbose = 1;
		opt.condit = 0;
		opt.full_recurse = 0;
		opt.debug = 0;
		/* #Specification: umssync / depth
			Normally, umssync won't recurse into directory.
			Option -r allows for depth control. You may specify
			how deep you want umssync to work.

			When recursing into directory, umssync will use
			the owner and group specified on the command line
			(see option -g and -u). If option -i+ is specified
			the specs of the sub-directory itself may be used.

			umssync won't follow symlinks. And it won't cross mount
			points.
		*/
		opt.dir_mode = 0755;
		opt.fil_mode = 0644;
		opt.inherit = 1;
		ret = 0;
		for (i=1; i<argc && ret == 0; i++){
			const char *arg = argv[i];
			if (arg[0] == '-'){
				if (arg[1] == '-' && arg[2] == '\0'){
					struct UMS_DIRINFO tbsub[2000];
					i++;
					ret = UMSSYNC_DODIRS (argv[i],&opt,depth,tbsub,2000);
				}else if (!isalpha(arg[1])){
					fprintf (stderr,"Invalid option %s\n",arg);
					exit (-1);
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
					if (arg[1] == 'r'){
						depth = xtoi(parm, &ret);
					}else if (arg[1] == 'R'){
						opt.full_recurse = xtoi(parm, &ret);
					}else if (arg[1] == 'd'){
						opt.debug = xtoi(parm, &ret);
					}else if (arg[1] == 'v'){
						opt.verbose = umssync_checkoptplus(parm,'v',&ret);
					}else if (arg[1] == 'c'){
						opt.condit = umssync_checkoptplus(parm,'c',&ret);
					}else{
						int ok = 1;
						/* #Specification: umssync / setuid root
							All ioctl to the umsdos file system must
							performed by root. It is possible to
							make umssync setuid root. In that
							case, it will prohibit the following
							option

							#
								-d,-f,-g,-u
							#
	
							And it will force option -i+ (inheriting mode).
							It means that anyone will be able to use
							umssync but the file that may be digged from
							the DOS directory will have the same owner
							as the parent directory. I don't think this
							is a security hole (DOS is a security hole :-))
							Off course, this is yours to choose.
						*/
						if (arg[1] == 'd'){
							opt.dir_mode = octal_toi(parm,&ret);
						}else if (arg[1] == 'f'){
							opt.fil_mode = octal_toi(parm,&ret);
						}else if (arg[1] == 'g'){
							if (isdigit(parm[0])){
								opt.gid = xtoi(parm, &ret);
							}else{
								struct group *pt = getgrnam (parm);
								if (pt == NULL){
									fprintf (stderr,"Invalid option -g : "
										"Group %s unknown\n",parm);
									ret = -1;
								}else{
									opt.gid = pt->gr_gid;
								}
							}
						}else if (arg[1] == 'i'){
							opt.inherit = umssync_checkoptplus(parm,'i',&ret);
						}else if (arg[1] == 'u'){
							if (isdigit(parm[0])){
								opt.uid = xtoi(parm, &ret);
							}else{
								struct passwd *pt = getpwnam (parm);
								if (pt == NULL){
									fprintf (stderr,"Invalid option -u : "
										"User %s unknown\n",parm);
									ret = -1;
								}else{
									opt.uid = pt->pw_uid;
								}
							}
						}else{
							fprintf (stderr,"Invalid option %s\n",arg);
							ret = -1;
							ok = 0;
						}
						if (ok && usermode){
							fprintf (stderr,"Option %s is only available to root\n",arg);
						}
					}
				}
			}else if (ret != -1){
				struct UMS_DIRINFO tbsub[2000];
				if (usermode){
					/* #Specification: umssync / user mode
						To execute umssync, the effective user id must be root.
						It it possible to configure umssync to run setuid root.
						In this case (when getuid() != geteuid()), umssync
						show a special behavior: Options -d -f -g -i -u are not
						available anymore. The inheriting mode is automaticly
						activated. No way to desactivated.

						A user should be able to umssync its own directory. If a user
						apply umssync to a directory, all file uncovered will be given
						to the owner of the directory with restrictive permissions
						(600 for files, 700 for directory).

						Another way would be to limit umssync operation to directory
						which belong to the user. Suggestion welcome.
					*/
					opt.dir_mode = 0700;
					opt.fil_mode = 0600;
					opt.inherit  = 1;
				}
				ret = UMSSYNC_DODIRS (arg,&opt,depth,tbsub,2000);
			}
		}
	}
	return ret;
}

