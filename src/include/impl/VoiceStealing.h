#ifndef MFP_VOICESTEALING_H
#define MFP_VOICESTEALING_H

#include "MFPEvents.h"

namespace VoiceStealing {

// BASE STRATEGY CLASS /////////////////////////////////////////////////////////

class Strategy {
public:
  virtual ~Strategy() {}

  virtual void performVoiceStealing(
    std::vector<noteData>& notes,
    commandData cmd
  ) = 0;
  
  virtual void reset() = 0;
};

// LIST OF STRATEGY IMPLEMENTATIONS ////////////////////////////////////////////

enum class StrategyType {
  None,
  LastNoteOffWins,
  OnlyStaccato
};

// STRATEGY FACTORY FUNCTION ///////////////////////////////////////////////////

std::shared_ptr<Strategy> createStrategy(StrategyType s);

} /* END NAMESPACE VoiceStealing */

#endif /* MFP_VOICESTEALING_H */
