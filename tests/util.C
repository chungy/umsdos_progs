#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <stdlib.h>
#include <fcntl.h>
#include "umsdos_test.h"


static int util_check (
	int retcod,		// 0 == success, other mean error
	UMSDOS_REGISTER &reg,
	int err_expected,
	const char *opr,
	...)
{
	int ret = -1;
	va_list list;
	va_start (list,opr);
	char msg[1000];
	vsprintf (msg,opr,list);
	va_end  (list);
	if (retcod == 0){
		if (err_expected != 0){
			// Operation successfull: unexpected
			reg.prterr ("'%s' succeeded ???\n",msg);
		}else{
			ret = 0;
		}
	}else{
		if (err_expected == 0){
			// Was expecting success
			reg.prterr ("'%s' failed\n\terrno=%d '%s'\n"
				,msg,errno,strerror(errno));
		}else if (errno == err_expected){
			ret = 0;
		}else{
		}
	}
	return ret;
}
/*
	This file holds some front end to commun file system operation.
	All these function return 0 if ok, meaning that the operation
	produces what was expected, either success or failure.

	It return -1 in all other cases.
*/

/*
	Directory creation
*/	
int util_mkdir(
	const char *path,
	int mode,
	UMSDOS_REGISTER &reg,
	int err_expected)
{
	int ret = -1;
	if (reg.getnberr()==0){
		reg.verbose ("mkdir %s %o\n",path,mode);
		ret = mkdir(path,mode);
		ret = util_check (ret,reg,err_expected,"mkdir %s",path);
	}
	return ret;
}
/*
	Change current directory
*/	
int util_chdir(
	const char *path,
	UMSDOS_REGISTER &reg,
	int err_expected)
{
	int ret = -1;
	if (reg.getnberr()==0){
		reg.verbose ("chdir %s\n",path);
		ret = chdir(path);
		ret = util_check (ret,reg,err_expected,"chdir %s",path);
	}
	return ret;
}
/*
	File creation (file left closed).
	Fail if the file already exist.
*/	
int util_create(
	const char *path,
	int mode,
	UMSDOS_REGISTER &reg,
	int err_expected)
{
	int ret = -1;
	if (reg.getnberr()==0){
		reg.verbose ("creat %s %o\n",path,mode);
		ret = open (path,O_RDWR|O_CREAT|O_EXCL,mode);
		if (ret != -1) close (ret);
		ret = util_check (ret==-1,reg,err_expected,"creat %s",path);
	}
	return ret;
}
/*
	Special device file creation.
*/	
int util_mkdev(
	const char *path,
	int mode,
	int is_chr,
	int major,
	int minor,
	UMSDOS_REGISTER &reg,
	int err_expected)
{
	int ret = -1;
	if (reg.getnberr()==0){
		reg.verbose ("mknod %s %o %c %d %d\n",path,mode
			,is_chr ? 'c' : 'b'
			,major,minor);
		ret = mknod (path,mode | (is_chr ? S_IFCHR : S_IFBLK)
			,major<<8+minor);
		ret = util_check (ret==-1,reg,err_expected,"mknod %s",path);
	}
	return ret;
}
/*
	Fifo creation (file left closed).
*/	
int util_mkfifo(
	const char *path,
	int mode,
	UMSDOS_REGISTER &reg,
	int err_expected)
{
	int ret = -1;
	if (reg.getnberr()==0){
		reg.verbose ("mkfifo %s %o\n",path,mode);
		ret = mkfifo (path,mode);
		ret = util_check (ret==-1,reg,err_expected,"mkfifo %s",path);
	}
	return ret;
}
/*
	Symbolic link creation
*/	
int util_symlink(
	const char *path,
	const char *slink,
	UMSDOS_REGISTER &reg,
	int err_expected)
{
	int ret = -1;
	if (reg.getnberr()==0){
		reg.verbose ("symlink %s %s\n",path,slink);
		ret = symlink (path,slink);
		ret = util_check (ret==-1,reg,err_expected,"symlink %s %s",path,slink);
	}
	return ret;
}
/*
	Hard link creation
*/	
int util_hardlink(
	const char *path,
	const char *hlink,
	UMSDOS_REGISTER &reg,
	int err_expected)
{
	int ret = -1;
	if (reg.getnberr()==0){
		reg.verbose ("hardlink %s %s\n",path,hlink);
		ret = link (path,hlink);
		ret = util_check (ret==-1,reg,err_expected,"hardlink %s %s",path,hlink);
	}
	return ret;
}

/*
	Erase a Directory
*/	
int util_rmdir(
	const char *path,
	UMSDOS_REGISTER &reg,
	int err_expected)
{
	int ret = -1;
	if (reg.getnberr()==0){
		reg.verbose ("rmdir %s\n",path);
		ret = rmdir(path);
		ret = util_check (ret,reg,err_expected,"rmdir %s",path);
	}
	return ret;
}

