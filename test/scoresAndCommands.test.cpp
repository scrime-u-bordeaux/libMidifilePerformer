#include <iostream>
#include <catch2/catch_test_macros.hpp>
#include "SequencePerformer.h"
#include "./utilities.h"

// VARIOUS SCORES //////////////////////////////////////////////////////////////

const std::vector<noteEvent> minimalScore = {
  { 1, makeNote(true,   60 )},
  { 1, makeNote(true,   62 )},
  { 1, makeNote(false,  60 )},
  { 1, makeNote(false,  62 )},
};

const std::vector<noteEvent> incoherentScore = {
  { 0, makeNote(false,  55) },
  { 1, makeNote(false,  56) },
  { 0, makeNote(false,  57) },
  { 1, makeNote(true,   60) },
  { 1, makeNote(true,   61) },
  { 1, makeNote(false,  61) },
  { 0, makeNote(false,  60) },
  { 1, makeNote(true,   62) },
  { 0, makeNote(true,   63) },
  { 1, makeNote(true,   64) },
  { 1, makeNote(false,  64) },
  { 0, makeNote(false,  63) },
  { 1, makeNote(false,  62) },
  { 1, makeNote(true,   60) }
};

const std::vector<noteEvent> incompleteCoherentScore = {
  { 1, makeNote(true,   40) },

  { 2, makeNote(true,   50) },
  { 0, makeNote(false,  40) },

  { 3, makeNote(false,  50) },

  { 4, makeNote(true,   80) },

  { 5, makeNote(true,   20) },
  { 0, makeNote(false,  80) },

  { 6, makeNote(false,  20) },

  { 7, makeNote(true,   20) },
  { 0, makeNote(true,   40) },
  { 0, makeNote(true,   80) },

  { 9, makeNote(false,  20) },
  { 0, makeNote(false,  40) },
  { 0, makeNote(false,  80) }
};

const std::vector<noteEvent> incompleteCoherentScoreComplement = {
  { 1, makeNote(true,   70) },
  { 1, makeNote(true,   75) },

  { 1, makeNote(false,  75) },

  { 1, makeNote(false,  70) },

};

const std::vector<noteEvent> maxDisplacementScore = {
  { 1, makeNote(true,   40) },
  { 1, makeNote(true,   50) },
  { 1, makeNote(true,   60) },
  { 1, makeNote(true,   70) },
  { 1, makeNote(true,   80) },
  { 1, makeNote(true,   90) },
  { 1, makeNote(true,   100) },
  { 1, makeNote(false,  100) },
  { 1, makeNote(false,  90) },
  { 1, makeNote(false,  80) },
  { 1, makeNote(false,  70) },
  { 1, makeNote(false,  60) },
  { 1, makeNote(false,  50) },
  { 1, makeNote(false,  40) },
};

const std::vector<noteEvent> makeDesyncScore(std::uint8_t dt) {
  return {
    { 1,  makeNote(true,  20) },
    { 0,  makeNote(true,  40) },
    { 0,  makeNote(true,  80) },

    { 1,  makeNote(false, 20) },
    { dt, makeNote(false, 40) },
    { dt, makeNote(false, 80) }
  };
};

// GENERIC COMMANDS ////////////////////////////////////////////////////////////

const std::vector<commandData> genericCommands = {
  makeCommand(true,   62),
  makeCommand(true,   61),
  makeCommand(false,  61),
  makeCommand(false,  62),
  makeCommand(true,   61),
  makeCommand(true,   60),
  makeCommand(false,  61),
  makeCommand(false,  60),

  makeCommand(true,   60),
  makeCommand(true,   61),
  makeCommand(false,  61),
  makeCommand(true,   62),
  makeCommand(false,  62),
  makeCommand(false,  60)
};

SequencePerformer performer;

// TESTS ///////////////////////////////////////////////////////////////////////

TEST_CASE("minimal") {

  const std::vector<commandData> minimalCommands = {
    makeCommand(true,   60),
    makeCommand(false,  60),
    makeCommand(true,   62),
    makeCommand(false,  62)
  };

  std::vector<std::vector<noteData>> expected = {
    { makeNote(true,  60) },
    {},
    { makeNote(true,  62) },
    {
      makeNote(false, 60),
      makeNote(false, 62)
    }
  };

  feedPerformer(performer, minimalScore);
  auto res = getPerformanceResults(performer, minimalCommands);

  REQUIRE(performanceResultsAreIdentical(res, expected));

  /*
  // FINER GRAIN ALTERNATIVE :

  REQUIRE(res.size() == expected.size());

  for (auto i = 0; i < res.size(); ++i) {
    REQUIRE(std::equal(res[i].begin(), res[i].end(), expected[i].begin()));
  }
  //*/
}

