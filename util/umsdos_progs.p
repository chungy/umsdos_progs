/* main.c 13/01/95 01.20.46 */
/* numconst.c 20/01/94 01.31.18 */
int octal_toi (const char *parm, int *error);
int hexa_toi (const char *parm, int *error);
int deci_toi (const char *parm, int *error);
int xtoi (const char *parm, int *error);
/* printk.c 12/05/94 07.13.08 */
int printk (const char *msg, ...);
/* udosctl.c 13/01/95 01.11.46 */
int udosctl_main (int argc, char *argv[]);
/* umssetup.c 13/01/95 01.11.32 */
int umssetup_main (int argc, char *argv[]);
/* umssync.c 24/01/95 23.05.22 */
int umssync_main (int argc, char *argv[]);
/* umsdosio.c */
int UM_ioctl(int fd, int cmd, struct umsdos_ioctl *data);
void UM_rewind(int fd);
int UM_getversion(int fd, int *version, int *release);
int UM_initemd(int fd);
int UM_readdir(int fd, struct dirent *dirent);
int UM_ureaddir(int fd, struct umsdos_dirent *udirent, struct dirent *dirent);
int UM_create(int fd, const char *fname, mode_t mode, time_t atime, time_t ctime, time_t mtime, int uid, int gid, int rdev);
int UM_dosunlink(int fd, const char *fname);
int UM_dosrmdir(int fd, const char *fname);
int UM_dosrename(int fd, const char *src, const char *dst);
int UM_urmdir(int fd, const char *fname);
int UM_uunlink(int fd, const char *fname);
int UM_dosstat(int fd, const char *fname, struct stat *fstat);
int UM_dossetup(int fd, int uid, int gid, mode_t mode);
int UM_ispromoted(int fd);
int UM_open(const char *dpath, int _verbose, int quit);
void UM_close(int *fd);
int UM_isok(int fd);
