#ifndef MFP_CHRONOLOGY_H
#define MFP_CHRONOLOGY_H

#include <iostream>
#include <list>
#include "Event.h"

template <typename T>
class Chronology {
private:
  bool unmeet;
  uint64_t date;
  // the set under construction :
  Events::Set<T> inputSet;
  // the set we need to decide what to do before we push to fifo :
  Events::Set<T> bufferSet;
  std::list<Events::Set<T>> fifo;

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
          // this prevents us from pushing an ending set in the first place
          if (!fifo.empty()) {
            fifo.push_back(bufferSet);
          }

          bufferSet = inputSet;
        }
      } else {
        /*if(!bufferSet.events.empty())*/ fifo.push_back(bufferSet);

        if (Events::hasStart<T>(inputSet)) {
          Events::Set<T> insertSet{inputSet.dt, {}};

          if (unmeet) {
            for (auto& e : bufferSet.events) {
              if (Events::isStart<T>(e)) {
                auto it = inputSet.events.begin();
                while (it != inputSet.events.end()) {
                  if (Events::correspond<T>(e, *it) && !Events::isStart<T>(*it)) {
                    insertSet.events.push_back(*it);
                    it = inputSet.events.erase(it);
                  } else {
                    it++;
                  }
                }
              }
            }
          }
         /*if(!insertSet.events.empty())*/ fifo.push_back(insertSet);
        }
        bufferSet = inputSet;
      }
      // after we updated everything, we can reinit inputSet with latest input
      inputSet = {dt, {data}};
    } else { // dt == 0
      inputSet.events.push_back(data);
    }
  }

  void finalize() {
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