TEST_CASE("displaced ending") {
  std::vector<std::vector<noteData>> expected = {
    { makeNote(true,  40) },
    { makeNote(true,  50) },
    { makeNote(false, 50) },
    { makeNote(false, 40) },
    { makeNote(true,  60) },
    { makeNote(true,  70) },
    { makeNote(false, 60) },
    { makeNote(false, 70) },
    { makeNote(true,  80) },
    { makeNote(true,  90) },
    { makeNote(false, 90) },
    { makeNote(true,  100) },
    { makeNote(false, 100) },
    { makeNote(false, 80) },
  };

  feedPerformer(performer, maxDisplacementScore);
  auto res = getPerformanceResults(performer, genericCommands);

  REQUIRE(performanceResultsAreIdentical(res, expected));
  // REQUIRE(true);
}

TEST_CASE("full score") {
  std::vector<std::vector<noteData>> expected = {
    { makeNote(true,  40) },
    { makeNote(true,  50) },
    { makeNote(false, 50) },
    { makeNote(false, 40) },
    { makeNote(true,  80) },
    { makeNote(true,  20) },
    { makeNote(false, 80) },
    { makeNote(false, 20) },
    {
      makeNote(true,  20),
      makeNote(true,  40),
      makeNote(true,  80)
    },
    { makeNote(true,  70) },
    { makeNote(false, 70) },
    { makeNote(true,  75) },
    { makeNote(false, 75) },
    {
      makeNote(false, 20),
      makeNote(false, 40),
      makeNote(false, 80)
    }
  };

  std::vector<noteEvent> fullScore(incompleteCoherentScore);
  
  fullScore.insert(
    fullScore.end(),
    incompleteCoherentScoreComplement.begin(),
    incompleteCoherentScoreComplement.end()
  );

  feedPerformer(performer, fullScore);
  auto res = getPerformanceResults(performer, genericCommands);

  REQUIRE(performanceResultsAreIdentical(res, expected));
  // REQUIRE(true);
}

TEST_CASE("incomplete score") {
  std::vector<std::vector<noteData>> expected = {
    { makeNote(true,  40) },
    { makeNote(true,  50) },
    { makeNote(false, 50) },
    { makeNote(false, 40) },
    { makeNote(true,  80) },
    { makeNote(true,  20) },
    { makeNote(false, 80) },
    { makeNote(false, 20) },
    {
      makeNote(true,  20),
      makeNote(true,  40),
      makeNote(true,  80)
    },
    {},
    {},
    {},
    {},
    {
      makeNote(false, 20),
      makeNote(false, 40),
      makeNote(false, 80)
    }
  };

  feedPerformer(performer, incompleteCoherentScore);
  auto res = getPerformanceResults(performer, genericCommands);

  REQUIRE(performanceResultsAreIdentical(res, expected));
  // REQUIRE(true);
}

TEST_CASE("incoherent score") {
  //(60)(61)()(!61,!60)(62,63)(64)()
  std::vector<std::vector<noteData>> expected = {
    { makeNote(true,  60) },
    { makeNote(true,  61) },
    {},
    {
      makeNote(false, 61),
      makeNote(false, 60),
    },
    {
      makeNote(true,  62),
      makeNote(true,  63)
    },
    { makeNote(true,  64) },
    {}
  };

  feedPerformer(performer, incoherentScore);
  auto res = getPerformanceResults(performer, genericCommands);

  REQUIRE(performanceResultsAreIdentical(res, expected));
  // REQUIRE(true);
}

TEST_CASE("sync") {
  std::vector<std::vector<noteData>> expected = {
    {
      makeNote(true,  20),
      makeNote(true,  40),
      makeNote(true,  80)
    },
    {},
    {},
    {
      makeNote(false, 20),
      makeNote(false, 40),
      makeNote(false, 80)
    },
    {},
    {},
    {},
    {},
    {},
    {},
    {},
    {},
    {},
    {}
  };

  feedPerformer(performer, makeDesyncScore(0));
  auto res = getPerformanceResults(performer, genericCommands);

  REQUIRE(performanceResultsAreIdentical(res, expected));
  // REQUIRE(true);
}

TEST_CASE("desync") {
  std::vector<std::vector<noteData>> expected = {
    // with merge :
    //(20,40,80)()()(!20,!40,!80)()()()()()()()()()()
    /*
    {
      makeNote(true,  20),
      makeNote(true,  40),
      makeNote(true,  80)
    },
    {},
    {},
    {
      makeNote(false, 20),
      makeNote(false, 40),
      makeNote(false, 80)
    },
    {},
    {},
    {},
    {},
    {},
    {},
    {},
    {},
    {},
    {}
    //*/

    // without merge :
    //(20,40,80)()(!40)(!20)()()(!80)()()()()()()()
    //*
    {
      makeNote(true,  20),
      makeNote(true,  40),
      makeNote(true,  80)
    },
    {},
    { makeNote(false, 40) },
    { makeNote(false, 20) },
    {},
    {},
    { makeNote(false, 80) },
    {},
    {},
    {},
    {},
    {},
    {},
    {}
    //*/
  };

  feedPerformer(performer, makeDesyncScore(1));
  auto res = getPerformanceResults(performer, genericCommands);

  REQUIRE(performanceResultsAreIdentical(res, expected));
  // REQUIRE(true);
}
