
all: get_scons
	$(SCONS_EXE)

SCONS_VERSION=2.3.4

scons-local-%.tar.gz:
	curl -L http://sourceforge.net/projects/scons/files/scons-local/$(SCONS_VERSION)/scons-local-$(SCONS_VERSION).tar.gz > scons-local-$(SCONS_VERSION).tar.gz
	touch scons-local-$(SCONS_VERSION).tar.gz

scons-local: scons-local-$(SCONS_VERSION).tar.gz
	mkdir -p scons-local
	tar xzf scons-local-$(SCONS_VERSION).tar.gz --directory scons-local
	touch scons-local

NATIVE_SCONS=$(strip $(shell which scons 2>/dev/null))

ifeq ($(NATIVE_SCONS),)
SCONS_EXE=python2 ./scons-local/scons.py
get_scons: scons-local
	@echo "Couldn't find an installation of scons, using a local copy"
else
SCONS_EXE=$(NATIVE_SCONS)
get_scons:
	@echo "Found scons installation at $(SCONS_EXE)"
endif

clean:
	$(SCONS_EXE) -c
	rm -rf scons-local
	rm -f scons-local-*.tar.gz

.PHONY: all clean get_scons
