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
    vcpkg install proj4[core,tiff]:x86-windows-static-md proj4[core,tiff]:x64-windows-static-md tiff[core]:x86-windows-static-md tiff[core]:x64-windows-static-md
    cd ..
    
The explicit feature selection here explicitly builds PROJ without the builtin network support to remove the curl dependency. It also disables 'jpeg' support in tiff.
This shrinks the library and allows configuring http(s) the .NET way. The network requests are just forwared to the .Net WebClient class using a bit
of code in SharpProj (see `ProjNetworkHandler.cpp`).

This script handles the assumption inside SharpProj that the library and header files required can be found in ../vcpkg/installed/<triplet>.
If you choose a different layout you will need a custom setup later on. But with just this you are now able to build using either Visual Studio
2019 or 2022.

### Custom setup details

`SharpProj.dll` is a shared library (.dll) built using C++/CLI. The triplets `x86-windows-static-md` and `x64-windows-static-md` ensure that the
required third party libraries are linked **statically** with `SharpProj.dll`, while the **md** part of the triplet tells `SharpProj.dll` to use
the **M**ultithread-specific and **D**LL-specific version of the Windows run-time library. This combination is a requirement for C++/CLI
code, and produces DLLs that only depend on the Windows+VC runtime.

The following line installs proj4 and all its required dependencies in the `installed` subdir of the vcpkg checkout.
```
vcpkg install proj4[core,tiff]:x86-windows-static-md proj4[core,tiff]:x64-windows-static-md --overlay-ports=../SharpProj/vcpkg_overlays
```

Lastly, make all installed packages available user-wide for use with `CMake`. This requires admin privileges on first use:

```
vcpkg integrate install
```

Because `SharpProj` depends on aforementioned `vcpkg` packages, you need to let `Visual Studio` know where these packages reside. This is done in `SharpProj -> Properties -> Configuration Properties -> vcpkg -> Installed Directory` . Please point `Installed Directory` to the `vcpkg` folder on your PC. Do this for `All Configurations` and for `All Platforms`.

Now you are ready to build the `SharpProj.dll`. As this DLL has been built using C++/CLI it is (unlike C#) not platform neutral, and the correct (32/64-bit) version needs to be linked against, depending on the end-user's platform.

The linker will most likely throw up a number of `LNK4248` errors. These can be ignored.

Success....

