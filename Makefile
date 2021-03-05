R = R

.PHONY: all build check document test

all: document build check

build: document
	$(R) CMD build .

check: build
	$(R) CMD check lazr*tar.gz

clean:
	-rm -f lazr*tar.gz
	-rm -fr lazr.Rcheck
	-rm -rf src/*.o src/*.so

document:
	$(R) -e 'devtools::document()'

test:
	$(R) -e 'devtools::test()'

lintr:
	$(R) --slave -e "lintr::lint_package()"

install: clean
	$(R) CMD INSTALL .
