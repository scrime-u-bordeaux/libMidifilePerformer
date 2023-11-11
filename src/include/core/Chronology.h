#ifndef MFP_CHRONOLOGY_H
#define MFP_CHRONOLOGY_H

#include <iostream>
#include <list>
#include "Events.h"

namespace ChronologyParams{
    struct parameters{
        bool unmeet; // Indicates whether or not to displace end events when possible
        // ONLY RELEVANT FOR MODEL DATA. DISABLE FOR COMMANDS.

        bool complete; // Indicates whether or not to complete empty sets when possible
        // ONLY RELEVANT FOR MODEL DATA. DISABLE FOR COMMANDS.

        Events::correspondOption shiftMode; // Indicates whether or not to place the end events that have been
        // clustered together with their beginning before that beginning.

        int temporalResolution; // Time threshold under which the events will be considered simultaneous
        // and merged in a single set.
        // MIDI Standard is 0.
    };

    static parameters constexpr default_params = {
        .unmeet = true,
        .complete = false,
        .temporalResolution = 0
    };

    static parameters constexpr no_unmeet = {
        .unmeet = false,
        .complete = false,
        .temporalResolution = 0
    };
}

// to be instantiated like Chronology<SomeEventType, std::vector>
// or Chronology<SomeEventType, std::list>
template <
  typename T,
  template <
    typename = Events::SetPair<T>,
    typename = std::allocator<Events::SetPair<T>>
  > class Container
>
class Chronology {

  // ---------------------------------------------------------------------------
  // ------------------------------DATA TYPES-----------------------------------
  // ---------------------------------------------------------------------------

private:

  // Describes a place where a set containing at least one beginning event
  // was left without immediate ending

  struct incompleteEventSet{
    Events::Set<T> set; // a copy of the set of events left without endings
    Events::Set<T>* followingEmptySet; // a POINTER to the empty set that follows
    // HAS to be a pointer for chronologies to be assignable.
  };

  // ---------------------------------------------------------------------------
  // ----------------------------PRIVATE FIELDS---------------------------------
  // ---------------------------------------------------------------------------

  bool finalizedFlag;

  std::int64_t dtAccum;

  ChronologyParams::parameters params;

  Events::Set<T> inputSet; // Set containing the most recent input

  Events::Set<T> bufferSet; // Set containing previous data not yet pushed
  // to the container, to be altered depending on various conditions

  Events::SetPair<T> bufferPair; // SetPair constructed along and pushed
  // when both start and end fields have been filled

  std::pair<bool, bool> bufferPairState; // flags to keep track of bufferPair
  // construction state (if both flags true, bufferPair can be pushed)

  Container<Events::SetPair<T>> container; // The user-facing front of the
  // chronology. It is where events are pushed to and pulled from.

  std::list<struct incompleteEventSet> incompleteEvents; // The events for which
  // a beginning was pushed, but no immediate end
  // kept track of in case the ending is found later

  // ---------------------------------------------------------------------------
  // ---------------------------PRIVATE METHODS---------------------------------
  // ---------------------------------------------------------------------------

  // Shifts ending events contained in the inputSet into the insertSet
  // if they match start events contained in the bufferSet

  bool constructInsertSet(Events::Set<T>& inputSet,
                          Events::Set<T> const& bufferSet,
                          Events::Set<T>& insertSet) {

    for (auto& bufferEvent : bufferSet.events) {
      if (Events::isStart<T>(bufferEvent)) {
        auto it = inputSet.events.begin();
        while (it != inputSet.events.end()) {
          if (Events::isMatchingEnd(bufferEvent,*it,params.shiftMode)) {
            insertSet.events.push_back(*it);
            it = inputSet.events.erase(it);
          } else {
            it++;
          }
        }
      }
    }

    return !insertSet.events.empty();
  }

  // Checks whether incomplete events can be completed using the current
  // inputSet. Inserts the corresponding endings directly into the empty set
  // that follows them.

