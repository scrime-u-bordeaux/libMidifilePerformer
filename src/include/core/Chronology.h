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

        uint64_t date; // Unused for now, purpose unknown
    };

    static parameters constexpr default_params = {
        .unmeet = true,
        .complete = false,
        .temporalResolution = 0,
        .date = 0
    };

    static parameters constexpr no_unmeet = {
        .unmeet = false,
        .complete = false,
        .temporalResolution = 0,
        .date = 0
    };
}

template <typename T>
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

  ChronologyParams::parameters params;

  Events::Set<T> inputSet; // Set containing the most recent input

  Events::Set<T> bufferSet; // Set containing previous data
  // not yet pushed to the fifo, to be altered depending on various conditions

  std::list<Events::Set<T>> fifo; // The user-facing front of the chronology.
  // It is where events are pushed to and pulled from.

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

  // Checks whether incomplete events can be completed using the current inputSet.
  // Inserts the corresponding endings directly into the empty set that follows them.

  void checkForEventCompletion() {

    auto predicate = [this](incompleteEventSet& s) {
        return constructInsertSet(inputSet,s.set,*(s.followingEmptySet));
    };

    if (!incompleteEvents.empty()) std::erase_if(incompleteEvents, predicate);
  }

  // The set of steps followed when modifying or pushing the bufferSet and inputSet.
  // Used by pushEvent to update the chronology, but also lastPush to finalize it.

  void genericPushLogic(bool last) {

    // In this case, the bufferSet is either empty or an ending set
    if (!Events::hasStart<T>(bufferSet)) {

      // The inputSet is ALSO an ending set. So they can be merged.
      if (!Events::hasStart<T>(inputSet)) {

        Events::mergeSets(bufferSet,inputSet);
        bufferSet.dt += inputSet.dt;
        if (last) fifo.push_back(bufferSet);
        return;

      } else { // the inputSet is a starting set (it has a least one start event)

        // Guard against pushing an empty set in the first position of the fifo
        // Pushing an empty set in other cases is fine, it is an artifical ending

        if (!fifo.empty()) {
          fifo.push_back(bufferSet);
        }

        if (last) fifo.push_back(inputSet);
        else bufferSet = inputSet;

        return;
      }

    } else {

      // The bufferSet is a starting set

      fifo.push_back(bufferSet); // First, push it.

      if (Events::hasStart<T>(inputSet)) { // The inputSet is ALSO a starting set.
        // so the two will have to be separated by an empty set,
        // EXCEPT if unmeet is enabled.
        Events::Set<T> insertSet{inputSet.dt, {}};

        // If unmeet is enabled, try to fill the empty set.
        if (params.unmeet) constructInsertSet(inputSet,bufferSet,insertSet);

        // Push the set regardless to stay consistent with the format.
        fifo.push_back(insertSet);

        // If it IS empty, register the bufferSet as incomplete.
        if (params.complete && insertSet.events.empty())
            incompleteEvents.push_back({bufferSet,&fifo.back()});
      }

      if (last) fifo.push_back(inputSet);
      else bufferSet = inputSet;

      return;
    }
  }

  // Called by finalize() to cleanly push the last sets,
  // AFTER checking for incomplete events one last time.

  void lastPush() {
    if (params.complete) checkForEventCompletion();
    genericPushLogic(true);
  }

  // Move the ending of any events that have been synchronized with an identical start and placed later in the set
  // (causing that start not to play)
  // before said start in the set.

  void shiftSameEventEndings(){
      std::vector<T> otherEvents;
      std::vector<T> endingsToShift;
      bool matched=false;

      for(Events::Set<T>& set : fifo){
          otherEvents.clear();
          endingsToShift.clear();
          matched=false;

          int eventIndex = 0;

          for(T const& event : set.events){

              for(T const& otherEvent : otherEvents){

                  if(Events::isStart<T>(otherEvent)
                  && Events::isMatchingEnd(otherEvent,event,params.shiftMode)){
                      matched=true;
                      endingsToShift.push_back(event);
                      break;
                  }

              }

              if(!matched) otherEvents.push_back(event);
              matched = false;

          }

          Events::Set<T> newSet = { set.dt, endingsToShift };
          Events::mergeSets(newSet,otherEvents);
          set = newSet;
      }
  }

  // ---------------------------------------------------------------------------

