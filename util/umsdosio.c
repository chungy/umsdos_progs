#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include "umsdos_progs.h"
#include "umsdos_gen.h"


int UM_verbose = 0;

/*
	Initialise the name fields in struct umsdos_ioctl.
	Does both name and d_name initialisation even if unnecessary.
*/
static void fname_copy (
	struct umsdos_ioctl *data,
	const char *fname)
{
	strcpy (data->dos_dirent.d_name, fname);
	data->dos_dirent.d_reclen = strlen(fname);
	strcpy (data->umsdos_dirent.name, fname);
	data->umsdos_dirent.name_len = data->dos_dirent.d_reclen;
}

/*
	memcpy for 'struct dirent' which is different in kernel and in glibc.
*/
static void dirent_copy (
	struct dirent *libc,
	struct umsdos_ioctl *data)
{

	libc->d_ino = data->dos_dirent.d_ino;
	libc->d_off = data->dos_dirent.d_off;
	libc->d_reclen = data->dos_dirent.d_reclen;
	memcpy (libc->d_name, data->dos_dirent.d_name, NAME_MAX);
}


/*
	Perform ioctl and (conditionnally) reports error.
*/
int UM_ioctl (
	int fd,
	int cmd,
	struct umsdos_ioctl *data)
{
	int ret = ioctl (fd,cmd,data);
	if (ret < 0 && UM_verbose){
		/* This table must be kept in sync with linux/umsdos_fs.h */
		static char *tb[]={
			"UMSDOS_READDIR_DOS",
			"UMSDOS_UNLINK_DOS",
			"UMSDOS_RMDIR_DOS",
			"UMSDOS_STAT_DOS",
			"UMSDOS_CREAT_EMD",
			"UMSDOS_UNLINK_EMD",
			"UMSDOS_READDIR_EMD",
			"UMSDOS_GETVERSION",
			"UMSDOS_INITEMD",
			"UMSDOS_DOS_SETUP",
			"UMSDOS_RENAME_DOS",
		};
		char buf[100];
		sprintf (buf,"umsdos_ioctl: %s ",tb[cmd-UMSDOS_READDIR_DOS]);
		perror (buf);
	}
	return ret;
}

/*
	Rewind to beginning of dir (for readdir()).
*/
void UM_rewind(int fd)
{
	if (lseek (fd, 0, SEEK_SET)==-1) perror ("UM_rewind");
}
	
/*
	Gets the version and release number of the umsdos fs driver.
	Return -1 if any error.
*/
int UM_getversion(int fd, int *version, int *release)
{
	int ret = -1;
	struct umsdos_ioctl data;
	
	memset (&data, 0, sizeof (struct umsdos_ioctl));
	*version = *release = -1;
	if (UM_ioctl (fd, UMSDOS_GETVERSION,&data)==0){
		*version = data.version;
		*release = data.release;
		ret = 0;
	}
	return ret;
}

/*
	Make sure the EMD file exist in a directory. This automaticly
	promote the directory to UMSDOS semantic (long name, links, etc).
	Return -1 if any error.
*/
int UM_initemd(int fd)
{
	int ret = -1;
	struct umsdos_ioctl data;
	if (UM_ioctl (fd, UMSDOS_INIT_EMD, &data) == 0){
		ret = 0;
	}
	return ret;
}

/*
	Read an entry in the DOS directory.
	Return -1 if error.
*/
int UM_readdir (int fd, struct dirent *dirent)
{
	int ret = -1;
	struct umsdos_ioctl data;
	if (UM_ioctl (fd, UMSDOS_READDIR_DOS, &data) > 0){
		dirent_copy (dirent, &data);
		ret = 0;
	}
	return ret;
}

/*
	Read an entry in the EMD directory.
	Return -1 if error.
*/
int UM_ureaddir (
	int fd,
	struct umsdos_dirent *udirent,
	struct dirent *dirent)
{
	int ret = -1;
	struct umsdos_ioctl data;
	if (UM_ioctl(fd, UMSDOS_READDIR_EMD,&data)>0){
		memcpy (udirent, &data.umsdos_dirent, sizeof (struct umsdos_dirent));
		dirent_copy (dirent, &data);
		
/*		printf ("/mn/: size = %d name=>%s<\n", udirent->name_len, udirent->name);*/
		ret = 0;
	} else {
		memcpy (udirent, &data.umsdos_dirent, sizeof (struct umsdos_dirent));
		dirent_copy (dirent, &data);
/*		printf ("/mn/: error %d, size = %d name=>%s<\n", errno, udirent->name_len, udirent->name);*/
	}
	return ret;
}

/*
	Create a file or a directory in the EMD file (or whatever in fact).
	This function simply create an entry with the supplied parameter.
	It is expected that the caller know what he is doing.
	Return -1 if error.
*/
int UM_create (
	int fd,
	const char *fname,
	mode_t mode,
	time_t atime,
	time_t ctime,
	time_t mtime,
	int uid,
	int gid,
	int rdev)
{ 
	struct umsdos_ioctl data;
	fname_copy (&data, fname);
	data.umsdos_dirent.mode = mode;
	data.umsdos_dirent.nlink = 1;
	data.umsdos_dirent.ctime = ctime;
	data.umsdos_dirent.atime = atime;
	data.umsdos_dirent.mtime = mtime;
	data.umsdos_dirent.flags = 0;
	data.umsdos_dirent.gid = gid;
	data.umsdos_dirent.uid = uid;
	data.umsdos_dirent.rdev = rdev;
	return UM_ioctl (fd, UMSDOS_CREAT_EMD, &data);
}

