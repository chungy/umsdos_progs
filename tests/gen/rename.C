/*
 *  umsdos_tests/gen/rename.c
 *
 *  Written 1994 by Jacques Gelinas jacques@solucorp.qc.ca
 *
 *  UMSDOS rename testing
*/
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include "umsdos_test.h"
#include "ums_config.h"

REGISTER (rename_exist,"Rename a file and the target exists");

enum r_test {
	r_file,			// Destination is a simple file
	r_file_link,	// Destination is a hard link to another file
	r_dir,			// Destination is an empty dir
	r_dir_full		// Destination is a non empty dir
};

/*
	Test sequence in 2 directory (path1 may be the same as path2).
*/
static void rename_seq (
	const char *path1,		// directory containing the source
							// file or directory
	const char *path2,		// directory containing the destination
							// file or directory
	const char *pathl,		// Path for the original file of the link
	UMSDOS_REGISTER &reg,
	r_test dst_type,		// Identify the type of destination
	int err_expected)
{
	if (reg.getnberr()==0){
		char fname1[MAXSIZ_PATH];
		sprintf (fname1,"%s/file1",path1);
		char fname2[MAXSIZ_PATH];
		sprintf (fname2,"%s/file2",path1);
		char sfile[MAXSIZ_PATH];
		sprintf (sfile,"%s/sfile",fname2);
		char flink[MAXSIZ_PATH];
		sprintf (flink,"%s/link",pathl);
		util_create (fname1,0777,reg,0);
		switch (dst_type){
		case r_file:
			util_create (fname2,0777,reg,0);
			break;
		case r_file_link:
			util_create (flink,0777,reg,0);
			util_hardlink  (flink,fname2,reg,0);
			break;
		case r_dir:
			util_mkdir (fname2,0777,reg,0);
			break;
		case r_dir_full:
			util_mkdir (fname2,0777,reg,0);
			util_create (sfile,0777,reg,0);
			break;
		}
		if (reg.getnberr()==0){
			util_rename (fname1,fname2,reg,err_expected);
			if (reg.getnberr()==0){
				switch(dst_type){
				case r_file:
					util_unlink (fname2,reg,0);
					break;
				case r_file_link:
					util_unlink (fname2,reg,0);
					util_unlink (flink,reg,0);
					break;
				case r_dir:
#ifdef KERN_22X
					util_unlink (fname1,reg,0);
					util_rmdir (fname2,reg,0);
#else
					util_unlink (fname2,reg,0);
#endif
					break;
				case r_dir_full:
					util_unlink (sfile,reg,0);
					util_rmdir (fname2,reg,0);
					util_unlink (fname1,reg,0);
					break;
				}
			}
		}
	}
}
/*
	Test sequence in 2 directory (path1 may be the same as path2).
*/
static void rename_seq (
	const char *path1,		// directory containing the source
							// file or directory
	const char *path2,		// directory containing the destination
							// file or directory
	const char *pathl,		// Path for the original file of the link
	UMSDOS_REGISTER &reg)
{
	rename_seq (path1,path2,pathl,reg,r_file,0);
	rename_seq (path1,path2,pathl,reg,r_file_link,0);
#ifdef KERN_22X
	rename_seq (path1,path2,pathl,reg,r_dir,EISDIR);
#else
	rename_seq (path1,path2,pathl,reg,r_dir,0);
#endif
	rename_seq (path1,path2,pathl,reg,r_dir_full,ENOTEMPTY);
}

static int rename_exist (const char *path)
{
	/* #Specification: utstgen / rename / destination exist
		The following rename tests are done. The source is
		always file1 and the destination file2 always exist.
		file2 is a file or a directory. Here a the different
		case.

		file1 is a file, file2 is a file.
		file1 is a file, file2 is a hard link to a file.
		file1 is a file, file2 exist and is a empty directory.
		file1 is a file, file2 exist and is a non empty directory.

		file1 is a directory, file2 is a file.
		file1 is a directory, file2 is a hard link to a file.
		file1 is a directory, file2 exist and is a empty directory.
		file1 is a directory, file2 exist and is a non empty directory.

		This sequence is performed in the same directory and accros.
		The following combination are tested.

		path/dir1	-> path/dir1
		path/dir1	-> path/dir2
		path/dir1	-> path
		path		-> path/dir1
	*/
	char dir1[MAXSIZ_PATH];
	char dir2[MAXSIZ_PATH];
	char dir3[MAXSIZ_PATH];
	sprintf (dir1,"%s/dir1",path);
	sprintf (dir2,"%s/dir2",path);
	sprintf (dir3,"%s/dir3",path);
	util_mkdir (dir1,0777,Rrename_exist,0);
	util_mkdir (dir2,0777,Rrename_exist,0);
	util_mkdir (dir3,0777,Rrename_exist,0);

	rename_seq (path,path,path,Rrename_exist);
	rename_seq (path,dir2,path,Rrename_exist);
	rename_seq (path,dir2,dir3,Rrename_exist);

	rename_seq (dir1,path,path,Rrename_exist);
	rename_seq (dir1,dir2,path,Rrename_exist);
	rename_seq (dir1,dir2,dir3,Rrename_exist);

//	rename_seq (dir1,path,path,Rrename_exist);
//	rename_seq (dir1,path,dir3,Rrename_exist);

	util_rmdir (dir1,Rrename_exist,0);
	util_rmdir (dir2,Rrename_exist,0);
	util_rmdir (dir3,Rrename_exist,0);
	return Rrename_exist.getnberr();
}

	


