all: 
#mcapi_design.html 

EXTRA_DIST = default-rst.css  mcapi.png fsl-header.rst mcapi_design.rst  mcapiV2000_100310.pdf 

PrestFlags = -W stylesheet="default-rst.css" -D keep_title_section

CLEANFILES = *.html

# We have to do a fixup here in order to convert eps references to pdf
# in order for pdflatex to work.  This can be removed once prest doesn't
# automatically convert graphics include commands to ".eps" files.
%.tex : %.rst ;
	$(PREST) -w latex $< > $@
	perl -pi -e "s/\\\\includegraphics\\[(.*)\\]\\{(.*)\\.eps\}/\\\\includegraphics\\[\\1\\]\\{\\2\\.pdf}/g" $@

%.html : %.rst ;
	$(PREST) $(PrestFlags) $< > $@
