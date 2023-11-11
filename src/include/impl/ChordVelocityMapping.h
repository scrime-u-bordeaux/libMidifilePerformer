#ifndef MFP_CHORDVELOCITYMAPPING_H
#define MFP_CHORDVELOCITYMAPPING_H

#include <memory>

#include "NoteAndCommandEvents.h"

namespace ChordVelocityMapping {

// BASE STRATEGY CLASS /////////////////////////////////////////////////////////

class Strategy {
public:
  virtual ~Strategy() {}

  virtual void adjustToCommandVelocity(
    std::vector<noteData>& notes,
    uint8_t cmd_velocity
  ) = 0;
};

// LIST OF STRATEGY IMPLEMENTATIONS ////////////////////////////////////////////

enum class StrategyType {
  None,
  SameForAll,
  ClippedScaledFromMean,
  AdjustedScaledFromMean,
  ClippedScaledFromMax
};

// STRATEGY FACTORY FUNCTION ///////////////////////////////////////////////////

// will return a nullptr is strategy type is none
std::shared_ptr<Strategy> createStrategy(StrategyType s);

} /* END NAMESPACE ChordVelocityMapping */

#endif /* MFP_CHORDVELOCITYMAPPING_H */
