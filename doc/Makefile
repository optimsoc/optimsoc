OBJDIR?=$(CURDIR)/_build
SPHINXBUILD?=$(OBJDIR)/.venv/bin/sphinx-build

all: html

$(OBJDIR)/.venv: requirements.txt
	python3 -m venv $(OBJDIR)/.venv
	$(OBJDIR)/.venv/bin/pip install --upgrade pip
	# Work around deficiencies in the pip dependency resolver to install
	# cairocffi
	#
	# - cairocffi 1.0 (a dependency of cairosvg) requires a newer setuptools
	#   than Ubuntu 16.04 provides. Error message during "pip install":
	#   "Generating metadata for package cairocffi produced metadata for
	#   project name unknown. Fix your #egg=cairocffi fragments."
	#   This version needs to be installed before requirements.txt is installed
	#   as package metadata is not parsed correctly otherwise.
	# - cairocffi and setuptools >= 42.0.0 need to have wheel pre-installed.
	$(OBJDIR)/.venv/bin/pip install --upgrade 'setuptools >= 39.2.0' wheel
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
