Detailed list of changes/steps needed to get MicroPython and the engine running on RPI RP235x (performed on Linux)

# Setup
1. Setup and install picotool
    1. Get it: `git clone https://github.com/raspberrypi/picotool-rp2350.git picotool`
    2. `cd picotool`
    3. `mkdir build && cd build`
    4. `cmake ..`
    5. `make -j8`
    6. `sudo make install` (sdk will use it automatically to convert elf to uf2)
    7. `cd ..`
2. Clone MicroPython:
    1. `git clone https://github.com/micropython/micropython.git mp-thumby`
    2. `cd mp-thumby`
    3. Set to latest supported release: `git reset --hard a61c446`
3. Setup mpy-cross
    1. cd `mpy-cross`
    2. `make -j8`
    3. `cd ..`
4. Setup port
    1. `cd ports/rp2`
    2. `make submodules`
    3. `make clean`
    4. `cd ../..`
5. Go into lib folder: `cd lib`
6. Get the new processor sdk
    1. Remove old sdk (from make submodules command earlier): `rm -r pico-sdk`
    2. Get new sdk: `git clone https://github.com/raspberrypi/pico-sdk-rp235x.git pico-sdk`
    3. `cd pico-sdk`
    4. Switch to new processor branch: `git checkout ax`
    5. `cd ..`
7. Get the new processor tinyusb
    1. Remove the old tinyusb: `rm -r tinyusb`
    2. `git clone https://github.com/raspberrypi/tinyusb-rp235x.git tinyusb`
    3. `cd tinyusb`
    4. Switch to new processor branch: `git checkout ax`
    5. `cd ../..`
8. Clone engine: `git clone https://github.com/TinyCircuits/TinyCircuits-Tiny-Game-Engine.git`
    1. `cd TinyCircuits-Tiny-Game-Engine`
    2. `git submodule update --init --recursive`
    3. `cd ..`

Now all the source is downloaded to build MicroPython, but we will get a lot of errors that need to be manually tweaked until the SDK is stable and an actual MicroPython port is released

To build the firmware: `make -j8 BOARD=THUMBY_COLOR USER_C_MODULES=../../TinyCircuits-Tiny-Game-Engine/src/micropython.cmake` and `make clean BOARD=THUMBY_COLOR`

# Add custom board to MicroPython in rp2 port
1. Go to `ports/rp2/boards`
2. Copy and paste `RPI_PICO` and rename folder to `THUMBY_COLOR`
3. Open `mpconfigboard.h`
    1. Change `#define MICROPY_HW_BOARD_NAME "Raspberry Pi Pico"` -> `#define MICROPY_HW_BOARD_NAME "TinyCircuits Thumby Color"`
    2. Change `#define MICROPY_HW_FLASH_STORAGE_BYTES (1408 * 1024)` -> `#define MICROPY_HW_FLASH_STORAGE_BYTES (14 * 1024 * 1024)` (2MB are used for game scratch space)
4. Because the flash size is different than pico and our Thumby Color board is not in pico-sdk, copy `mp-thumby/lib/pico-sdk/src/boards/include/boards/pico2.h` to `mp-thumby/ports/rp2/boards/THUMBY_COLOR/thumby_color.h`
    1. Inside `thumby_color.h`
        1. Change `_BOARDS_PICO2_H` -> `_BOARDS_THUMBY_COLOR_H`
        2. Change `RASPBERRYPI_PICO2` -> `THUMBY_COLOR`
        3. Remove ifdef and define `#define PICO_FLASH_SIZE_BYTES (16 * 1024 * 1024)`
5. Open `mpconfigboard.cmake`
    1. Make sure `thumby_color.h` is used by adding: `list(APPEND PICO_BOARD_HEADER_DIRS ${MICROPY_BOARD_DIR})`
    2. Add non-zero C heap size: `set(MICROPY_C_HEAP_SIZE 128000)` (128kB, should be adjusted or removed in the future)

# Edit `ports/rp2/Makefile`
1. Add `-DPICO_PLATFORM=rp235x-arm-s` to `CMAKE_ARGS`

# Edit `ports/rp2/CMakeLists.txt`
1. Starting at line 167 or inside `set(PICO_SDK_COMPONENTS` (do this otherwise will get missing header errors during compile)
    1. Comment out: `# hardware_rtc`
    2. Add `hardware_exception`
    3. Add `hardware_sync_spin_lock`
    4. Add `pico_platform_compiler`
    5. Add `pico_platform_sections`
    6. Add `pico_platform_panic`
2. Starting at line 210 (after the above edits) or starting at `pico_set_float_implementation(${MICROPY_TARGET} micropython)` comment out everything to do with software float (not sure if this is exactly correct, but these sources do not exist anymore and we want the FPU to be used, not software implementation)

# Edit `ports/rp2/mphalport.h`
1. Change `#include "RP2040.h"` -> `#include "RP235x.h"`

# Edit `ports/rp2/pendsv.c`
1. Change `#include "RP2040.h"` -> `#include "RP235x.h"`

# Edit `mp-thumby/lib/pico-sdk/src/rp2_common/cmsis/stub/CMSIS/Core/include/cmsis_gcc.h
1. Starting at line 996, comment out everything except `#include "m-profile/cmsis_gcc_m.h"` (not sure why arch is not being defined correctly)

