# libMidifilePerformer

### C++ version of Bernard P. Serpette's MidifilePerformer core functionalities

#### Requirements

* `cmake`
* `ninja`

#### Building and running the tests

* `$ cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug`
* `$ cd build`
* `$ ninja`
* `./test/AllTests`

#### Notes

The library doesn't deal directly with MIDI files, it must be fed with NOTE
events only, which should be obtained by parsing MIDI files and have their
time stamps adjusted after other MIDI event types (e.g. MIDI CC events) have
been discarded.
