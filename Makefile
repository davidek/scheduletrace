
SCONS_VERSION=2.3.4

scons-local-%.tar.gz:
	curl -L http://sourceforge.net/projects/scons/files/scons-local/$(SCONS_VERSION)/scons-local-$(SCONS_VERSION).tar.gz > scons-local-$(SCONS_VERSION).tar.gz
	touch scons-local-$(SCONS_VERSION).tar.gz

scons-local-%: scons-local-%.tar.gz
	tar xzf scons-local-$(SCONS_VERSION).tar.gz

get_scons: scons-local-$(SCONS_VERSION).tar.gz

all:
	scons

clean:
	scons -c

.PHONY: all clean get_scons
