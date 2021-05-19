# SharpProj - Wrapping the real PROJ (previously known as PROJ4) for .Net

[![latest version](https://img.shields.io/nuget/v/SharpProj)](https://www.nuget.org/packages/SharpProj)


Use the real OSGeo PROJ implementation, implemented the .Net way, reusable and available as simple to use package on NuGet

https://www.nuget.org/packages/SharpProj/


Setup build environment using:

    mkdir dev
    cd dev
    git clone https://github.com/Microsoft/vcpkg.git
    git clone https://github.com/AmpScm/SharpProj.git
    cd vcpkg
    bootstrap-vcpkg.bat
    vcpkg install proj4:x86-windows-static-md proj4:x64-windows-static-md --overlay-ports=../SharpProj/vcpkg_ports
    cd ..
    

The '--overlay-ports' argument makes sure that Proj4 is build in a compatible way, and the (current) last
version of PROJ is used. I'm trying to get these fixes backported in plain vcpkg.