  void checkForEventCompletion() {

    auto predicate = [this](incompleteEventSet& s) {
        return constructInsertSet(inputSet,s.set,*(s.followingEmptySet));
    };

    if (!incompleteEvents.empty()) std::erase_if(incompleteEvents, predicate);
  }

  // The set of steps followed when modifying or pushing bufferSet and inputSet.
  // Used by pushEvent to update the chronology and by finalize to finalize it.

  void genericPushLogic(bool last) {

    // bufferSet is an ending set //////////////////////////////////////////////
    if (!Events::hasStart<T>(bufferSet)) {
      // inputSet is ALSO an ending set ////////////////////////////////////////
      if (!Events::hasStart<T>(inputSet)) {
        // bufferSet and inputSet must be merged :
        Events::mergeSets(bufferSet,inputSet);
        bufferSet.dt += inputSet.dt;

        if (last && bufferPairState.first) {
          bufferPair.end = bufferSet;
          bufferPairState.second = true;
          container.push_back(bufferPair);
        }
      // inputSet is a starting set ////////////////////////////////////////////  
      } else { 
        // Guard against pushing an incomplete pair if no starting set has been
        // created yet
        if (bufferPairState.first) {
          bufferPair.end = bufferSet;
          bufferPairState.second = true;
          container.push_back(bufferPair);
        }

        if (last) {
          bufferPair.start = inputSet;
          bufferPair.end = { 0, {} };
          bufferPairState = { true, true };
          container.push_back(bufferPair);
        } else {
          bufferSet = inputSet;
        }
      }
    // bufferSet is a starting set /////////////////////////////////////////////
    } else {
      bufferPair.start = bufferSet;
      bufferPairState = { true, false };

      // inputSet is ALSO a starting set ///////////////////////////////////////
      if (Events::hasStart<T>(inputSet)) {
        // bufferSet and inputSet will have to be separated by an empty set,
        // EXCEPT if unmeet is enabled.
        Events::Set<T> insertSet{ inputSet.dt, {} };
        // we set the input set to come immediately after the insert set
        inputSet.dt = 0;

        // If unmeet is enabled, try to fill the empty set.
        if (params.unmeet) constructInsertSet(inputSet, bufferSet, insertSet);

        // complete and push the SetPair
        bufferPair.end = insertSet;
        bufferPairState.second = true;
        container.push_back(bufferPair);

        // If it IS empty, register the bufferSet as incomplete.
        if (params.complete && insertSet.events.empty()) {
          // incompleteEvents.push_back({ bufferSet, &container.back() });
          incompleteEvents.push_back({ bufferSet, &(container.back().end) });
        }
      }

      if (last) {
        // input set is a starting set so the pair was completed with insertSet
        // and pushed to the container, now we need to create the last pair
        if (bufferPairState.second) {
          bufferPair.start = inputSet;
          bufferPair.end = { 0, {} };
        // inputSet is an ending set so we just complete the pair with it
        } else {
          bufferPair.end = inputSet;
        }
        container.push_back(bufferPair);
      } else {
        bufferSet = inputSet;
      }
    }
  }

  // Move the ending of any events that have been synchronized with an identical
  // start and placed later in the set (causing that start not to play) before
  // said start in the set.

  void shiftSameEventEndings(Events::Set<T>& set) {
    std::vector<T> otherEvents;
    std::vector<T> endingsToShift;
    bool matched = false;

    for (T const& event : set.events) {
      for (T const& otherEvent : otherEvents) {

        if (
          Events::isStart<T>(otherEvent) &&
          Events::isMatchingEnd(otherEvent, event, params.shiftMode)
        ) {
          matched = true;
          endingsToShift.push_back(event);
          break;
        }
      }

      if (!matched) otherEvents.push_back(event);
      matched = false;
    }

    Events::Set<T> newSet = { set.dt, endingsToShift };
    Events::mergeSets(newSet, otherEvents);
    set = newSet;
  }

  void shiftSameEventEndings() {
    for (Events::SetPair<T>& set : container) {
      shiftSameEventEndings(set.start);
    }
  }

  // ---------------------------------------------------------------------------

public:

