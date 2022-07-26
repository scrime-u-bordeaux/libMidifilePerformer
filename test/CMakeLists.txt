include(CTest)

include(FetchContent)
FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG devel
)
FetchContent_MakeAvailable(Catch2)

FetchContent_Declare(
    libremidi
    GIT_REPOSITORY https://github.com/jcelerier/libremidi.git
    GIT_TAG master
)
FetchContent_MakeAvailable(libremidi)

############################# Multiple sources #################################
if(BUILD_TESTING)
    add_executable(
        AllTests
        scoresAndCommands.test.cpp
        chordVelocityMapping.test.cpp
        voiceStealing.test.cpp
        invalidMapEntries.test.cpp
        midiFiles.test.cpp
    )

    target_link_libraries(
        AllTests
        PRIVATE libMidifilePerformer Catch2::Catch2WithMain libremidi
    )

    add_test(NAME AllTests COMMAND AllTests)
endif()

############################## Multiple tests ##################################
#[[
if(BUILD_TESTING)
    set(ALL_TESTS
        chordVelocityMappingTest
        voiceStealingTest
        invalidMapEntriesTest
    )

    add_executable(chordVelocityMappingTest chordVelocityMapping.test.cpp)
    add_executable(voiceStealingTest        voiceStealing.test.cpp)
    add_executable(invalidMapEntriesTest    invalidMapEntries.test.cpp)

    foreach(target ${ALL_TESTS})
        # set_target_properties(${target} PROPERTIES CXX_EXTENSIONS OFF)
        # target_compile_features(${target} PUBLIC cxx_std_20)
        # target_compile_options(${target} PRIVATE ${DEFAULT_COMPILER_OPTIONS_AND_WARNINGS})
        target_link_libraries(${target}
            PRIVATE libMidifilePerformer Catch2::Catch2WithMain
        )

        add_test(NAME ${target} COMMAND ${target})
    endforeach()
endif()
]]
