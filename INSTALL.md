Installing BasKet Note Pads
============================

To install BasKet Note Pads, just run the installer executable in a terminal and follow the on screen instructions.

Dependencies
------------

To build BasKet, the following development packages are required:

- Qt v5
- kdelibs v5
- qimageblitz
- kdepimlibs
- GnuPG 1.x (optional)

If you get an error message about `FindGPGMe.cmake not found` you are probably missing kdepimlibs.
In addition, handbook generation requires DocBook stylesheets, which are typically found in docbook-xsl package.

On Debian or Ubuntu you can install the required building dependencies with these packages:

    sudo apt install build-essential cmake extra-cmake-modules kdelibs5-dev kdepimlibs5-dev libqimageblitz-dev libgit2-dev libgpgme11-dev libkf5crash-dev libkf5kcmutils-dev libkf5filemetadata-dev libkf5style-dev libphonon4qt5-dev libphonon4qt5experimental4

Building on Linux from source
-------------------------------
To build and install BasKet system-wide, follow below steps:

    mkdir build
    cd build
    cmake .. -DCMAKE_INSTALL_PREFIX=$(kf5-config --prefix) -DQT_PLUGIN_INSTALL_DIR=$(qtpaths --plugin-dir)
    make

Then you can install it with:

    sudo make install

Or you can try your luck with the installer script:
    
    ./installer

    
Building on Windows from source
-------------------------------
- install cmake (http://cmake.org), automoc (git://anongit.kde.org/automoc.git)
- download and run KDE for Windows installer (http://download.kde.org/stable/kdewin/installer/kdewin-installer-gui-latest.exe)
  .. choose Install Mode: Package Manager, Compiler Mode: (for example) MSVC 2010 32bit
  .. install essential packages as described on http://techbase.kde.org/Projects/KDE_on_Windows/Compiling_Applications
  .. besides, install oxygen-icons-* package
- use CMake to configure and build Basket
