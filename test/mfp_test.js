// const fs = require('fs');
// const { serialize } = require('v8');
const MidifilePerformer = require('../js/MidifilePerformer.js');

// const data = fs.readFileSync(`${process.cwd()}/assets/mid/chopin-etude-op10-no4.mid`, null);

// This test will use only one channel and constant velocity.
// It only aims to test the validity of command-partition interaction.

const velocity = 127;
const channel = 1;
// Commands will stay fixed for this test.

// Command list is : x x !x !x y z !y !z z y !y x !x !z
// How is the x x sequence possible ?

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


// -----------------------------------------------------------------------------
// ----------------------------UTIL FUNCTIONS-----------------------------------
// -----------------------------------------------------------------------------

function setIncoherentPartition(renderer){
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
}

function setCoherentPartition(renderer){
    renderer.pushEvent(0, note(true,40));

    renderer.pushEvent(1, note(false,40));
    renderer.pushEvent(1, note(true,50));

    renderer.pushEvent(2, note(false,50));

    renderer.pushEvent(3, note(true,80));

    renderer.pushEvent(4, note(true,20));
    renderer.pushEvent(4, note(false,80));

    renderer.pushEvent(5, note(false,20));

    renderer.pushEvent(6, note(true,20));
    renderer.pushEvent(6, note(true,40));
    renderer.pushEvent(6, note(true,80));

    renderer.pushEvent(8, note(false,20));
    renderer.pushEvent(8, note(false,40));
    renderer.pushEvent(8, note(false,80));

    renderer.finalize();
}

function play(commands,renderer){
    let i = 0;

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

function note(on, pitch) {
  return { on, pitch, velocity, channel };
}

function command(pressed, id) {
  return { pressed, id, velocity, channel };
}

// -----------------------------------------------------------------------------
// ------------------------------INITIALIZE-------------------------------------
// -----------------------------------------------------------------------------

MidifilePerformer.onRuntimeInitialized = () => {

  console.log("Test with set commands : x x !x !x y z !y !z z y !y x !x !z");
  console.log("\n");

  // ---------------------------------------------------------------------------

  console.log("Test performance of the given commands with partition :");
  console.log("(40)(!40,50)(!50)(80)(20,!80)(!20)(20,40,80)()(!20,!40,!80)");
  console.log("Expected output :")
  console.log("(40)(50)(!40)(!50)(80)(20)(!80)(!20)(20)(40)(!40)(80)(!80)(!20)")

  const renderer1 = new MidifilePerformer.Renderer();

  setCoherentPartition(renderer1);

  play(commands,renderer1);

  console.log("\n");

  // ---------------------------------------------------------------------------

  console.log("Test robustness against an incoherent partition")
  console.log("(orphan endings and a note starting twice without ending)");

  const renderer2 = new MidifilePerformer.Renderer();

  setIncoherentPartition(renderer2);

  play(commands,renderer2)

  console.log("\n");

  // ---------------------------------------------------------------------------

  console.log("Test over");

}
