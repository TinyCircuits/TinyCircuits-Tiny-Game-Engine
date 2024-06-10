# TinyCircuits-Tiny-Game-Engine
This is a game engine for embedded devices that can run MicroPython.

# Building on Linux
1. Clone MicroPython: `git clone https://github.com/micropython/micropython.git mp-thumby`
2. CD into MicroPython: `cd mp-thumby`
3. Reset to MicroPython version 1.23.0: `git reset --hard a61c446`
4. Clone latest engine module code with `lib` submodules: `git clone --recurse-submodules https://github.com/TinyCircuits/TinyCircuits-Tiny-Game-Engine.git`
5. Setup UNIX port:
   1. `cd ports/unix`
   2. `make submodules`
6. Inside `thumby-mp/ports/unix/variants/mpconfigvariant_common.h` do:
   1. Change `#define MICROPY_FLOAT_IMPL (MICROPY_FLOAT_IMPL_DOUBLE)` -> `#define MICROPY_FLOAT_IMPL (MICROPY_FLOAT_IMPL_FLOAT)`
   2. Change `#define MICROPY_LONGINT_IMPL (MICROPY_LONGINT_IMPL_MPZ)` -> `#define MICROPY_LONGINT_IMPL (MICROPY_LONGINT_IMPL_LONGLONG)`
   3. Add `#define MICROPY_TRACKED_ALLOC (1)` anywhere
7. UGLY: inside thumby-mp/tools/mpy-tool.py
   1. Change line 1784 to `default="longlong"` (don't know where this tool is used and how to pass it a different value, will just set it for now)
8. CD to filesystem root: `cd thumby-mp/TinyCircuits-Tiny-Game-Engine/filesystem`
9. Build MicroPython UNIX port: `(cd ../../ports/unix && make -j8 USER_C_MODULES=../../TinyCircuits-Tiny-Game-Engine DEBUG=1)`
10. Run MicroPython port: `../../ports/unix/build-standard/micropython launcher.py`

Use `(cd ../../ports/unix && make clean)` to make clean if needed

# Updating MicroPython version
Make sure to check that the copied structures in src/utility/engine_mp.h are still the same in the version of MicroPython you're updating to. Some structures are not exposed so they had to be copied to where the engine can use them.