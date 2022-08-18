#ifndef MFP_RENDERER_H
#define MFP_RENDERER_H

#include <iostream>
#include <list>
#include <map>
#include "Chronology.h"

template <typename Model, typename Command, typename CommandKey, typename ModelKey>
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
    std::map<ModelKey, CommandKey> antiStealMap; // A map keeping track of which key
    // is associated with each combination of pitch and channel,
    // in order to prevent voice stealing.

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
        CommandKey commandKey = Events::keyFromData<Command, CommandKey>(cmd);
        std::vector<Model> emptyEvents = {};

        // If the command is a key press, search for the next event.

        if (Events::isStart<Command>(cmd)) {
            //std::cout << "start command" << std::endl;
            std::vector<Model> events = modelEvents.pullEvents();

            // For every event associated with this key press,
            // See if it the same note and pitch have been assigned to another key press.
            // If so, delete this note-pitch combination's release
            // from the set of release events assigned to that other key's release.

            for (Model& event : events) {
                ModelKey modelKey = Events::keyFromData<Model, ModelKey>(event);

                if (antiStealMap.find(modelKey) != antiStealMap.end()) {

                    // This means this pitch-channel combination is associated
                    // with another key.

                    CommandKey assignedCommandKey = antiStealMap[modelKey];
                    std::vector<Model>& endEvents = map3[assignedCommandKey];
                    auto it = endEvents.begin();

                    while (it != endEvents.end()){
                        if (Events::correspond<Model>(*it, event))
                            it = endEvents.erase(it); // prevent the other key from releasing the note
                        else it++;
                    }
                }

                antiStealMap[modelKey] = commandKey; // register the new key as this note's owner
            }

            // Then, if the event set that has been pulled is a starting set
            // (Which should always be the case) :

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
                if (!nextEvents.empty()) {
                    for (Model& e : nextEvents) {
                        std::cout << e << std::endl;
                    }
                } else {
                    std::cout << "no release events" << std::endl;
                }*/

                // Indicate that the last event has been pulled.
                if (!modelEvents.hasEvents()) lastEventPulled = true;

                // Manage multi-controller interaction :
                // If this same key already has events registered in the combine map,
                // Trigger them, and then register the new ones.

                if (map3.find(commandKey) != map3.end()) {
                    std::vector<Model> extraEvents = map3[commandKey];
                    events.insert(events.end(), extraEvents.begin(), extraEvents.end());
                    // Should we rather append them to nextEvents ?
                }

                // Map the key to this event, so as to bind its release to it.
                map3[commandKey] = nextEvents;

                return events;

            } else { // this should not happen, but the fallback is here
                orphanedEndings.push_back(events);
                if (!modelEvents.hasEvents()) lastEventPulled = true;
                return emptyEvents;
            }
        } else { // the key was released, so we look in the map to see what to trigger

            //std::cout << "end command" << std::endl;

            if (map3.find(commandKey) == map3.end() && !lastEventPulled)
                return emptyEvents;

            std::vector<Model> events = map3[commandKey];
            if (events.empty() && !orphanedEndings.empty()) {
                events = orphanedEndings.front();
                orphanedEndings.pop_front();
            }
            map3.erase(commandKey);
            return events;
        }
    }

    virtual void clear() {
        modelEvents.clear();
    }

    // Replace the partition chronology entirely.
    // DEEP COPY so that the original partition will be left unmodified.

    void setPartition(Chronology<Model> const newPartition) {
        this->clear();
        modelEvents = newPartition;
    }

    Chronology<Model> getPartition(){
        return modelEvents;
    }

};

#endif /* MFP_RENDERER_H */
