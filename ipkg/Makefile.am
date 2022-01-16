EXTRA_DIST = control.in 

clean-generic:
	rm -rf nothing yet

ipkg: Makefile control
	mkdir -p  ./build/CONTROL
	cp control ./build/CONTROL/
	DESTDIR=`pwd`/build fakeroot make install-strip -C ../
	ipkg-build ./build ./
	rm -fr ./build