#include <filesystem>
#include <libremidi/reader.hpp>
#include <catch2/catch_test_macros.hpp>
#include "SequencePerformer.h"
#include "./utilities.h"

TEST_CASE("test libremidi integration") {
  libremidi::reader r;
  REQUIRE(true);
}