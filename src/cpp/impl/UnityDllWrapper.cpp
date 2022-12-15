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

unsigned long processNoteData(noteData event) {
    uint8_t firstByte = 0;
    if (event.on) firstByte = 0x90;
    else firstByte = 0x80;

    firstByte += event.channel;

    uint8_t secondByte = event.pitch;
    uint8_t thirdByte = event.velocity;

    unsigned long result = (firstByte << 16 | secondByte << 8 | thirdByte);
    return result;
}

unsigned long renderCommand(bool pressed, int ID, unsigned long* dataContainer, unsigned int size) {
    commandData command(pressed, ID, DEFAULT_VELOCITY, DEFAULT_CHANNEL);
    std::vector<noteData> result = unityPerformer.render(command);
    return processNoteData(result[0]);
    /*unsigned int i = 0;
    for (noteData event : result) {
        if (i >= size) break; //this should never happen
        dataContainer[i] = processNoteData(event);
        i++;
    }
    if (i >= size) i--; //sacrifice last event ; again, this should never happen, size is absurdly big
    dataContainer[i] = 0;*/
}
