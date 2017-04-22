SPHINXBUILD?=.venv/bin/sphinx-build
OBJDIR?=_build

all: html

.env:
	python3 -m venv .venv
	.venv/bin/pip install --upgrade pip
	.venv/bin/pip install --upgrade -r requirements.txt

apidoc:
	make -C api

html: .env apidoc
	$(SPHINXBUILD) -b html . $(OBJDIR)/html

pdf: .env apidoc
	$(SPHINXBUILD) -b latex . $(OBJDIR)/pdf
	make -C $(OBJDIR)/pdf/ all-pdf

clean:
	make -C api clean
	rm -rf $(OBJDIR)
