#ifndef MFP_ABSTRACTPERFORMER_H
#define MFP_ABSTRACTPERFORMER_H

#include "NoteAndCommandEvents.h"
#include "VoiceStealing.h"
#include "ChordRendering.h"
#include "../core/Renderer.h"

// This class just defines strategy related functionalities and a renderer
// variable. It is abstract because it doesn't implement the getNextSetPair()
// pure virtual method of the abstract Renderer<T,U,V>::Provider,
// as it has no Chronology variable to get SetPairs from.

class AbstractPerformer :
public Renderer<noteData, commandData, commandKey>::Provider {
  std::shared_ptr<VoiceStealing::Strategy> stealingStrategy;
  std::shared_ptr<ChordRendering::Strategy> chordStrategy;

  void setDefaultStrategies() {
    setVoiceStealingStrategy(
      // VoiceStealing::StrategyType::None
      VoiceStealing::StrategyType::LastNoteOffWins
      // VoiceStealing::StrategyType::OnlyStaccato
    );

    setChordRenderingStrategy(
      // ChordRendering::StrategyType::None
      ChordRendering::StrategyType::SameForAll
      // ChordRendering::StrategyType::ClippedScaledFromMean
      // ChordRendering::StrategyType::AdjustedScaledFromMean
      // ChordRendering::StrategyType::ClippedScaledFromMax
    );
  }

protected:
  Renderer<noteData, commandData, commandKey> renderer;

  virtual void performVoiceStealing(
    std::vector<noteData>& notes,
    commandData cmd
  ) const {
    if (stealingStrategy.get() == nullptr) return;
    stealingStrategy->performVoiceStealing(notes, cmd);
  }

  virtual void adjustToCommandVelocity(
    std::vector<noteData>& notes,
    uint8_t cmd_velocity
  ) const {
    if (chordStrategy.get() == nullptr) return;
    chordStrategy->adjustToCommandVelocity(notes, cmd_velocity);
  }

public:
  AbstractPerformer() :
  Renderer<noteData, commandData, commandKey>::Provider() {
    setDefaultStrategies();
  }

  virtual ~AbstractPerformer() {}

  // must be implemented by Renderer<T,U,V>::Provider
  virtual Events::SetPair<noteData> getNextSetPair() = 0;

  void setVoiceStealingStrategy(VoiceStealing::StrategyType s) {
    stealingStrategy = VoiceStealing::createStrategy(s);
  }

  void setChordRenderingStrategy(ChordRendering::StrategyType s) {
    chordStrategy = ChordRendering::createStrategy(s);
  }
};

#endif /* MFP_ABSTRACTPERFORMER_H */