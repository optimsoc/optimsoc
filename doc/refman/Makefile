PDFLATEX ?= pdflatex
LATEXML ?= latexml
LATEXMLPOST ?= latexmlpost

TEXFILES = refman-preamble.tex \
           refman-title.tex \
           refman-changes.tex \
           refman-content.tex \
           refman-systemconfiguration.tex \
	   refman-tracedebugging.tex \
	   refman-systemsoftware.tex \
	   refman-documentation.tex \
	   ../docoverview.tex

all: pdf html

html: html/refman.xhtml

pdf: refman.pdf

%.pdf: %.tex $(TEXFILES) Makefile
	$(PDFLATEX) $*.tex 
	$(PDFLATEX) $*.tex 

html/%.xhtml: %-html.tex $(TEXFILES) Makefile
	$(LATEXML) --dest=$*.xml $*-html.tex
	$(LATEXMLPOST) --dest=html/$*.xhtml $*.xml

clean:
	rm -rf *.pdf *.aux *.toc *.out *.log *.xml html/