# Edit `mp-thumby/lib/pico-sdk/src/rp235x/hardware_regs/include/hardware/regs/addressmap.h`
1. At line 18, comment out  `// #define RISCV_CSR_BASE_FIXME _u(0x00000000)`
2. At line 87, comment out `// #define RISCV_DM_DUMMY_ADDRESS_FIXME _u(0x4014a000)` (hoping this only affects RISCV stuff that we are not using)

# Edit `ports/rp2/rp2_pio.c`
1. Comment out line 40: `// #define PIO_NUM(pio) ((pio) == pio0 ? 0 : 1)` (a macro that does the same thing is defined in the SDK now)

# Edit `ports/rp2/modtime.c` (on new processor, RTC is replaced)
1. Comment out line 29: `// #include "hardware/rtc.h"`
2. Inside `mp_time_localtime_get` comment out everything inside and then add `return mp_const_none;`
3. Inside `mp_time_time_get` comment out everything inside and then add `return mp_const_none;`

# Edit `ports/rp2/machine_rtc.c` (on new processor, RTC is replaced)
1. Comment out line 40: `// #include "hardware/rtc.h"`
2. Comment anything to with RTC in `machine_rtc_make_new` from lines 53 to 62
3. Inside `machine_rtc_datetime` comment out everything except `return mp_const_none;` at the end

# Edit `ports/rp2/fatfs_port.c` (on new processor, RTC is replaced)
1. Comment out line 28: `// #include "hardware/rtc.h"`
2. Comment out everything in `get_fattime` and add `return 0;` at the end

# Edit `ports/rp2/mbedtls/mbedtls_port.c` (on new processor, RTC is replaced)
1. Comment out line 32: `// #include "hardware/rtc.h"`
2. Comment out everything in `rp2_rtctime_seconds` and add `return 0;` at the end

# Edit `ports/rp2/mphalport.c` (on new processor, RTC is replaced)
1. Comment out line 38: `// #include "hardware/rtc.h"`
2. Inside `mp_hal_time_ns_set_from_rtc` comment out everything

# Edit `ports/rp2/main.c` (on new processor, RTC is replaced)
1. Comment out line 53: `// #include "hardware/rtc.h"`
2. Comment out lines 98 to 110 related to RTC

# Edit `ports/rp2/modmachine.c`
1. Comment out everything in `mp_machine_lightsleep`

# Edit `ports/rp2/modules/rp2.py`
1. Comment out everything after and including line 18 related to PIO

# Do not use infinite ints or double math on RPI or UNIX (does not seem necessary)
1. Inside `ports/unix/variants/mpconfigvariant_common.h`
    1. Change `#define MICROPY_FLOAT_IMPL (MICROPY_FLOAT_IMPL_DOUBLE)` -> `#define MICROPY_FLOAT_IMPL (MICROPY_FLOAT_IMPL_FLOAT)`
    2. Change `#define MICROPY_LONGINT_IMPL (MICROPY_LONGINT_IMPL_MPZ)` -> `#define MICROPY_LONGINT_IMPL (MICROPY_LONGINT_IMPL_LONGLONG)`
2. Inside `ports/rp2/mpconfigport.h` change `#define MICROPY_LONGINT_IMPL (MICROPY_LONGINT_IMPL_MPZ)` -> `#define MICROPY_LONGINT_IMPL (MICROPY_LONGINT_IMPL_LONGLONG)`
3. Inside `mp-thumby/tools/mpy-tools.py` change line 1784 from `default="mpz"` -> default="longlong",

# Copy and edit mammap_mp.ld (very important!)
1. Rename `ports/rp2/memmap_mp.ld` ->  `ports/rp2/memmap_mp_old.ld`
2. Copy `mp-thumby/lib/pico-sdk/src/rp2_common/pico_crt0/rp2350/memmap_default.ld` -> `mp-thumby/ports/rp2/memmap_mp.ld`
3. Change line 61 from `*(EXCLUDE_FILE(*libgcc.a: *libc.a:*lib_a-mem*.o *libm.a:) .text*)` -> 
```
/* Change for MicroPython... exclude gc.c, parse.c, vm.c from flash */
*(EXCLUDE_FILE(*libgcc.a: *libc.a: *lib_a-mem*.o *libm.a: *gc.c.obj *vm.c.obj *parse.c.obj) .text*)
```
(for speed, has a decent effect)

4. At line 255 change `__StackLimit = ORIGIN(RAM) + LENGTH(RAM);` -> `__StackLimit = __bss_end__ + __micropy_c_heap_size__;` (from memmap_mp_old.ld)
5. After `__StackLimit` add these lines anywhere:
```
/* Define start and end of GC heap */
__GcHeapStart = __StackLimit; /* after the C heap (sbrk limit) */
__GcHeapEnd = ORIGIN(RAM) + LENGTH(RAM);
``
(from memmap_mp_old.ld and these are used in `ports/rp2/main.c` to setup the GC heap)

# Config UNIX port to work
1. Inside `ports/unix/mpconfigport.mk` change `MICROPY_PY_BTREE = 1` -> `MICROPY_PY_BTREE = 0`
2. Inside `ports/unix/variants/mpconfigvariant_common.h` add `#define MICROPY_TRACKED_ALLOC (1)` anywhere