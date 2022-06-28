const test = require('tape');
const MidifilePerformer = require('../dist/node/MidifilePerformer.js');

/* * * * * * * * * * * * * * * * * UTILITIES * * * * * * * * * * * * * * * * */

const velocity = 127;
const channel = 1;

function note(on, pitch) {
  return { on, pitch, velocity, channel };
}

function command(pressed, id) {
  return { pressed, id, velocity, channel };
}

////////// scores

const before = [
    [ 0, true,  60 ],
    [ 5, false, 60 ],
    [ 5, true,  62 ],
    [ 5, false, 62 ]
];

const meets = [
    [ 0, true,  60 ],
    [ 5, false, 60 ],
    [ 0, true,  62 ],
    [ 5, false, 62 ]
];

const overlaps = [
    [ 0, true,  60 ],
    [ 5, true,  62 ],
    [ 5, false, 60 ],
    [ 5, false, 62 ]
];

const starts = [
    [ 0, true,  60 ],
    [ 0, true,  62 ],
    [ 5, false, 60 ],
    [ 5, false, 62 ]
];

const during = [
    [ 0, true,  60 ],
    [ 5, true,  62 ],
    [ 5, false, 62 ],
    [ 5, false, 60 ]
];

const finishes = [
    [ 0, true,  60 ],
    [ 5, true,  62 ],
    [ 5, false, 60 ],
    [ 0, false, 62 ]
];

const equals = [
    [ 0, true,  60 ],
    [ 0, true,  62 ],
    [ 5, false, 60 ],
    [ 0, false, 62 ]
];

const normalScore = [
    [ 0, true, 60 ],
    [ 0, true, 64 ],
    [ 0, true, 67 ],
    [ 0, true, 68 ],
    [ 5, false, 67 ],
    [ 5, false, 64 ],
    [ 5, false, 60 ],
    [ 5, false, 68 ],
];

////////// commands

const beforeCmd = [
    [ true,  60 ],
    [ false, 60 ],
    [ true,  62 ],
    [ false, 62 ]
];

const overlapsCmd = [
    [ true,  60 ],
    [ true,  62 ],
    [ false, 60 ],
    [ false, 62 ]
];

const duringCmd = [
    [ true,  60 ],
    [ true,  62 ],
    [ false, 62 ],
    [ false, 60 ]
];

const longCommandSeries = [
    [ true, 60 ],
    [ false, 60 ],
    [ true, 62 ],
    [ false, 62 ],
];

function makeArray(array) {
    const out = [];
    for (let j = 0; j < array.size(); j++) {
        out.push(array.get(j));
    }
    return out;
}

function writeScoreToRenderer(renderer, notes) {
    //renderer.clear();
    notes.forEach(n => {
        const [ dt, on, pitch ] = n;
        renderer.pushEvent(dt, note(on, pitch));
    });
    renderer.finalize();
}

function getRendererEventSets(renderer) {
    const out = [];
    while (renderer.hasEvents()) {
        const eventsSet = makeArray(renderer.pullEvents());
        out.push(eventsSet);
    }
    return out;
}

function getRendererOutput(renderer, commands) {
    const out = [];
    commands.forEach(c => {
        const [ pressed, id ] = c;
        const eventSet = makeArray(renderer.combine3(command(pressed, id)));
        out.push(eventSet);
    })
    return out;
}


/* * * * * * * * * * * * * * * * * * TESTS * * * * * * * * * * * * * * * * * */

MidifilePerformer.onRuntimeInitialized = function() {
    const renderer = new MidifilePerformer.Renderer();

    test('overlap', function(t) {
        t.plan(1);
        writeScoreToRenderer(renderer, normalScore);
        // const output = getRendererOutput(renderer, longCommandSeries);
        const output = getRendererEventSets(renderer);
        console.log(output);
        t.equal(true, true);
    });

    test('begins', function(t) {
        t.plan(1);
        t.equal(false, false);
    });
};
