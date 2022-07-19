#ifndef MFP_MFPRENDERER_H
#define MFP_MFPRENDERER_H

#include "MFPEvents.h"
#include "ChordVelocityMapping.h"
#include "../core/Renderer.h"
#include "../core/Chronology.h"

/*
// INHERITANCE

typedef Renderer<noteData, commandData, commandKey> R;

template<>
std::vector<noteData>
R::combine3(commandData cmd) {
  std::vector<noteData> res
    = R::combine3(cmd);

  // always use command velocity bc we can't pass extra parameters to combine3 :
  for (auto& note : res) {
    if (note.on) {
      note.velocity = cmd.velocity;
    }
  }

  return res;
}
//*/


//*
// COMPOSITION

class MFPRenderer {
private:
  std::shared_ptr<ChordVelocityMapping::Strategy> chordStrategy;
  Renderer<noteData, commandData, commandKey> renderer;

  void adjustToCommandVelocity(std::vector<noteData>& notes,
                               uint8_t cmd_velocity) const {
    if (chordStrategy.get() == nullptr) return;
    chordStrategy->adjustToCommandVelocity(notes, cmd_velocity);
    // dynamic_cast<std::type_info(*strategy)>(strategy)->adjustToCommandVelocity(notes, cmd_velocity);
  }

public:

  MFPRenderer() : renderer() {
    setChordRenderingStrategy(
      ChordVelocityMapping::StrategyType::SameForAll
      // ChordVelocityMapping::StrategyType::ClippedScaledFromMean
      // ChordVelocityMapping::StrategyType::AdjustedScaledFromMean
      // ChordVelocityMapping::StrategyType::ClippedScaledFromMax
    );
  }

  MFPRenderer(ChronologyParams::parameters params) : renderer(params) {}
  
  void setChordRenderingStrategy(ChordVelocityMapping::StrategyType s) {
    chordStrategy = ChordVelocityMapping::createStrategy(s);
  }

  void pushEvent(int dt, noteData event) { renderer.pushEvent(dt, event); }

  void finalize() { renderer.finalize(); }

  bool hasEvents(bool countLastEvent = true) {
    return renderer.hasEvents(countLastEvent);
  }

  std::vector<noteData> pullEvents() { return renderer.pullEvents(); }

  Events::Set<noteData> pullEventsSet() { return renderer.pullEventsSet(); }

  std::vector<noteData> combine3(commandData cmd,
                                 bool useCommandVelocity = true) {
    std::vector<noteData> res = renderer.combine3(cmd);
    if (useCommandVelocity) adjustToCommandVelocity(res, cmd.velocity);
    return res;
  }

  void clear() { renderer.clear(); }

  void setPartition(Chronology<noteData> const newPartition){ renderer.setPartition(newPartition); }

  Chronology<noteData> getPartition() { return renderer.getPartition(); }
};
//*/

#endif /* MFP_MFPRENDERER_H */
