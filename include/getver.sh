#!/bin/sh
#
# call from ../.., not from current dir!
#

VER=`awk '/define UPROG_VERSION/ {print \$3}'  umsdos_progs/util/umsdos_progs.h` 
REL=`awk '/define UPROG_RELEASE/ {print \$3}'  umsdos_progs/util/umsdos_progs.h`

FULL=${VER}.${REL}

# fix .spec file
sed "s/^Version:.*/Version: ${FULL}/" umsdos_progs/umsdos_progs.spec.tmpl > umsdos_progs/umsdos_progs.spec

# return result
echo $FULL

