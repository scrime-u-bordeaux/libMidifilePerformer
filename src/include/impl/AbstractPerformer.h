#ifndef MFP_ABSTRACTPERFORMER_H
#define MFP_ABSTRACTPERFORMER_H

#include "NoteAndCommandEvents.h"
#include "ChordVelocityMapping.h"
#include "../core/Renderer.h"

// This class just defines strategy related functionalities and adds a renderer
// variable. It is abstract because it doesn't implement the getNextSetPair()
// pure virtual method of the abstract Renderer<T,U,V>::Provider,
// as it has no Chronology variable to get SetPairs from.

class AbstractPerformer :
public Renderer<noteData, commandData, commandKey>::Provider {
  std::shared_ptr<ChordVelocityMapping::Strategy> chordStrategy;

  void setDefaultStrategies() {
    setChordVelocityMappingStrategy(
      ChordVelocityMapping::StrategyType::None
      // ChordVelocityMapping::StrategyType::SameForAll
      // ChordVelocityMapping::StrategyType::ClippedScaledFromMean
      // ChordVelocityMapping::StrategyType::AdjustedScaledFromMean
      // ChordVelocityMapping::StrategyType::ClippedScaledFromMax
    );
  }

protected:
  Renderer<noteData, commandData, commandKey> renderer;

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

  virtual void setChordVelocityMappingStrategy(
    ChordVelocityMapping::StrategyType s
  ) {
    chordStrategy = ChordVelocityMapping::createStrategy(s);
  }
};

#endif /* MFP_ABSTRACTPERFORMER_H */