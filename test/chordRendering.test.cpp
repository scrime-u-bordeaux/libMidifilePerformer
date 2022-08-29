#include <iostream>
#include <catch2/catch_test_macros.hpp>
#include "SequencePerformer.h"
#include "./utilities.h"

SCENARIO("mapping the relative note velocities of a chord") {

  // SOME VARIABLES USED ACROSS TESTS //////////////////////////////////////////

  SequencePerformer performer;

  // UTILITIES /////////////////////////////////////////////////////////////////

  auto makeChord = [](
    std::vector<std::uint8_t> velocities
  ) -> std::vector<noteEvent> {
    std::vector<noteEvent> chord;
    for (auto i = 0; i < velocities.size(); ++i) {
      chord.push_back({ 0, makeNote(true, 60 + i, velocities[i]) });
    }
    chord.push_back({ 10, makeNote(false, 60) });
    for (auto i = 1; i < velocities.size(); ++i) {
      chord.push_back({ 0, makeNote(false, 60 + i) });
    }
    return chord;
  };

  auto makeChordCommand = [](
    std::uint8_t velocity
  ) -> std::vector<commandData> {
    return {
      makeCommand(true, 60, velocity),
      makeCommand(false, 60)
    };
  };

  auto getFirstScoreEventsFromVelocities = [](
    SequencePerformer& performer,
    const std::vector<noteEvent>& score,
    const std::vector<std::uint8_t>& velocities
  ) -> std::vector<noteData> {
      std::vector<noteData> allRes;
      for (auto velocity : velocities) {
        feedPerformer(performer, score);
        auto res = getPerformanceResults(
          performer,
          { makeCommand(true, 60, velocity) }
        );
        allRes.insert(allRes.end(), res[0].begin(), res[0].end());
      }
      return allRes;
  };

  // PERFORMING TESTS //////////////////////////////////////////////////////////

  GIVEN("the SameForAll strategy") {
    performer.setChordRenderingStrategy(
      ChordRendering::StrategyType::SameForAll
    );

    WHEN("we play a chord containing random velocities") {
      std::srand(std::time(0));

      feedPerformer(performer, makeChord({
        static_cast<std::uint8_t>(std::rand() * 126 / RAND_MAX + 1),
        static_cast<std::uint8_t>(std::rand() * 126 / RAND_MAX + 1),
        static_cast<std::uint8_t>(std::rand() * 126 / RAND_MAX + 1)
      }));

      THEN("the velocity of the command is applied to all notes") {
        const std::uint8_t cmd_velocity = 120;

        auto res = getPerformanceResults(
          performer,
          makeChordCommand(cmd_velocity)
        )[0];

        bool different = false;
        for (auto& note : res) {
          if (note.velocity != cmd_velocity) {
            different = true;
          }
        }

        REQUIRE(!different);
      }
    }
  }

  GIVEN("the ClippedScaledFromMean strategy") {
    performer.setChordRenderingStrategy(
      ChordRendering::StrategyType::ClippedScaledFromMean
    );

    WHEN("we play a chord including notes with velocities 1 and 127") {
      auto chord = makeChord({ 1, 64, 127 });

      THEN("the resulting velocities are always between 1 and 127") {
        bool outOfBounds = false;
        auto allRes = getFirstScoreEventsFromVelocities(
          performer,
          chord,
          { 1, 64, 127 }
        );

        for (const auto& nd : allRes) {
          if (nd.velocity < 1 || nd.velocity > 127) {
            std::cout << (int) nd.velocity << std::endl;
            outOfBounds = true;
          }
        }

        REQUIRE(!outOfBounds);
      }
    }
  }

  GIVEN("using the AdjustedScaledFromMean strategy") {
    performer.setChordRenderingStrategy(
      ChordRendering::StrategyType::AdjustedScaledFromMean
    );
    // TODO
  }
  
  GIVEN("using the ClippedScaledFromMax strategy") {
    performer.setChordRenderingStrategy(
      ChordRendering::StrategyType::ClippedScaledFromMax
    );
    WHEN("we play a chord including note velocities 1 to 127") {
      auto chord = makeChord({ 1, 64, 127 });

      THEN("the resulting velocities are always between 1 and 127") {
        bool outOfBounds = false;
        auto allRes = getFirstScoreEventsFromVelocities(
          performer,
          chord,
          { 1, 64, 127 }
        );

        for (const auto& nd : allRes) {
          if (nd.velocity < 1 || nd.velocity > 127) {
            outOfBounds = true;
          }
        }

        REQUIRE(!outOfBounds);
      }
    }
  }
}

