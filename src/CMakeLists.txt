set(CMAKE_STATIC_LIBRARY_PREFIX "")

add_library(libMidifilePerformer SHARED
  cpp/impl/ChordVelocityMapping.cpp
  cpp/impl/UnityDllWrapper.cpp
)

set_target_properties(libMidifilePerformer
  PROPERTIES OUTPUT_NAME libMidifilePerformer
)

# set_target_properties(libMidifilePerformer
#   PROPERTIES CXX_EXTENSIONS OFF
# )

# target_compile_features(libMidifilePerformer
#   PUBLIC cxx_std_20
# )

# target_compile_options(libMidifilePerformer
#   PRIVATE ${SANITIZER_FLAGS} ${DEFAULT_COMPILER_OPTIONS_AND_WARNINGS}
# )

target_compile_options(libMidifilePerformer
    PRIVATE -DNATIVECPPLIBRARY_EXPORTS=1 -static-libgcc -static-libstdc++
)

target_link_libraries(libMidifilePerformer
  # PRIVATE ${SANITIZER_FLAGS}
)

target_include_directories(libMidifilePerformer
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/core
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/impl
)
