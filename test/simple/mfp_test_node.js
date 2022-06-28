// const fs = require('fs');
// const { serialize } = require('v8');
// import MidifilePerformer from '../dist/MidifilePerformer.js';
const MidifilePerformer = require('../../dist/node/MidifilePerformer.js');

// const data = fs.readFileSync(`${process.cwd()}/assets/mid/chopin-etude-op10-no4.mid`, null);

// const renderer = new MidifilePerformer.Renderer();
// console.log(MidifilePerformer());
// MidifilePerformer({ onRuntimeInitialized() {
MidifilePerformer.onRuntimeInitialized = () => {
  const velocity = 127;
  const channel = 1;

  function note(on, pitch) {
    return { on, pitch, velocity, channel };
  }

  function command(pressed, id) {
    return { pressed, id, velocity, channel };
  }

  const renderer = new MidifilePerformer.Renderer();

  renderer.pushEvent(0, note(false, 55));
  renderer.pushEvent(1, note(false, 56));
  renderer.pushEvent(0, note(false, 57));
  renderer.pushEvent(1, note(true, 60));
  renderer.pushEvent(1, note(true, 61));
  renderer.pushEvent(1, note(false, 61));
  renderer.pushEvent(0, note(false, 60));
  renderer.pushEvent(1, note(true, 62));
  renderer.pushEvent(0, note(true, 63));
  renderer.pushEvent(1, note(true, 64));
  renderer.pushEvent(1, note(false, 64));
  renderer.pushEvent(0, note(false, 63));
  renderer.pushEvent(1, note(false, 62));
  renderer.pushEvent(1, note(true, 60));
  renderer.finalize(); // integrate last pushed events into the model

  const commands = [
    command(true, 62),
    command(true, 62),
    command(false, 62),
    command(false, 62),
    command(true, 61),
    command(true, 60),
    command(false, 61),
    command(false, 60),

    command(true, 60),
    command(true, 61),
    command(false, 61),
    command(true, 62),
    command(false, 62),
    command(false, 60),
  ];

  let i = 0, cmd = true;

  while (renderer.hasEvents() && i < commands.length) {
  // while (renderer.hasEvents()) {

    const events = renderer.combine3(commands[i++]);
    // const events = renderer.pullEvents();

    console.log('--------------------------');
    for (let j = 0; j < events.size(); j++) {
      const e = events.get(j);
      console.log(`note ${e.on ? 'on' : 'off'} : ${e.pitch}`);
    }
  }
}
// })
