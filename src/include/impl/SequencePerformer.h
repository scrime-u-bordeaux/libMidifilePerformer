#ifndef MFP_SEQUENCERENDERER_H
#define MFP_SEQUENCERENDERER_H

#include "NoteAndCommandEvents.h"
#include "AbstractPerformer.h"
#include "../core/Chronology.h"

class SequencePerformer : public AbstractPerformer {
private:
  bool finalized;
  Chronology<noteData, std::vector> score;

  bool looping;
  std::size_t minIndex, maxIndex, currentIndex;

  // keep track of started events to allow "all active note offs" generation
  // std::list<noteData> startedEvents;

  virtual Events::SetPair<noteData> getNextSetPair() override {
    Events::SetPair<noteData> res;
    auto it = score.begin() + currentIndex;

    if (it != score.end())
      res = *it;
    else
      res = {
        { 0, {} },
        { 0, {} }
      };

    if (currentIndex < maxIndex) {
      currentIndex++;
    } else {
      currentIndex = minIndex;
    }

    return res;
  }

  // we need something like this to stop dangling notes :
  // (maybe this should be in voicestealing strategy instead)
  void flushStartedEvents(Events::SetPair<noteData>& pair) {
    // todo
  }

public:
  SequencePerformer() :
  AbstractPerformer(), score(Chronology<noteData, std::vector>()),
  finalized(false), minIndex(0), maxIndex(0), currentIndex(0) {}

  SequencePerformer(ChronologyParams::parameters params) :
  AbstractPerformer(), score(Chronology<noteData, std::vector>(params)),
  finalized(false), minIndex(0), maxIndex(0), currentIndex(0) {}

  std::size_t size() {
    return finalized ? score.size() : 0;
  }

  void clear() {
    finalized = false;
    score.clear();
    setLoopPoints(0, 0);
    setCurrentIndex(0);
  }

  void pushEvent(std::uint32_t dt, const noteData& data) {
    if (finalized) return;
    score.pushEvent(dt, data);
  }

  void finalize() {
    score.finalize();
    setLoopPoints(0, score.size() - 1);
    setCurrentIndex(0);
    finalized = true;
  }

  bool getLooping() { return looping; }
  void setLooping(bool l) { looping = l; }

  std::size_t getLoopStartIndex() { return minIndex; }

  std::size_t setLoopStartIndex(std::size_t mini) {
    minIndex = std::min(std::min(mini, maxIndex), size() - 1);
    return minIndex;
  }

  std::size_t getLoopEndIndex() { return maxIndex; }

  std::size_t setLoopEndIndex(std::size_t maxi) {
    maxIndex = std::min(std::max(minIndex, maxi), size() - 1);
    return maxIndex;
  }

  void setLoopPoints(std::size_t mini, std::size_t maxi) {
    minIndex = std::min(std::min(mini, maxi), size() - 1);
    maxIndex = std::min(std::max(mini, maxi), size() - 1);
  }

  std::size_t setCurrentIndex(std::size_t index) {
    resetVoiceStealing();
    currentIndex = std::min(maxIndex, std::max(minIndex, index));
    return currentIndex;
  }

  std::size_t getCurrentIndex() {
    return currentIndex;
  }

  Events::SetPair<noteData> getCurrentSetPair() {
    return *(score.begin() + currentIndex);
  }

  std::vector<noteData> render(commandData cmd) {
    return render(cmd, true);
  }

  std::vector<noteData> render(commandData cmd, bool useCommandVelocity) {
    if (!finalized) return {};

    std::vector<noteData> res = renderer.combine3(cmd, this).events;
    preventVoiceStealing(res, cmd);
    if (useCommandVelocity) adjustToCommandVelocity(res, cmd.velocity);
    return res;
  }

  Chronology<noteData, std::vector> getChronology() {
    return score;
  }

  void setChronology(const Chronology<noteData, std::vector>& newScore) {
    score = newScore;
  }
};

#endif /* MFP_SEQUENCERENDERER_H */
