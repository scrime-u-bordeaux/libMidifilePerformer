#ifndef MFP_RENDERER_H
#define MFP_RENDERER_H

#include <iostream>
#include <list>
#include <map>
#include "Chronology.h"

template <typename Model, typename Command, typename CommandKey>
class Renderer {
    Chronology<Model> modelEvents;
    Chronology<Command> commandEvents;
    bool lastEventPulled;

    std::map<CommandKey, std::vector<Model>> map3;
public:

    Renderer() : lastEventPulled(false) {}

    void pushEvent(int dt, Model event) {
        modelEvents.pushEvent(dt, event);
    }

    // this should be private if implemented at all
    // void pushCommandEvent(int dt, Command cmd) {
    //     commandEvents.pushEvent(dt, cmd);
    // }

    void finalize() {
        modelEvents.finalize();
        //std::cout << "C++ debug : " << std::endl << modelEvents << std::endl;
    }

    bool hasEvents() {
        return modelEvents.hasEvents() || lastEventPulled;
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
                    nextEvents = modelEvents.pullEvents();
                    if(Events::hasStart<Model>(nextEvents)) throw nextEvents;
                }catch(std::vector<Model> nextEvents){
                    std::cout << "ASSOCIATED START IN COMBINE MAP" << std::endl;
                    exit(1);
                }

                /*std::cout << "C++ debug : " << cmd << " associated with ";
                if(!nextEvents.empty()){
                    for(Model& e : nextEvents){
                        std::cout << e << std::endl;
                    }
                }else{
                    std::cout << "no release events" << std::endl;
                }*/

                if(!modelEvents.hasEvents()) lastEventPulled=true;

                map3[key] = nextEvents;
                return events;

            } else {
                return emptyEvents;
            }
        } else {


            // This can only happen if two controllers pressed the same key on the same channel
            // This should not happen

            try{
                if(map3.find(key) == map3.end() && !lastEventPulled)
                    throw std::runtime_error("INVALID MAP ENTRY FOR KEY");
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
