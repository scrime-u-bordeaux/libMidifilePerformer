default: build

build:
	mkdir dist; cd dist; mkdir node; mkdir browser; cd ..;\

	mkdir build; cd build; \
	emcmake cmake -DBROWSER_VERSION=ON ..; make; cd ..; \
	mv build/MidifilePerformer.* dist/browser; \
	rm -rf build; \
	
	mkdir build; cd build; \
	emcmake cmake ..; make; cd ..; \
	mv build/MidifilePerformer.* dist/node; \
	rm -rf build; \

clean:
	rm -rf dist; \
