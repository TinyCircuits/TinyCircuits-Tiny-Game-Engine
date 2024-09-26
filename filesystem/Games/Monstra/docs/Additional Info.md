If you run into performance issues (i.e. with voxel stuff), you can use:

```python
import engine

engine.freq(250 * 1000 * 1000)
```



The VoxelSpaceNode converts each RGB channel, internally, to 0.0 ~ 1.0
separately and then uses an RGB to grayscale conversion defined here:
https://en.wikipedia.org/wiki/Grayscale#:~:text=Ylinear%2C-,which%20is%20given%20by,-%5B6%5D.
I think this does give 16-bits of height resolution.

The docs are missing the `Color` documentation since I need to add a way to add
groups of optional parameters to my doc generator. Anyway, you can create Color
objects in two ways:

```python
import engine
from engine_draw import Color

C0 = Color(1.0, 1.0, 1.0) # white
C1 = Color(0xffff) # white

engine.set_background_color(C1)
```
