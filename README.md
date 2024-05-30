# TinyCircuits-Tiny-Game-Engine
This is a Object-Oriented-Programming (OOP) style game engine for embedded devices running MicroPython.

# INFO:

1.
```
cmd_parser.add_argument(
   "-mlongint-impl",
   choices=["none", "longlong", "mpz"],
   default="longlong", # Changed this from mpz since we don't need that!
   help="long-int implementation used by target (default mpz)",
)
```

# Background
This engine is tightly coupled with MicroPython (https://github.com/micropython/micropython) and will not work without it. This engine is built into MicroPython as an external C module (https://docs.micropython.org/en/latest/develop/cmodules.html). Currently, the engine works on the rp3 and unix ports, with webassembly support planned in the future.

Unfortunately, MicroPython's port make/cmake scripts are not flexible enough to allow configuration from the external c module build scripts. Because of that, customized versions of the supported ports are zipped and stored in the `mp_ports` folder. 

# Updating MicroPython version
Make sure to check that the copied structures in src/utility/engine_mp.h are still the same in the version of MicroPython you're updating to

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
[X] Text that can display ` !"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_`abcdefghijklmnopqrstuvwxyz{|}~`
[X] Fix blit so that it does not cut off some sprites (like 'W' in Text2DNode)
[X] Fix positioning and rotation for nodes that are children of the camera
[X] Fix non square scale not positioning Text2DNode correctly
[X] Have some way to generate typical polygon shapes like rectangle and hexagon without using decomposing PolygonCollisionShape2D classes
[X] Weird sprite jumping/offset during rotation and scaling
[X] Add Line2DNode that draws a rectangle between two points, Allow filled and outline modes
[X] Fix bitmaps not rendering correctly when they have sizes like 7x7 (see padding in https://en.wikipedia.org/wiki/BMP_file_format#:~:text=optional%20color%20list.-,Pixel%20storage,-%5Bedit%5D)
[X] Get rid of polygon collider and polygon drawing to make things more simple, only have rotated rectangles for colliders + circles
[X] Do not generate collisions when two static bodies are touching/overlapping
[X] Add enums for each type of easing function in cglm in tween `start(...)` function and add documentation
[X] Add speed multiplier to tween
[X] If an object is passed to tween `start(...)` but the string for the attribute to tween is "", tween the value without a lookup
[X] Add button nodes
[X] Fix engine reset issue when switching from one file to the next that imports engine: Fixed, manually go through all nodes and mark for gc collect. Call their __del__ functions
[X] Allow setting string to "" in Tween to directly interpolate value
[X] Add different easing types ot Tween (https://www.reddit.com/r/godot/comments/sg2nqq/reference_gif_for_all_the_interpolation_methods/, https://easings.net/#)
[X] Buttons
[X] Add a git hook that runs a performance test on the hardware:
   1. Build and upload the firmware with performance tests baked in
   2. Run each performance test file using serial, Python, and REPL. Soft reset after each test. Each test should test performance of specific parts of the engine, nodes, audio, main loop, etc.
   3. After collecting FPS from each test, create a new plot of the data and save as a png. 
   4. Run `git add -A` to add this new performance data point and plot
   5. Exit with returning zero to allow commit to complete
[X] Line2DNode disappears when thickness is 1 and scale is slightly smaller than 1.0 due to camera zoom
[X] Add outline and fill to each primitive
[.] Add simple shader that happens per pixel for all drawing functions, still need to expose to users somehow
[X] Need to figure out what to do if user has two files that import engine that then resets the engine... Should be possible to import `engine` without resetting everything: Now required to run `import engine_main` to start and reset engine consistently
[X] Make GUIBitmap2DButton for main menu
[X] Fix VoxelSpace performance after last changes
[X] Render voxel sprites with depth buffer
[X] RTTTL Music implementation
[X] Weird shaking on UNIX when line is a child of camera and the camera is rotating: should be fixed now, child nodes were still be translated by little bits
[-] When the camera is 1 unit from the sprite, it should take up the sprite_height number of pixels: NO, they should be scaled and render at the same texel density as the terrain (done)
[.] When the FOV of the camera is changed, the sprites should get wider: messed with this, could use some work
[X] Change engine_input to engine_io so that it makes more sense for rumble to be in there
[.] Somehow get better error when a game raises an exception in launcher: made it a little better, says <module> instead of file name though...

[] Major bug!!! If you set a value to 0.0 in Python and then change that value directly in c, it changes the value of 0.0... See TweenTest/main.py
[] Tween ints

[] Game saving:
   1. When a node is given a name, it can be saved. When a node is instantiated, if the name is provided then it look at the saving module's save location for the nodes properties
   2. Take the entire heap and save it as a file (might be fun if possible)
   3. Add engine_save, engine_save.save(name, object), engine_save.load(name, object, default), engine_save.set_location(location)

[] Revisit opacity after fix, seems to be a lot slower

[] Issue of camera being moved by other things causing flicker. See UI game example. If the camera instantiation is moved before the creation
   of the buttons, then the tick callback for the camera is called and moved over the focused button, which is correct, otherwise get flicker

[] Instead of focusing GUI nodes, make menu button bring up default menu to exit game

[] Make physics independent of FPS

[] Saving a game should be as easy as engine.save() to generate representations of all objects that can then be initialized again from flash (if possible)

[] Tones need duration
[] Reset fps limit to disabled when the engine resets

[] Rotation of the sprites should be taken into account for voxelspace
[] When the camera rotates and that shifts the terrain, should sprites also rotate a bit?
[] Curvature of terrain needs to be taken into account for child nodes too

[.] Figure out how to scale and rotate voxelspace sprites
[] Add 1 rendering pass for all voxel nodes, how to do layers? Only works with depth buffer (required for sprites in 3D anyways)
[] Sprite2DNode and VoxelSpaceBillboardSpriteNode use a lot of the same data and logic, should make a common sprite code base and combine

[.] Menu (carousel)

[] Return something for width and height for GUIBitmap2DButton (what to do when the bitmap changes?)
[] Decide on what text inside GUIBitmap2DButton should do (scale to unfocused, focused, and pressed bitmaps or not? Allow additional text scale to be set by the user)

[] Games should have at least 2MB of flash scratch space but maybe that can be configurable per game. Some games may like to use a lot and an error could let the user know they need to allow for more space
[] Figure out how to draw voxelspace at any angle
[] Should everything the engine allocates be manually de-allocated? Nodes are like this now but what about Vector2, Vector3, and resources? The GC should get them at some
   point and they aren't drawn the screen, they just take up RAM so maybe not the end of the world to leave it for the GC to collect later
[] Does it matter that on engine reset that we only delete native engine node types and not instance child classes? They will be collected later...
[] Test PWM RGB LED light
[] When objects collide, figure out how to ensure another collision is found on the same object from its perspective
[] When objects collide and one is static (not dynamic), move the dynamic object the full penetration distance instead of half (like what happens if two dynamic bodies collide)
[] Physics: got rotated circles and rectangles but still need to figure out a better way of separating nodes without an impulse, resolve angular velocity during collision
Adam:
[X] Need a just_pressed, just_released, and just_changed function for input buttons
[.] Delete nodes, need a .destroy() node function
[] Collision (lots of work/testing still needed)
[X] Camera children for HUD
[X] VoxelSpace
X Need good title for influencer
X Mock ups/concepts of other games
X Voxel game for influencer
X Menu?
[.] Figure out resetting engine when main loop ends
   * [X] Nodes
   * [.] Audio <- Needs special attention! (still has an error where if text played quickly on same channel at start up that it crashes the device in the ISR while calling `get_data`)
   * [X] Resources/Flash
[] Detect if a node adds itself as a child to itself
[] Collision layers
[] Need to revisit flash scratch space. Need to copy data faster. Would be best if we did not need this area...
[] Use DMA to clear background screen buffer (that's not being drawn to) after it is sent out to the screen and the game loop is still running (chaining?): https://e2e.ti.com/support/microcontrollers/c2000-microcontrollers-group/c2000/f/c2000-microcontrollers-forum/509048/fast-way-to-zero-out-an-array/1848882#1848882
[] Clear physics collision buffer bit collection using DMA: https://e2e.ti.com/support/microcontrollers/c2000-microcontrollers-group/c2000/f/c2000-microcontrollers-forum/509048/fast-way-to-zero-out-an-array/1848882#1848882
[] With lots of sprites on screen and big background, drawing or rotating slows down when rotated 90 degrees into the big 128x128 texture
[] Expose low level drawing functions through engine_draw
[] Fix camera view ports not being taken into account when drawing. Defines offset and then clip. Need to think about how view ports should really work, offset and clip into destination buffer (camera destinations should be able to be set to other buffers other than screen buffer if want to render one camera to a texture and then the next camera renders that node with that texture (TV!))
[] Basic culling per node that can be drawn. Two things can be done here
   1. Each node has a bounding box (scaled by node scale and camera zoom), quickly check if that box at all overlaps the camera view before doing the draw algorithm. This doesn't have to be perfect, if the node still gets drawn if close to the camera, that's alright, same for if the node is very big and that technically blank area overlaps the camera view. This will lead to a small performance penalty for games that always draw in bounds, but it provides a much higher flexibility of games to have the culling handled for them
   2. For each drawing algorithm we currently have per-pixel checks for bounds, if the algorithms drawing bounds could quickly be cropped that would be perfect, very very hard though..
[.] Physics: just polygons, rotation (simple init for common shapes), no friction. Need to figure out what to do when physics collision shape is rotated, cache normals?
[] Physics: smooth: https://code.tutsplus.com/how-to-create-a-custom-2d-physics-engine-the-core-engine--gamedev-7493t#:~:text=Here%20is%20a%20full%20example%3A
[] Performance, we'll see how it goes
[] Should the scale of a parent node affect the position and scales of child nodes? Sounds more useful. What about opacity? What about if a person doesn't want this to happen, should that even be supported?

Game ideas
[] ATV Gameboy game
[] Racing, blimp
[] Pool
[] Tanks
[] Platformer
[] Golf
[] Rocketcup
[] FTL

[] Implement audio on unix (need to break up implementation of unix and rp3 audio)
[] Need to revisit Audio to get it working on the other core and with dual DMA (sort of working now)
[] In the future, probably in a soft atomic API for the cases where a sound resource
   gets deleted on core0 and is still being used by core1. It would just be copies
   of the data that get switched out when assigned. Reading will just the one active copy
   while the other is free to be assigned to
[] Second core could be used to run draw functions while physics is run at the same time.
   Use a queue and block when full to run draw functions to make more room. Would help
   all games but especially those that only draw a small amount of nodes
[] When core frequency is changed, wrap value of fractional PWM divisor needs to be adjusted for audio
[] Use voxelspace rotation to render the node? Might be too slow to do that for little gain
[] Make voxelspace camera rotation->z correspond to line drawn at angle in radians. Make camera rotation->x correspond to radians (hard one)
[] Need to make sure collision normals are correct. Seem to be the same for both objects sometimes (circle vs circle).
[] Crash log file (what to do about time? Maybe just overwrite with latest crash info?)
[] Start screen as early as possible and also make a bootloader that starts the screen too
[] Implement file system operations for micropython webassembly
[] VoxelSapce could be rendered faster and need to incorporate node parameters like position and rotation. Implement pixel transformer callbacks
[.] Make child nodes rotate and be positioned correctly about parent (what about scale?)(need to handle all types when getting position and rotation since not all have those and some have 3D structures)
[] Implement PhysicsShapes that are used by Physics2dNodes to define size and shape of collision box/polygon/circle
[.] Figure out how to clear all linked lists at game startup, game end (engine.stop(), so repl is clean), and when wanted on the repl (engine.stop()). Added engine.stop and reset but still need to figure out game start and back to repl
[] Make all drawing functions take in camera node and then rotate, offset, and clip based on position and viewport (write to buffer raw if camera is null)
[] Write tests for all math, physics, and node functions
[] Figure out physics timing, dt, gravity, and frequency for arm/rp2 and UNIX ports
[] Look into MICROPY_MODULE_ATTR_DELEGATION
[] Add options to give names to each node and then get nodes by name (gives list of nodes if more than one have the same name)
[] Grid renderer (with offset and cell scale)
[] Tests for different configurations of child/parent relations (physics objects colliding with child physics objects, cameras are children of nodes, empty nodes without positions, etc.)
[] Particle node that keeps track of a bunch of different particles and allows users to define velocity, direction, and duration
[] If we went back to PIO DMA for SPI to the screen, could we do per-pixel operations are they are being sent out? Would PIO be flexible enough to support a very very simple shading language (most for changing pixel based on screen position)
[] Listen to serial for commands like button inputs or stop
[] Battery indicator and soft shutoff when close to minimum useful voltage
[] Add option for fog, fog start, and fog color on voxelspace nodes
[] In engine_file.c, need to find a better way of getting cwd without creating a new string. The string object could get collected and cause a seg fault, could happen!