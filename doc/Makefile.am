CLEANFILES = xmonobut.1
MAINTAINERCLEANFILES = Makefile.in
EXTRA_DIST = xmonobut.1.in
man_MANS = xmonobut.1

distclean-local:
	rm -f *\~

xmonobut.1: xmonobut.1.in
	sed -e "s,@pkgdatadir@,$(pkgdatadir)," xmonobut.1.in > xmonobut.1
