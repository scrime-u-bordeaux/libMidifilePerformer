// const fs = require('fs');
// const { serialize } = require('v8');
const testArgs = process.argv.slice(2);
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

function setMinimalTestPartition(renderer){
    renderer.pushEvent(1, note(true, 60));
    renderer.pushEvent(1, note(true, 62));
    renderer.pushEvent(1, note(false, 60));
    renderer.pushEvent(1, note(false, 62));
}

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

function setMaxDisplacementPartition(renderer){
    renderer.pushEvent(1,note(true,40));
    renderer.pushEvent(1,note(true,50));
    renderer.pushEvent(1,note(true,60));
    renderer.pushEvent(1,note(true,70));
    renderer.pushEvent(1,note(true,80));
    renderer.pushEvent(1,note(true,90));
    renderer.pushEvent(1,note(true,100));
    renderer.pushEvent(1,note(false,100));
    renderer.pushEvent(1,note(false,90));
    renderer.pushEvent(1,note(false,80));
    renderer.pushEvent(1,note(false,70));
    renderer.pushEvent(1,note(false,60));
    renderer.pushEvent(1,note(false,50));
    renderer.pushEvent(1,note(false,40));
}

function setDesyncPartition(renderer,dt){
    renderer.pushEvent(1,note(true,20));
    renderer.pushEvent(0,note(true,40));
    renderer.pushEvent(0,note(true,80));

    renderer.pushEvent(1,note(false,20));
    renderer.pushEvent(dt,note(false,40));
    renderer.pushEvent(dt,note(false,80));
}

function play(commands,renderer){
    let i = 0;

    console.log("Output : ")

    while (renderer.hasEvents(true) && i < commands.length) {
    // while (renderer.hasEvents()) {

      //console.log("debug : ",commands[i]);
      const events = renderer.combine3(commands[i++], true);

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
// ---------------------------------TESTS---------------------------------------
// -----------------------------------------------------------------------------

function minimalTest(){
    const minCommands = [
        command(true, 60),
        command(false, 60),
        command(true, 62),
        command(false, 62)
    ];

    console.log("Testing for minimal compliance to combine3 logic with partition :");
    console.log("(60)(62)(!60)(!62)");
    console.log("Expected output :");
    console.log("(60)()(62)(!60,!62)");

    const renderer = new MidifilePerformer.Renderer();

    setMinimalTestPartition(renderer);
    renderer.finalize();

    play(minCommands,renderer);

    console.log("\n");
}

// x y !y !x y z !y !z z y !y x !x !z

function displacedEndingTest(){
    console.log("Extended test for incomplete events handling in partition : ");
    console.log("(40)(50)(60)(70)(80)(90)(100)(!100)(!90)(!80)(!70)(!60)(50)(!40)");
    console.log("Expected output : ");
    console.log("(40)(50)(!50)(!40)(60)(70)(!60)(!70)(80)(90)(!90)(100)(!100)(!80)")

    const renderer = new MidifilePerformer.Renderer();

    setMaxDisplacementPartition(renderer);
    renderer.finalize();

    play(commands,renderer);

    console.log("\n");
}

function fullPartitionTest(){
    console.log("Test performance of the given commands with FULL partition :");
    console.log("(40)(!40,50)(!50)(80)(20,!80)(!20)(20,40,80)(!20,!40,!80)(70)(75)(!75)(!70)");
    console.log("Expected output :")
    console.log("(40)(50)(!50)(!40)(80)(20)(!80)(!20)(20,40,80)(70)(!70)(75)(!75)(!20,!40,!80)")

    const renderer = new MidifilePerformer.Renderer();

    setFullCoherentPartition(renderer);
    renderer.finalize();

    play(commands,renderer);

    console.log("\n");
}

function incompletePartitionTest(){
    console.log("Test performance of the given commands with incomplete partition :");
    console.log("(40)(!40,50)(!50)(80)(20,!80)(!20)(20,40,80)(!20,!40,!80)");
    console.log("Expected output :")
    console.log("(40)(50)(!50)(!40)(80)(20)(!80)(!20)(20,40,80)()()()()(!20,!40,!80) (useless presses and releases)")

    const renderer = new MidifilePerformer.Renderer();

    setIncompleteCoherentPartition(renderer);
    renderer.finalize();

    play(commands,renderer);

    console.log("\n");
}

function incoherentPartitionTest(){
    console.log("Test robustness against incoherent partition :")
    console.log("(!55)(!56,!57)(60)(61)(!61,!60)(62,63)(64)(!64,!63)(!62)(60)");
    console.log("(orphan endings and a note starting twice without ending)");
    //console.log("Expected output : ")
    // (60)(61)()(!61,!60)(62,63)(64)() what even ???
    // x y !y !x y z !y !z z y !y x !x !z

    const renderer = new MidifilePerformer.Renderer();

    setIncoherentPartition(renderer);
    renderer.finalize();

    play(commands,renderer)

    console.log("\n");
}

function syncTest(){
    console.log("Test synchronization of chord release");
    console.log("(20,40,80)(!20)(!40)(!80)");
    console.log("Expected output : ");
    console.log("(20,40,80)()()(!20,!40,!80)()()()()()()()()()()");

    const renderer = new MidifilePerformer.Renderer();

    setDesyncPartition(renderer,0);
    renderer.finalize();

    play(commands,renderer);

    console.log("\n");
}

function desyncTest(){
    console.log("Test desynchronization of chord release");
    console.log("(20,40,80)(!20)(!40)(!80)");
    console.log("Expected output : ");
    console.log("(20,40,80)()()(!20,!40,!80)()()()()()()()()()()");
    console.log("(because of merge ; without merge, would be :)");
    console.log("(20,40,80)()(!40)(!20)()()(!80)()()()()()()()");

    const renderer = new MidifilePerformer.Renderer();

    setDesyncPartition(renderer,1);
    renderer.finalize();

    play(commands,renderer);

    console.log("\n");
}

// -----------------------------------------------------------------------------
// ------------------------------INITIALIZE-------------------------------------
// -----------------------------------------------------------------------------

MidifilePerformer.onRuntimeInitialized = () => {

  if(testArgs.includes("all") || testArgs.includes("min")){
      console.log("Test set with commands : x !x y !y");
      console.log("\n");

      minimalTest();
  }


  if(testArgs.length > 0 && !(testArgs.length == 1 && testArgs[0]=="min")){
      console.log("Test set with commands : x y !y !x y z !y !z z y !y x !x !z");
      console.log("\n");
  }

  if(testArgs.includes("all")){
      fullPartitionTest();
      incompletePartitionTest();
      incoherentPartitionTest();
      displacedEndingTest();
      syncTest();
      desyncTest();
  }else{
      if(testArgs.includes("full")) fullPartitionTest();
      if(testArgs.includes("incomplete")) incompletePartitionTest();
      if(testArgs.includes("incoherent")) incoherentPartitionTest();
      if(testArgs.includes("displace")) displacedEndingTest();
      if(testArgs.includes("sync")) syncTest();
      if(testArgs.includes("desync")) desyncTest();
  }

  if(testArgs.length > 0) console.log("Test over");

}
