#ifndef MFP_VOICESTEALING_H
#define MFP_VOICESTEALING_H

#include "NoteAndCommandEvents.h"

namespace VoiceStealing {

// BASE STRATEGY CLASS /////////////////////////////////////////////////////////

class Strategy {
public:
  virtual ~Strategy() {}

  virtual void preventVoiceStealing(
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

// will return a nullptr is strategy type is none
std::shared_ptr<Strategy> createStrategy(StrategyType s);

} /* END NAMESPACE VoiceStealing */

#endif /* MFP_VOICESTEALING_H */
