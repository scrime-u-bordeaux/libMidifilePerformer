#ifndef MFP_CHORDVELOCITYMAPPING_H
#define MFP_CHORDVELOCITYMAPPING_H

#include <tuple>
#include <memory>
#include "MFPEvents.h"

namespace ChordVelocityMapping {

// BASE STRATEGY CLASS /////////////////////////////////////////////////////////

class Strategy {
public:
  virtual ~Strategy() {}

  virtual void
  adjustToCommandVelocity(std::vector<noteData>& notes,
                          uint8_t cmd_velocity) = 0;
};

// LIST OF STRATEGY IMPLEMENTATIONS ////////////////////////////////////////////

enum class StrategyType {
  SameForAll,
  ClippedScaledFromMean,
  AdjustedScaledFromMean,
  ClippedScaledFromMax
};

// STRATEGY FACTORY FUNCTION ///////////////////////////////////////////////////

std::shared_ptr<Strategy> createStrategy(StrategyType s);

} /* END NAMESPACE ChordVelocityMapping */

#endif /* MFP_CHORDVELOCITYMAPPING_H */
