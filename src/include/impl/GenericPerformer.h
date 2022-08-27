#ifndef MFP_GENERICPERFORMER_H
#define MFP_GENERICPERFORMER_H

#include "NoteAndCommandEvents.h"
#include "AbstractPerformer.h"
#include "../core/Chronology.h"

////////////////////////////////////////////////////////////////////////////////
// GENERIC RENDERER CLASS TEMPLATE IMPLEMENTATION //////////////////////////////
////////////////////////////////////////////////////////////////////////////////

// to be finalized :

// attempt at writing a general purpose API with balanced benefits from using
// std::list or std::vector as container types

// NB : this API is not clean and mostly experimental
// it could be used wrongly if pull AND iterate approaches are mixed


template <
  template <
    typename = Events::SetPair<noteData>,
    typename = std::allocator<Events::SetPair<noteData>>
  > class Container
>
class GenericPerformer : public AbstractPerformer {
private:
  bool pullMode;

  typedef
  Chronology<noteData, Container<Events::SetPair<noteData>>> NoteChronology;
  NoteChronology score;
  NoteChronology::iterator it;

  // this MUST be defined or the class will be abstract :
  virtual Events::SetPair<noteData> getNextSetPair() override {
    if (pullMode) return pullSetPair();
    return getSetPairAndAdvance();
  }

public:
  GenericPerformer() :
  AbstractPerformer(), pullMode(false) {
    it = score.begin();
  }

  GenericPerformer(ChronologyParams::parameters params) :
  AbstractPerformer(), pullMode(false), score(NoteChronology(params)) {
    it = score.begin()
  }

  std::size_t size() {
    return score.size();
  }

  void clear() {
    score.clear();
  }

  void pushEvent(std::int64_t dt, const noteData& note) {
    score.pushEvent(dt, note);
  }

  void finalize() {
    score.finalize();
  };

  // will perform best with std::list
  Events::SetPair<noteData> pullSetPair() {
    if (score.size() > 0) {
      // this should be of same complexity as calling front() then pop_front()
      // on a std::list
      auto it = score.begin();
      Events::SetPair<noteData> res = *it;
      score.getContainer().erase(it);
      return res;
    }
    return {
      { 0, {} },
      { 0, {} }
    };
  }

  // will perform best with std::vector
  Events::SetPair<noteData> getSetPairAndAdvance() {
    if (it != score.end()) {
      Events::SetPair res = *it;
      std::advance(it, 1);
      if (it == score.end()) it = score.begin(); // loop by default
      return res;
    }
    return {
      { 0, {} },
      { 0, {} }
    };
  }

  // will perform best with std::vector
  Events::SetPair<noteData> getSetPairAtIndex(std::size_t index) {
    if (index < score.size()) {
      auto it = score.begin();
      std::advance(it, index);
      return *it;
    }
    return {
      { 0, {} },
      { 0, {} }
    };
  }

  std::vector<noteData> render(commandData cmd) {
    std::vector<noteData> res = renderer.combine3(cmd, this).events;
    performVoiceStealing(res, cmd);
    adjustToCommandVelocity(res, cmd.velocity);
    return res.events;
  }

  NoteChronology getChronology() {
    return score;
  }

  void setChronology(NoteChronology const& newScore) {
    score = newScore;
  }
};

////////////////////////////////////////////////////////////////////////////////
// ORIGINAL MFPRenderer CLASS //////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

/*
class MFPRenderer {
private:
  std::shared_ptr<VoiceStealing::Strategy> stealingStrategy;
  std::shared_ptr<ChordVelocityMapping::Strategy> chordStrategy;
  Renderer<noteData, commandData, commandKey> renderer;

  void setDefaultStrategies() {
    setVoiceStealingStrategy(
      // VoiceStealing::StrategyType::None
      VoiceStealing::StrategyType::LastNoteOffWins
      // VoiceStealing::StrategyType::OnlyStaccato
    );

    setChordRenderingStrategy(
      ChordVelocityMapping::StrategyType::SameForAll
      // ChordVelocityMapping::StrategyType::ClippedScaledFromMean
      // ChordVelocityMapping::StrategyType::AdjustedScaledFromMean
      // ChordVelocityMapping::StrategyType::ClippedScaledFromMax
    );
  }

  void performVoiceStealing(
    std::vector<noteData>& notes,
    commandData cmd
  ) const {
    if (stealingStrategy.get() == nullptr) return;
    stealingStrategy->performVoiceStealing(notes, cmd);
  }

  void adjustToCommandVelocity(
    std::vector<noteData>& notes,
    uint8_t cmd_velocity
  ) const {
    if (chordStrategy.get() == nullptr) return;
    chordStrategy->adjustToCommandVelocity(notes, cmd_velocity);
  }

public:

  MFPRenderer() : renderer() {
    setDefaultStrategies();
  }

  MFPRenderer(ChronologyParams::parameters params) : renderer(params) {
    setDefaultStrategies();
  }

  void setVoiceStealingStrategy(VoiceStealing::StrategyType s) {
    stealingStrategy = VoiceStealing::createStrategy(s);
  }

  void setChordRenderingStrategy(ChordVelocityMapping::StrategyType s) {
    chordStrategy = ChordVelocityMapping::createStrategy(s);
  }

  void pushEvent(int dt, noteData event) { renderer.pushEvent(dt, event); }

  void finalize() { renderer.finalize(); }

  bool hasEvents(bool countLastEvent = true) {
    return renderer.hasEvents(countLastEvent);
  }

  std::vector<noteData> pullEvents() { return renderer.pullEvents(); }

  Events::Set<noteData> pullEventsSet() { return renderer.pullEventsSet(); }

  std::vector<noteData> combine3(commandData cmd,
                                 bool useCommandVelocity = true) {
    std::vector<noteData> res = renderer.combine3(cmd);
    performVoiceStealing(res, cmd);
    if (useCommandVelocity) adjustToCommandVelocity(res, cmd.velocity);
    return res;
  }

  void clear() { renderer.clear(); }

  void setPartition(Chronology<noteData> const newPartition){ renderer.setPartition(newPartition); }

  Chronology<noteData> getPartition() { return renderer.getPartition(); }
};
//*/

#endif /* MFP_GENERICPERFORMER_H */
