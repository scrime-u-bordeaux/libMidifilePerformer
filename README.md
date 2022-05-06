# Midifile Performer lib

### C++ implementation of Bernard P. Serpette's Midifile Performer formalism

Warning : this is a very rough work in progress.
It is only targeting web environments through Emscripten at the moment.

Requirements :

* Building :
    * cmake
    * emscripten (clone [emsdk](https://github.com/emscripten-core/emsdk) and follow installation procedure)
* Testing :
    * nodejs

Building :

* `$ mkdir js`
* `$ cd js`
* `$ emcmake ..`
* `$ make`
* `cd ..`

Testing :

* `node test/mfp_test.js`
