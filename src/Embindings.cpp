#include <emscripten/bind.h>
#include "include/impl/MFPRenderer.h"

using namespace emscripten;

EMSCRIPTEN_BINDINGS(MidifilePerformer) {
  value_object<noteData>("noteData")
    .field("on",      &noteData::on)
    .field("pitch",   &noteData::pitch)
    .field("velocity",&noteData::velocity)
    .field("channel", &noteData::channel)
    ;

  value_object<commandData>("commandData")
    .field("pressed", &commandData::pressed)
    .field("id",      &commandData::id)
    .field("velocity",&commandData::velocity)
    .field("channel", &commandData::channel)
    ;

  register_vector<noteData>("noteDataSet");

  class_<Chronology<noteData>>("Chronology")
    .constructor()
    .function("pushEvent",    &Chronology<noteData>::pushEvent)
    .function("finalize",     &Chronology<noteData>::finalize)
    .function("hasEvents",    &Chronology<noteData>::hasEvents)
    .function("pullEvents",   &Chronology<noteData>::pullEvents)
    .function("clear",        &Chronology<noteData>::clear)
    ;

  // typedef Renderer<noteData, commandData, commandKey> Renderer;
  typedef MFPRenderer Renderer;

  class_<Renderer>("Renderer")
    .constructor()
    .function("pushEvent",    &Renderer::pushEvent)
    .function("finalize",     &Renderer::finalize)
    .function("hasEvents",    &Renderer::hasEvents)
    .function("pullEvents",   &Renderer::pullEvents)
    .function("pullEventsSet",&Renderer::pullEventsSet)
    .function("combine3",     &Renderer::combine3)
    .function("clear",        &Renderer::clear)
    ;
}
