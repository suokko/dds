/*
   DDS, a bridge double dummy solver.

   Copyright (C) 2006-2014 by Bo Haglund /
   2014-2018 by Bo Haglund & Soren Hein.

   See LICENSE and README.
*/

#ifndef DDS_PORTAB_H
#define DDS_PORTAB_H


#if defined(_WIN32) || defined (__CYGWIN__)
  #if defined(__MINGW32__) && !defined(WINVER)
    #define WINVER 0x500
  #endif

  #include <windows.h>
  #include <process.h>

  #if defined (_MSC_VER)
    #include <intrin.h>
  #endif

#elif defined (__linux) || defined (__APPLE__)
  #include <unistd.h>
  /* DLL uses a constructor function for initialization */

  typedef long long __int64;

#endif

#if __cplusplus < 201703L
template<class T, size_t N>
constexpr size_t size(const T (&)[N])
{
  return N;
}
#endif


// http://stackoverflow.com/a/4030983/211160
// Use to indicate a variable is being intentionally not referred to (which
// usually generates a compiler warning)
#ifndef UNUSED
  #define UNUSED(x) ((void)x)
#endif

#endif
