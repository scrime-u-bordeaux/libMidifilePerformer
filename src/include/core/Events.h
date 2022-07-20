#ifndef MFP_EVENT_H
#define MFP_EVENT_H

#include <iostream>
#include <cstdint>
#include <vector>

namespace Events {

// Pseudo macros : macros and namespaces don't mix.
// Still written in macro case for legibility reasons.

const int MERGE_AT_BEGINNING=1;
const int MERGE_AT_END=0;

template <typename T>
struct Set {
    int64_t dt;
    std::vector<T> events;

    // Used for sorting IN THE CASE OF ABSOLUTE TICKS
    // (No longer in use, but can still come in handy at some point)

    bool operator<(const Set<T>& set) const {
        return dt < set.dt;
    }
};

template <typename T>
std::ostream& operator<<(std::ostream& os, struct Set<T> const &s){
    os << "Set at dt " << s.dt << " with elements [ " ;
    for(T const & e : s.events){
        os << e << " , ";
    }
    os << " ] " << std::endl;
    return os;
}

// Self-explanatory

template <typename T>
bool isStart(T const& e) { return true; }

template <typename T>
bool correspond(T const& e1, T const& e2) { return false; }

enum class correspondOption : int; //define in implementations

template <typename T>
bool correspond(T const& e1, T const& e2, correspondOption o) { return false; }

// Determines if compEvent is an ending event matching refEvent

template <typename T>
bool isMatchingEnd(T const& refEvent, T const& compEvent) {
  return Events::correspond<T>(refEvent, compEvent)
      && !Events::isStart<T>(compEvent);
}

template <typename T>
bool isMatchingEnd(T const& refEvent, T const& compEvent, correspondOption o) {
  return Events::correspond<T>(refEvent, compEvent, o)
      && !Events::isStart<T>(compEvent);
}

template <typename T>
bool hasStart(std::vector<T> const& events) {
    for (auto& e : events)
        if (isStart<T>(e))
            return true;
    return false;
}

template <typename T>

// Merging at the beginning of a vector should be avoided, it is an inefficient operation that requires element shifting

void mergeSets(Events::Set<T>& greaterSet, std::vector<T> const& mergedSet, int mergePoint=MERGE_AT_END){
    typename std::vector<T>::iterator it;
    if(mergePoint==MERGE_AT_BEGINNING) it = greaterSet.events.begin();
    else it = greaterSet.events.end();
    greaterSet.events.insert(
      it,
      mergedSet.begin(),
      mergedSet.end()
    );
}

template <typename T>
void mergeSets(Events::Set<T>& greaterSet, Events::Set<T> const& mergedSet, int mergePoint=MERGE_AT_END) {
    mergeSets(greaterSet,mergedSet.events,mergePoint);
}

template <typename T>
bool hasStart(Set<T> const& set) { return hasStart<T>(set.events); }

template <typename T, typename K>
K keyFromData(T const& e) { K res; return res; }

}; /* end namespace Events */

#endif /* MFP_EVENT_H */
