#include <map>
#include "../../include/impl/VoiceStealing.h"

namespace VoiceStealing {

// THROUGHOUT THIS FILE, "note" means "combination of pitch and channel".

// STRATEGY IMPLEMENTATIONS ////////////////////////////////////////////////////

class LastNoteOffWins : public Strategy {
private:

  std::map<noteKey, std::uint8_t> triggerCountMap;

public:
  void preventVoiceStealing(std::vector<noteData>& notes, commandData cmd) {
    // std::cout < "preventing voice stealing" << std::endl;

    std::vector<noteData> addedNoteOffs;

    auto it = notes.begin();

    while(it!=notes.end()) {
      auto nData = *it;
      noteKey key = Events::keyFromData<noteData, noteKey>(nData);

      if (triggerCountMap.find(key) == triggerCountMap.end()) { // note was not found
        if (nData.on) {
          triggerCountMap[key] = 1; // register the first trigger of this note
        } else {
          // do nothing ?
          // isn't this an error case ?
          // this means : a note off is in the vector,
          // but the corresponding note on HAS NOT been registered ?!
        }
        it++;
    } else { // note has already been triggered at least once
        if (nData.on) { // note is being triggered again
          // insert a note off before it
          addedNoteOffs.push_back({ false, nData.pitch, 0, nData.channel });
          // keep track of simultaneous note ons
          triggerCountMap[key]++;
          it++;
        } else if (triggerCountMap[key] > 1) {
          notes.erase(it);
          // keep track of simultaneous note ons
          triggerCountMap[key]--;
        } else {
          // don't touch anything and remove the entry from the map
          triggerCountMap.erase(key);
          it++;
        }
      }
    }

    addedNoteOffs.insert(addedNoteOffs.end(),notes.begin(),notes.end());
    notes = addedNoteOffs;
  }

  void reset() {
    triggerCountMap.clear();
  }
};

// ???

class OnlyStaccato : public Strategy {
private:

public:
  void preventVoiceStealing(std::vector<noteData>& notes, commandData cmd) {
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
