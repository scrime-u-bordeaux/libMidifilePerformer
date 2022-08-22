#ifndef MFP_MFPRENDERER_H
#define MFP_MFPRENDERER_H

#include "MFPEvents.h"
#include "VoiceStealing.h"
#include "ChordVelocityMapping.h"
#include "../core/Renderer.h"
#include "../core/Chronology.h"

class MFPRenderer {
private:
  std::shared_ptr<VoiceStealing::Strategy> stealingStrategy;
  std::shared_ptr<ChordVelocityMapping::Strategy> chordStrategy;
  Renderer<noteData, commandData, commandKey> renderer;

  void setDefaultStrategies() {
    setVoiceStealingStrategy(
      // VoiceStealing::StrategyType::None
      VoiceStealing::StrategyType::LastNoteOffWins
      // VoiceStealing::StrategyType::OnlyStaccato
    );

    setChordRenderingStrategy(
      ChordVelocityMapping::StrategyType::SameForAll
      // ChordVelocityMapping::StrategyType::ClippedScaledFromMean
      // ChordVelocityMapping::StrategyType::AdjustedScaledFromMean
      // ChordVelocityMapping::StrategyType::ClippedScaledFromMax
    );
  }

  void preventVoiceStealing(
    std::vector<noteData>& notes,
    commandData cmd
  ) const {
    if (stealingStrategy.get() == nullptr) return;
    stealingStrategy->preventVoiceStealing(notes, cmd);
  }

  void adjustToCommandVelocity(
    std::vector<noteData>& notes,
    uint8_t cmd_velocity
  ) const {
    if (chordStrategy.get() == nullptr) return;
    chordStrategy->adjustToCommandVelocity(notes, cmd_velocity);
  }

public:

  MFPRenderer() : renderer() {
    setDefaultStrategies();
  }

  MFPRenderer(ChronologyParams::parameters params) : renderer(params) {
    setDefaultStrategies();
  }

  void setVoiceStealingStrategy(VoiceStealing::StrategyType s) {
    stealingStrategy = VoiceStealing::createStrategy(s);
  }

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
    preventVoiceStealing(res, cmd);
    if (useCommandVelocity) adjustToCommandVelocity(res, cmd.velocity);
    return res;
  }

  void clear() { renderer.clear(); }

  void setPartition(Chronology<noteData> const newPartition){ renderer.setPartition(newPartition); }

  Chronology<noteData> getPartition() { return renderer.getPartition(); }
};

#endif /* MFP_MFPRENDERER_H */
