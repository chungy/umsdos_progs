#ifndef UMSDOS_TEST_H
#define UMSDOS_TEST_H

#define MAXSIZ_PATH		200		// Longest path

class UMSDOS_REGISTER{	// All UMSDOS_REGISTER object must be static
						// So the link list will be initialised before
						// main.
	int nberr;		// Number of error signal so far
	int intro_shown;	// Have we print the heading of the test
					// once ?
public:
	const char *path;
	const char *desc;
	int (*fct)(const char *);
	UMSDOS_REGISTER *next;
	/*~PROTOBEG~ UMSDOS_REGISTER */
protected:
	UMSDOS_REGISTER (int (*_fct)(const char *),
		 const char *_path,
		 const char *_desc);
public:
	int call (const char *path);
	int getnberr (void);
	void intro (void);
	void prterr (const char *ctl, ...);
	void verbose (const char *ctl, ...);
	void warning (const char *ctl, ...);
	/*~PROTOEND~ UMSDOS_REGISTER */
};
class UMSDOS_REGISTER_GEN: public UMSDOS_REGISTER{
		// All UMSDOS_REGISTER object must be static
		// So the link list will be initialised before
		// main.
public:
	static UMSDOS_REGISTER_GEN *first;
	/*~PROTOBEG~ UMSDOS_REGISTER_GEN */
public:
	UMSDOS_REGISTER_GEN (int (*_fct)(const char *),
		 const char *_path,
		 const char *_desc);
	UMSDOS_REGISTER_GEN *getnext (void);
	static void sort (void);
	/*~PROTOEND~ UMSDOS_REGISTER_GEN */
};

class UMSDOS_REGISTER_SPC: public UMSDOS_REGISTER{
		// All UMSDOS_REGISTER object must be static
		// So the link list will be initialised before
		// main.
public:
	const char *mount_option;	// value of -o option of mount or NULL
	int mount_umsdos;
	static UMSDOS_REGISTER_SPC *first;
	/*~PROTOBEG~ UMSDOS_REGISTER_SPC */
public:
	UMSDOS_REGISTER_SPC (int (*_fct)(const char *),
		 const char *_path,
		 const char *_desc,
		 const char *_mount_option,
		 int _mount_umsdos);
	UMSDOS_REGISTER_SPC *getnext (void);
	static void sort (void);
	/*~PROTOEND~ UMSDOS_REGISTER_SPC */
};

extern int register_verbose;

// For utstgen
#define REGISTER(f,d) \
	static int f(const char*); \
	UMSDOS_REGISTER_GEN R##f(f,__FILE__ "/" #f,d);

// For utstspc
#define REGISTER_SPC(f,d,m,u) \
	static int f(const char*); \
	UMSDOS_REGISTER_SPC R##f(f,__FILE__ "/" #f,d,m,u);


#include <stdio.h>
#include <sys/types.h>

#include "umsdos_test.p"

#endif

