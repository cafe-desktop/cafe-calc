bin_PROGRAMS = cafe-calc cafe-calc-cmd
noinst_PROGRAMS = test-mp test-mp-equation

TESTS = test-mp test-mp-equation

AM_CPPFLAGS = \
	-DVERSION=\""$(VERSION)"\" \
	-DLOCALE_DIR=\""$(localedir)"\" \
	-DGETTEXT_PACKAGE=\"$(GETTEXT_PACKAGE)\" \
	$(WARN_CFLAGS) \
	$(CAFE_CALC_CFLAGS)

BUILT_SOURCES = \
	cafe-calc-resources.c \
	cafe-calc-resources.h

nodist_cafe_calc_SOURCES= $(BUILT_SOURCES)

cafe_calc_SOURCES = \
	cafe-calc.c \
	currency.c \
	currency.h \
	currency-manager.c \
	currency-manager.h \
	math-buttons.c \
	math-buttons.h \
	math-converter.c \
	math-converter.h \
	math-display.c \
	math-display.h \
	math-equation.c \
	math-equation.h \
	math-preferences.c \
	math-preferences.h \
	math-variables.c \
	math-variables.h \
	math-variable-popup.c \
	math-variable-popup.h \
	math-window.c \
	math-window.h \
	mp.c \
	mp.h \
	mp-binary.c \
	mp-convert.c \
	mp-enums.c \
	mp-enums.h \
	mp-equation.c \
	mp-equation.h \
	mp-private.h \
	mp-serializer.c \
	mp-serializer.h \
	mp-trigonometric.c \
	financial.c \
	financial.h \
  	unit.c \
	unit.h \
	unit-category.c \
	unit-category.h \
	unit-manager.c \
	unit-manager.h \
	prelexer.c \
	prelexer.h \
	lexer.c \
	lexer.h \
	parserfunc.c \
	parserfunc.h \
	parser.c \
	parser.h \
	utility.h

cafe_calc_LDADD = \
	$(CAFE_CALC_LIBS)        

cafe_calc_cmd_SOURCES = \
	cafe-calc-cmd.c \
	currency.c \
	currency.h \
	currency-manager.c \
	currency-manager.h \
	mp.c \
	mp-binary.c \
	mp-convert.c \
	mp-enums.c \
	mp-enums.h \
	mp-equation.c \
	mp-serializer.c \
	mp-serializer.h\
	mp-trigonometric.c \
	unit.c \
	unit.h \
	unit-category.c \
	unit-category.h \
	unit-manager.c \
	unit-manager.h \
	prelexer.c \
	prelexer.h \
	lexer.c \
	lexer.h \
	parserfunc.c \
	parserfunc.h \
	parser.c \
	parser.h

cafe_calc_cmd_LDADD = \
	$(CAFE_CALC_CMD_LIBS) \
	-lm

test_mp_SOURCES = \
	test-mp.c \
	mp.c \
	mp-binary.c \
	mp-convert.c \
	mp-enums.c \
	mp-enums.h \
	mp-serializer.c \
	mp-serializer.h \
	mp-trigonometric.c

test_mp_LDADD = \
	$(CAFE_CALC_CMD_LIBS) \
	-lm

test_mp_equation_SOURCES = \
	test-mp-equation.c \
	currency.c \
	currency.h \
	currency-manager.c \
	currency-manager.h \
	mp.c \
	mp-convert.c \
	mp-binary.c \
	mp-enums.c \
	mp-enums.h \
	mp-equation.c \
	mp-serializer.c \
	mp-serializer.h \
	mp-trigonometric.c \
	unit.c \
	unit.h \
	unit-category.c \
	unit-category.h \
	unit-manager.c \
	unit-manager.h \
	prelexer.c \
	prelexer.h \
	lexer.c \
	lexer.h \
	parserfunc.c \
	parserfunc.h \
	parser.c \
	parser.h

test_mp_equation_LDADD = \
	$(CAFE_CALC_CMD_LIBS) \
	-lm

CLEANFILES = \
	mp-enums.c \
	mp-enums.h \
	$(BUILT_SOURCES)

# Generate enum types
mp-enums.h: mp-enums.h.template mp-serializer.h
	$(AM_V_GEN)$(GLIB_MKENUMS) --template $(srcdir)/mp-enums.h.template $(srcdir)/mp-serializer.h > mp-enums.h

mp-enums.c: mp-enums.c.template mp-enums.h mp-serializer.h
	$(AM_V_GEN)$(GLIB_MKENUMS) --template $(srcdir)/mp-enums.c.template $(srcdir)/mp-serializer.h > mp-enums.c

# Fix dependencies
math-serializer.c: mp-enums.h
math-equation.c: mp-enums.h

# Generate resources
cafe-calc-resources.h cafe-calc-resources.c: org.cafe.calculator.gresource.xml Makefile $(shell $(GLIB_COMPILE_RESOURCES) --generate-dependencies --sourcedir $(srcdir) $(srcdir)/org.cafe.calculator.gresource.xml)
	$(AM_V_GEN) XMLLINT=$(XMLLINT) $(GLIB_COMPILE_RESOURCES) --target $@ --sourcedir $(srcdir) --generate --c-name calculator $<


# Install a symlink between cafe-calc and cafe-calculator
install-exec-hook:
	test -e "$(DESTDIR)$(bindir)/cafe-calculator" \
	|| (cd "$(DESTDIR)$(bindir)" && ln -s cafe-calc cafe-calculator)

# Remove the symlink between cafe-calc and cafe-calculator
uninstall-local:
	test -h "$(DESTDIR)$(bindir)/cafe-calculator" \
	&& rm -f "$(DESTDIR)$(bindir)/cafe-calculator"

EXTRA_DIST = \
	buttons-advanced.ui \
	buttons-basic.ui \
	buttons-financial.ui \
	buttons-programming.ui \
	cafe-calc.about \
	mp-enums.c.template \
	mp-enums.h.template \
	org.cafe.calculator.gresource.xml \
	preferences.ui

DISTCLEANFILES = \
	Makefile.in

test: cafe-calc
	./cafe-calc -u
