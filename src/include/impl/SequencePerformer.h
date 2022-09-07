#ifndef MFP_SEQUENCEPERFORMER_H
#define MFP_SEQUENCEPERFORMER_H

#include "NoteAndCommandEvents.h"
#include "AntiVoiceStealing.h"
#include "AbstractPerformer.h"
#include "../core/Chronology.h"

/*
 * Note : we could make this class a template, only strategies really need to
 * use concrete types (noteData and commandData) to make sense out of them, like
 * the chordVelocityMapping strategy.
 * The strategy mechanism itself could also be a template, but in this case the
 * specializations would use types representing MIDI note events for the Model
 * abstract type : this would allow the SequencePerformer (or other performers)
 * to adapt to different formalisms of a MIDI message, depending on the MIDI
 * library we want to interface with (the Command abstract type would probably
 * use the same MIDI note event type as Model). All we would need to do then is
 * create a file similar to NoteAndCommandEvents.h with specializations of the
 * required functions for the 3rd-party library defined types, and instantiate a
 * Performer specialized with these types.
 */

class SequencePerformer : public AbstractPerformer {
public:
  // Stopping state : when not looping and reaching the end we want to let the
  // last command ring, so we enter this state in which 
  enum State {
    Armed,
    Playing,
    Stopping,
    Stopped
  };

private:
  State currentState;

  // a callback for note event listeners, it will be called on any call to
  // render, setCurrentIndex and stop
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

  // internal utilities ////////////////////////////////////////////////////////

  virtual void executeNoteEventsCallback(std::vector<noteData>& notes) {
    noteEventsCallback(notes);
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

  void mergeAvsStates(
    std::map<noteKey, std::uint8_t>& target,
    const std::map<noteKey, std::uint8_t> source
  ) const {
    for (auto it = source.begin(); it != source.end(); ++it) {
      if (target.find(it->first) != target.end()) {
        target[it->first] += it->second;
      } else {
        target[it->first] = 1;
      }
    }
  }

  void updateIndexFromLoopIndices() {
    if (currentIndex < minIndex) {
      setCurrentIndex(minIndex);
    } else if (currentIndex > maxIndex) {
      setCurrentIndex(maxIndex);
      currentState = State::Stopped;
    } else {
      // setCurrentIndex(currentIndex);
    }
  }

  /**
   * Implementation of the pure virtual method getNextSetPair defined in
   * Renderer::Provider that we inherit from AbstractPerformer, and which is
   * called from render when we execute `renderer.combine3(cmd, this)`
   *
   * This method is responsible for maintaining the index of the SetPair it
   * returns by itself, most of the time by incrementing it, but jumps must be
   * handled carefully because they often leave dangling notes (which can be
   * taken care of using the AntiVoiceStealing class and the avsHistory)
   *
   * This means we can access the next event sets only by rendering them.
   * To preview the next event set pair, the peekNextSetPair method is provided
   * which helps for playback situations (we want to know how long to wait
   * before playing an event set, without having to play it first to get this
   * information).
   */
  virtual Events::SetPair<noteData> getNextSetPair() override { // <=> pull
    currentIndex = getNextIndex();
    auto res = getCurrentSetPair();
    
    // this will be executed as long as currentState != State::Stopped
    if (currentIndex == score.size()) {
      currentState = State::Stopping;
      currentIndex = maxIndex;
      return res; // will be empty if currentIndex == score.size()
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

  virtual void setNoteEventsCallback(
    std::function<void(std::vector<noteData>)> cb
  ) {
    noteEventsCallback = cb;
  }

  virtual std::size_t size() const {
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
  // as for stop, make sure to call getAllNoteOffs before to stop pending notes
  // if you don't have a note event listener
  virtual std::size_t setCurrentIndex(std::size_t index) {
    stop();
    currentIndex = std::min(maxIndex, std::max(minIndex, index));
    currentState = State::Armed;
    return currentIndex;
  }

  virtual std::size_t getCurrentIndex() {
    return currentIndex;
  }

  virtual Events::SetPair<noteData> getCurrentSetPair() {
    return getSetPairAtIndex(currentIndex);
  }

  // used by getNextSetPair and peekNextSetPair
  virtual std::size_t getNextIndex() const {
    if (currentState == State::Armed) return currentIndex;
    if (currentIndex < maxIndex) return currentIndex + 1;
    if (!looping) return score.size();
    // else : currentIndex >= maxIndex && looping
    return minIndex;
  }

  virtual Events::SetPair<noteData> peekNextSetPair() { // <=> "peek"
    return getSetPairAtIndex(getNextIndex());
  }

  /**
   * the only way to move forward in the chronology is to call this method
   * (or its overload below) !!!
   */
  virtual std::vector<noteData> render(commandData cmd) {
    return render(cmd, true);
  }

  virtual std::vector<noteData> render(commandData cmd, bool useCommandVelocity) { // <=> "pull"
    if (!score.finalized() || currentState == State::Stopped) return {};

    std::vector<noteData> res = renderer.combine3(cmd, this).events;
    avs.preventVoiceStealing(res);

    if (currentState == State::Stopping && avs.getTriggerCountMap().empty()) {
      currentState = State::Stopped;
    }

    if (useCommandVelocity) {
      adjustToCommandVelocity(res, cmd.velocity);
    }

    executeNoteEventsCallback(res);
    return res;
  }

  std::vector<noteData> getAllNoteOffs() {
    return avs.getAllNoteOffs();
  }

  virtual Chronology<noteData, std::vector> getChronology() {
    return score;
  }

  virtual void setChronology(const Chronology<noteData, std::vector>& newScore) {
    score = newScore;

    if (score.finalized()) {
      setLoopIndices(0, score.size() - 1);
      setCurrentIndex(0);
    }
  }
};

#endif /* MFP_SEQUENCEPERFORMER_H */
