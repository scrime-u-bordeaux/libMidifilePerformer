default: build

build:
	mkdir js ; cd js ; emcmake cmake .. ; make ; cd ..

run:
	node test/mfp_test.js all

clean:
	rm -rf js
