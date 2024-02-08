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
[X] Fix rotation starting at incorrect angle and direction
[X] Clip sprites when scaled
[X] Fast backgrounds
[X] Fix BMPs loading vertically flipped
[X] Fix child objects rotating opposite about the parent object (occurred after fixing incorrect parent rotation)
[X] Animated sprites
[X] Physics: Acceleration
[X] Camera zoom: add zoom parameter to camera and apply it to all draw nodes: circles, rectangles, sprites, and polygons.
                this will mean that as the camera zooms in, all nodes get larger but also "further" away as they are drawn
[X] Figure out inversion and stop at camera zoom 0.5 and below (doesn't occur camera is a child of another object)
[X] Add scale to polygon2dnode
[X] Make camera render items centered at 0,0 so that inheritance is easier
[X] Make camera 0,0 render centered at 0,0
[X] Reimplement __del__ for cameras, physics nodes, and engine nodes (was handled) (eventually GUI nodes too) to delete themselves from their respective linked lists: added custom __del__ for physics and camera nodes
[x] Hierarchy translation bug in node_base when a child is an inherited class: seems to be fixed after adding node_base qstr attr to all nodes and using that to lookup the node base for the child node.
[X] Audio: one channel wave files, data goes to contiguous flash space (lfs read() too slow)
          still have SoundResources, return channel objects that the user can reference,
          fixed number of channels (4 at first, maybe 8 later), play sounds by doing something
          like engine_audio.play(source, channel_number) <- returns channel object and also
          channel_object.play(source). Also need engine_audio.get_channel(). Add attributes
          to channel objects like 'loop', 'running', source, duration (seconds), etc.
[X] In all node drawing code, allow camera to be a child of any node
[X] Add way to look into sprite data on flash to get pixels when needed. Sprite/texture data should be read from flash since not enough ram to store lots of RGB565 bitmaps in SRAM. Instead of dedicating a portion of flash to aligned texture/sprite data, make a fast_file module that uses lfs/unix read that will be used to load up 32x32 portions of sprites. Sprite files will be data aligned into grids (this doesn't work for fonts though...). Tested 100 x 32x32 sprite 2d node drawing at 40ms game loop, that should be fast enough.
[X] Outline drawing for rects, circles, and polygons
[X] Add outline parameter to Polygon2DNode but make it default to outline and not draw + error if set to true
[X] Documentation: markdown to HTML
[X] Documentation: add callbacks for nodes to each
[X] Add get_child to each node that gets children based on index and errors if out of bounds
[X] Fix Circle2DNode being drawn at different scales
[X] Add keyword arguments for all constructors

[] Text that can display ` !"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_`abcdefghijklmnopqrstuvwxyz{|}~`
[] Blending colors for partially transparent pixels in some sources (don't ahve any yet)
[.] Add Line2DNode that draws a rectangle between two points, Allow filled and outline modes
[] Expose low level drawing functions through engine_draw
[] Fix camera view ports not being taken into account when drawing. Defines offset and then clip. Need to think about how view ports should really work, offset and clip into destination buffer (camera destinations should be able to be set to other buffers other than screen buffer if want to render one camera to a texture and then the next camera renders that node with that texture (TV!))
[] Filled polygons
[] Basic culling per node that can be drawn. Two things can be done here
   1. Each node has a bounding box (scaled by node scale and camera zoom), quickly check if that box at all overlaps the camera view before doing the draw algorithm. This doesn't have to be perfect, if the node still gets drawn if close to the camera, that's alright, same for if the node is very big and that technically blank area overlaps the camera view. This will lead to a small performance penalty for games that always draw in bounds, but it provides a much higher flexibility of games to have the culling handled for them
   2. For each drawing algorithm we currently have per-pixel checks for bounds, if the algorithms drawing bounds could quickly be cropped that would be perfect, very very hard though..
[] Have some way to generate typical polygon shapes like rectangle and hexagon without using decomposing PolygonCollisionShape2D classes
[.] Physics: just polygons, rotation (simple init for common shapes), no friction. Need to figure out what to do when physics collision shape is rotated, cache normals?
[] Physics: smooth: https://code.tutsplus.com/how-to-create-a-custom-2d-physics-engine-the-core-engine--gamedev-7493t#:~:text=Here%20is%20a%20full%20example%3A
[] Weird sprite jumping/offset during rotation and scaling
[] UI
[] Reset
[] Performance, we'll see how it goes
[] Allow for audio sources to play at different sample rates. This will mean
   knowing where we are in the time of the file and interpolating between samples
   if not exactly on a sample
[] Round end caps for Line2DNode

Game ideas
[] ATV Gameboy game
[] Racing, blimp
[] Pool
[] Tanks
[] Platformer
[] Golf
[] Rocketcup


[] Implement audio on unix (need to break up implementation of unix and rp3 audio)
[] Need to revisit Audio to get it working on the other core and with dual DMA (sort of working now)
[] Audio: since the playback timer is on the other core, the following needs to be taken care of
         1.   When setting 'source', 'gain', or 'looping' on an audio channel from core0, make sure
               core1 isn't also reading/setting those values
         2. If an audio source (wave, tone) is collected, make sure the other core using that does not
            crash. Maybe when a source is set, copy the source to the other core. When an attribute on
            the source is changed, change the copy. When deleted, mark the copy for deletion on the other
            core
         3. Eventually, ToneSoundResource will be able to be regenerated. At that point each sound resource
            accessed by the each core should be safely locked behind mutexes too.
[] In the future, probably in a soft atomic API for the cases where a sound resource
   gets deleted on core0 and is still being used by core1. It would just be copies
   of the data that get switched out when assigned. Reading will just the one active copy
   while the other is free to be assigned to
[] Second core could be used to run draw functions while physics is run at the same time.
   Use a queue and block when full to run draw functions to make more room. Would help
   all games but especially those that only draw a small amount of nodes
[] When core frequency is changed, wrap value of fractional PWM divisor needs to be adjusted
[] Use voxelspace rotation to render the node? Might be too slow to do that for little gain
[] Make voxelspace camera rotation->z correspond to line drawn at angle in radians. Make camera rotation->x correspond to radians (hard one)
[] Add collision points to collision callback for polygon vs. polygon: https://dyn4j.org/2011/11/contact-points-using-clipping/
[] Add outline and fill to each primitive
[] Change node/math class prints to white with no newline and always forced
[] Need to make sure collision normals are correct. Seem to be the same for both objects sometimes (circle vs circle).
[] Turns out that we should only need to do mp_load_attr once into internal struct for each node! During physics, modifying the x and y parameters of the pointers to positions from mp_load_attr really did modify the attrs! Could also just directly create and store then load in each init!!!!! See Reference commit for attr loading... commit and look at commented out lines in engine_physics.c related to storing attrs that's not needed!
[] Test that collisions work correctly (at least the contact points) in each quad of the cord system
[] Crash log file (what to do about time? Maybe just overwrite with latest crash info?)
[] Start screen as early as possible and also make a bootloader that starts the screen too
[] Implement file system operations for micropython webassembly

[] Should a flag be set in sprite2dnode to enable transparency? Or just use special color 0b0000100000100001?
[] VoxelSapce could be rendered faster and need to incorporate node parameters like position and rotation. Implement pixel transformer callbacks
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
[] Main menu and utility scalable UI elements with element traversal based on inputs with UI is active (make best guess on next element to go to based on position). Good for consistency
[] Saving games
[] Grid renderer (with offset and cell scale)
[] Game format?
[] Fast sin/cos/tan lookups to replace math functions (only need to be fast, not accurate)
[] Tests for different configurations of child/parent relations (physics objects colliding with child physics objects, cameras are children of nodes, empty nodes without positions, etc.)
[] Particle node that keeps track of a bunch of different particles and allows users to define velocity, direction, and duration
[] If we went back to PIO DMA for SPI to the screen, could we do per-pixel operations are they are being sent out? Would PIO be flexible enough to support a very very simple shading language (most for changing pixel based on screen position)
[] Listen to serial for commands like button inputs or stop