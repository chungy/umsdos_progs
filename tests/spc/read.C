/*
 *  umsdos_progs/tests/spc/hlink.c
 *
 *  Written 1994 by Jacques Gelinas jacques@solucorp.qc.ca
 *
 *  MSDOS read write test.
*/
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdlib.h>
#include "umsdos_test.h"

static char read_getvalue (int pos, int line_length)
{
	int y = pos % line_length;
	char ret = y + ' ';
	if (y == line_length) ret = '\n';
	return ret;
}

/*
	Initialise a file with a complex but computed pattern.
	It returns -1 if error, or the size written (which may be different
	from the size of the file in text mode).
*/
static int read_initfile (
	const char *fname,		// File to initialise
	UMSDOS_REGISTER &reg,
	int line_length)
{
	int ret = -1;
	FILE *fout = util_fopen (fname,"w",reg);
	if (fout != NULL){
		ret = 0;
		for (int i=0; i<500; i++){
			char buf[50];
			for (int j=0; j<50; j++,ret++){
				buf[j] = read_getvalue(ret,line_length);
			}
			if (fwrite (buf,1,sizeof(buf),fout) != sizeof(buf)){
				reg.prterr ("fwrite failed, errno %d '%s'\n"
					,errno,strerror(errno));
				ret = -1;
				break;
			}			
		}
		fclose (fout);
	}
	return ret;
}
/*
	Read and check the content for an expect content.
	Each read operation is done with a specific block size.
	The length is also tested
*/

static int read_checkfile (
	const char *fname,		// File to read and validate
	UMSDOS_REGISTER &reg,
	int blocksize,
	int length,
	int line_length)
{
	int ret = -1;
	int fd = util_open (fname,O_RDONLY,reg,0);
	if (fd != -1){
		char *buf = (char*)malloc(blocksize);
		if (buf == NULL){
			reg.prterr ("Can't allocate %d bytes\n",blocksize);
		}else{
			ret = 0;
			while (reg.getnberr()==0){
				int readlen = read (fd,buf,blocksize);
				if (readlen <= 0) break;
				for (int j=0; j<readlen; j++,ret++){
					char expected = read_getvalue(ret,line_length);
					if (buf[j] != expected){
						reg.prterr ("Read mismatch"
							" [%d] %d != %d\n"
							,ret,buf[j],expected);
						ret = -1;
						break;
					}
				}
			}
			if (reg.getnberr()==0 && ret != length){
				reg.prterr ("Size mismatch"
					"expected %d, got %d\n"
					,length,ret);
				ret = -1;
			}
			free (buf);
		}
		close (fd);
	}
	return ret;
}


static int read_write (
	const char *basepath,
	UMSDOS_REGISTER &reg,
	int line_length)
{
	/* #Specification: utstspc / read write
		We write files with special pattern and read it back
		using different blocking scheme. This is to make sure
		the new read ahead support in the msdos fs is not screwing.
	*/
	char fname[MAXSIZ_PATH];
	sprintf (fname,"%s/file.c",basepath);
	int size = read_initfile(fname,reg,line_length);
	if (size != -1){
		reg.verbose ("Creating a %d bytes file %s (line length %d)\n"
			,size,fname,line_length);
		static int tb[]={
			4096,2048,1024,512,513,255,50,256*1024
		};
		for (unsigned int i=0; i<sizeof(tb)/sizeof(tb[0])
			&& reg.getnberr()==0; i++){
			reg.verbose ("\tTesting block size %d\n",tb[i]);
			read_checkfile (fname,reg,tb[i],size,line_length);
		}
		util_unlink (fname,reg,0);
	}
	return reg.getnberr();
}

/*
	The following test are the same, but with different mount option.
	The REGISTER system want one function per test, so we provide simple
	one which simply call read_write().
*/

REGISTER_SPC (rw_bin,"Many read write tests (conv=binary)",NULL,1);

static int rw_bin(const char *basepath)
{
	int ret = read_write (basepath,Rrw_bin,50);
	if (ret == 0) read_write (basepath,Rrw_bin,5);
	if (ret == 0) read_write (basepath,Rrw_bin,2);
	return ret;
}
REGISTER_SPC (rw_text,"Many read write tests (conv=text)","conv=text",0);

static int rw_text(const char *basepath)
{
	/* #Specification: utstspc / read write / text mode
		The read/write test is done is text conversion mode also.
		This prove the read ahead of fs/msdos/file.c is working
		even in conv=text mode.
	*/
	int ret = read_write (basepath,Rrw_text,50);
	if (ret == 0) read_write (basepath,Rrw_text,5);
	if (ret == 0) read_write (basepath,Rrw_text,2);
	return ret;
}




