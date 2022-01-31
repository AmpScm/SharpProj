# SharpProj - Providing [OSGEO](https://www.osgeo.org/projects/proj/) [PROJ](https://proj.org/) for .Net (Core)

[![latest version](https://img.shields.io/nuget/v/SharpProj)](https://www.nuget.org/packages/SharpProj)

Use the real OSGeo PROJ implementation, implemented the .Net way, reusable and available as simple to use package on NuGet.

https://www.nuget.org/packages/SharpProj/

## Usage

```csharp
using SharpProj;

using (var pc = new ProjContext())
{
   using (var rd = CoordinateReferenceSystem.CreateFromEpsg(28992, pc))
   using (var wgs84 = CoordinateReferenceSystem.CreateFromEpsg(4326, pc))
   {
       var area = rd.UsageArea;
       Assert.AreEqual("Netherlands - onshore, including Waddenzee, Dutch Wadden Islands and 12-mile offshore coastal zone.", area.Name);

       using (var t = CoordinateTransform.Create(rd, wgs84))
       {
          var r = t.Apply(new PPoint(155000, 463000));
          Assert.AreEqual(new PPoint(52.155, 5.387), r.ToXY(3)); // Round to 3 decimals for easy testing
       }
   }
}
```

## Building SharpProj
Setup build environment using:

    mkdir dev
    cd dev
    git clone https://github.com/Microsoft/vcpkg.git
    git clone https://github.com/AmpScm/SharpProj.git
    cd vcpkg
    bootstrap-vcpkg.bat
    vcpkg install tiff[core]:x86-windows-static-md
    vcpkg install tiff[core]:x64-windows-static-md
    vcpkg install proj4[core,tiff]:x86-windows-static-md
    vcpkg install proj4[core,tiff]:x64-windows-static-md
    cd ..

Lastly, make all installed packages available user-wide for use with `CMake`. This requires admin privileges on first use:

```
vcpkg integrate install
```

The explicit feature selection `[core,tiff]` builds PROJ without the built-in network support to remove the curl dependency and allows configuring http(s) the .NET way. The network requests are forwarded to the .Net WebClient class using code from SharpProj (see `ProjNetworkHandler.cpp`). 

The explicit feature selection `tiff[core]` disables 'jpeg' support in tiff which shrinks the library. 

Note that the script above assumes that the proj-library and header files required can be found in ../vcpkg/installed/<triplet>.
If you choose a different layout you will need a custom setup as described further on. But with just this you are now able to build the project using either Visual Studio 2019 or 2022.

### Custom setup details

`SharpProj.dll` is a shared library (.dll) built using C++/CLI. The triplets `x86-windows-static-md` and `x64-windows-static-md` ensure that the required third party libraries are linked **statically** with `SharpProj.dll`, while the **md** part of the triplet tells `SharpProj.dll` to use the **M**ultithread-specific and **D**LL-specific version of the Windows run-time library. This combination is a requirement for C++/CLI code, and produces DLLs that only depend on the Windows+VC runtime.

Because `SharpProj` depends on aforementioned `vcpkg` packages, you need to let `Visual Studio` know where these packages reside. This is done in `SharpProj -> Properties -> Configuration Properties -> vcpkg -> Installed Directory` . Please point `Installed Directory` to the `vcpkg` folder on your PC. Ensure this is valid for `All Configurations` and for `All Platforms`.

Now you are ready to build the `SharpProj.dll`. As this DLL has been built using C++/CLI it is (unlike C#) not platform neutral, and the correct (32/64-bit) version needs to be linked against, depending on the end-user's platform.

At the time of writing this readme file, the proj4 library has version 8.2.1#1. If your build-steps result in an earlier version, please run `git pull` from the command line in the `vcpkg` directory.

Success....