public:

  // ---------------------------------------------------------------------------
  // -------------------------------OVERLOADING---------------------------------
  // ---------------------------------------------------------------------------

  template <class E>
  friend std::ostream& operator<<(std::ostream& os, struct Chronology<E> const &c);

  // ---------------------------------------------------------------------------
  // ------------------------CONSTRUCTORS/DESTRUCTORS---------------------------
  // ---------------------------------------------------------------------------

  Chronology() : params(ChronologyParams::default_params) {}
  Chronology(ChronologyParams::parameters initParams) : params(initParams) {}
  ~Chronology() {}

  // ---------------------------------------------------------------------------
  // -----------------------------PUBLIC METHODS--------------------------------
  // ---------------------------------------------------------------------------

  typename std::list<Events::Set<T>>::iterator begin() { return fifo.begin(); }

  typename std::list<Events::Set<T>>::iterator end() { return fifo.end(); }

  std::size_t size() { return fifo.size(); }

  // Called when a new event is added to the chronology.

  void pushEvent(int dt, T const& data) {

    // This only happens on start or after calling finalize() or clear() ;
    // the inputSet is made to be the first input.

    if (inputSet.events.empty()) {
      inputSet = {dt, {data}};
      return;
    }

    // Before doing anything else, check whether the current inputSet
    // can complete a previously incomplete event.

    if (params.complete) checkForEventCompletion();

    if (dt > params.temporalResolution) { // Event begins at a different time ; inputSet and bufferSet will change

      genericPushLogic(false);

      // The inputSet is now the most recent input.
      inputSet = {dt, {data}};

    } else { // this is a synchronized event ; just append to the input.
      inputSet.events.push_back(data);
    }

    //std::cout << *this << std::endl;
  }

  // ---------------------------------------------------------------------------

  // Called after all events have been pushed, and the chronology is ready.

  void finalize() {

    // Pushes the bufferSet and inputSet to fifo with the same rules as a normal push

    lastPush();

    if (Events::hasStart<T>(inputSet)) {
      fifo.push_back({1, {}});
    }

    // Ensure no start events precede a corresponding end event in any set.

    shiftSameEventEndings();

    // Reset the inner sets.

    bufferSet.events.clear();
    inputSet.events.clear();

    //std::cout << *this << std::endl;
  }

  // Self-explanatory.

  bool hasEvents() {
    return fifo.size() > 0;
  }

  // Simply get the first set of events in the fifo.
  // Returns an empty vector if the fifo is empty.

  std::vector<T> pullEvents() {
    Events::Set<T> res;

    if (!this->hasEvents()) {
      return std::vector<T>();
    }

    res = fifo.front();
    fifo.pop_front();

    /*std::cout << "C++ debug : pulled events = [ ";
    for(T& e : res.events){
        std::cout << e << " , ";
    }
    std::cout << " ]" << std::endl;*/

    return res.events;
  }

  Events::Set<T> pullEventsSet() {
    if (!this->hasEvents()){
      return {0,std::vector<T>()};
    }

    Events::Set<T> res = fifo.front();
    fifo.pop_front();

    return res;
  }

  // Completely reset the chronology.

  void clear() {
    fifo.clear();
    inputSet.events.clear();
    bufferSet.events.clear();
  }
};

// -----------------------------------------------------------------------------
// ----------------------------FRIEND FUNCTIONS---------------------------------
// -----------------------------------------------------------------------------

template <typename T>
std::ostream& operator<<(std::ostream& os, struct Chronology<T> const &c){
  os << "Chronology Input Set : " << c.inputSet  << std::endl;
  os << "Chronology Buffer Set : " << c.bufferSet << std::endl;
  os << "Chronology Fifo : " << std::endl;
  for(Events::Set<T> const & s : c.fifo){
    os << s << std::endl;
  }
  return os;
}

#endif /* MFP_CHRONOLOGY_H */
