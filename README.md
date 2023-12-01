# TODO
[] Implement PhysicsShapes that are used by Physics2dNodes to define size and shape of collision box/polygon/circle
[] Draw node origins if flag in engine_debug is true
[] Make all drawing functions take in camera node and then rotate, offset, and clip based on position and viewport (write to buffer raw if camera is null)
[] Expose enough drawing functions to be able to mimic Thumby games
[] Add way to look into sprite data on flash to get pixels when needed. Sprite/texture data should be read from flash since not enough ram to store lots of RGB565 bitmaps in SRAM
[] Make child nodes rotate and be positioned correctly about parent
[] Write tests for all math, physics, and node functions
[] Figure out physics timing, dt, gravity, and frequency for arm/rp2 and UNIX ports
[] Look into MICROPY_MODULE_ATTR_DELEGATION
[] To avoid mp_load_attr calls, at the start of the game loop collect all node attributes into some local structure then load the local structure back into the micropython object (only really matters if inherited because of weird MicroPython attr storage for that case)
[] Add options to give names to each node and then get nodes by name (gives list of nodes if more than one have the same name)
[] Text/font
[] Sound/music

# Building and running
1. `git clone https://github.com/TinyCircuits/micropython.git`
2. `git submodule update --init --recursive` https://git-scm.com/book/en/v2/Git-Tools-Submodules#:~:text=you%20can%20use%20the%20foolproof%20git%20submodule%20update%20%2D%2Dinit%20%2D%2Drecursive
3. `git checkout top-secret-engine-callback-test`
4. `wsl`
5. `cd mpy-cross`
6. `make -j8`

# Updating box2d to upstream
1. `cd libs/box2d`
2. `git fetch` https://git-scm.com/book/en/v2/Git-Tools-Submodules#:~:text=If%20you%20want%20to%20check%20for%20new%20work%20in%20a%20submodule%2C%20you%20can%20go%20into%20the%20directory%20and%20run%20git%20fetch%20and%20git%20merge%20the%20upstream%20branch%20to%20update%20the%20local%20code.
3. `git merge`
4. `git add -A`
5. `git commit -m "Update box2d to latest upstream"`

# Making changes to box2d and pushing them

https://git-scm.com/book/en/v2/Git-Tools-Submodules#:~:text=Working%20on%20a%20Submodule

1. Make changes inside box2d folder and push them to box2d fork
2. When pushing main project changes, can make sure that submodule changes were pushed with `git push --recurse-submodules=check`

# Syncing submodules as a collaborator

https://git-scm.com/book/en/v2/Git-Tools-Submodules#:~:text=Pulling%20Upstream%20Changes%20from%20the%20Project%20Remote

1. `git pull --recurse-submodules`

If the submodule URL changes and you get an error indicating that:

https://git-scm.com/book/en/v2/Git-Tools-Submodules#:~:text=that%20case%2C%20it%20is%20possible%20for%20git%20pull%20%2D%2Drecurse%2Dsubmodules%2C%20or%20git%20submodule%20update%2C%20to%20fail

1. `git submodule sync --recursive`
2. `git submodule update --init --recursive`

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