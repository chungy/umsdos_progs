#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "umsdos_test.h"


int register_verbose=1;
UMSDOS_REGISTER *first = NULL;


PROTECTED UMSDOS_REGISTER::UMSDOS_REGISTER(
	int (*_fct)(const char *),
	const char *_path,		// Key to acces the test
	const char *_desc)		// Description of the test
{
	nberr = 0;
	intro_shown = 0;
	path = _path;
	desc = _desc;
	fct = _fct;
}
/*
	Record a path and an associate test function in a link list.
	The idea here is that simply by linking a new sources, the main
	will know how to dispatch the test function.

	This object also controle error reporting and log the status
	of the test case.
*/

PUBLIC UMSDOS_REGISTER_GEN::UMSDOS_REGISTER_GEN(
	int (*_fct)(const char *),
	const char *_path,		// Key to acces the test
	const char *_desc)		// Description of the test
	: UMSDOS_REGISTER (_fct,_path,_desc)
{
	next = first;
	first = this;
}

PUBLIC UMSDOS_REGISTER_GEN *UMSDOS_REGISTER_GEN::getnext()
{
	return (UMSDOS_REGISTER_GEN*)next;
}

UMSDOS_REGISTER_GEN *UMSDOS_REGISTER_GEN::first = NULL;
UMSDOS_REGISTER_SPC *UMSDOS_REGISTER_SPC::first = NULL;
/*
	Special case for utstspc
*/
PUBLIC UMSDOS_REGISTER_SPC::UMSDOS_REGISTER_SPC(
	int (*_fct)(const char *),
	const char *_path,		// Key to acces the test
	const char *_desc,		// Description of the test
	const char *_mount_option,	// Value of the -o option of the mount
					// command. May be NULL
	int _mount_umsdos)		// Mount -t umsdos or mount -t msdos
	: UMSDOS_REGISTER (_fct,_path,_desc)
{
	mount_umsdos = _mount_umsdos;
	mount_option = _mount_option;
	next = first;
	first = this;
}
PUBLIC UMSDOS_REGISTER_SPC *UMSDOS_REGISTER_SPC::getnext()
{
	return (UMSDOS_REGISTER_SPC*)next;
}
/*
	Execute the test case
*/
PUBLIC int UMSDOS_REGISTER::call(const char *path)
{
	int ret = 0;
	if (fct != NULL) ret = (*fct)(path);
	return ret;
}

/*
	Print the intro of the test case.
	This function disable itself after the first call.
*/
PUBLIC void UMSDOS_REGISTER::intro()
{
	if (!intro_shown){
		printf ("TEST CASE: (%s) %s\n",path,desc);
		intro_shown = 1;
	}
}
/*
	Print an error message and disable operation
*/
PUBLIC void UMSDOS_REGISTER::prterr (
	const char *ctl,
	...)
{
	intro();
	va_list list;
	va_start (list,ctl);
	vprintf (ctl,list);
	va_end (list);
	nberr++;
}
/*
	Print a warning
*/
PUBLIC void UMSDOS_REGISTER::warning (
	const char *ctl,
	...)
{
	intro();
	va_list list;
	va_start (list,ctl);
	vprintf (ctl,list);
	va_end (list);
}

/*
	Print conditionnally something.
	Normally, the test case process silently, unless there is an error.
	An option of the test program (-v) activate some messages.
*/
PUBLIC void UMSDOS_REGISTER::verbose (
	const char *ctl,
	...)
{
	if (register_verbose){
		printf ("\t");
		va_list list;
		va_start (list,ctl);
		vprintf (ctl,list);
		va_end (list);
	}
}

/*
	Return the number of error so far in this test case.
	Most operation are disabled after the first error.
*/
PUBLIC int UMSDOS_REGISTER::getnberr()
{
	return nberr;
}

static int cmp (const void *p1, const void *p2)
{
	UMSDOS_REGISTER *up1 = *(UMSDOS_REGISTER**)p1;
	UMSDOS_REGISTER *up2 = *(UMSDOS_REGISTER**)p2;
	return strcmp(up1->path, up2->path);
}
/*
	Sort the link list by path.
*/
static UMSDOS_REGISTER *register_sort (UMSDOS_REGISTER *first)
{
	UMSDOS_REGISTER *pt = first;
	int count = 0;
	while (pt != NULL){
		count++;
		pt = pt->next;
	}
	if (count > 0){
		UMSDOS_REGISTER **tb = new UMSDOS_REGISTER *[count+1];
		if (tb == NULL){
			fprintf (stderr,"Not enough memory to sort %d test case\n",count);
			exit (-1);
		}else{
			pt = first;
			int no = 0;
			while (pt != NULL){
				tb[no++] = pt;
				pt = pt->next;
			}
			qsort (tb,count,sizeof(UMSDOS_REGISTER*),cmp);
			first = tb[0];
			tb[count] = NULL;
			for (int i=0; i<count; i++){
				tb[i]->next = tb[i+1];
			}
			delete [] tb;
		}
	}
	return first;
}

PUBLIC STATIC void UMSDOS_REGISTER_GEN::sort()
{
	first = (UMSDOS_REGISTER_GEN*)register_sort(first);
}
PUBLIC STATIC void UMSDOS_REGISTER_SPC::sort()
{
	first = (UMSDOS_REGISTER_SPC*)register_sort(first);
}

