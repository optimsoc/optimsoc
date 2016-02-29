PDFLATEX ?= pdflatex
LATEXML ?= latexml
LATEXMLPOST ?= latexmlpost

OBJDIR := .

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

html: $(OBJDIR)/html/user_guide.xhtml

pdf: $(OBJDIR)/user_guide.pdf

$(OBJDIR)/%.pdf: %.tex $(TEXFILES) Makefile
	$(PDFLATEX) -output-directory=$(OBJDIR) -interaction nonstopmode $*.tex
	# Run again to get references right
	$(PDFLATEX) -output-directory=$(OBJDIR) -interaction nonstopmode $*.tex

$(OBJDIR)/html/%.xhtml: %-html.tex $(TEXFILES)
	$(LATEXML) --dest=$(OBJDIR)/$*.xml $<
	$(LATEXMLPOST) --dest=$(OBJDIR)/html/$*.xhtml $(OBJDIR)/$*.xml

clean:
	rm -rf $(OBJDIR)/*.pdf $(OBJDIR)/*.aux $(OBJDIR)/*.toc $(OBJDIR)/*.out \
		$(OBJDIR)/*.log $(OBJDIR)/*.xml $(OBJDIR)/html


.PHONY: clean
