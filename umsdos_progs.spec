Name: umsdos_progs
Version: 1.13
Release: 1
Summary: Utilities for doing UMSDOS FS operations
Source0: umsdos_progs-%{version}.tgz
ExclusiveOS: Linux
ExclusiveArch: i386

Copyright: GPL
Group: Base/Kernel
URL: http://cvs.linux.hr/
Vendor: Matija Nalis <mnalis@jagor.srce.hr>

%description
These are the utilities for doing UMSDOS file system operations. You can use
these programs to have long filenames on a DOS partition under Linux and
still have those files available under DOS (with mangled filenames).

This version can be used (and rebuild) on 2.2.x (and 2.3.x) kernels with
glibc 2.0/2.1 based systems. To use it on 2.0.x and/or libc5 kernels, you
need to change the defines in include/ums_config.h and recompile.


%prep
%setup -n umsdos_progs

%build
make all

%install
make install

%files
%doc README
/sbin/umssetup
/sbin/udosctl
/sbin/umssync
/usr/man/man8/umssync.8

%changelog
* Mon Jun 28 1999 Matija Nalis <mnalis@jagor.srce.hr>
 - .spec file created

