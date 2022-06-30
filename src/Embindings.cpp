#include <emscripten/bind.h>
#include "impl/MidiRenderer.h"

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

  class_<MidiRenderer>("Renderer")
    .constructor()
    .function("pushEvent",    &MidiRenderer::pushEvent)
    .function("finalize",     &MidiRenderer::finalize)
    .function("hasEvents",    &MidiRenderer::hasEvents)
    .function("pullEvents",   &MidiRenderer::pullEvents)
    .function("pullEventsSet",&MidiRenderer::pullEventsSet)
    .function("combine3",     &MidiRenderer::combine3)
    .function("clear",        &MidiRenderer::clear)
    ;
}