/*
	Erase a file
*/	
int util_unlink(
	const char *path,
	UMSDOS_REGISTER &reg,
	int err_expected)
{
	int ret = -1;
	if (reg.getnberr()==0){
		reg.verbose ("unlink %s\n",path);
		ret = unlink(path);
		ret = util_check (ret,reg,err_expected,"unlink %s",path);
	}
	return ret;
}
/*
	Rename a file or directory
*/	
int util_rename(
	const char *path1,
	const char *path2,
	UMSDOS_REGISTER &reg,
	int err_expected)
{
	int ret = -1;
	if (reg.getnberr()==0){
		reg.verbose ("rename %s -> %s\n",path1,path2);
		ret = rename(path1,path2);
		ret = util_check (ret,reg,err_expected,"rename %s -> %s"
			,path1,path2);
	}
	return ret;
}
/*
	Change owner and group of a file or directory
*/	
int util_chown(
	const char *path,
	int owner,		/* May be -1 to change only the group */
	int group,		/* May be -1 to change only owner */
	UMSDOS_REGISTER &reg,
	int err_expected)
{
	int ret = -1;
	if (reg.getnberr()==0){
		reg.verbose ("chown %s %d %d\n",path,owner,group);
		ret = chown(path,owner,group);
		ret = util_check (ret,reg,err_expected,"chown %s",path);
	}
	return ret;
}
/*
	Change permissions flags of a file or directory.
*/	
int util_chmod(
	const char *path,
	mode_t mode,
	UMSDOS_REGISTER &reg,
	int err_expected)
{
	int ret = -1;
	if (reg.getnberr()==0){
		reg.verbose ("chmod %s %o\n",path,mode);
		ret = chmod(path,mode);
		ret = util_check (ret,reg,err_expected,"chmod %s",path);
	}
	return ret;
}

/*
	Get information about a file or directory
*/
int util_stat (
	const char *path,
	struct stat *info,
	UMSDOS_REGISTER &reg,
	int err_expected)
{
	int ret = -1;
	if (reg.getnberr()==0){
		reg.verbose ("stat %s\n",path);
		ret = stat (path,info);
		ret = util_check (ret,reg,err_expected,"stat %s",path);
	}
	return ret;
}
/*
	Get information about a file or directory or symbolic links
*/
int util_lstat (
	const char *path,
	struct stat *info,
	UMSDOS_REGISTER &reg,
	int err_expected)
{
	int ret = -1;
	if (reg.getnberr()==0){
		reg.verbose ("lstat %s\n",path);
		ret = lstat (path,info);
		ret = util_check (ret,reg,err_expected,"lstat %s",path);
	}
	return ret;
}

/*
	Get information about an open file.
*/
int util_fstat (
	int fd,
	struct stat *info,
	UMSDOS_REGISTER &reg,
	int err_expected)
{
	int ret = -1;
	if (reg.getnberr()==0){
		reg.verbose ("fstat %d\n",fd);
		ret = fstat (fd,info);
		ret = util_check (ret,reg,err_expected,"fstat %d",fd);
	}
	return ret;
}


/*
	Scan a directory and count the number of entry in it.
*/
int util_dir_count(const char *path, UMSDOS_REGISTER &reg)
{
	int count = -1;
	DIR *dir = opendir (path);
	if (dir == NULL){
		reg.prterr ("Can't open directory %s\n",path);
	}else{
		count = 0;
		while (readdir(dir) != NULL){
			count++;
		}
		closedir(dir);
	}
	return count;
}

/*
	fopen() with error message
*/
FILE *util_fopen (
	const char *fname,
	const char *mode,
	UMSDOS_REGISTER &reg)
{
	FILE *ret = NULL;
	if (reg.getnberr()==0){
		reg.verbose ("fopen %s %s\n",fname,mode);
		ret = fopen (fname,mode);
		if (ret == NULL){
			reg.prterr ("Can't open file %s, mode %s\n"
				"(%s)\n"
				,fname,mode,strerror(errno));
		}
	}
	return ret;
}
/*
	open() with error message.
	Returne -1 if error, the handle if ok.
*/
int util_open (
	const char *fname,
	int mode,
	UMSDOS_REGISTER &reg,
	int err_expected)
{
	int ret = -1;
	if (reg.getnberr()==0){
		reg.verbose ("open %s %o\n",fname,mode);
		ret = open (fname,mode,0);
		util_check (ret==-1,reg,err_expected,"open %s %o",fname,mode);
	}
	return ret;
}
/*
	fgets () with error message
	Return -1 if error
*/
int util_fgets (
	const char *path,
	char *buf,
	int sizebuf,
	FILE *fin,
	UMSDOS_REGISTER &reg,
	int err_expected)
{
	int ret = -1;
	if (reg.getnberr()==0){
		reg.verbose ("fgets %s %d bytes\n",path,sizebuf);
		ret = fgets (buf,sizebuf,fin)!=NULL ? 0 : -1;
		ret = util_check (ret,reg,err_expected,"fgets %s",path);
	}
	return ret;
}

























