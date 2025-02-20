# TinyCircuits-Tiny-Game-Engine

This is a 2D/3D game engine for MicroPython. Core engine features are written in C and integrated into MicroPython as external C modules. The engine API is exposed through Python modules.

![Games banner](images/games.png)

The engine has the following modules and features:
* Node API with hierarchy format (parent/child) to render rectangles, circles, lines, text, GUI buttons, sprites, and 3D meshes.
* Resource API for loading and holding data like 1/4/8/16-bit bitmap textures, .wav sound files, RTTTL ringtone tracks, fonts, and 2D/3D noise
* Basic button API
* Audio API for 4 audio channels to play .wav, RTTTL, or tones
* Basic 2D physics engine with rotated rectangle and circle colliders exposed as nodes
* Math API for `Vector2` and `Vector3` types
* Tween/animation API to animate attributes of nodes
* Saves API for reading and writing certain common types of data
* Time API for utilizing hardware real-time-clock (RTC) for keeping persistent time across device power-downs
* Link API for connecting two Thumby Colors together to make local multiplayer games

You can also read the engine documentation, visit the arcade to download games, or make your own games using the Code Editor Web App:
* [Engine Documentation](https://color.thumby.us/doc/landing.html)
* [Arcade](https://color.thumby.us/code/arcade)
* [MicroPython Code Editor](https://color.thumby.us/code/)


# Platforms
The engine can run on most platforms that run [MicroPython](https://github.com/TinyCircuits/micropython/tree/engine). However, some drivers may need to be implemented for buttons, screen, and audio.

The engine has been tested on and ported to the following platforms:
* Linux/Unix
* RP2350 with 16MiB flash
* Webassembly


# Building on Linux for RP2350
1. Update package list: `sudo apt update`
2. Install build chain dependencies (https://datasheets.raspberrypi.com/pico/getting-started-with-pico.pdf#page=33): `sudo apt install git python3 cmake gcc-arm-none-eabi libnewlib-arm-none-eabi build-essential g++ libstdc++-arm-none-eabi-newlib`
3. Clone TinyCircuits MicroPython: `git https://github.com/TinyCircuits/micropython.git mp-thumby`
4. `cd` into MicroPython: `cd mp-thumby`
5. Checkout engine branch: `git checkout engine`
6. Init the engine submodule: `git submodule update --init --recursive`
7. Setup cross compiler:
   1. cd `mpy-cross` (folder is inside `mp-thumby`)
   2. `make -j8`
8. Go back to MicroPython root folder: `cd ..`
9. Setup `rp2` port:
   1. `cd ports/rp2`
   2. `make submodules`
   3. `make clean`
10. Go back to MicroPython root folder: `cd ../..`
11. Go into engine folder: `cd TinyCircuits-Tiny-Game-Engine`
12. Run the custom Python build and upload script: `python3 build_and_upload.py`

(NOTE: you may need to install `pyserial`: `python3 -m pip install pyserial` and the .uf2 will be output to `mp-thumby/ports/rp2/build-THUMBY_COLOR/firmware.uf2` once you see the message `Finding drive letter...`)

# Building on Linux for Linux
These instructions assume that you are cloning MicroPython from the TinyCircuits Fork that has been pre-modified
1. Install SDL2: `sudo apt install libsdl2-dev`
2. Install FFI: `sudo apt install libffi-dev`
3. Install build tools: `sudo apt install build-essential`
4. Clone TinyCircuits MicroPython: `git clone https://github.com/TinyCircuits/micropython.git mp-thumby`
5. `cd` into MicroPython: `cd mp-thumby`
6. Checkout engine branch: `git checkout engine`
7. Init the engine submodule: `git submodule update --init --recursive`
8. Setup UNIX port:
   1. `cd ports/unix`
   2. `make submodules`
9. `cd` to engine file system to build and run MicroPython and the engine
   1. `cd`: `cd ../../TinyCircuits-Tiny-Game-Engine/filesystem`
   2. build: `(cd ../../ports/unix && make -j8 USER_C_MODULES=../../TinyCircuits-Tiny-Game-Engine DEBUG=1)`
   3. run: `../micropython_loop ../../ports/unix/build-standard/micropython -X heapsize=2617152 main.py`

Use `(cd ../../ports/unix && make clean)` to make clean if needed

# Building and Running On Linux Quickly 
If you followed the one of two methods to setup everythign above, you can run the following from `micropython/TinyCircuits-Tiny-Game-Engine/filesystem` to build and run a MicroPython script on Linux quickly:

`(cd ../../ports/unix && make -j8 USER_C_MODULES=../../TinyCircuits-Tiny-Game-Engine) && (../../ports/unix/build-standard/micropython main.py)`

# Building on Linux for WebAssembly
1. Follow instructions here https://emscripten.org/docs/getting_started/downloads.html and finish after executing `source ./emsdk_env.sh` (will need to execute this last command in `emsdk` in every new terminal/session)
2. `git clone https://github.com/TinyCircuits/micropython/tree/engine micropython`
3. `cd micropython/ports/webassembly`
4. `make -j8 USER_C_MODULES=../../TinyCircuits-Tiny-Game-Engine`

# Linux and Webassembly heap size
To mimic the hardware, the heap needs to be `520kB` SRAM + `2MiB` of FLASH scratch = `520*1000 + 2*1024*1024 = 2617152`

# Updating MicroPython version
Make sure to check that the copied structures in src/utility/engine_mp.h are still the same in the version of MicroPython you're updating to. Some structures are not exposed so they had to be copied to where the engine can use them.

# How UF2s are made
1. Download and compile https://github.com/raspberrypi/picotool (requires pico-sdk be installed in default known location or pass `-DPICO_SDK_PATH=` to `cmake`)
2. Upload a firmware and all files that should be included to Thumby Color
3. Connect Thumby Color to computer with picotool installed in BOOTSEL mode (turn off, press and hold down DPAD direction, turn back on)
4. Run: `sudo ./picotool save -r 0x10000000 0x11000000 thumby_color_dev_kit_full_image_08_20_2024.bin` (saves from `XIP_BASE` to `16MiB`s after to bin file)
5. Run: `sudo ./picotool uf2 convert thumby_color_dev_kit_full_image_08_20_2024.bin thumby_color_dev_kit_full_image_08_20_2024.uf2`