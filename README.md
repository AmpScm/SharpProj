# SharpProj - Wrapping the reall PROJ (previously known as PROJ4) for .Net

Use the real OSGeo PROJ implementation,implemented the .Net way, reusable and available as simple to use package on NuGet

https://www.nuget.org/packages/SharpProj/


Setup build environment using:

    mkdir dev
    cd dev
    git clone https://github.com/Microsoft/vcpkg.git
    cd vcpkg
    bootstrap-vcpkg.bat
    vcpkg install proj4:x86-windows-static-md
    vcpkg install proj4:x64-windows-static-md
    cd ..
    git clone https://github.com/AmpScm/SharpProj.git


There are still some issues in the vcpkg port of proj that break projection grids in static builds. I use a local
workaround while I'm working to get these fixes incorporated in vcpkg.
