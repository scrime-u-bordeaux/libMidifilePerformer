#ifndef MFP_ANTIVOICESTEALING_H
#define MFP_ANTIVOICESTEALING_H

class AntiVoiceStealing {
private:
  std::map<noteKey, std::uint8_t> triggerCountMap;

public:
  void preventVoiceStealing(std::vector<noteData>& notes) {
    // std::cout < "preventing voice stealing" << std::endl;

    std::vector<noteData> addedNoteOffs;

    auto it = notes.begin();

    while (it != notes.end()) {
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