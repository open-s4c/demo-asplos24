LATEXMK ?= scripts/docker-latex.sh latexmk

all:
	$(LATEXMK) --pdf slides.tex
clean:
	$(LATEXMK) -C slides.tex
