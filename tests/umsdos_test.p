/* printk.c 12/05/94 07.17.02 */
extern "C" int printk (const char *msg, ...);
/* register.c 16/11/94 22.51.58 */
/* util.c 09/01/95 23.20.58 */
int util_mkdir (const char *path,
	 int mode,
	 UMSDOS_REGISTER&reg,
	 int err_expected);
int util_chdir (const char *path,
	 UMSDOS_REGISTER&reg,
	 int err_expected);
int util_create (const char *path,
	 int mode,
	 UMSDOS_REGISTER&reg,
	 int err_expected);
int util_mkdev (const char *path,
	 int mode,
	 int is_chr,
	 int major,
	 int minor,
	 UMSDOS_REGISTER&reg,
	 int err_expected);
int util_mkfifo (const char *path,
	 int mode,
	 UMSDOS_REGISTER&reg,
	 int err_expected);
int util_symlink (const char *path,
	 const char *slink,
	 UMSDOS_REGISTER&reg,
	 int err_expected);
int util_hardlink (const char *path,
	 const char *hlink,
	 UMSDOS_REGISTER&reg,
	 int err_expected);
int util_rmdir (const char *path,
	 UMSDOS_REGISTER&reg,
	 int err_expected);
int util_unlink (const char *path,
	 UMSDOS_REGISTER&reg,
	 int err_expected);
int util_rename (const char *path1,
	 const char *path2,
	 UMSDOS_REGISTER&reg,
	 int err_expected);
int util_chown (const char *path,
	 int owner,
	 int group,
	 UMSDOS_REGISTER&reg,
	 int err_expected);
int util_chmod (const char *path,
	 mode_t mode,
	 UMSDOS_REGISTER&reg,
	 int err_expected);
int util_stat (const char *path,
	 struct stat *info,
	 UMSDOS_REGISTER&reg,
	 int err_expected);
int util_lstat (const char *path,
	 struct stat *info,
	 UMSDOS_REGISTER&reg,
	 int err_expected);
int util_fstat (int fd,
	 struct stat *info,
	 UMSDOS_REGISTER&reg,
	 int err_expected);
int util_dir_count (const char *path, UMSDOS_REGISTER&reg);
FILE *util_fopen (const char *fname,
	 const char *mode,
	 UMSDOS_REGISTER&reg);
int util_open (const char *fname,
	 int mode,
	 UMSDOS_REGISTER&reg,
	 int err_expected);
int util_fgets (const char *path,
	 char *buf,
	 int sizebuf,
	 FILE *fin,
	 UMSDOS_REGISTER&reg,
	 int err_expected);
/* utilspc.c 16/11/94 23.05.58 */
void utilspc_check (void);
int utilspc_isfloppy (const char *drive);
int utilspc_format (const char *drive, UMSDOS_REGISTER&reg);
int utilspc_mount (const char *drive,
	 const char *path,
	 int umsdos,
	 const char *option,
	 UMSDOS_REGISTER&reg);
int utilspc_umount (const char *path, UMSDOS_REGISTER&reg);
int utilspc_usync (const char *path, UMSDOS_REGISTER&reg);
int utilspc_mkdir_udos (const char *dpath,
	 int mode,
	 UMSDOS_REGISTER&reg);
/* utstgen.c 20/11/94 23.45.02 */
/* utstspc.c 16/11/94 22.58.52 */
/* dev.c 09/01/95 23.27.52 */
/* dir.c 21/11/94 00.03.32 */
/* file.c 30/01/94 01.21.02 */
/* hlink.c 30/01/94 01.21.44 */
/* mmap.c 10/12/94 23.58.14 */
/* mount.c 15/11/93 23.32.42 */
/* multi.c 30/01/94 01.22.08 */
/* name.c 05/12/94 13.13.10 */
/* perm.c 21/09/93 00.31.58 */
/* pipe.c 20/09/93 22.55.42 */
/* rename.c 18/05/94 23.06.14 */
/* sticky.c 19/01/95 20.45.24 */
/* syml.c 30/01/94 01.22.22 */
/* time.c 27/09/93 21.57.44 */
/* read.c 17/11/94 00.47.02 */
