version := $(shell tools/get-version.sh)

OBJDIR := objdir
INSTALL_PREFIX := /opt/optimsoc
INSTALL_TARGET := $(INSTALL_PREFIX)/$(version)

build:
	tools/build.py -o $(OBJDIR)

install: build
	mkdir -p $(INSTALL_TARGET)
	cp -r $(OBJDIR)/dist $(INSTALL_TARGET)

dist: build
	tar -cz --directory $(OBJDIR) --exclude examples \
		--transform "s/dist/$(version)/" \
		-f $(OBJDIR)/optimsoc-$(version)-base.tar.gz dist
	tar -cz --directory $(OBJDIR) \
		--transform "s/dist/$(version)/" \
		-f $(OBJDIR)/optimsoc-$(version)-examples.tar.gz dist/examples

	@echo
	@echo The binary distribution packages have been written to
	@echo $(OBJDIR)/optimsoc-$(version)-base.tar.gz and
	@echo $(OBJDIR)/optimsoc-$(version)-examples.tar.gz

