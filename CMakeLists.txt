cmake_minimum_required(VERSION 3.12)
project(MidifilePerformer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(${PROJECT_NAME}
  ${PROJECT_SOURCE_DIR}/src/Embindings.cpp
)

set_target_properties(${PROJECT_NAME}
  PROPERTIES LINK_FLAGS "--bind"
)

include(CTest)
add_test(NAME jstest COMMAND node ${PROJECT_SOURCE_DIR}/test/mfp_tests.js)
