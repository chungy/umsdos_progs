/* this is user-editable config file, you MUST edit it !!!  */

#ifndef _UMS_CONFIG_H
#define _UMS_CONFIG_H

/*
 * define BE_UVFAT to:
 *
 * 0 == no, be plain UMSDOS
 * 1 == yes, be UVFAT, does not work on UMSDOS !
 *
 * note that UMSDOS and UVFAT will have different compatibility versions,
 * so umsdos tools will refuse to run on uvfat kernel module and vice versa!
 *
 * FIXME: one day, we will build both utils in same build. but not today.
 *
 * IMPORTANT: if you get error about this being -1, you havent RTFM and set
 * this to valid value. Read the toplevel README carefully.
 */
 
#define BE_UVFAT -1

/* 
 * define GNU_HACK to: 
 *
 * 0 = no hack, should work fine for libc5 based system
 * 1 = should work with some warnings on libc6 (glibc) based systems
 *
 * 0 should be OK for any newer (glibc 2.2+, kernel 2.4.x+) system
 * also on any system with BE_UVFAT == 1 ?
 *
 * If you don't know, try '0' and only if it doesn't work use '1'
 */
 
#define GNU_HACK 0

/*
 * define KERN_22X to:
 *
 * modify tests to match 2.2.x+ kernels dentry-setup
 * (e.g. you can do 'rmdir .' etc)
 *
 */
#define KERN_22X

/*
 * define UID/GID for gen/sticky.C tests
 * that UID and GID must be of user not in same group with group ROOT
 * defaults should work on most systems...
 *
 */
#define MYUID 1
#define MYGID 1

#ifndef BE_UVFAT
#error BE_UVFAT MUST BE DEFINED TO EITHER 0 OR 1
#else
# if ((BE_UVFAT != 0) && (BE_UVFAT != 1))
# error Read the README! You must edit ems_config.h first!
# endif
#endif
#endif