/*
	Remove a file from the DOS directory.
	Return -1 if error.
*/
int UM_dosunlink(int fd, const char *fname)
{
	struct umsdos_ioctl data;
	fname_copy (&data, fname);
	return UM_ioctl (fd, UMSDOS_UNLINK_DOS, &data);
}

/*
	Remove a directory from the DOS directory.
	Return -1 if error.
*/
int UM_dosrmdir(int fd, const char *fname)
{
	struct umsdos_ioctl data;
	fname_copy (&data, fname);
	return UM_ioctl (fd, UMSDOS_RMDIR_DOS, &data);
}

/*
	Rename a file or directory in the DOS directory.
	Return -1 if error.
*/
int UM_dosrename(
	int fd,
	const char *src,
	const char *dst)
{
	struct umsdos_ioctl data;
	fname_copy (&data, src);
	strcpy (data.umsdos_dirent.name,dst);
	data.umsdos_dirent.name_len = strlen(dst);
	return UM_ioctl (fd, UMSDOS_RENAME_DOS, &data);
}

/*
	Remove a directory from the EMD file.
	Return -1 if error.
*/
int UM_urmdir(int fd, const char *fname)
{
	struct umsdos_ioctl data;
	fname_copy (&data, fname);
	data.umsdos_dirent.mode = S_IFDIR|0777;
	return UM_ioctl (fd, UMSDOS_UNLINK_EMD,&data);
}

/*
	Remove a file from the EMD file.
	Return -1 if error.
*/
int UM_uunlink(int fd, const char *fname)
{
	struct umsdos_ioctl data;
	fname_copy (&data, fname);
	data.umsdos_dirent.mode = S_IFREG|0777;
	return UM_ioctl (fd, UMSDOS_UNLINK_EMD, &data);
}

/*
	Do a stat of the file using DOS directory info only
	Return -1 if error.	 
*/
int UM_dosstat (int fd, const char *fname, struct stat *fstat)
{
	struct umsdos_ioctl data;
	int ret = -1;
	fname_copy (&data, fname);
	data.umsdos_dirent.mode = S_IFREG|0777;
//printf ("  /mn/ ide dosstat reclen=%d name=%s\n", data.dos_dirent.d_reclen, data.dos_dirent.d_name);

	ret = UM_ioctl (fd, UMSDOS_STAT_DOS, &data);
	fstat->st_ino  = data.stat.st_ino;
	fstat->st_mode = data.stat.st_mode;
	fstat->st_size = data.stat.st_size;
	fstat->st_atime = data.stat.st_atime;
	fstat->st_ctime = data.stat.st_ctime;
	fstat->st_mtime = data.stat.st_mtime;
	return ret;
}

/*
	Set the default permissions and owner for DOS directories.
	Return -1 if error.	 
*/
int UM_dossetup (
	int fd,
	int uid,
	int gid,
	mode_t mode)
{
	struct umsdos_ioctl data;
	data.umsdos_dirent.uid  = uid;
	data.umsdos_dirent.gid  = gid;
	data.umsdos_dirent.mode = mode;
	return UM_ioctl (fd, UMSDOS_DOS_SETUP, &data);
}

/*
	Return != 0 if the directory is promoted (Has a --linux-.--- file)
	A promoted directory behave like a Unix directory. A non promoted
	behave like DOS.
*/
int UM_ispromoted(int fd)
{
	struct stat fstat;
	int ret = -1;
	int cur_verbose = UM_verbose;
	UM_verbose = 0;
	ret = UM_dosstat (fd, UMSDOS_EMD_FILE, &fstat) != -1;
	UM_verbose = cur_verbose;
	return ret;
}


/*
	Open a directory. Don't return if error.
*/
int UM_open (
	const char *dpath,	/* Directory path */
	int _verbose,		/* Report error with perror() ? */
	int quit)		/* exit(-1) if directory can't be open */
				/* if quit == 0, isok() should be used */
{
	char buf[PATH_MAX];
	struct stat dir_stat;
	int fd;
	
	fd = -1;
	sprintf (buf,"Can't open directory %s",dpath);
	if (stat(dpath,&dir_stat)==-1){
		if (_verbose) perror (buf);
	}else if (!S_ISDIR(dir_stat.st_mode)){
		if (_verbose) fprintf (stderr,"%s is not a directory\n",dpath);
	}else{
		fd = open (dpath,O_RDONLY);
		if (fd == -1){
			if (_verbose){
				perror (buf);
			}
			if (quit) exit (-1);
		}else{
			unsigned version, release;
			if (UM_getversion (fd, &version, &release) != 0){
				fprintf (stderr,"This utility must operate on a directory\n"
					"of a umsdos mounted file system. Even an msdos\n"
					"file system won't do. If this is the case\n"
					"umount the file system and remount it as a umsdos one.\n");
				exit (-1);
			}else if (version != UMSDOS_VERSION || release != UMSDOS_RELEASE){
				fprintf (stderr,"This utility is incompatible with the\n"
					"current release of the UMSDOS file system.\n"
					"This utility was issued for version %u.%u\n"
					"and the UMSDOS drivers currently running is %u.%u.\n"
					"Can't continue\n"
					,UMSDOS_VERSION,UMSDOS_RELEASE,version,release);
				exit (-1);
			}
		}
	}
	UM_verbose = _verbose;
	return fd;
}

void UM_close (int *fd)
{
	if (*fd != -1) close (*fd);
	*fd = -1;
}

/*
	Return != 0 if the directory was successfully opened
*/
int UM_isok(int fd)
{
	return fd != -1;
}
