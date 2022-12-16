#include "../../include/impl/UnityDllWrapper.h"
#include "../../include/impl/SequencePerformer.h"

static ChronologyParams::parameters constexpr unityParams = {
        .unmeet = true,
        .complete = false,
        .shiftMode = Events::correspondOption::PITCH_ONLY,
        //.temporalResolution = 10 THIS MUST BE CORRECTED 
        // WE HAVE TO CONVERT EVERYTHING TO MS FOR THIS TO MEAN SOMETHING
        // For now it's disabled
};

SequencePerformer unityPerformer(unityParams);
noteData exampleData(true, 60, 92, 1);

void pushMPTKEvent(long tick, bool pressed, int pitch, int channel, int velocity) {
    noteData event(pressed, pitch, velocity, channel);
    unityPerformer.pushEvent(tick, event);
    return;
}

void clearPerformer() {
    unityPerformer.clear();
}

void finalizePerformer() {
    unityPerformer.setLooping(true);
    unityPerformer.finalize();
}

void renderCommand(bool pressed, int ID, unsigned long* dataContainer) {
    commandData command(pressed, ID, DEFAULT_VELOCITY, DEFAULT_CHANNEL);
    std::vector<noteData> result = unityPerformer.render(command);
    for (noteData const& event : result) *dataContainer++ = static_cast<unsigned long>(event);
}
