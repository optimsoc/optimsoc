PANDOC?=pandoc
PANDOC_OPTIONS?=-s -S -V documentclass=scrartcl -H ../../../doc-shared/pandoc-header.tex

FILES=pandoc-metadata.yaml \
      specification.md

EXTRA_DEP=Makefile \
          ../../../doc-shared/pandoc-header.tex

all: specification.pdf

specification.pdf: $(FILES) $(EXTRA_DEP)
	$(PANDOC) $(PANDOC_OPTIONS) -o $@ $(FILES)

