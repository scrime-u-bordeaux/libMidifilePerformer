cmake_minimum_required(VERSION 3.12)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_SYSTEM_NAME Windows)
SET(CMAKE_C_COMPILER x86_64-w64-mingw32-gcc)
SET(CMAKE_CXX_COMPILER x86_64-w64-mingw32-g++)
SET(CMAKE_RC_COMPILER x86_64-w64-mingw32-windres)
set(CMAKE_RANLIB x86_64-w64-mingw32-ranlib)

project(MidifilePerformer LANGUAGES CXX)

add_subdirectory(src)
