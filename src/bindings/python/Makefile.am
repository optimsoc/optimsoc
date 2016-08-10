
SWIGFILE = opensocdebug.i
SWIGGEN = opensocdebug.py

PKGFILES = __init__.py session.py
PKGSRCS = $(addprefix opensocdebug/,$(PKGFILES))

all-local: $(SWIGGEN) $(PKGSRCS) setup.py
	$(PYTHON) setup.py build

opensocdebug:
	mkdir -p opensocdebug

install-exec-local: setup.py
	$(PYTHON) setup.py install --prefix=$(prefix)

$(SWIGGEN): opensocdebug $(SWIGFILE)
	swig -python -o opensocdebug_wrap.c -outdir opensocdebug $(srcdir)/$(SWIGFILE)

opensocdebug/%.py: $(srcdir)/module/%.py opensocdebug
	cp $< $@
	
EXTRA_DIST = $(SWIGFILE) $(PKGSRCS)

examplesdir = $(datadir)/opensocdebug/examples
examples_DATA = $(srcdir)/examples/runelf.py
