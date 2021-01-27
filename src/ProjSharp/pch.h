// pch.h: This is a precompiled header file.
// Files listed below are compiled only once, improving build performance for future builds.
// This also affects IntelliSense performance, including code completion and many code browsing features.
// However, files listed here are ALL re-compiled if any one of them is updated between builds.
// Do not add files here that you will be updating frequently as this negates the performance advantage.

#ifndef PCH_H
#define PCH_H

#include <vcclr.h>

// add headers that you want to pre-compile here

#pragma comment(lib, "jpeg.lib")
#pragma comment(lib, "lzma.lib")
#pragma comment(lib, "proj.lib")
#pragma comment(lib, "sqlite3.lib")
#pragma comment(lib, "tiff.lib")
#pragma comment(lib, "tiffxx.lib")
#pragma comment(lib, "turbojpeg.lib")
#pragma comment(lib, "zlib.lib")


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

#endif //PCH_H
