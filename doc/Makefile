OBJDIR?=$(CURDIR)/_build
SPHINXBUILD?=$(OBJDIR)/.venv/bin/sphinx-build

all: html

$(OBJDIR)/.venv: requirements.txt
	python3 -m venv $(OBJDIR)/.venv
	$(OBJDIR)/.venv/bin/pip install --upgrade pip
	$(OBJDIR)/.venv/bin/pip install --upgrade -r requirements.txt
# Patch breathe to work with Sphinx 1.6, because we need new features
# Get the python minor version. Actually we just want to know where the
# site-packages folder is located, hence also just look there for simplicity.
# Then some strange patch return handling, because it gives error when patch
# was applied already..
	@patch -N --silent -d $(OBJDIR)/.venv/lib/`ls $(OBJDIR)/.venv/lib/`/site-packages/breathe/renderer/ < hotfix/breathe-sphinx1.6.patch; \
	if [ $$? -gt 1 ]; then exit $$?; fi

apidoc: $(OBJDIR)/.venv
	make -C api VENV=$(OBJDIR)/.venv

html: $(OBJDIR)/.venv apidoc
	$(SPHINXBUILD) -b html . $(OBJDIR)/html

pdf: $(OBJDIR)/.venv apidoc
	$(SPHINXBUILD) -b latex . $(OBJDIR)/pdf
	make -C $(OBJDIR)/pdf/ all-pdf

clean:
	make -C api clean
	rm -rf $(OBJDIR)
