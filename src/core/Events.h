#ifndef MFP_EVENT_H
#define MFP_EVENT_H

#include <iostream>
#include <cstdint>
#include <vector>

namespace Events {

template <typename T>
struct Set {
    int64_t dt;
    std::vector<T> events;

    // Used for sorting IN THE CASE OF ABSOLUTE TICKS
    // (Of use in the ossia score binding)

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

template <typename T>
bool isStart(T const& e) { return true; }

template <typename T>
bool correspond(T const& e1, T const& e2) { return false; }

template <typename T>
bool hasStart(std::vector<T> const& events) {
    for (auto& e : events)
        if (isStart<T>(e))
            return true;
    return false;
}

template <typename T>
bool hasStart(Set<T> const& set) { return hasStart<T>(set.events); }

template <typename T, typename K>
K keyFromData(T const& e) { K res; return res; }

}; /* end namespace Events */

#endif /* MFP_EVENT_H */
