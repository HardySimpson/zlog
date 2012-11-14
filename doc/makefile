doc =		\
	UsersGuide-EN.pdf	\
	UsersGuide-CN.pdf	\
	UsersGuide-EN.tex	\
	UsersGuide-CN.tex	\
	UsersGuide-EN.html	\
	UsersGuide-CN.html	\
	GettingStart-EN.txt	\
	GettingStart-CN.txt	

all : $(doc)

UsersGuide-EN.pdf : UsersGuide-EN.lyx
	lyx -f -e pdf2 $^

UsersGuide-CN.pdf : UsersGuide-CN.lyx
	lyx -f -e pdf4 $^

%.tex : %.lyx
	lyx -f -e pdflatex $<

%.html : %.tex
	hevea book.hva -s $<
	hevea book.hva -s $<

clean :
	-rm -f *.pdf *.haux *.html *.htoc *.tex *.lyx~
