
#if GNU_HACK == 1
#	define _SYS_TYPES_H
#	undef __NFDBITS
#	undef __FDMASK
/*#	warning "GNU HACK is enabled. Expect some warning (ssize_t redefined etc)."*/
#endif
