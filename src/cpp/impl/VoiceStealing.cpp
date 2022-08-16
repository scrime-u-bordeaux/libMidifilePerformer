#include <map>
#include "../../include/impl/VoiceStealing.h"

namespace VoiceStealing {

// STRATEGY IMPLEMENTATIONS ////////////////////////////////////////////////////

class None : public Strategy {
public:
  void performVoiceStealing(std::vector<noteData>& notes, commandData cmd) {}
  void reset() {}
};

class LastNoteOffWins : public Strategy {
private:
  std::map<noteKey, std::uint8_t> sameNotePolyphony;

public:
  void performVoiceStealing(std::vector<noteData>& notes, commandData cmd) {
    // std::cout < "performing voice stealing" << std::endl;
    for (auto i = 0; i < notes.size(); ++i) {
      const std::vector<noteData>::iterator it = notes.begin() + i;
      noteData nData = notes[i];
      noteKey key = Events::keyFromData<noteData, noteKey>(nData);

      if (sameNotePolyphony.find(key) == sameNotePolyphony.end()) {
        if (nData.on) {
          sameNotePolyphony[key] = 1;
        } else {
          // do nothing or remove the note off event ?
        }
      } else {
        if (nData.on) {
          // insert a note off before
          notes.insert(it, { false, nData.pitch, 0, nData.channel });
          // avoid processing the same note on event again
          i++;
          // keep track of simultaneous note ons
          sameNotePolyphony[key]++;
        } else if (sameNotePolyphony[key] > 1) {
          // discard the note off event
          notes.erase(it);
          // avoid missing next noteData struct from the loop
          i--;
          // keep track of simultaneous note ons
          sameNotePolyphony[key]--;
        } else {
          // don't touch anything and remove the entry from the map
          sameNotePolyphony.erase(key);
        }
      }
    }
  }

  void reset() {
    sameNotePolyphony.clear();
  }
};

class OnlyStaccato : public Strategy {
private:

public:
  void performVoiceStealing(std::vector<noteData>& notes, commandData cmd) {
    // todo
  }

  void reset() {
    // todo
  }
};

// STRATEGY FACTORY FUNCTION ///////////////////////////////////////////////////

std::shared_ptr<Strategy> createStrategy(StrategyType s) {
  Strategy* strategy;
  switch (s) {
    case StrategyType::None:
      strategy = new None();
      break;
    case StrategyType::LastNoteOffWins:
      strategy = new LastNoteOffWins();
      break;
    case StrategyType::OnlyStaccato:
      strategy = new OnlyStaccato();
      break;
    default:
      strategy = nullptr;
      break;
  }
  return std::shared_ptr<Strategy>(strategy);
}

} /* end namespace VoiceStealing */