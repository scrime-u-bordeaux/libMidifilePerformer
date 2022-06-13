// const fs = require('fs');
// const { serialize } = require('v8');
const MidifilePerformer = require('../js/MidifilePerformer.js');

// const data = fs.readFileSync(`${process.cwd()}/assets/mid/chopin-etude-op10-no4.mid`, null);

// This test will use only one channel and constant velocity.
// It only aims to test the validity of command-partition interaction.

const velocity = 127;
const channel = 1;
// Commands will stay fixed for this test.

// Command list is : x y !y !x y z !y !z z y !y x !x !z

const commands = [
  command(true, 62),
  command(true, 61),
  command(false, 61),
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
  command(false, 60)
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
}

function setIncompleteCoherentPartition(renderer){
    renderer.pushEvent(1, note(true,40));

    renderer.pushEvent(2, note(true,50));
    renderer.pushEvent(0, note(false,40));

    renderer.pushEvent(3, note(false,50));

    renderer.pushEvent(4, note(true,80));

    renderer.pushEvent(5, note(true,20));
    renderer.pushEvent(0, note(false,80));

    renderer.pushEvent(6, note(false,20));

    renderer.pushEvent(7, note(true,20));
    renderer.pushEvent(0, note(true,40));
    renderer.pushEvent(0, note(true,80));

    renderer.pushEvent(9, note(false,20));
    renderer.pushEvent(0, note(false,40));
    renderer.pushEvent(0, note(false,80));
}

function setFullCoherentPartition(renderer){
    setIncompleteCoherentPartition(renderer);

    renderer.pushEvent(1, note(true,70));
    renderer.pushEvent(1, note(true,75));

    renderer.pushEvent(1, note(false,75));

    renderer.pushEvent(1, note(false,70));
}

function play(commands,renderer){
    let i = 0;

    console.log("Output : ")

    while (renderer.hasEvents() && i < commands.length) {
    // while (renderer.hasEvents()) {

      //console.log("debug : ",commands[i]);
      const events = renderer.combine3(commands[i++]);

      /*console.log('--------------------------');
      for (let j = 0; j < events.size(); j++) {
        const e = events.get(j);
        console.log(`note ${e.on ? 'on' : 'off'} : ${e.pitch}`);
    }*/

     process.stdout.write("(");
     for(let j = 0 ; j < events.size() ; j++){
         const e = events.get(j);
         process.stdout.write((e.on ? '' : '!') + e.pitch);
         if(j != events.size()-1) process.stdout.write(",");
     }
     process.stdout.write(")");
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

  console.log("Test with set commands : x y !y !x y z !y !z z y !y x !x !z");
  console.log("\n");

  // ---------------------------------------------------------------------------

  console.log("Test performance of the given commands with FULL partition :");
  console.log("(40)(!40,50)(!50)(80)(20,!80)(!20)(20,40,80)()(!20,!40,!80)(70)(75)(!75)(!70)");
  console.log("Expected output :")
  console.log("(40)(50)(!50)(!40)(80)(20)(!80)(!20)(20,40,80)(70)(!70)(75)(!75)(!20,!40,!80)")

  const renderer0 = new MidifilePerformer.Renderer();

  setFullCoherentPartition(renderer0);
  renderer0.finalize();

  play(commands,renderer0);

  console.log("\n");

  // ---------------------------------------------------------------------------

  console.log("Test performance of the given commands with incomplete partition :");
  console.log("(40)(!40,50)(!50)(80)(20,!80)(!20)(20,40,80)()(!20,!40,!80)");
  console.log("Expected output :")
  console.log("(40)(50)(!50)(!40)(80)(20)(!80)(!20)(20,40,80)(!20,!40,!80)")

  const renderer1 = new MidifilePerformer.Renderer();

  setIncompleteCoherentPartition(renderer1);
  renderer1.finalize();

  play(commands,renderer1);

  console.log("\n");

  // ---------------------------------------------------------------------------

  console.log("Test robustness against incoherent partition :")
  console.log("(!55)(!56,!57)(60)(61)(!61,!60)(62,63)(64)(!64,!63)(!62)(60)");
  console.log("(orphan endings and a note starting twice without ending)");
  //console.log("Expected output : ")
  // (60)(61)()(!61,!60)(62,63)(64)() what even ???
  // x y !y !x y z !y !z z y !y x !x !z

  const renderer2 = new MidifilePerformer.Renderer();

  setIncoherentPartition(renderer2);
  renderer2.finalize();

  play(commands,renderer2)

  console.log("\n");

  // ---------------------------------------------------------------------------

  console.log("Test over");

}
