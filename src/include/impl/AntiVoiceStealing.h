#ifndef MFP_ANTIVOICESTEALING_H
#define MFP_ANTIVOICESTEALING_H


#include <map>
#include "NoteAndCommandEvents.h"

// Note : this could be made into a template if we had a template function that
// could generate a generic end event from a start event, e.g :
// Events::genericEndFromStart<noteData>(startData)"
// we already have Events::isStart<noteData>

class AntiVoiceStealing {
private:
  std::map<noteKey, std::uint8_t> triggerCountMap;

public:
  std::vector<noteData> preventVoiceStealing(const std::vector<noteData>& notes) {
    std::vector<noteData> copiedNotes = notes;
    std::vector<noteData> addedNoteOffs;
    auto it = copiedNotes.begin();

    while (it != copiedNotes.end()) {
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
          copiedNotes.erase(it);
          // keep track of simultaneous note ons
          triggerCountMap[key]--;
        } else {
          // don't touch anything and remove the entry from the map
          triggerCountMap.erase(key);
          it++;
        }
      }
    }

    addedNoteOffs.insert(addedNoteOffs.end(), copiedNotes.begin(), copiedNotes.end());
    copiedNotes = addedNoteOffs;
    return copiedNotes;
  }

  std::vector<noteData> getAllNoteOffs() {
    return getAllNoteOffs(triggerCountMap);
  }

  std::vector<noteData> getAllNoteOffs(
    const std::map<noteKey, std::uint8_t>& state
  ) {
    std::vector<noteData> res;

    for (auto it = state.begin(); it != state.end(); ++it) {
      for (auto i = 0; i < it->second; ++i) {
        res.push_back({ false, it->first.pitch, 0, it->first.channel });
      }
    }

    return res;
  }

  std::map<noteKey, std::uint8_t> getTriggerCountMap() const {
    return triggerCountMap;
  }

  void setTriggerCountMap(std::map<noteKey, std::uint8_t>& map) {
    triggerCountMap = map;
  }

  void clear() {
    triggerCountMap.clear();
  }
};

#endif /* MFP_ANTIVOICESTEALING_H */
