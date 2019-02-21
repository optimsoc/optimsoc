OBJDIR?=$(CURDIR)/_build
SPHINXBUILD?=$(OBJDIR)/.venv/bin/sphinx-build

all: html

$(OBJDIR)/.venv: requirements.txt
	python3 -m venv $(OBJDIR)/.venv
	$(OBJDIR)/.venv/bin/pip install --upgrade pip
	# cairocffi 1.0 (a dependency of cairosvg) requires a newer setuptools
	# than Ubuntu 16.04 provides. Error message during "pip install":
	# "Generating metadata for package cairocffi produced metadata for project
	# name unknown. Fix your #egg=cairocffi fragments."
	$(OBJDIR)/.venv/bin/pip install --upgrade setuptools
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
