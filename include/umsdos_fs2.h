/*
 * FIXME: this is hand-made for kernel 2.2.9 for i386 arhitecture.
 *
 * this should be auto generated one day, idea goes like this:
 *
 * do gcc -E some_test_proggy.c which #include "umsdos_fs.h" from kernel source
 * and contains dummy main(), and we compile it using kernel flags. Then we fire up 
 * perl script that replaces off_t with __kernel_off_t and others, and which then kills
 * all remaining typedefs for non __* types. (e.g. namespace-polluting ones)
 * then it generates this file. Any volunteers ?
 *
 * ideally, kernel would not polute namespace with such definitions but
 * would always use __kernel_ typedefs in any header that might be exported
 * (errr, yes, I should do that for umsdos module. It is too late now
 * unfortunetaly, so we have to take this approach. But it WILL be fixed)
 *
 */

#ifndef _UMSDOS_FS2_H
#define _UMSDOS_FS2_H
 
typedef unsigned short	__kernel_dev_t;
typedef unsigned long	__kernel_ino_t;
typedef unsigned short	__kernel_nlink_t;
typedef long		__kernel_off_t;
typedef unsigned short	__kernel_uid_t;
typedef unsigned short	__kernel_gid_t;
typedef long		__kernel_time_t;


struct __my_dirent {
	long		d_ino;
	__kernel_off_t	d_off;
	unsigned short	d_reclen;
	char		d_name[256];  
};

struct umsdos_fake_info {
	char fname[13];
	int len;
};


struct umsdos_dirent {
	unsigned char name_len;	 
	unsigned char flags;	 
	unsigned short nlink;	 
	__kernel_uid_t uid;		 
	__kernel_gid_t gid;		 
	__kernel_time_t atime;		 
	__kernel_time_t mtime;		 
	__kernel_time_t ctime;		 
	__kernel_dev_t rdev;		 
	unsigned short mode;		 
	char spare[12];		 
	char name[220 ];	 
};

struct umsdos_info {
	int msdos_reject;	 
	struct umsdos_fake_info fake;
	struct umsdos_dirent entry;
	__kernel_off_t f_pos;		 
	int recsize;		 
};

 
struct umsdos_ioctl {
	struct __my_dirent dos_dirent;
	struct umsdos_dirent umsdos_dirent;

	struct {
		__kernel_dev_t st_dev;
		unsigned short __pad1;
		__kernel_ino_t st_ino;
		unsigned short st_mode;
		__kernel_nlink_t st_nlink;
		__kernel_uid_t st_uid;
		__kernel_gid_t st_gid;
		__kernel_dev_t st_rdev;
		unsigned short __pad2;
		__kernel_off_t st_size;
		unsigned long st_blksize;
		unsigned long st_blocks;
		__kernel_time_t st_atime;
		unsigned long __unused1;
		__kernel_time_t st_mtime;
		unsigned long __unused2;
		__kernel_time_t st_ctime;
		unsigned long __unused3;
		unsigned long __unused4;
		unsigned long __unused5;
	} stat;
	char version, release;
};
#endif
