#ifndef MFP_MIDIRENDERER_H
#define MFP_MIDIRENDERER_H

#include "MidiEvents.h"
#include "../core/Renderer.h"

class MidiRenderer {
private:
  Renderer<noteData, commandData, commandKey> renderer;

public:
  void pushEvent(int dt, noteData event) { renderer.pushEvent(dt, event); }
  
  void finalize() { renderer.finalize(); }
  
  bool hasEvents(bool countLastEvent = true) {
    return renderer.hasEvents(countLastEvent);
  }

  std::vector<noteData> pullEvents() { return renderer.pullEvents(); }

  Events::Set<noteData> pullEventsSet() { return renderer.pullEventsSet(); }

  std::vector<noteData> combine3(commandData cmd, bool useCommandVelocity = true) {
    std::vector<noteData> res = renderer.combine3(cmd);
    if (useCommandVelocity) {
      for (auto& note : res) {
        if (note.on) {
          note.velocity = cmd.velocity;
        }
      }      
    }

    return res;
  }

  void clear() { renderer.clear(); }
};

#endif /* MFP_MIDIRENDERER_H */