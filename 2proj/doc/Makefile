NAME = protokol
.SUFFIXES: .tex .dvi .pdf

.tex.dvi:
	latex $(NAME).tex

.dvi.pdf:
	dvipdf $(NAME).dvi

all: $(NAME).pdf
	
clean:
	rm -f $(NAME).aux $(NAME).dvi $(NAME).log
