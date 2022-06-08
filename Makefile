default: build

build:
	mkdir js ; cd js ; emcmake cmake .. ; make ; cd ..

clean:
	rm -rf js
