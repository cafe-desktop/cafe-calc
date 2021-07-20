appdatadir = $(datadir)/metainfo
appdata_in_files = cafe-calc.appdata.xml.in
appdata_DATA = $(appdata_in_files:.xml.in=.xml)
$(appdata_DATA): $(appdata_in_files)
	$(AM_V_GEN) $(MSGFMT) --xml --template $< -d $(top_srcdir)/po -o $@

gsettings_SCHEMAS = org.cafe.calc.gschema.xml
@GSETTINGS_RULES@

Utilitiesdir = $(datadir)/applications
Utilities_in_files = cafe-calc.desktop.in
Utilities_DATA = $(Utilities_in_files:.desktop.in=.desktop)
$(Utilities_DATA): $(Utilities_in_files)
	$(AM_V_GEN) $(MSGFMT) --desktop --keyword= --keyword=Name --keyword=GenericName --keyword=Comment --keyword=Keywords --template $< -d $(top_srcdir)/po -o $@

man1_MANS = cafe-calc.1 cafe-calc-cmd.1

EXTRA_DIST = \
	cafe-calc.desktop.in \
	$(appdata_in_files) \
	$(gsettings_SCHEMAS) \
	$(man1_MANS)

DISTCLEANFILES = \
	Makefile.in \
	cafe-calc.desktop \
	$(appdata_DATA)
