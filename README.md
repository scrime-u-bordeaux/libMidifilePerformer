# libMidifilePerformer

### C++ version of Bernard P. Serpette's MidifilePerformer core functionalities

Warning : this is a very rough work in progress, only targeting web environments
through Emscripten at the moment.

The library doesn't deal directly with MIDI files, it must be fed with NOTE
events only, which should be obtained by parsing MIDI files and have their
time stamps adjusted after other MIDI event types (e.g. MIDI CC events) have
been discarded.

#### Requirements

* Building :
    * cmake
    * emscripten (clone [emsdk](https://github.com/emscripten-core/emsdk) and follow installation procedure)
* Testing :
    * nodejs

#### Building

* `$ mkdir js`
* `$ cd js`
* `$ emcmake cmake ..`
* `$ make`
* `cd ..`

#### Testing

* `node test/mfp_test.js`
