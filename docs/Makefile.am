# Makefile for KevEdit documentation

AUTOMAKE_OPTIONS = 1.7

pkgdata_DATA = kevedit.zml

HELPFILES = creature.hlp index.hlp item.hlp kadvance.hlp kbackbuf.hlp kbasics.hlp kbrdinfo.hlp kevedit.hlp kkeys.hlp knew.hlp kobjedit.hlp kselect.hlp kstats.hlp ksupport.hlp ktileinf.hlp kwldinfo.hlp lang.hlp langref.hlp langtut.hlp terrain.hlp

EXTRA_DIST = makehelp.sh $(HELPFILES)

MOSTLYCLEANFILES = kevedit.zml -R kevedit/

kevedit.zml: $(srcdir)/*.hlp
	chmod a+x $(srcdir)/makehelp.sh
	$(srcdir)/makehelp.sh $(srcdir) .

# Rules to generate doxygen documentation
dox: kevedit/html/index.html

kevedit/html/index.html: $(srcdir)/kevedit.dox
	SRCPREFIX=$(srcdir)/ doxygen $(srcdir)/kevedit.dox

