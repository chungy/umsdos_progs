#include <stdio.h>
#include <stdarg.h>
/*
	This is a stub because mangle.c is a kernel module
*/
int printk (const char *msg, ...)
{
	int ret;
	va_list list;
	va_start (list,msg);
	ret = vfprintf (stderr,msg,list);
	va_end (list);
	return ret;
}
