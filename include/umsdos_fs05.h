/*
 * FIXME: this was hand-made for kernel 2.4.13 for i386 arhitecture.
 *
 * This is include file for compatibility level 0.5 (AKA "2.4 new UMSDOS/UVFAT")
 *
 * Note: requires /usr/include/asm/types.h
 */

#ifndef _UMSDOS_FS05_H
#define _UMSDOS_FS05_H

#include <asm/types.h>

#define UMSDOS_VERSION 0
#define UMSDOS_RELEASE 5
#define UMSDOS_EMD_FILE "--linux5.---"
#define UMSDOS_EMD_NAMELEN 12
#define UMSDOS_MAXNAME 220
#define UMSDOS_PSDROOT_NAME "linux"
#define UMSDOS_PSDROOT_LEN 5
#define UMSDOS_REC_SIZE 64
#define UMSDOS_HIDDEN 1
#define UMSDOS_HLINK 2
#define UMSDOS_READDIR_DOS 1234
#define UMSDOS_UNLINK_DOS 1235
#define UMSDOS_RMDIR_DOS 1236
#define UMSDOS_STAT_DOS 1237
#define UMSDOS_CREAT_EMD 1238
#define UMSDOS_UNLINK_EMD 1239
#define UMSDOS_READDIR_EMD 1240
#define UMSDOS_GETVERSION 1241
#define UMSDOS_INIT_EMD 1242
#define UMSDOS_DOS_SETUP 1243
#define UMSDOS_RENAME_DOS 1244


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
	__u8 name_len;
	__u8 flags;
	__u16 nlink;
	__u32 uid;
	__u32 gid;
	__u32 atime;
	__u32 mtime;
	__u32 ctime;
	__u16 rdev;
	__u16 mode;
	char spare[8];
	char name[220];
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
