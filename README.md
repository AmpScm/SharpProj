# SharpProj - Wrapping the reall PROJ (previously known as PROJ4) for .Net

Use the real PROJ,implemented the .Net way, reusable and hopefully as simple NuGet package soon.

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
