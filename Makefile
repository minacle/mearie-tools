SUBDIR_MAKEFILES := $(wildcard */Makefile)
SUBDIRS := $(patsubst %/Makefile,%,$(SUBDIR_MAKEFILES))

.PHONY: all %

all:
	@set -e; \
	for dir in $(SUBDIRS); do \
		$(MAKE) -C "$$dir" all; \
	done

%:
	@set -e; \
	for dir in $(SUBDIRS); do \
		$(MAKE) -C "$$dir" $@; \
	done
