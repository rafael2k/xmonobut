EXTRA_DIST = xmonobut.xpm debian/control debian/changelog debian/copyright \
		debian/postinst debian/prerm debian/rules
SUBDIRS = ipkg doc

LDADD = @X_LIBS@ -lX11 -lXpm -lXext

bin_PROGRAMS = xmonobut

xmonobut_SOURCES = main.c tray.c tray.h

ipkg: Makefile
	$(MAKE) -C ./ipkg ipkg

