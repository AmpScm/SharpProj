// pch.h: This is a precompiled header file.
// Files listed below are compiled only once, improving build performance for future builds.
// This also affects IntelliSense performance, including code completion and many code browsing features.
// However, files listed here are ALL re-compiled if any one of them is updated between builds.
// Do not add files here that you will be updating frequently as this negates the performance advantage.

#ifndef PCH_H
#define PCH_H

#include <vcclr.h>

// add headers that you want to pre-compile here

#if !defined(_DEBUG) || !defined(USE_PROJ_DEBUG)
#pragma comment(lib, "proj.lib")
#pragma comment(lib, "jpeg.lib")
#pragma comment(lib, "lzma.lib")
#pragma comment(lib, "tiff.lib")
#pragma comment(lib, "zlib.lib")
#else
#pragma comment(lib, "proj_d.lib")
#pragma comment(lib, "jpegd.lib")
#pragma comment(lib, "lzmad.lib")
#pragma comment(lib, "tiffd.lib")
#pragma comment(lib, "zlibd.lib")
#endif
#pragma comment(lib, "sqlite3.lib")

#pragma comment(lib, "Ole32.lib")
#pragma comment(lib, "Shell32.lib")

#include <proj.h>

namespace ProjSharp {
	ref class ProjContext;
}

using namespace System;

#include <string>
std::string utf8_string(String^ v);

using Out = System::Runtime::InteropServices::OutAttribute;
using Optional = System::Runtime::InteropServices::OptionalAttribute;

#endif //PCH_H
