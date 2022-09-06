#ifndef MFP_SEQUENCERENDERER_H
#define MFP_SEQUENCERENDERER_H

#include "NoteAndCommandEvents.h"
#include "AntiVoiceStealing.h"
#include "AbstractPerformer.h"
#include "../core/Chronology.h"

class SequencePerformer : public AbstractPerformer {
public:
  enum State {
    Armed,
    Playing,
    Stopping, // when not looping and reaching the end we want to let the last command ring
    Stopped
  };

private:
  State currentState;

  // a callback for note event listeners, it will be called on any call to
  // render()
  std::function<void(std::vector<noteData>)> noteEventsCallback;

  // anti voice stealing system that works by tracking the number of active
  // notes in real time. it can generate a lightweight "allnotesoff" event
  // anytime we want to reset the index
  AntiVoiceStealing avs;

  // history of states before/after each successive start/end pair has been fed
  // to avs : its size equals size() + 1 because it include the state
  // before index 0 (empty), and after the state score.size() - 1
  // like this, if we want the state before currentIndex has been processed we
  // we just avsHistory[currentIndex], and if we want the state after
  // currentIndex has been processed we  avsHistory[currentIndex + 1]
  // this will be fine as long as currentIndex < size()
  std::vector<std::map<noteKey, std::uint8_t>> avsHistory;

  // a noteData vector chronology
  Chronology<noteData, std::vector> score;

  // some specific vars
  bool looping; // true if loop is enabled
  std::size_t minIndex, maxIndex, currentIndex; // loop indices + current index

  // internal utilities

  virtual void executeNoteEventsCallback(std::vector<noteData>& notes) {
    noteEventsCallback(notes);
  }

  virtual std::size_t getNextIndex() {
    if (currentState == State::Armed) return currentIndex;
    if (currentIndex < maxIndex) return currentIndex + 1;
    if (!looping) return score.size();
    // else : currentIndex >= maxIndex && looping
    return minIndex;
  }

  virtual Events::SetPair<noteData> getCurrentSetPair() {
    return getSetPairAtIndex(currentIndex);
  }

  virtual Events::SetPair<noteData> getSetPairAtIndex(std::size_t index) {
    auto it = score.begin() + index;
    if (it != score.end()) return *it;
    return {{ 0, {} }, { 0, {} }};
  }

  void createAvsHistoryFromScore() {
    // create initial state (before first pair has been processed)
    std::map<noteKey, std::uint8_t> emptyMap;
    avsHistory = { emptyMap };

    auto it = score.begin();
  
    while (it != score.end()) {
      avs.preventVoiceStealing(it->start.events);
      avs.preventVoiceStealing(it->end.events);
      avsHistory.push_back(avs.getTriggerCountMap());
      it++;
    }
  }

  void mergeAvsStates(std::map<noteKey, std::uint8_t>& target,
                      const std::map<noteKey, std::uint8_t> source)
  {
    for (auto it = source.begin(); it != source.end(); ++it) {
      if (target.find(it->first) != target.end()) {
        target[it->first] += it->second;
      } else {
        target[it->first] = 1;
      }
    }
  }

  //////////////////////////////////////////////////////////////////////////////
  // IMPLEMENTATION OF THE PURE VIRTUAL METHOD DEFINED IN Renderer::Provider ///
  //////////////////////////////////////////////////////////////////////////////

  // this method is called by the render method exclusively, through combine3

  virtual Events::SetPair<noteData> getNextSetPair() override { // <=> pull
    currentIndex = getNextIndex();
    auto res = getCurrentSetPair();
    
    if (currentIndex == score.size()) {
      currentState = State::Stopping;
      // if we are stopping (because we reached the end index), we only let the
      // release commands through and return 
      currentIndex = maxIndex;
      return res;
    }

    // set to Playing after call to getNextIndex which checks currentState value
    if (currentState == State::Armed) {
      currentState = State::Playing;
    }

    auto activeNotes = avs.getTriggerCountMap();

    // if we are about to play first event in loop, we set avs to take upcoming
    // note offs from unplayed note ons into account
    if (currentIndex == minIndex) {
      // merge avs state snapshot with current avs state
      auto avsState = avsHistory[currentIndex];
      mergeAvsStates(avsState, activeNotes);
      avs.setTriggerCountMap(avsState);
    }

    // if we are getting the last set pair, we might have some pending endings
    // so we make sure to end them all in ending set
    if (currentIndex == maxIndex) {
      // get all pending ending events we are supposed to end up with at currentIndex
      // if we play "normally" (i.e. with a single command key, no command overlap)
      auto avsState = avsHistory[currentIndex + 1];
      std::vector<noteData> pendingNoteOffs = avs.getAllNoteOffs(avsState);

      // add these events to the ending set
      res.end.events.insert(
        res.end.events.end(),
        pendingNoteOffs.begin(),
        pendingNoteOffs.end()
      );
    }

    return res;
  }

  void updateIndexFromLoopIndices() {
    if (currentIndex < minIndex) {
      setCurrentIndex(minIndex);
    } else if (currentIndex > maxIndex) {
      setCurrentIndex(maxIndex);
      currentState = State::Stopped;
    } else {
      setCurrentIndex(currentIndex);
    }
  }

public:
  SequencePerformer() :
  AbstractPerformer(), score(Chronology<noteData, std::vector>()),
  minIndex(0), maxIndex(0), currentIndex(0),
  looping(false), currentState(State::Stopped),
  noteEventsCallback([](std::vector<noteData>){}) {}

