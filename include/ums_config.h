/* this is user-editable config file, please do edit it !!!  */

/* 
 * define GNU_HACK to: 
 *
 * 0 = no hack, should work fine for libc5 based system
 * 1 = should work with some warnings on libc6 (glibc) based systems
 */
 
#define GNU_HACK 1

/*
 * define KERN_22X to:
 *
 * modify tests to match 2.2.x kernels dentry-setup
 * (e.g. you can do 'rmdir .' etc)
 *
 */
#define KERN_22X

/*
 * define UID/GID for gen/sticky.C tests
 * that UID and GID must be of user not in same group sa group ROOT
 * defaults should work on most systems...
 *
 */
#define MYUID 1
#define MYGID 1
