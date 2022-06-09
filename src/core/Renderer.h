#ifndef MFP_RENDERER_H
#define MFP_RENDERER_H

#include <iostream>
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
            std::vector<Model> events = modelEvents.pullEvents();

            if (Events::hasStart<Model>(events)) {
                std::vector<Model> nextEvents = emptyEvents;

                try{
                    //while(nextEvents.empty() || Events::hasStart<Model>(nextEvents))
                    nextEvents = modelEvents.pullEvents();
                    if(Events::hasStart<Model>(nextEvents)) throw nextEvents;
                }catch(std::vector<Model> nextEvents){
                    std::cout << "ASSOCIATED START IN COMBINE MAP" << std::endl;
                    exit(1);
                }

                map3[key] = nextEvents;
                return events;

            } else {
                return emptyEvents;
            }
        } else {

            try{
                if(map3.find(key) == map3.end()) throw std::runtime_error("INVALID MAP ENTRY FOR KEY");
            }catch(std::runtime_error e){
                std::cout << e.what() << std::endl;
                exit(1);
            }

            std::vector<Model> events = map3[key];
            map3.erase(key);
            return events;
        }
    }
};

#endif /* MFP_RENDERER_H */