  SequencePerformer(ChronologyParams::parameters params) :
  AbstractPerformer(), score(Chronology<noteData, std::vector>(params)),
  minIndex(0), maxIndex(0), currentIndex(0),
  looping(false), currentState(State::Stopped),
  noteEventsCallback([](std::vector<noteData>){}) {}

  virtual void setNoteEventsCallback(std::function<void(std::vector<noteData>)> cb) {
    noteEventsCallback = cb;
  }

  virtual std::size_t size() {
    return score.finalized() ? score.size() : 0;
  }

  virtual void clear() {
    score.clear();
    setLoopIndices(0, 0);
    setCurrentIndex(0);
  }

  virtual void pushEvent(std::uint64_t dt, const noteData& data) {
    if (score.finalized()) return;
    score.pushEvent(dt, data);
  }

  virtual void finalize() {
    score.finalize();
    createAvsHistoryFromScore();
    setLoopIndices(0, (score.size() == 0) ? 0 : (score.size() - 1));
    setCurrentIndex(0);
  }

  virtual bool getLooping() { return looping; }
  virtual void setLooping(bool l) { looping = l; }

  virtual std::size_t getLoopStartIndex() { return minIndex; }

  virtual std::size_t setLoopStartIndex(std::size_t mini) {
    if (!score.finalized()) return 0;
    minIndex = std::min(std::min(mini, maxIndex), size() - 1);
    updateIndexFromLoopIndices();
    return minIndex;
  }

  virtual std::size_t getLoopEndIndex() { return maxIndex; }

  virtual std::size_t setLoopEndIndex(std::size_t maxi) {
    if (!score.finalized()) return 0;
    maxIndex = std::min(std::max(minIndex, maxi), size() - 1);
    updateIndexFromLoopIndices();
    return maxIndex;
  }

  virtual void setLoopIndices(std::size_t mini, std::size_t maxi) {
    if (!score.finalized()) return;
    minIndex = std::min(std::min(mini, maxi), size() - 1);
    maxIndex = std::min(std::max(mini, maxi), size() - 1);
    updateIndexFromLoopIndices();
  }

  // stop immediately
  // be careful when calling this method : you should either have a note event
  // listener attached or call getAllNoteOffs before to stop pending notes
    virtual void stop() {
    currentState = State::Stopped;
    std::vector<noteData> allNoteOffs = avs.getAllNoteOffs();
    executeNoteEventsCallback(allNoteOffs);
    avs.clear();
    renderer.clear();
  }

  virtual bool stopped() {
    return currentState == State::Stopped;
  }

  // this method calls stop, then updates currentIndex and sets state to Armed
  // as for stop, make sure not to leave pending notes by calling getAllNoteOffs
  // before to stop them
  virtual std::size_t setCurrentIndex(std::size_t index) {
    stop();
    currentIndex = std::min(maxIndex, std::max(minIndex, index));
    currentState = State::Armed;
    return currentIndex;
  }

  virtual std::size_t getCurrentIndex() {
    return currentIndex;
  }

  virtual Events::SetPair<noteData> peekNextSetPair() { // <=> peek
    return getSetPairAtIndex(getNextIndex());
  }

  std::vector<noteData> getAllNoteOffs() {
    // std::vector<noteData> allNoteOffs = avs.getAllNoteOffs();
    // executeNoteEventsCallback(allNoteOffs);
    // return allNoteOffs;
    return avs.getAllNoteOffs();
  }

  virtual std::vector<noteData> render(commandData cmd) { // <=> step
    return render(cmd, true);
  }

  virtual std::vector<noteData> render(commandData cmd, bool useCommandVelocity) {
    if (!score.finalized() || currentState == State::Stopped) {
      return {};
    }
    /*
    if (currentState == State::Stopping) {
      std::cout << "stopping ..................................." << std::endl;
      std::cout << "avs size " << avs.getTriggerCountMap().size() << std::endl;
      std::cout << "current index " << currentIndex << std::endl;
    }
    //*/
    std::vector<noteData> res = renderer.combine3(cmd, this).events;
    avs.preventVoiceStealing(res);
    /*
    auto state = avs.getTriggerCountMap();
    std::cout << "trigger count map :" << std::endl;
    for (auto it = state.begin(); it != state.end(); ++it) {
      std::cout << static_cast<int>(it->first.pitch) << " ";
      std::cout << static_cast<int>(it->first.channel) << " ";
      std::cout << static_cast<int>(it->second) << std::endl;
    }
    //*/
    if (useCommandVelocity) {
      adjustToCommandVelocity(res, cmd.velocity);
    }
    executeNoteEventsCallback(res);
    if (currentState == State::Stopping && avs.getTriggerCountMap().empty()) {
      // we let all the endings commands through
      // std::cout << "just stopped !" << std::endl;
      currentState = State::Stopped;
    }
    return res;
  }

  virtual Chronology<noteData, std::vector> getChronology() {
    return score;
  }

  virtual void setChronology(const Chronology<noteData, std::vector>& newScore) {
    score = newScore;
    if (score.finalized()) {
      setLoopIndices(0, score.size() - 1);
    }
  }
};

#endif /* MFP_SEQUENCERENDERER_H */
