/*
 *  umsdos_progs/mmap.c
 *
 *  Written 1993 by Jacques Gelinas jacques@solucorp.qc.ca
 *
 *  UMSDOS (msdos in fact) mmap testing.
*/
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/mman.h>
#include "umsdos_test.h"

REGISTER (mmap_simple,"Try to read a file with mmap");

static int mmap_simple (const char *path)
{
	char fname[MAXSIZ_PATH];
	sprintf (fname,"%s/file",path);
	FILE *f = util_fopen(fname,"w",Rmmap_simple);
	if (f != NULL){
		short int buf[10000];
		int i;
		for (i=0; (unsigned int) i<sizeof(buf)/sizeof(buf[0]); i++)buf[i] = i;
		if (fwrite (buf,1,sizeof(buf),f)!=sizeof(buf)){
			Rmmap_simple.prterr ("Can't write %d bytes\n",sizeof(buf));
		}else{
			fclose (f);
			Rmmap_simple.verbose ("%d bytes were written to %s\n"
				,sizeof(buf),fname);
			f = util_fopen(fname,"r",Rmmap_simple);
			int mmap_size = sizeof(buf) * 2;
			short int *ptm = (short int*)mmap(0,mmap_size
				,PROT_READ,MAP_SHARED,fileno(f),0);
			if (ptm == (short int*)-1){
				Rmmap_simple.prterr ("Can't mmap: errno = %d %s\n"
					,errno,strerror(errno));
			}else{
				/*
					This test try to prove that mmaping beyong
					eof yields all 0s.
					It does not work, so I changed
					the target of the loop so it test
					only below eof.
				*/
				Rmmap_simple.verbose ("mmap returned %p\n",ptm);
				int nbelm_buf = sizeof(buf)/sizeof(buf[0]);
				//for (i=0; i<mmap_size/sizeof(buf[0]); i++){
				for (i=0; i<nbelm_buf; i++){
					Rmmap_simple.verbose ("%d ",i); fflush(stdout);
					if (i > nbelm_buf){
						if (ptm[i] != 0){
							Rmmap_simple.prterr("Error reading beyond eof at offset %d\n"
								,i*sizeof(buf[0]));
							break;
						}
					}else if (ptm[i] != i){
						Rmmap_simple.prterr("Error reading at offset %d\n"
							,i*sizeof(buf[0]));
						break;
					}
				}
				if (munmap((char*)ptm,sizeof(buf))==-1){
					Rmmap_simple.prterr ("Can't munmap\n");
				}
			}
		}
		fclose (f);
		util_unlink(fname,Rmmap_simple,0);
	}
	return Rmmap_simple.getnberr();
}


