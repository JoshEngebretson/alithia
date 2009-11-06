all:
	scons -j 8

.PHONY: clean
clean:
	scons -c

.PHONY: distclean
distclean: clean
