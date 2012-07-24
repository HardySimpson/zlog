# Top level makefile, the real shit is at src/makefile

TARGETS=noopt 32bit

all:
	cd src && $(MAKE) $@

install:
	cd src && $(MAKE) $@

$(TARGETS):
	cd src && $(MAKE) $@

doc:
	cd doc && $(MAKE)

test:
	cd test && $(MAKE)

clean:
	cd src && $(MAKE) $@
	cd test && $(MAKE) $@
	cd doc && $(MAKE) $@

distclean: clean

dummy:

.PHONY: doc install test
