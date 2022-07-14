#ifndef MFP_RENDERER_H
#define MFP_RENDERER_H

#include <iostream>
#include <list>
#include <map>
#include "Chronology.h"

template <typename Model, typename Command, typename CommandKey>
class Renderer {

    // -------------------------------------------------------------------------
    // --------------------------PRIVATE FIELDS---------------------------------
    // -------------------------------------------------------------------------

    Chronology<Model> modelEvents; // A chronology of the model (partition)

    Chronology<Command> commandEvents; // A chronology of the commands

    bool lastEventPulled; // Indicates whether the last event of the model
    // has already been pulled, so as to react differently when asked if any are left.

    std::list<std::vector<Model>> orphanedEndings; // A fifo of ending events
    // that should have been associated to a key press, and have thus been thrown out.
    // They are associated to releases which would otherwise have no effect.
    // Note : this list should never be used under normal circumstances
    // (because if ending events have been associated to a key press,
    // that means the preprocessing of the model chronology went wrong.)

    std::map<CommandKey, std::vector<Model>> map3; // A map between a start event
    // and its correspondent ending.

    // -------------------------------------------------------------------------

public:

    // -------------------------------------------------------------------------
    // ----------------------CONSTRUCTORS/DESTRUCTORS---------------------------
    // -------------------------------------------------------------------------

    Renderer() : lastEventPulled(false), modelEvents(Chronology<Model>()) {}
    Renderer(ChronologyParams::parameters params) :
        lastEventPulled(false), modelEvents(Chronology<Model>(params)) {}

    // -------------------------------------------------------------------------
    // ---------------------------PUBLIC METHODS--------------------------------
    // -------------------------------------------------------------------------

    // Push a new model event.
    // For now, command chronologies are not supported, so this is the only push

    virtual void pushEvent(int dt, Model event) {
        modelEvents.pushEvent(dt, event);
    }

    // This should be private if implemented at all,
    // since commands will be pushed and pulled live

    // void pushCommandEvent(int dt, Command cmd) {
    //     commandEvents.pushEvent(dt, cmd);
    // }

    // Finalize the fixed partition.

    virtual void finalize() {
        modelEvents.finalize();
        //std::cout << "C++ debug : " << std::endl << modelEvents << std::endl;
    }

    // If the last event isn't pulled, refer to the partition chronology.
    // If it has, then it is still available, so there are still events.
    // However, if the user has explicitly stated to exclude it, then it is possible.
    // (This is necessary when checking to see if the chronology is simply empty.)

    virtual bool hasEvents(bool countLastEvent = true) {
        return modelEvents.hasEvents() || (countLastEvent&&lastEventPulled);
    }

    // Pull events from the partition chronology.
    // Is there any use for this ??

    virtual std::vector<Model> pullEvents() {
        return modelEvents.pullEvents();
    }

    virtual Events::Set<Model> pullEventsSet() {
        return modelEvents.pullEventsSet();
    }

    // Combine a command with the appropriate model events.
    // The combineN methods could be invoked from live commands or by pulling
    // the commandEvents chronology.

    virtual std::vector<Model> combine3(Command cmd) {
        CommandKey key = Events::keyFromData<Command, CommandKey>(cmd);
        std::vector<Model> emptyEvents = {};

        // If the command is a press, search for the next event.

        if (Events::isStart<Command>(cmd)) {
            //std::cout << "start command" << std::endl;
            std::vector<Model> events = modelEvents.pullEvents();

            // If these next events are not all endings :
            // (isn't this always the case ??)
            // (isn't it counter-intuitive to press a key and only release notes ?)

            if (Events::hasStart<Model>(events)) {
                std::vector<Model> nextEvents = emptyEvents;

                // Get the associated end,
                // which thanks to the chronology spec,
                // is always found right next to the beginning.

                try {
                    nextEvents = modelEvents.pullEvents();
                    if (Events::hasStart<Model>(nextEvents)) throw nextEvents;
                } catch (std::vector<Model> nextEvents) {
                    // nextEvents should be an ending set.
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

                // Indicate that the last event has been pulled.
                if (!modelEvents.hasEvents()) lastEventPulled = true;

                // Map the key to this event, so as to bind its release to it.
                map3[key] = nextEvents;
                return events;

            } else {
                orphanedEndings.push_back(events);
                if (!modelEvents.hasEvents()) lastEventPulled = true;
                return emptyEvents;
            }
        } else {

            //std::cout << "end command" << std::endl;

            try {
                if (map3.find(key) == map3.end() && !lastEventPulled)
                    throw std::runtime_error("INVALID MAP ENTRY FOR KEY");
            } catch (std::runtime_error e) {
                // Original idea (early 2022) :
                // This can only happen if two controllers pressed the same key on the same channel
                // This should not happen

                // Update 08/07/22 ; this is FAR more common than we thought.
                // For example, this happens if the system is launched with a key already held;
                // The release of that key will trigger the exception because no note was associated with the key.

                // This causes an instant segfault in the ossia binding.
                // So the question is, should we keep it that way ?
                std::cout << e.what() << std::endl;
                exit(1);
            }

            std::vector<Model> events = map3[key];
            if (events.empty() && !orphanedEndings.empty()) {
                events = orphanedEndings.front();
                orphanedEndings.pop_front();
            }
            map3.erase(key);
            return events;
        }
    }

    virtual void clear() {
        modelEvents.clear();
    }

    // Replace the partition chronology entirely.
    // DEEP COPY so that the original partition will be left unmodified.

    void setPartition(Chronology<Model> const newPartition){
        this->clear();
        modelEvents = newPartition;
    }

    Chronology<Model> getPartition(){
        return modelEvents;
    }

};

#endif /* MFP_RENDERER_H */
