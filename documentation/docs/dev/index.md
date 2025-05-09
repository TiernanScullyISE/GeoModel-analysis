# Build the GeoModel libraries

Here below, you will find instructions on how to compile all the GeoModel libraries and tools, and their main dependencies, from sources. You may want to do that for different reasons:

- you want to build the libraries for an OS which is not officially supported or for a version of it for which no pre-compiled packages are provided (for info, see the [Start](../start/install.md) section)
- you want to test the latest version of the code
- you want to contribute to the development of the GeoModel libraries and tools

In the following, you will find basic strategies to effectively build the GeoModel software stack, based on different use cases.

!!! warning

    Before building the GeoModel libraries from source, please check if you have old versions of them installed on your system and remove them.
    Also, you should remove all GeoModel packages previously installed through `brew`, because they could be picked up by the build system while building the sources, affecting the compilation.

    Please remove **all** the `geomodel`-packages before trying to build the packages from source.

    See the [Troubleshooting](troubleshooting.md) page for additional details and instructions.


## System Dependencies

### Compilers

In the GeoModel packages, we use **modern C++** constructs (C++ 20). Thus, before trying to compile the GeoModel libraries, be sure your compiler supports **C++20**.

!!! note

    If you use GCC, we successfully tested the build of GeoModel packages with GCC 13.X and 14.X

    If you use Clang, we successfully tested the GeoModel build with Clang 17 and 19


### Third-party libraries - macOS

