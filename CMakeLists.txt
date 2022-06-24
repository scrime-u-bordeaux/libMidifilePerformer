cmake_minimum_required(VERSION 3.12)
project(MidifilePerformer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(EMSCRIPTEN)
  add_executable(${PROJECT_NAME}
    ${PROJECT_SOURCE_DIR}/src/Embindings.cpp
  )
  
  # see all options here :
  # https://github.com/emscripten-core/emscripten/blob/main/src/settings.js

  set(LINK_FLAGS "-lembind")

  if(BROWSER_VERSION)
    # set(CMAKE_EXECUTABLE_SUFFIX ".html")
    # set(LINK_FLAGS "${LINK_FLAGS} -sENVIRONMENT=web")
    # set(LINK_FLAGS "${LINK_FLAGS} -sENVIRONMENT=web -sMODULARIZE=1")
    set(LINK_FLAGS "${LINK_FLAGS} -sENVIRONMENT=web -sMODULARIZE=1 -sEXPORT_ES6=1")
  endif()

  set_target_properties(${PROJECT_NAME}
    PROPERTIES LINK_FLAGS ${LINK_FLAGS}
  )
endif()