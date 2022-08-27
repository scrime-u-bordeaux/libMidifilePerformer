#ifndef MFP_CHORDRENDERING_H
#define MFP_CHORDRENDERING_H

#include "NoteAndCommandEvents.h"

namespace ChordRendering {

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

} /* END NAMESPACE ChordRendering */

#endif /* MFP_CHORDRENDERING_H */
