#ifndef MFP_MFPEVENTS_H
#define MFP_MFPEVENTS_H

#include <iostream>
#include <string>
#include <cstdint>
#include <vector>
#include "../core/Events.h"

static std::string noteNames[12] = {"A","A#","B","C","C#","D","D#","E","F","F#","G","G#"};

inline std::string convertIdToNoteName(uint8_t id){
    if(id < 21 || id > 108) return "??";

    uint8_t relativeId = id - 21;

    uint8_t name = (relativeId % 12);
    uint8_t pitch = (relativeId / 12);
    if(name > 2) pitch++;

    return noteNames[name]+std::to_string(pitch);
}

// NB : commandKey and noteKey structs are for use as std::map keys

struct commandData {
    bool pressed;
    uint8_t id;
    uint8_t velocity;
    uint8_t channel;
};

inline std::ostream& operator<<(std::ostream& os, struct commandData const &cmd){
    std::string name = convertIdToNoteName(cmd.id);
    return os << "[ pressed : " << cmd.pressed << " , " <<
    "id : " << int(cmd.id) <<
    " (" << name << "), " <<
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

inline std::ostream& operator<<(std::ostream& os, struct noteData const &note){
    std::string name = convertIdToNoteName(note.pitch);
    return os << "[ on : " << note.on << " , " <<
    "pitch : " << int(note.pitch) <<
    " (" << name << "), " <<
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

/* * * * * * * * * * * * * specializations for commands * * * * * * * * * * * */

template<>
inline bool Events::isStart<commandData>(commandData const& cmd) { return cmd.pressed; }

template<>
inline bool Events::correspond<commandData>(commandData const& cmd1, commandData const& cmd2) {
    return cmd1.id == cmd2.id && cmd1.channel == cmd2.channel;
}

template <>
inline commandKey Events::keyFromData<commandData, commandKey>(commandData const& cmd) {
    return { cmd.id, cmd.channel };
}

//* * * * * * * * * * * * * specializations for notes * * * * * * * * * * * * */

template<>
inline bool Events::isStart<noteData>(noteData const& note) { return note.on; }

template<>
inline bool Events::correspond<noteData>(noteData const& note1, noteData const& note2) {
    return note1.pitch == note2.pitch && note1.channel == note2.channel;
}

template <>
inline noteKey Events::keyFromData<noteData, noteKey>(noteData const& note) {
    return { note.pitch, note.channel };
}


#endif /* MFP_MFPEVENTS_H */
