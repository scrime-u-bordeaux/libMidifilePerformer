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

public:
  class Provider {
  public:
    /** Destructor. */
    virtual ~Provider() = default;
    /** Called by combine3. */
    virtual Events::SetPair<Model> getNextSetPair() = 0;
  };

  virtual void clear() {
    pendingEndingSets.clear();
  }

  virtual Events::Set<Model> combine3(Command cmd, Provider* provider) {
    CommandKey key = Events::keyFromData<Command, CommandKey>(cmd);
    Events::Set<Model> emptySet = { 0, {} };

    if (Events::isStart<Command>(cmd)) {
      auto pair = provider->getNextSetPair();
      Events::Set<Model> startingSet = pair.start;
      Events::Set<Model> endingSet = pair.end;

      // Manage multi-controller interaction :
      // if this same key already has events registered in the combine map,
      // trigger them before registering the new ones.
      if (pendingEndingSets.find(key) != pendingEndingSets.end()) {

        // despite it being non optimal, we use MERGE_AT_BEGINNING in order to
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
      // This should never happen thanks to multi-controller interaction
      // management, except if we start rendering while some key is pressed,
      // then release this key during rendering, in which case we simply return
      // an emptySet and this will have no effect
      if (pendingEndingSets.find(key) == pendingEndingSets.end()) {
        return emptySet;
      }

      Events::Set<Model> endingSet = pendingEndingSets[key];
      pendingEndingSets.erase(key);
      return endingSet;
    }
  }

public:
  virtual ~Renderer() {}
};

#endif /* MFP_RENDERER_H */
