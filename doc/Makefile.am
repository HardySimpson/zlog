docdir = $(datadir)/doc/$(PACKAGE)
man3dir = $(mandir)/man3

doc_DATA =				\
		UsersGuide-EN.pdf	\
		UsersGuide-CN.pdf	\
		UsersGuide-EN.tex	\
		UsersGuide-CN.tex	\
		UsersGuide-EN.html	\
		UsersGuide-CN.html	\
		GettingStart-EN.txt	\
		GettingStart-CN.txt	

EXTRA_DIST =    $(doc_DATA)		\
		UsersGuide-EN.lyx	\
		UsersGuide-CN.lyx


UsersGuide-EN.pdf : UsersGuide-EN.lyx
	lyx -f -e pdf2 $^

UsersGuide-CN.pdf : UsersGuide-CN.lyx
	lyx -f -e pdf4 $^

.lyx.tex :
	lyx -f -e pdflatex $<

.tex.html :
	hevea book.hva -s $<
	hevea book.hva -s $<

clean-local :
	-rm -f *.pdf *.haux *.html *.htoc *.tex