On macOS, you can install all the needed third-party dependencies by using the [Homebrew](https://brew.sh) package manager:

```bash
# install external dependencies
brew install cmake eigen doxygen wget boost xerces-c nlohmann-json sqlite googletest hdf5

# if you want to build the visualization tools as well, please install and configure Qt6, Coin3D and SoQt
# all of them can be installed by one command
brew install coin3d
```
Then, add the path to your shell, by running the command below, depending on your macOS machine:

* Intel chip:
```
echo 'export PATH="/usr/local/opt/qt/bin:$PATH"' >> ~/.zshrc
```

* Apple M1 (ARM64) chip:
```
echo 'export PATH="/opt/homebrew/opt/qt/bin:$PATH"' >> ~/.zshrc
```

!!! note 

    After having installed `qt` with brew, and having run the `echo` command stated above, you should either open a new shell or run `source ~/.zshrc` in order to get the `qt` in the `PATH`.


### Linux/Ubuntu

On Ubuntu, you can install all the needed third-party dependencies by using the built-in `apt` package manager:

```bash

sudo add-apt-repository ppa:kaktusjoe/geomodel
sudo apt update

sudo apt install git cmake wget unzip build-essential nlohmann-json3-dev libsoqt-bb-dev libxerces-c-dev libeigen3-dev geant4-dev libsqlite3-dev zlib1g-dev libhdf5-dev qt6-base-dev qt6-5compat-dev libhepmc3-dev pythia-dev
```

<!--
### Linux/Fedora

**On Fedora < 34**:
On Fedora, you can install all the needed third-party dependencies by using the built-in `dnf` package manager:

```bash
dnf install --assumeyes make automake gcc gcc-c++ cmake git qt5  boost mercurial xerces-c-devel unzip freeglut-devel wget eigen3-devel
```

**On Fedora >= 34**:

```
dnf install --assumeyes make automake gcc gcc-c++ cmake git qt5-qtbase-devel boost mercurial xerces-c-devel unzip freeglut-devel wget eigen3-devel giflib-devel libjpeg-devel sqlite-devel
```


### Centos7

Packages shipped with Centos7, or installed by the YUM package manager, unfortunately are often too old. So, one needs to circumvent that, either by using pre-compiled bundles, or building from sources.

#### CMake and Qt 

You need a recent version of `CMake` to build `GeoModel`, so you might need to install a newer version compared to what is installed on your Centos7 machine. Please, refer to the original `CMake` documentation for [updated installation instructions](https://cmake.org/).

You also need a recent version of the Qt graphics framework installed. Please refer to the [official documentation](https://www.qt.io/) for updated instructions. When installing, please be aware that **you only need** the "Community" version of Qt (Open Source), not the commercial one.

#### Using the built-in third-party packages 

For the `Xerces-C`, `nlohmann-json`, `Coin3D`, `Simage`, and `SoQt` dependencies, one can use the built-in versions shipeed with `GeoModel`. They are **tested** to work smoothly with the related `GeoModel` version, and they avoid the installation from sources of all these packages.

For example, on Centos7, after having set a recent version of `CMake`, you can build the full `GeoModel` tools-suite, with support for the XML parser and the visualization tools, with the command below:

```bash
cmake -DCMAKE_INSTALL_PREFIX=../install -DGEOMODEL_USE_BUILTIN_EIGEN3=1 -DGEOMODEL_USE_BUILTIN_XERCESC=1 -DGEOMODEL_USE_BUILTIN_JSON=1 -DGEOMODEL_USE_BUILTIN_COIN3D=1 -DGEOMODEL_BUILD_VISUALIZATION=1 -DGEOMODEL_BUILD_TOOLS=1 ../GeoModel/
```

#### Using CVMFS and LCG

If one has CVMFS access, one way to easily set up a recent version of `CMake`, `Qt` and all the other dependency packages, is the use of the `LCG` bundles provided by the CERN "SFT" group:

```bash
source /cvmfs/sft.cern.ch/lcg/views/LCG_100/x86_64-centos7-gcc9-opt/setup.sh
```

After that, you can build the `GeoModel` without any extra options. 
-->


## Visualization 3D graphics dependencies

### 3D graphics dependencies - Linux

On Linux, you can build the needed graphics libraries by following the instructions below.


#### a) Simage

On all platforms *except Centos7*, you can build Simage by followoing these instructions:

```bash
# Build Simage
wget http://cern.ch/atlas-software-dist-eos/externals/Simage/simage-1.8.1-src.zip
unzip simage-1.8.1-src.zip
mkdir build_simage
cd build_simage
cmake -DCMAKE_INSTALL_PREFIX=../install -DSIMAGE_BUILD_DOCUMENTATION=0 -DSIMAGE_BUILD_EXAMPLES=0 -DSIMAGE_LIBSNDFILE_SUPPORT=0 -DSIMAGE_MPEG2ENC_SUPPORT=0 -DSIMAGE_OGGVORBIS_SUPPORT=0 ../simage
make -j4
make install
cd ..
```

*On Centos7*, you have to apply a patch to build the package. Therefore, on Centos7 please build Simage by following the instructions below:

```bash
# Build Simage
wget http://cern.ch/atlas-software-dist-eos/externals/Simage/Coin3D-simage-2c958a61ea8b.zip
unzip Coin3D-simage-2c958a61ea8b.zip
cd Coin3D-simage-2c958a61ea8b
./configure --prefix=$PWD/../install
wget -O cc7.patch https://gitlab.cern.ch/atlas/atlasexternals/-/raw/main/External/Simage/patches/libpng_centos7.patch?inline=false 
patch -p1 < cc7.patch
make -j4
make install 
cd ..
```


#### b) Coin3D & SoQt

Now, you should build Coin3D (the 3D graphics engine) and SoQt (the glue package between the 3D graphics engine, Coin, and the windowing system, Qt6):


```bash
# Build Coin3D
wget https://geomodel.web.cern.ch/sources/coin-5a97506-20210210.zip -O coin.zip
unzip coin.zip; mv coin-* coin
mkdir build_coin
cd build_coin
cmake -DCMAKE_INSTALL_PREFIX=../install -DCMAKE_BUILD_TYPE=Release -DCOIN_BUILD_TESTS=0 -DCMAKE_CXX_FLAGS=-Wno-deprecated-declarations ../coin
make -j4
make install
cd ..

# Build SoQt
wget https://geomodel.web.cern.ch/sources/soqt_6b1c74f_20210210.zip -O soqt.zip 
unzip soqt.zip
mkdir build_soqt
cd build_soqt
cmake -DCMAKE_INSTALL_PREFIX=../install -DCMAKE_BUILD_TYPE=Release -DSOQT_BUILD_DOCUMENTATION=0 -DSOQT_BUILD_MAC_X11=0 -DSOQT_BUILD_TESTS=0 -DCMAKE_CXX_FLAGS=-Wno-deprecated-declarations ../soqt
make -j4
make install
cd ..
```


## Quick instructions - Build everything

With these instructions you will build the whole software stack for GeoModel development. The GeoModel libraries will be built from the HEAD version of the 'main' branch. If something does not compile, please [let the developers' team know](../about/contacts.md). 

With these instructions, you will build: `GeoModelCore`, `GeoModelIO`, `GeoModelTools`, `GeoModelVisualization` (a.k.a., `gmex`).


!!! note

    These instructions will install the libraries and the tools in a **local** `install` folder. That is very useful for developmemnt, because you can handle multiple versions installed on the same system.

    However, if you prefer to install the tools and the libraries in the `/usrl/local` system directory, just remove from the commands the option `-DCMAKE_INSTALL_PREFIX=../install`.



This will install everything: the base libraries contained in the packages `GeoModelCore` (the kernel libraries and utilities) and `GeoModelIO` (the I/O libraries and the interface to the underlying data formats and/or databases), the tools contained in `GeoModelTools` (XML and JSON parsers, the `GeoModelXML` interface, the `gmcat` concatenation command line tool), and the viusalization tools contained in `GeoModelVisualization` (`gmex`):


```
git clone https://gitlab.cern.ch/GeoModelDev/GeoModel.git
mkdir build_geomodel
cd build_geomodel
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=../install -DGEOMODEL_BUILD_TOOLS=1 -DGEOMODEL_BUILD_VISUALIZATION=1 ../GeoModel
make -j
make install
cd ..
```

!!! note

    On some platforms (for example, on Centos 7 and some Ubuntu releases), the default `Eigen`, `Xerces-C`, and `nlohmann-json` libraries installed through the platform's package manager are quite old. If you desire, you can build GeoModel with a built-in, tested version of those libraries by enabling the corresponding options at configuration time with these additional build options:

    ```
    -DGEOMODEL_USE_BUILTIN_EIGEN3=1 -DGEOMODEL_USE_BUILTIN_XERCESC=1 -DGEOMODEL_USE_BUILTIN_JSON=1
    ```

    If you compile on Centos7, that above could be a very convenient option, which lets you skip the installation of custom versions of those libraries directly on your system. 

    A full example on Centos 7 could be:

    ```
    cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=../install -DGEOMODEL_BUILD_TOOLS=1 -DGEOMODEL_BUILD_VISUALIZATION=1 -DGEOMODEL_USE_BUILTIN_EIGEN3=1 -DGEOMODEL_USE_BUILTIN_XERCESC=1 -DGEOMODEL_USE_BUILTIN_JSON=1  ../GeoModel
    ```

!!! note

    In these instructions we are building everything, because that is the most used configuration. But the GeoModel build is highly modular, and you can install different combinations of the GeoModel subpackages. 

    You can see all the available options, and you can enable/disable them interactively, by using the command for the interactive CMake configuration `ccmake`, instead of the classical `cmake`; *i.e.*: `ccmake ../GeoModel`.

    In the section below, a list of all build options will be presented.




## Post install settings

Assuming you followed the above instructions and you used `../install` as the installation path, in order to run the applications you need to set a couple of system variables.

!!! note

    You don't need to set the variables below, or run any system tool, if you installed GeoModel binaries by following the instructions on the [Install](../start/install.md) page.

    Also, yuo don't need them if you have omitted the local installation option (*i.e.*, the `-DCMAKE_INSTALL_PREFIX=../install` build option) while building and, therefore, you have installed everything i to system folders (like, for example, into `/usr/local/`).


### macOS

```bash
install_name_tool -add_rpath ../install/lib ../install/bin/gmex  
```

**Note:** You need to run the command above each time you install a new `gmex` executable while developing; *i.e.*, every time you run `make install`, if you are developing `GeoModelVisualization`. That is needed because a new `gmex` executable does not have the `rpath` set, and you must set it before running it, otherwise the executable cannot find the libraries it is linked against.

This will be automated in a future version.


### Linux/Ubuntu

```bash
export LD_LIBRARY_PATH=${PWD}/../install/lib/ # this is a temporary fix
```

### Linux / Fedora & Centos7

```bash
export LD_LIBRARY_PATH=../install/lib:../install/lib64:$LD_LIBRARY_PATH # this is a temporary fix, we will fix the installation on Fedora
```



## Run GeoModelExplorer (gmex)


Then, you can download an example geometry data file:

{!start/get-geometry-example-file.md!}

and run your local copy of the GeoModel visualization tool, `gmex`, with (assuming you are still in the build folder):

```bash
../install/bin/gmex
```

----

## Troubleshooting

For a collection of suggestions on how to fix potential errors and glitches, please refer to the dedicated [Troubleshooting](troubleshooting.md) page.


----

## Build Options


Whend configuring the build of GeoModel, you can switch on and off the building of optional components, as explained in the following subsections.


!!! note

    Please note that you can build some of the external dependencies (e.g., the JSON parser, the `Coin3D` and `SoQt` libraries) as part of the GeoModel building process. This is useful for platforms where the versions of externals installed by the system package managers are old. For more details about building externals see [the instructions](#building-dependencies-as-part-of-the-main-build) below.


!!! info

    The configuration variables listed below can be passed to `cmake` via the command-line interface. Multiple variables can be passed to `cmake` in one go. When doing that all variables must be prefixed by `-D`.

    For example, this CMake command:

    ```
    cmake -DGEOMODEL_BUILD_VISUALIZATION=1 -DGEOMODEL_BUILD_TESTING=1 -DGEOMODEL_BUILD_EXAMPLES=1 ../GeoModel
    ```

    configures building of the core library, the visualization tool `gmex`, all unit tests, and those examples that do not depend on `Geant4`.



#### GEOMODEL_BUILD_TOOLS -- Build the GeoModelTools


```
cmake -DGEOMODEL_BUILD_TOOLS=1 ../GeoModel
```

will enable the build of the tools in the `GeoModelTools` directory. 

This option also brings in the dependencies on `Xerces-C` and `nlohmann_json` third-party libraries.



#### GEOMODEL_BUILD_VISUALIZATION -- Build the visualization tools

```
cmake -DGEOMODEL_BUILD_VISUALIZATION=1 ../GeoModel
```

will enable the build of the geometry visualization tool, `GeoModelExplorer` (aka `gmex`).

This option brings in the dependencies on either Qt6 (default) or Qt5 (activated explicitly, see below for details), Coin3D, SoQt, and nlohmann_json. 

#### GEOMODEL_USE_QT5 -- Build GeoModel visualization against Qt5

```
cmake -DGEOMODEL_USE_QT5=1 -DGEOMODEL_BUILD_VISUALIZATION=1 ../GeoModel
```

when combined with `GEOMODEL_BUILD_VISUALIZATION` configures the building of `GeoModel` visualization against `Qt5`.

#### GEOMODEL_BUILD_EXAMPLES -- Build the examples


```
cmake -DGEOMODEL_BUILD_EXAMPLES=1 ../GeoModel
```

will enable the build of all the examples except those requiring Geant4, see below.

#### GEOMODEL_BUILD_TESTING -- Build unit tests

```
cmake -DGEOMODEL_BUILD_TESTING=1 ../GeoModel
```

will enable the build of all GeoModel unit tests.

This option brings in the dependency on `GoogleTest`.


#### GEOMODEL_BUILD_GEOMODELG4 -- Build the GeoModel --> Geant4 interface

```
cmake  -DGEOMODEL_BUILD_GEOMODELG4=1 ../GeoModel
```

will enable the build of `GeoModelG4` - a tool for translating transient GeoModel geometry tree into Geant4 geometry.

This option brings in the dependency  on `Geant4`.

#### Build the examples for the GeoModel --> Geant4 interface

```
cmake  -DGEOMODEL_BUILD_EXAMPLES_W_GEANT4=1 ../GeoModel
```

will enable the build of all GeoModel examples including those depending on `Geant4` as well as the `GeoModelG4` translator.

This option brings in the dependency  on `Geant4`.

#### GEOMODEL_BUILD_FULLSIMLIGHT -- Build the standalone Geant4-based detector simulation application

```
cmake  -DGEOMODEL_BUILD_FULLSIMLIGHT=1 ../GeoModel
```

will enable the build of `FullSimLight` - the Geant4-based standalone simulation application, as well as the `GeoModelG4` translator.

This option brings in the dependency  on `Geant4`.

### Building dependencies as part of the main build

You can use CMake compilation flags to configure a built-in build of some of the dependencies. That options will download and build a tested version of those dependencies as part of the build process of GeoModel.

This is especially useful on platforms where the version of those dependencies installed by the system package manager is old, like on Ubuntu.

*Note:* You don't need to use those options on macOS, where the versions installed by the `brew` package manager are often the latest.

#### nlohmann_json

You can enable the built-in build of `nlohmann_json` by using the CMake option:

```
-DGEOMODEL_USE_BUILTIN_JSON=TRUE
```

#### Xerces-C

You can enable the built-in build of `Xerces-C` by using the CMake option:

```
-DGEOMODEL_USE_BUILTIN_XERCESC=TRUE
```

#### Eigen3

You can enable the built-in build of `Eigen3` by using the CMake option:

```
-DGEOMODEL_USE_BUILTIN_EIGEN=TRUE
```

#### Coin3D (Coin + SoQt)

You can enable the built-in build of `Coin3D` (*i.e.*, Coin + SoQt) by using the CMake option:

```
-DGEOMODEL_USE_BUILTIN_COIN3D=TRUE
```



### Examples

For example, you can build the base GeoModel packages, plus the GeoModelVisualization (`gmex`), and a built-in version of nlohmann_json and Xerces-C by configuring the build with:

```
cmake -DCMAKE_INSTALL_PREFIX=../install/ -DGEOMODEL_BUILD_VISUALIZATION=1 -DGEOMODEL_USE_BUILTIN_JSON=TRUE -DGEOMODEL_USE_BUILTIN_XERCESC=TRUE ../GeoModel/
```