  // ---------------------------------------------------------------------------
  // -------------------------------OVERLOADING---------------------------------
  // ---------------------------------------------------------------------------

  template <
    typename E,
    template <
      typename = Events::SetPair<E>,
      typename = std::allocator<Events::SetPair<E>>
    > class C
  >
  friend std::ostream& operator<<(
    std::ostream& os,
    struct Chronology<E, C> const &c
  );

  // ---------------------------------------------------------------------------
  // ------------------------CONSTRUCTORS/DESTRUCTORS---------------------------
  // ---------------------------------------------------------------------------

  Chronology() :
  params(ChronologyParams::default_params),
  bufferPairState(false, false), dtAccum(0) {}

  Chronology(ChronologyParams::parameters initParams) :
  params(initParams),
  bufferPairState(false, false), dtAccum(0) {}

  ~Chronology() {}

  // ---------------------------------------------------------------------------
  // -----------------------------PUBLIC METHODS--------------------------------
  // ---------------------------------------------------------------------------

  typename Container<Events::SetPair<T>>::iterator begin() {
    return container.begin();
  }

  typename Container<Events::SetPair<T>>::iterator end() {
    return container.end();
  }

  std::size_t size() const { return container.size(); }

  bool finalized() const { return finalizedFlag; }

  // Called when a new event is added to the chronology.

  void pushEvent(std::int64_t dt, T const& data) {
    finalizedFlag = false;

    // This only happens on start or after calling finalize() or clear() ;
    // the inputSet is made to be the first input.
    if (inputSet.events.empty()) {
      inputSet = { dt, { data } };
      return;
    }

    // Before doing anything else, check whether the current inputSet
    // can complete a previously incomplete event.
    if (params.complete) checkForEventCompletion();

    // Event begins at a different time ; inputSet and bufferSet will change
    if ((dt + dtAccum) > params.temporalResolution) {
    // if (dt > params.temporalResolution) {
      genericPushLogic(false);

      // The inputSet is now the most recent input.
      inputSet = { dt + dtAccum, { data } };
      dtAccum = 0;
      // inputSet = { dt, { data }};
    } else { // this is a synchronized event ; just append to the input.
      inputSet.events.push_back(data);
      // inputSet.dt += dt;
      dtAccum += dt;
      // or use dtAccum var to keep track of "drift" due to temporalResolution 
      // and separate far enough events?
    }
  }

  // ---------------------------------------------------------------------------

  // Called after all events have been pushed, and the chronology is ready.

  void finalize() {

    // Pushes the bufferSet and inputSet to container with the same rules
    // as a normal push
    if (params.complete) checkForEventCompletion();
    genericPushLogic(true);

    // Ensure no start events precede a corresponding end event in any set.
    shiftSameEventEndings();

    // Reset the inner sets.
    dtAccum = 0;
    inputSet = { 0, {} };
    bufferSet = { 0, {} };
    bufferPairState = { false, false };

    finalizedFlag = true;
  }

  // Required for cases where we want to use container specific methods

  Container<Events::SetPair<T>>& getContainer() {
    return container;
  }

  // Completely reset the chronology.

  void clear() {
    finalizedFlag = false;

    dtAccum = 0;
    inputSet = { 0, {} };
    bufferSet = { 0, {} };
    bufferPairState = { false, false };
    container.clear();
  }
};

// -----------------------------------------------------------------------------
// ----------------------------FRIEND FUNCTIONS---------------------------------
// -----------------------------------------------------------------------------

template <
  typename T,
  template <
    typename = Events::SetPair<T>,
    typename = std::allocator<Events::SetPair<T>>
  > class Container
>
std::ostream& operator<<(
  std::ostream& os,
  struct Chronology<T, Container> const &c
) {
  os << "Chronology Input Set : " << c.inputSet  << std::endl;
  os << "Chronology Buffer Set : " << c.bufferSet << std::endl;
  os << "Chronology Container : " << std::endl;
  for (const Events::SetPair<T>& s : c.container) {
    os << s.start << std::endl;
    os << s.end << std::endl;
  }
  return os;
}

#endif /* MFP_CHRONOLOGY_H */
