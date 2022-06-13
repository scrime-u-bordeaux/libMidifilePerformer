#ifndef MFP_CHRONOLOGY_H
#define MFP_CHRONOLOGY_H

#include <iostream>
#include <list>
#include "Event.h"

template <typename T>
class Chronology {
private:

  struct incompleteEventSet{
      Events::Set<T> set;
      Events::Set<T>& followingEmptySet;
  };

  bool unmeet;
  uint64_t date;
  // the set under construction :
  Events::Set<T> inputSet;
  // the set we need to decide what to do before we push to fifo :
  Events::Set<T> bufferSet;
  std::list<Events::Set<T>> fifo;
  // the events for which a beginning was pushed but no immediate end
  // kept track of in case the ending is found later
  std::list<struct incompleteEventSet> incompleteEvents;

  bool isMatchingEnd(T const& refEvent, T const& compEvent){
      return Events::correspond<T>(refEvent, compEvent) && !Events::isStart<T>(compEvent);
  }

  bool constructInsertSet(Events::Set<T>& inputSet, Events::Set<T> const& bufferSet, Events::Set<T>& insertSet){
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

  void checkForIncompleteEvents(){
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

public:
    template <class E>
    friend std::ostream& operator<<(std::ostream& os, struct Chronology<E> const &c);
  Chronology() : unmeet(true), date(0) {}
  ~Chronology() {}

  void pushEvent(int dt, T& data) {
    // this only happens on start or after calling finalize() or clear() :
    if (inputSet.events.empty()) {
      inputSet = {dt, {data}};
      return;
    }

    checkForIncompleteEvents();

    if (dt > 0) {
      if (!Events::hasStart<T>(bufferSet)) {
        if (!Events::hasStart<T>(inputSet)) {
          // merge inputSet into bufferSet :
          bufferSet.events.insert(
            bufferSet.events.end(),
            inputSet.events.begin(),
            inputSet.events.end()
          );
        } else {
          // this prevents us from pushing an ending set in the first position
          if (!fifo.empty()) {
            fifo.push_back(bufferSet);
          }

          bufferSet = inputSet;
        }
      } else {
        fifo.push_back(bufferSet);

        if (Events::hasStart<T>(inputSet)) {
            Events::Set<T> insertSet{inputSet.dt, {}};

            if (unmeet) constructInsertSet(inputSet,bufferSet,insertSet);

            fifo.push_back(insertSet);
            if(insertSet.events.empty())
                incompleteEvents.push_back({bufferSet,fifo.back()});
        }

        bufferSet = inputSet;
      }

      // after we updated everything, we can reinit inputSet with latest input
      inputSet = {dt, {data}};

  } else { // dt == 0 ; this is a synchronized event ; just append to the input.
      inputSet.events.push_back(data);
    }

    //std::cout << *this << std::endl;
  }

  void finalize() {
    checkForIncompleteEvents();
    fifo.push_back(bufferSet);
    fifo.push_back(inputSet);

    // always end with an ending set
    if (Events::hasStart<T>(inputSet)) {
      fifo.push_back({1, {}});
    }

    bufferSet.events.clear();
    inputSet.events.clear();
  }

  bool hasEvents() {
    return fifo.size() > 0;
  }

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

  void clear() {
    fifo.clear();
    inputSet.events.clear();
    bufferSet.events.clear();
  }
};

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
