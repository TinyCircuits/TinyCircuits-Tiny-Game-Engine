# Building and running
1. `git clone https://github.com/TinyCircuits/micropython.git`
2. `git checkout top-secret-engine-callback-test`
3. `wsl`
4. `cd mpy-cross`
5. `make -j8`

## Unix building and running (WSL)
1. `cd ../ports/unix`
2. `make submodules`
3. `make -j8 USER_C_MODULES=../../examples/usercmodule DEBUG=1` (`DEBUG=1` is only required for using `gdb`)
4. `./micropython ../rp2/modules/test.py`

## RP2040 building and running (WSL)
1. `cd ../ports/rp2`
2. `make submodules` (only need to do this step once)
3. `make clean`
4. `powershell.exe` (exit WSL)
5. `cd ..`
6. `python build_and_run_rp2.py` (make sure `pyserial` is installed)

# Enabling MicroPython Debug Prints
1. Open `/micropython/py/mpconfig`
2. Enable `#define MICROPY_DEBUG_PRINTERS (1)`
3. Enable `#define MICROPY_DEBUG_VERBOSE (1)`
4. Rebuild MicroPython