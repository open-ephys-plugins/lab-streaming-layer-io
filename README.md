# Lab Streaming Layer I/O

![lsl-inlet-screenshot](https://open-ephys.github.io/gui-docs/_images/lslinlet-01.png)

Creates a lab streaming layer (LSL) client for streaming continuous data into the Open Ephys GUI. An LSL Outlet plugin may be released in the future.

## Installation

This plugin can be added via the Open Ephys GUI's built-in Plugin Installer. Press **ctrl-P** or **⌘P** to open the Plugin Installer, browse to "Lab Streaming Layer IO", and click the "Install" button. The LSL Inlet plugin should now be available to use.


## Usage

Instructions for using the LSL Inlet Plugin are available [here](https://open-ephys.github.io/gui-docs/User-Manual/Plugins/LSL-Inlet.html).

## Building from source

First, follow the instructions on [this page](https://open-ephys.github.io/gui-docs/Developer-Guide/Compiling-the-GUI.html) to build the Open Ephys GUI.

Then, clone this repository into a directory at the same level as the `plugin-GUI`, e.g.:
 
```
Code
├── plugin-GUI
│   ├── Build
│   ├── Source
│   └── ...
├── OEPlugins
│   └── lab-streaming-layer-io
│       ├── Build
│       ├── Source
│       └── ...
```

### Windows

**Requirements:** [Visual Studio](https://visualstudio.microsoft.com/) and [CMake](https://cmake.org/install/)

From the `Build` directory, enter:

```bash
cmake -G "Visual Studio 17 2022" -A x64 ..
```

Next, launch Visual Studio and open the `OE_PLUGIN_lab-streaming-layer-io.sln` file that was just created. Select the appropriate configuration (Debug/Release) and build the solution.

Selecting the `INSTALL` project and manually building it will copy the `.dll` and any other required files into the GUI's `plugins` directory. The next time you launch the GUI from Visual Studio, the LSL Inlet plugin should be available.


### Linux

**Requirements:** [CMake](https://cmake.org/install/)

From the `Build` directory, enter:

```bash
cmake -G "Unix Makefiles" ..
make install
```

This will build the plugin and copy the `.so` file into the GUI's `plugins` directory. The next time you launch the GUI compiled version of the GUI, the LSL Inlet plugin should be available.


### macOS

**Requirements:** [Xcode](https://developer.apple.com/xcode/) and [CMake](https://cmake.org/install/)

From the `Build` directory, enter:

```bash
cmake -G "Xcode" ..
```

Next, launch Xcode and open the `lab-streaming-layer-io.xcodeproj` file that now lives in the “Build” directory.

Running the `ALL_BUILD` scheme will compile the plugin; running the `INSTALL` scheme will install the `.bundle` file to `/Users/<username>/Library/Application Support/open-ephys/plugins-api8`. The LSL Inlet plugin should be available the next time you launch the GUI from Xcode.

## Attribution

This plugin was developed and opened to the community with :heart: by [AE Studio](https://ae.studio/) and [Chadwick Boulay](https://github.com/cboulay). The original repository can be found at https://github.com/labstreaminglayer/OpenEphysLSL-Inlet