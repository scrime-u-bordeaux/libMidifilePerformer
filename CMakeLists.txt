cmake_minimum_required(VERSION 3.12)
project(MidifilePerformer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(${PROJECT_NAME}
  ${PROJECT_SOURCE_DIR}/src/Embindings.cpp
  ${PROJECT_SOURCE_DIR}/src/cpp/impl/ChordVelocityMapping.cpp
)

set_target_properties(${PROJECT_NAME}
  PROPERTIES LINK_FLAGS "--bind"
)

target_compile_options(
    ${PROJECT_NAME} PUBLIC -sNO_DISABLE_EXCEPTION_CATCHING=1
)

target_link_libraries(
    ${PROJECT_NAME} PUBLIC -sNO_DISABLE_EXCEPTION_CATCHING=1
)

include(CTest)
add_test(NAME jstest COMMAND node ${PROJECT_SOURCE_DIR}/test/mfp_tests.js)
