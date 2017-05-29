OBJDIR?=_build
SPHINXBUILD?=$(OBJDIR)/.venv/bin/sphinx-build

all: html

$(OBJDIR)/.venv:
	python3 -m venv $(OBJDIR)/.venv
	$(OBJDIR)/.venv/bin/pip install --upgrade pip
	$(OBJDIR)/.venv/bin/pip install --upgrade -r requirements.txt

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
