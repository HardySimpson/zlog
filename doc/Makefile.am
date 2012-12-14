dist_doc_DATA = GettingStart-CN.txt \
	GettingStart-EN.txt \
	performence.txt \
	UsersGuide-CN.lyx \
	UsersGuide-EN.lyx \
	zlog.conf

if HAVE_LYX
UsersGuide-EN.pdf : UsersGuide-EN.lyx
	lyx -f -e pdf2 $^

UsersGuide-CN.pdf : UsersGuide-CN.lyx
	lyx -f -e pdf4 $^

%.tex : %.lyx
	lyx -f -e pdflatex $<
endif

if HAVE_HEVEA
%.html : %.tex
	hevea book.hva -s $<
	hevea book.hva -s $<
endif
