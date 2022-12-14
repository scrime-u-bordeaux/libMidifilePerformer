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

void pushMPTKEvent(long tick, bool pressed, int pitch, int channel, int velocity) {
    noteData event(pressed, pitch, channel, velocity);
    unityPerformer.pushEvent(tick, event);
    return;
}

void finalizePerformer() {
    unityPerformer.setLooping(true);
    unityPerformer.finalize();
}
