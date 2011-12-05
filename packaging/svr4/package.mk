.PHONY: dist-svr4

SVR4_WORKSPACE=$(shell pwd)/build

dist-svr4: dist
	rm -rf ${SVR4_WORKSPACE}
	mkdir -p ${SVR4_WORKSPACE}/src ${SVR4_WORKSPACE}/install ${SVR4_WORKSPACE}/pkg
	(cd ${SVR4_WORKSPACE}/src; gzcat ../../${PACKAGE}-${VERSION}.tar.gz | tar xf -)
	(cd ${SVR4_WORKSPACE}/src/${PACKAGE}-${VERSION}; \
	   ./configure --prefix=/usr --libdir=/usr/lib --disable-64bit --disable-static;)
	(cd ${SVR4_WORKSPACE}/src/${PACKAGE}-${VERSION}; $(MAKE) install DESTDIR=${SVR4_WORKSPACE}/install;)
	rm -rf ${SVR4_WORKSPACE}/src/${PACKAGE}-${VERSION}; \
	(cd ${SVR4_WORKSPACE}/src; gzcat ../../${PACKAGE}-${VERSION}.tar.gz | tar xf -)
	(cd ${SVR4_WORKSPACE}/src/${PACKAGE}-${VERSION}; \
	   ./configure --prefix=/usr --libdir=/usr/lib/64 --enable-64bit --disable-static;)
	(cd ${SVR4_WORKSPACE}/src/${PACKAGE}-${VERSION}; $(MAKE) install DESTDIR=${SVR4_WORKSPACE}/install;)
	cp packaging/svr4/* ${SVR4_WORKSPACE}/install
	cp LICENSE ${SVR4_WORKSPACE}/install
	pkgmk -o -d ${SVR4_WORKSPACE}/pkg -f ${SVR4_WORKSPACE}/install/prototype.libvbucket -r ${SVR4_WORKSPACE}/install ARCH=`uname -m` VERSION=${LIBVBUCKET_API_CURRENT}.${LIBVBUCKET_API_REVISION}.${LIBVBUCKET_API_AGE}
	pkgmk -o -d ${SVR4_WORKSPACE}/pkg -f ${SVR4_WORKSPACE}/install/prototype.libvbucket-dev -r ${SVR4_WORKSPACE}/install ARCH=`uname -m` VERSION=${LIBVBUCKET_API_CURRENT}.${LIBVBUCKET_API_REVISION}.${LIBVBUCKET_API_AGE}
	pkgtrans -o -s ${SVR4_WORKSPACE}/pkg `pwd`/libvbucket-dev-${LIBVBUCKET_API_CURRENT}.${LIBVBUCKET_API_REVISION}.${LIBVBUCKET_API_AGE}.pkg libvbucket-dev
	pkgtrans -o -s ${SVR4_WORKSPACE}/pkg `pwd`/libvbucket-${LIBVBUCKET_API_CURRENT}.${LIBVBUCKET_API_REVISION}.${LIBVBUCKET_API_AGE}.pkg libvbucket
	rm -rf ${SVR4_WORKSPACE}
