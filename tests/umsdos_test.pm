/* printk.c 12/05/94 07.17.02 */
/* register.c 16/11/94 22.51.58 */
PROTECTED UMSDOS_REGISTER::UMSDOS_REGISTER (int (*_fct)(const char *),
	 const char *_path,
	 const char *_desc);
PUBLIC UMSDOS_REGISTER_GEN::UMSDOS_REGISTER_GEN (int (*_fct)(const char *),
	 const char *_path,
	 const char *_desc);
PUBLIC UMSDOS_REGISTER_GEN *UMSDOS_REGISTER_GEN::getnext (void);
PUBLIC UMSDOS_REGISTER_SPC::UMSDOS_REGISTER_SPC (int (*_fct)(const char *),
	 const char *_path,
	 const char *_desc,
	 const char *_mount_option,
	 int _mount_umsdos);
PUBLIC UMSDOS_REGISTER_SPC *UMSDOS_REGISTER_SPC::getnext (void);
PUBLIC int UMSDOS_REGISTER::call (const char *path);
PUBLIC void UMSDOS_REGISTER::intro (void);
PUBLIC void UMSDOS_REGISTER::prterr (const char *ctl, ...);
PUBLIC void UMSDOS_REGISTER::warning (const char *ctl, ...);
PUBLIC void UMSDOS_REGISTER::verbose (const char *ctl, ...);
PUBLIC int UMSDOS_REGISTER::getnberr (void);
PUBLIC STATIC void UMSDOS_REGISTER_GEN::sort (void);
PUBLIC STATIC void UMSDOS_REGISTER_SPC::sort (void);
/* util.c 09/01/95 23.20.58 */
/* utilspc.c 16/11/94 23.05.58 */
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
