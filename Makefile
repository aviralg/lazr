R = R

.PHONY: all build check document test

all: document build check

build: document
	$(R) CMD build .

check: build
	$(R) CMD check strictr*tar.gz

clean:
	-rm -f strictr*tar.gz
	-rm -fr strictr.Rcheck
	-rm -rf src/*.o src/*.so

document:
	$(R) -e 'devtools::document()'

test:
	$(R) -e 'devtools::test()'

lintr:
	$(R) --slave -e "lintr::lint_package()"

install: clean
	$(R) CMD INSTALL .
