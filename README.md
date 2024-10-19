# SharpProj - Providing [OSGEO](https://www.osgeo.org/projects/proj/) [PROJ](https://proj.org/) for .Net (Core)

[![latest version](https://img.shields.io/nuget/v/SharpProj)](https://www.nuget.org/packages/SharpProj)
[![CI](https://github.com/AmpScm/SharpProj/actions/workflows/msbuild.yml/badge.svg)](https://github.com/AmpScm/SharpProj/actions/workflows/msbuild.yml)

Use the real OSGeo PROJ implementation, implemented the .Net way, reusable and available as simple to use package on NuGet.

https://www.nuget.org/packages/SharpProj/

## Usage

```csharp
using SharpProj;

using var rd = CoordinateReferenceSystem.CreateFromEpsg(28992);
using var wgs84 = CoordinateReferenceSystem.CreateFromEpsg(4326);

var area = rd.UsageArea;
Assert.AreEqual("Netherlands - onshore, including Waddenzee, Dutch Wadden Islands and 12-mile offshore coastal zone.", area.Name);

using (var t = CoordinateTransform.Create(rd, wgs84))
{
    var r = t.Apply(new PPoint(155000, 463000));
    Assert.AreEqual(new PPoint(52.155, 5.387), r.ToXY(3)); // Round to 3 decimals for easy testing
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
    vcpkg install proj[core,tiff]:x64-windows-static-md proj[core,tiff]:x86-windows-static-md proj[core,tiff]:arm64-windows-static-md tiff[core,lzma,zip]:x64-windows-static-md tiff[core,lzma,zip]:x86-windows-static-md tiff[core,lzma,zip]:arm64-windows-static-md
    cd ..
    
The explicit feature selection here explicitly builds PROJ without the builtin network support to remove the curl dependency. It also disables 'jpeg' support in tiff.
This shrinks the library and allows configuring http(s) the .NET way. The network requests are just forwared to the .Net WebClient class using a bit
of code in SharpProj (see `ProjNetworkHandler.cpp`).

This script handles the assumption inside SharpProj that the library and header files required can be found in ../vcpkg/installed/<triplet>.
If you choose a different layout you will need a custom setup later on. But with just this you are now able to build using either Visual Studio
2019 or 2022.

### Handling PROJ updates with major proj.db changes
SharpProj needs a working `proj.db` to run its tests. if you are bumping proj to a new version
you may need to run
    $ scripts\gh-version-setup ..\vcpkg\installed\x86-windows-static-md\include\proj.h 1
    $ scripts\gh-build-nuget.cmd --db-only
To create a local newer than official nuget package for the proj data. Then update the test package
to reference it. This should fix the chicken-and-egg problem for bootstrapping without a proper
package in the nuget repository. Usually I will fix this for you shortly after a new majr proj X is
released/

### Error in ALINK 'access denied'
If you see this on your box you can fix this by runnning

$ reg add HKEY_LOCAL_MACHINE\SOFTWARE\WOW6432Node\Microsoft\StrongName /v MachineKeyset /t REG_DWORD /d 0
$ reg add HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\StrongName /v MachineKeyset /t REG_DWORD /d 0
(From: https://stackoverflow.com/questions/4606342/signing-assembly-access-is-denied )