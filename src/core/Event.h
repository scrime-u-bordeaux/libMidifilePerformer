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
    int dt;
    std::vector<T> events;
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
bool isStart(T& e) { return true; }

template <typename T>
bool correspond(T& e1, T& e2) { return false; }

template <typename T>
bool hasStart(std::vector<T>& events) {
    for (auto& e : events)
        if (isStart<T>(e))
            return true;
    return false;
}

template <typename T>
bool hasStart(Set<T>& set) { return hasStart<T>(set.events); }

template <typename T, typename K>
K keyFromData(T& e) { K res; return res; }

/* * * * * * * * * * * * * specializations for commands * * * * * * * * * * * */

template<>
bool isStart<commandData>(commandData& cmd) { return cmd.pressed; }

template<>
bool correspond<commandData>(commandData& cmd1, commandData& cmd2) {
    return cmd1.id == cmd2.id && cmd1.channel == cmd2.channel;
}

template <>
commandKey keyFromData<commandData, commandKey>(commandData& cmd) {
    return { cmd.id, cmd.channel };
}

//* * * * * * * * * * * * * specializations for notes * * * * * * * * * * * * */

template<>
bool isStart<noteData>(noteData& note) { return note.on; }

template<>
bool correspond<noteData>(noteData& note1, noteData& note2) {
    return note1.pitch == note2.pitch && note1.channel == note2.channel;
}

template <>
noteKey keyFromData<noteData, noteKey>(noteData& note) {
    return { note.pitch, note.channel };
}

}; /* end namespace Events */

#endif /* MFP_EVENT_H */
