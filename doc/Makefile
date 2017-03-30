SPHINXBUILD=.venv/bin/sphinx-build

all: html

.env:
	python3 -m venv .venv
	.venv/bin/pip install --upgrade pip
	.venv/bin/pip install --upgrade -r requirements.txt

apidoc:
	make -C api

html: .env apidoc
	$(SPHINXBUILD) -b html . _build/html

pdf: .env apidoc
	$(SPHINXBUILD) -b latex . _build/pdf
	make -C _build/pdf/ all-pdf

clean:
	make -C api clean
	rm -rf _build
