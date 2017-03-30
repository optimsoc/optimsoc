VENV ?= ../.venv

all:
	doxygen Doxyfile
	$(VENV)/bin/python optimsoc-apidoc.py -o doxygen/ _xml/

clean:
	rm -rf _xml doxygen
