PDFLATEX ?= pdflatex
LATEXML ?= latexml
LATEXMLPOST ?= latexmlpost

TEXFILES = user_guide-preamble.tex \
           user_guide-title.tex \
           user_guide-changes.tex \
           user_guide-documentation.tex \
           ../docoverview.tex \
           user_guide-content.tex \
           user_guide-introduction.tex \
           user_guide-installation.tex \
           user_guide-tutorials.tex \
           user_guide-develop.tex

all: pdf html

html: html/user_guide.xhtml

pdf: user_guide.pdf

%.pdf: %.tex $(TEXFILES) Makefile
	$(PDFLATEX) -interaction nonstopmode $*.tex
	# Run again to get references right
	$(PDFLATEX) -interaction nonstopmode $*.tex

html/%.xhtml: %-html.tex $(TEXFILES)
	$(LATEXML) --dest=$*.xml $*-html.tex
	$(LATEXMLPOST) --dest=html/$*.xhtml $*.xml

clean:
	rm -rf *.pdf *.aux *.toc *.out *.log *.xml html/

