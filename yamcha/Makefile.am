# Process this file with automake to produce Makefile.in
#
# $Id: Makefile.am,v 1.34 2005/09/05 14:50:59 taku Exp $;
#
@SET_MAKE@
AUTOMAKE_OPTIONS = 1.4 no-dependencies

SUBDIRS = src libexec training tests man doc
EXTRA_DIR = perl ruby python swig example
EXTRA_DIST = training/Makefile.in yamcha.spec
bin_SCRIPTS = yamcha-config

dist-hook:
	for subdir in $(EXTRA_DIR); do \
	  cp -rp $$subdir $(distdir); \
	  rm -f $(distdir)/$$subdir/*~; \
	  rm -f $(distdir)/$$subdir/*.{bak,orig}; \
	  rm -rf $(distdir)/$$subdir/CVS; \
	  rm -rf $(distdir)/$$subdir/*/CVS; \
	done

rpm: dist
	env PATH="/usr/bin:$$PATH" rpmbuild -ta @PACKAGE@-@VERSION@.tar.gz

win:
	(cd src; wcmd /c make.bat)

win-dist:
	rm -fr @PACKAGE@-@VERSION@.zip  @PACKAGE@-@VERSION@
	mkdir -p @PACKAGE@-@VERSION@/bin
	mkdir -p @PACKAGE@-@VERSION@/doc 
	mkdir -p @PACKAGE@-@VERSION@/sdk
	mkdir -p @PACKAGE@-@VERSION@/libexec/yamcha
	cp -f example/example.cpp @PACKAGE@-@VERSION@/example
	cp -f libexec/mkmodel     @PACKAGE@-@VERSION@/libexec/yamcha
	cp -f libexec/mkdarts.exe @PACKAGE@-@VERSION@/libexec/yamcha
	cp -f libexec/mktrie.exe  @PACKAGE@-@VERSION@/libexec/yamcha	
	cp -f libexec/pkemine.exe @PACKAGE@-@VERSION@/libexec/yamcha		
	cp -f libexec/mksvmdata   @PACKAGE@-@VERSION@/libexec/yamcha
	cp -f libexec/svm_learn_wrapper @PACKAGE@-@VERSION@/libexec/yamcha
	cp -f libexec/mkparam   @PACKAGE@-@VERSION@/libexec/yamcha
	cp -f libexec/showse    @PACKAGE@-@VERSION@/libexec/yamcha
	cp -f libexec/zipmodel  @PACKAGE@-@VERSION@/libexec/yamcha
	cp -f libexec/selecttag @PACKAGE@-@VERSION@/libexec/yamcha
	cp -f libexec/PKI.pm    @PACKAGE@-@VERSION@/libexec/yamcha
	cp -f libexec/PKE.pm    @PACKAGE@-@VERSION@/libexec/yamcha	
	cp -f libexec/Makefile  @PACKAGE@-@VERSION@/libexec/yamcha
	cp -f src/libyamcha.lib @PACKAGE@-@VERSION@/sdk
	cp -f example/*.cpp     @PACKAGE@-@VERSION@/sdk 
	cp -f src/libyamcha.dll @PACKAGE@-@VERSION@/bin
	cp -f src/yamcha.h   @PACKAGE@-@VERSION@/sdk
	cp -f src/yamcha.exe @PACKAGE@-@VERSION@/bin
	cp -f doc/*.html doc/*.css @PACKAGE@-@VERSION@/doc
	cp -f README COPYING AUTHORS @PACKAGE@-@VERSION@
	zip -r @PACKAGE@-@VERSION@.zip @PACKAGE@-@VERSION@
	rm -fr @PACKAGE@-@VERSION@

script-clean:
	(cd perl; $(MAKE) clean; rm -f Makefile.old Makefile.PL Makefile)
	(cd ruby; $(MAKE) clean; rm -f Makefile *.log extconf.rb)
	(cd python; $(MAKE) clean; rm -f build setup.py)

install-exec-local:
	(cd $(DESTDIR)@prefix@/bin; @LN_S@ -f ../libexec/yamcha/mkmodel yamcha-mkmodel)

uninstall-local:	
	rm -f $(DESTDIR)@prefix@/bin/yamcha-mkmodel

dist-all-package:
	(test -f Makefile) && $(MAKE) script-clean distclean
	./configure
	$(MAKE) dist
	$(MAKE) clean
	$(MAKE) win
	$(MAKE) win-dist

export-package:
	scp yamcha-@VERSION@.tar.gz   chasen.org:public_html/software/yamcha/src
	scp yamcha-@VERSION@.zip      chasen.org:public_html/software/yamcha/win
	scp doc/*.html                 chasen.org:public_html/software/yamcha/	
#	scp ~/.rpm/SRPMS/yamcha-@VERSION@*.src.rpm      chasen.org:public_html/software/yamcha/redhat-9.x/SRPMS/
#	scp ~/.rpm/RPMS/i386/yamcha-*@VERSION@*.i386.rpm chasen.org:public_html/software/yamcha/redhat-9.x/RPMS/i386/

