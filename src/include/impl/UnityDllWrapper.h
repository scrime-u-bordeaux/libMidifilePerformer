#ifndef UNITY_DLL_WRAPPER_H
#define UNITY_DLL_WRAPPER_H

#ifdef NATIVECPPLIBRARY_EXPORTS
#define NATIVECPPLIBRARY_API __declspec(dllexport)
#else
#define NATIVECPPLIBRARY_API __declspec(dllimport)
#endif

#define ADDCALL __cdecl

extern "C" NATIVECPPLIBRARY_API int pushMPTKEvent();//(bool pressed, int pitch, int channel, int velocity);

#endif
