# Makefile wrapping a SCons build (SConstruct),
# fetching a local scons installation if needed
#
# The MIT License (MIT)
#
# Copyright (c) 2015 Davide Kirchner
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

all: get_scons
	@$(SCONS_EXE)

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
	@echo "Couldn't find an installation of SCons, using a local copy"
else
SCONS_EXE=$(NATIVE_SCONS)
get_scons:
	@echo "Found SCons installation at $(SCONS_EXE)"
endif

clean:
	$(SCONS_EXE) -c
	rm -rf scons-local
	rm -f scons-local-*.tar.gz

.PHONY: all clean get_scons
