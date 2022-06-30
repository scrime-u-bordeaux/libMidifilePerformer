#ifndef MFP_EVENT_H
#define MFP_EVENT_H

#include <iostream>
#include <cstdint>
#include <vector>

// NB : commandKey and noteKey structs are for use as std::map keys

struct commandData {
    bool pressed;
    uint8_t id;
    uint8_t velocity;
    uint8_t channel;
};

std::ostream& operator<<(std::ostream& os, struct commandData const &cmd){
    return os << "[ pressed : " << cmd.pressed << " , " <<
    "id : " << int(cmd.id) << " , " <<
    "velocity : " << int(cmd.velocity) << " , " <<
    "channel : " << int(cmd.channel) << " ]";
}

struct commandKey {
    uint8_t id;
    uint8_t channel;

    bool operator==(const commandKey& key) const {
        return channel == key.channel && id == key.id;
    };

    bool operator<(const commandKey& key) const {
        return channel < key.channel ||
               (channel == key.channel && id < key.id);
    };
};

struct noteData {
    bool on;
    uint8_t pitch;
    uint8_t velocity;
    uint8_t channel;
};

std::ostream& operator<<(std::ostream& os, struct noteData const &note){
    return os << "[ on : " << note.on << " , " <<
    "id : " << int(note.pitch) << " , " <<
    "velocity : " << int(note.velocity) << " , " <<
    "channel : " << int(note.channel) << " ]";
}

struct noteKey {
    uint8_t pitch;
    uint8_t channel;

    bool operator==(const noteKey& key) const {
        return channel == key.channel && pitch == key.pitch;
    };

    bool operator<(const noteKey& key) const {
        return channel < key.channel ||
               (channel == key.channel && pitch < key.pitch);
    };
};

/* * * * * * * * * * * * * * template event utilities * * * * * * * * * * * * */

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

/* * * * * * * * * * * * * specializations for commands * * * * * * * * * * * */

template<>
bool isStart<commandData>(commandData const& cmd) { return cmd.pressed; }

template<>
bool correspond<commandData>(commandData const& cmd1, commandData const& cmd2) {
    return cmd1.id == cmd2.id && cmd1.channel == cmd2.channel;
}

template <>
commandKey keyFromData<commandData, commandKey>(commandData const& cmd) {
    return { cmd.id, cmd.channel };
}

//* * * * * * * * * * * * * specializations for notes * * * * * * * * * * * * */

template<>
bool isStart<noteData>(noteData const& note) { return note.on; }

template<>
bool correspond<noteData>(noteData const& note1, noteData const& note2) {
    return note1.pitch == note2.pitch && note1.channel == note2.channel;
}

template <>
noteKey keyFromData<noteData, noteKey>(noteData const& note) {
    return { note.pitch, note.channel };
}

}; /* end namespace Events */

#endif /* MFP_EVENT_H */
