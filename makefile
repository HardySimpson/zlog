# Top level makefile, the real shit is at src/makefile

TARGETS=all test doc

all:
	cd src && $(MAKE) $@

install: dummy
	cd src && $(MAKE) $@

clean:
	cd src && $(MAKE) $@
	cd test && $(MAKE) $@
	cd doc && $(MAKE) $@

distclean: clean

$(TARGETS):
	cd src && $(MAKE) $@

src/help.h:
	@./utils/generate-command-help.rb > $@

dummy:
