#ifndef MFP_RENDERER_H
#define MFP_RENDERER_H

#include <iostream>
#include <list>
#include <map>
#include "Chronology.h"

// BASE RENDERER CLASS TEMPLATE ////////////////////////////////////////////////

template <typename Model, typename Command, typename CommandKey>
class Renderer {
private:
  std::map<CommandKey, Events::Set<Model>> pendingEndingSets;

  virtual Events::SetPair<Model> getNextSetPair() = 0;

protected:
  virtual Events::Set<Model> combine3(Command cmd) {
    CommandKey key = Events::keyFromData<Command, CommandKey>(cmd);
    Events::Set<Model> emptySet = { 0, {} };

    if (Events::isStart<Command>(cmd)) {
      auto pair = getNextSetPair();
      Events::Set<Model> startingSet = pair.start;
      Events::Set<Model> endingSet = pair.end;

      // Manage multi-controller interaction :
      // if this same key already has events registered in the combine map,
      // trigger them before registering the new ones.

      if (pendingEndingSets.find(key) != pendingEndingSets.end()) {
        // despite it being not optimal, we use MERGE_AT_BEGINNING in order to
        // keep all the end events first.
        // It is OK because this is not supposed to happen frequently.
        // todo : optimize if necessary
        Events::mergeSets(
          startingSet,
          pendingEndingSets[key],
          Events::MERGE_AT_BEGINNING
        );
      }

      pendingEndingSets[key] = endingSet;
      return startingSet;
    } else {
      // This should never happen, except if we start rendering while
      // some key is pressed then release this key during rendering, in which
      // case we simply return an emptySet
      if (pendingEndingSets.find(key) == pendingEndingSets.end()) {
        return emptySet;
      }

      Events::Set<Model> endingSet = pendingEndingSets[key];
      pendingEndingSets.erase(key);
      return endingSet;
    }
  }

  // is this really useful (not used anywhere ATM) ?
  // if we just return all pending endings set we miss all the
  // "not immediately following" endings

  // keeping track of all active notes to be able to generate
  // all their corresponding endings at any moment would be more appropriate as
  // it would allow to optimally handle an "all notes off" event.
  // plus, this could be performed without any knowledge of pendingEndingSets,
  // like the voice stealing stuff

  // virtual Events::Set<Model> getAllPendingEndingEvents() {
  //   Events::Set<Model> res = { 0, {} };

  //   for (auto& set : pendingEndingSets) {
  //     const auto& extraEvents = set.second.events;
  //     res.events.insert(
  //       res.events.end(),
  //       extraEvents.begin(),
  //       extraEvents.end()
  //     );
  //   }

  //   return res;
  // }

public:
  virtual ~Renderer() {}
  
  // virtual Events::Set<Model> render(Command cmd) = 0;
};

#endif /* MFP_RENDERER_H */
