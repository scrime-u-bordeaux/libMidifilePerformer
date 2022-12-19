#ifndef UNITY_DLL_WRAPPER_H
#define UNITY_DLL_WRAPPER_H

#ifdef NATIVECPPLIBRARY_EXPORTS
#define NATIVECPPLIBRARY_API __declspec(dllexport)
#else
#define NATIVECPPLIBRARY_API __declspec(dllimport)
#endif

#define ADDCALL __cdecl

#define DEFAULT_CHANNEL 0
#define DEFAULT_VELOCITY 64

#include <stdint.h>

extern "C" NATIVECPPLIBRARY_API void pushMPTKEvent(long tick, bool pressed, int pitch, int channel, int velocity);

extern "C" NATIVECPPLIBRARY_API void clearPerformer();

extern "C" NATIVECPPLIBRARY_API void finalizePerformer();

extern "C" NATIVECPPLIBRARY_API void renderCommand(bool pressed, int ID, uint64_t* dataContainer);

#endif
