# TinyCircuits-Tiny-Game-Engine
This is a Object-Oriented-Programming (OOP) style game engine for embedded devices running MicroPython.

# Background
This engine is tightly coupled with MicroPython (https://github.com/micropython/micropython) and will not work without it. This engine is built into MicroPython as an external C module (https://docs.micropython.org/en/latest/develop/cmodules.html). Currently, the engine works on the rp3 and unix ports, with webassembly support planned in the future.

Unfortunately, MicroPython's port make/cmake scripts are not flexible enough to allow configuration from the external c module build scripts. Because of that, customized versions of the supported ports are zipped and stored in the `mp_ports` folder. 

# Building and Using
1. Unzip and drop a port from `mp_ports` folder into the `micropython/ports` folder
2. Read the port's `README.md` and run the setup commands (probably involves build `mpy-cross` and getting submodules)
3. Use the `build_and_run.py` script to build the MicroPython software, upload, and then run a Python file containing MicroPython code.

The `build_and_run.py` scripts have a couple of arguments:
* `python build_and_run.py rp2/unix scr/tests/test.py`: Builds then runs the script at `scr/tests/test.py`
* `python build_and_run.py rp2/unix  clean`: runs `make clean` for the port, doesn't do anything else

To run the unix port on Windows 10 through WSL, follow this: https://ripon-banik.medium.com/run-x-display-from-wsl-f94791795376

# TODO
[X] Flash issue: Fixed. calculation on sectors erased already count used wrong information
[X] Sprite transparency: Define a color to make transparent. Could use the 1bit of alpha for BMPs in the future
[] Fix rotation starting at incorrect angle and direction
[] Clip sprites when scaled
[] Fast backgrounds
[] Animated sprites
[] Documentation: markdown to PDF
[] Physics: no rotation, no friction
[] Text
[] Audio/Music
[] Performance, we'll see how it goes
[] Weird sprite jumping/offset during rotation and scaling


[] Should a flag be set in sprite2dnode to enable transpareny? Or just use special color 0b0000100000100001?
[] VoxelSapce could be rendered faster and need to incorporate node parameters like position and rotation. Implement pixel transformer callbacks
[] Add way to look into sprite data on flash to get pixels when needed. Sprite/texture data should be read from flash since not enough ram to store lots of RGB565 bitmaps in SRAM. Instead of dedicating a portion of flash to aligned texture/sprite data, make a fast_file module that uses lfs/unix read that will be used to load up 32x32 portions of sprites. Sprite files will be data aligned into grids (this doesn't work for fonts though...). Tested 100 x 32x32 sprite 2d node drawing at 40ms game loop, that should be fast enough.
[.] Make child nodes rotate and be positioned correctly about parent (what about scale?)(need to handle all types when getting position and rotation since not all have those and some have 3D structures)
[] Implement PhysicsShapes that are used by Physics2dNodes to define size and shape of collision box/polygon/circle
[.] Figure out how to clear all linked lists at game startup, game end (engine.stop(), so repl is clean), and when wanted on the repl (engine.stop()). Added engine.stop and reset but still need to figure out game start and back to repl
[] Make all drawing functions take in camera node and then rotate, offset, and clip based on position and viewport (write to buffer raw if camera is null)
[] Expose enough drawing functions to be able to mimic Thumby games
[] Write tests for all math, physics, and node functions
[] Figure out physics timing, dt, gravity, and frequency for arm/rp2 and UNIX ports
[] Look into MICROPY_MODULE_ATTR_DELEGATION
[] To avoid mp_load_attr calls, at the start of the game loop collect all node attributes into some local structure then load the local structure back into the Micropython object (only really matters if inherited because of weird MicroPython attr storage for that case)
[] Add options to give names to each node and then get nodes by name (gives list of nodes if more than one have the same name)
[] Text/font (maybe scalable but scaled to bitmap/file in flash and then streamed after scale changed once (will need a text resource to act as a ledger))
[] Sound/music
[] Line segment 2d node with bezier curve drawer option as for stroke/pen with collision
[] Main menu and utility scalable UI elements with element traversal based on inputs with UI is active (make best guess on next element to go to based on position). Good for consistency
[] Saving games
[] Outline and filled polygon renderer (textured too?)
[] Grid renderer (with offset and cell scale)
[] Vector renderer? For all shapes?
[] Game format?
[] Fast sin/cos/tan lookups to replace math functions (only need to be fast, not accurate)
[] Tests for different configurations of child/parent relations (physics objects colliding with child physics objects, cameras are children of nodes, empty nodes without positions, etc.)
[] Add outline flag to draw primitives such as Circle2DNode, Rectangle2DNode, Oval2DNode (future, slower), Polygon2DNode (future, slow), etc
[] Add better draw line function that completes line fully
[] Particle node that keeps track of a bunch of different particles and allows users to define velocity, direction, and duration
[] If we went back to PIO DMA for SPI to the screen, could we do per-pixel operations are they are being sent out? Would PIO be flexible enough to support a very very simple shading language (most for changing pixel based on screen position)
[] Listen to serial for commands like button inputs or stop
[] When a child is being added, make sure that child doesn't already have a parent!

[X] Reimplement __del__ for cameras, physics nodes, and engine nodes (was handled) (eventually GUI nodes too) to delete themselves from their respective linked lists: added custom __del__ for physics and camera nodes
[x] Hierarchy translation bug in node_base when a child is an inherited class: seems to be fixed after adding node_base qstr attr to all nodes and using that to lookup the node base for the child node.