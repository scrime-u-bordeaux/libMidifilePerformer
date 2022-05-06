#ifndef MFP_RENDERER_H
#define MFP_RENDERER_H

#include <list>
#include "Chronology.h"

template <typename Model, typename Command, typename CommandKey>
class Renderer {
    Chronology<Model> modelEvents;
    Chronology<Command> commandEvents;

    std::map<CommandKey, std::vector<Model>> map3;
public:
    Renderer() {}

    void pushEvent(int dt, Model event) {
        modelEvents.pushEvent(dt, event);
    }

    // this should be private if implemented at all
    // void pushCommandEvent(int dt, Command cmd) {
    //     commandEvents.pushEvent(dt, cmd);
    // }

    void finalize() {
        modelEvents.finalize();
    }

    bool hasEvents() {
        return modelEvents.hasEvents();
    }

    std::vector<Model> pullEvents() {
        return modelEvents.pullEvents();
    }

    // the combineN methods could be invoked from live commands or by pulling
    // the commandEvents chronology.

    std::vector<Model> combine3(Command cmd) {
        CommandKey key = Events::keyFromData<Command, CommandKey>(cmd);
        std::vector<Model> emptyEvents = {};

        if (Events::isStart<Command>(cmd)) {
            // TODO : remove this (why is this even here ?)
            /*
            if (map3.find(key) == map3.end()) {
                return emptyEvents; // or throw exception
            }
            */

            std::vector<Model> events = modelEvents.pullEvents();

            if (Events::hasStart<Model>(events)) {
                std::vector<Model> nextEvents = modelEvents.pullEvents();
                map3[key] = nextEvents;
                return events;
            } else {
                return emptyEvents;
            }
        } else {
            if (map3.find(key) != map3.end()) {
                std::vector<Model> events = map3[key];
                map3.erase(key);
                return events;
            } else {
                // this shouldn't happen except if we allow command polyphony
                // todo : throw exception instead
                return emptyEvents;
            }
        }
    }
};

#endif /* MFP_RENDERER_H */