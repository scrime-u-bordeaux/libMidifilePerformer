#ifndef MFP_CHRONOLOGY_H
#define MFP_CHRONOLOGY_H

#include <iostream>
#include <list>
#include "Event.h"

template <typename T>
class Chronology {
private:

  // ---------------------------------------------------------------------------
  // ------------------------------DATA TYPES-----------------------------------
  // ---------------------------------------------------------------------------

  // Describes a place where a set containing at least one beginning event
  // was left without immediate ending

  struct incompleteEventSet{
      Events::Set<T> set; // a copy of the set of events left without endings
      Events::Set<T>& followingEmptySet; // a reference to the empty set that follows
  };

  // ---------------------------------------------------------------------------
  // ----------------------------PRIVATE FIELDS---------------------------------
  // ---------------------------------------------------------------------------

  bool unmeet; // Indicates whether or not to displace end events when possible
  // ONLY RELEVANT FOR MODEL DATA. DISABLE FOR COMMANDS.

  bool complete; // Indicates whether or not to complete empty sets when possible
  // ONLY RELEVANT FOR MODEL DATA. DISABLE FOR COMMANDS.

  uint64_t date; // Unused for now, purpose unknown

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

  void mergeSets(Events::Set<T>& greaterSet, Events::Set<T> const& mergedSet){
      greaterSet.events.insert(
        greaterSet.events.end(),
        mergedSet.events.begin(),
        mergedSet.events.end()
      );
  }

  // Determines if compEvent is an ending event matching refEvent

  bool isMatchingEnd(T const& refEvent, T const& compEvent){
      return Events::correspond<T>(refEvent, compEvent)
      && !Events::isStart<T>(compEvent);
  }

  // Shifts ending events contained in the inputSet into the insertSet
  // if they match start events contained in the bufferSet

  bool constructInsertSet(Events::Set<T>& inputSet,
      Events::Set<T> const& bufferSet, Events::Set<T>& insertSet){

      for (auto& bufferEvent : bufferSet.events) {
        if (Events::isStart<T>(bufferEvent)) {
          auto it = inputSet.events.begin();
          while (it != inputSet.events.end()) {
            if (isMatchingEnd(bufferEvent,*it)) {
              insertSet.events.push_back(*it);
              it = inputSet.events.erase(it);
            } else {
              it++;
            }
          }
        }
      }

      if(insertSet.events.empty()) return false;
      else return true;
  }

  // Checks whether incomplete events can be completed using the current inputSet.
  // Inserts the corresponding endings directly into the empty set that follows them.

  void checkForEventCompletion(){
      if(!incompleteEvents.empty()){
          auto it = incompleteEvents.begin();
          bool constructResult=false;
          while(it != incompleteEvents.end()) {
              constructResult=constructInsertSet(inputSet,it->set,it->followingEmptySet);
              if(constructResult) it=incompleteEvents.erase(it);
              else it++;
          }
      }
  }

  // The set of steps followed when modifying or pushing the bufferSet and inputSet.
  // Used by pushEvent to update the chronology, but also lastPush to finalize it.

  void genericPushLogic(bool last){

      // In this case, the bufferSet is either empty or an ending set
      if (!Events::hasStart<T>(bufferSet)) {

        // The inputSet is ALSO an ending set. So they can be merged.
        if (!Events::hasStart<T>(inputSet)) {

            mergeSets(bufferSet,inputSet);
            if(last) fifo.push_back(bufferSet);
            return;

        }else { // the inputSet is a starting set (it has a least one start event)

            // Guard against pushing an ending set in the first position of the fifo
            // Pushing an empty set in other cases is fine, it is an artifical ending

            if (!fifo.empty()) {
              fifo.push_back(bufferSet);
            }

            if(last) fifo.push_back(inputSet);
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
                if (unmeet) constructInsertSet(inputSet,bufferSet,insertSet);

                // Push the set regardless to stay consistent with the format.
                fifo.push_back(insertSet);

                // If it IS empty, register the bufferSet as incomplete.
                if(insertSet.events.empty())
                    incompleteEvents.push_back({bufferSet,fifo.back()});
                }

                if(last) fifo.push_back(inputSet);
                else bufferSet = inputSet;

                return;

        }
  }

  // Called by finalize() to cleanly push the last sets,
  // AFTER checking for incomplete events one last time.

  void lastPush(){
      if(complete) checkForEventCompletion();
      genericPushLogic(true);
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

  Chronology() : unmeet(true), date(0), complete(false) {}
  Chronology(bool _complete) : unmeet(true), date(0), complete(_complete) {}
  ~Chronology() {}

  // ---------------------------------------------------------------------------
  // -----------------------------PUBLIC METHODS--------------------------------
  // ---------------------------------------------------------------------------

  // Called when a new event is added to the chronology.

  void pushEvent(int dt, T& data) {

    // This only happens on start or after calling finalize() or clear() ;
    // the inputSet is made to be the first input.

    if (inputSet.events.empty()) {
      inputSet = {dt, {data}};
      return;
    }

    // Before doing anything else, check whether the current inputSet
    // can complete a previously incomplete event.

    if(complete) checkForEventCompletion();

    if (dt > 0) { // Event begins at a different time ; inputSet and bufferSet will change

      genericPushLogic(false);

      // The inputSet is now the most recent input.
      inputSet = {dt, {data}};

  } else { // dt == 0 ; this is a synchronized event ; just append to the input.
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

    // Reset the inner sets.

    bufferSet.events.clear();
    inputSet.events.clear();
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
