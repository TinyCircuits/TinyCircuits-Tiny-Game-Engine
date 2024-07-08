# tilemapper.py
import engine_draw
import engine_io
from engine_math import Vector2
from engine_nodes import Rectangle2DNode, Sprite2DNode

class TTileType:
    def __init__(self, name, texture, frame_count_x, frame_count_y):
        self.name = name
        self.texture = texture  # TextureResource for the tile type
        self.frame_count_x = frame_count_x  # Number of frames horizontally
        self.frame_count_y = frame_count_y  # Number of frames vertically

class TileRule:
    def __init__(self, tile_type, surrounding_pattern, output_tile_type, output_frame_x, output_frame_y):
        self.tile_type = tile_type
        self.surrounding_pattern = surrounding_pattern  # List of expected surrounding tiles
        self.output_tile_type = output_tile_type
        self.output_frame_x = output_frame_x
        self.output_frame_y = output_frame_y

class TileRules:
    def __init__(self):
        self.rules = []

    def add_rule(self, rule):
        print(f"Adding rule: {rule}")
        self.rules.append(rule)

    def get_tile(self, tile_type, surrounding):
        print(f"Getting tile for type: {tile_type.name} with surrounding: {surrounding}")
        for rule in self.rules:
            if self.matches_pattern(rule.surrounding_pattern, surrounding):
                print(f"Rule matched: {rule}")
                return rule.output_tile_type, rule.output_frame_x, rule.output_frame_y
        print("No matching rule found. Returning default tile and frame (0,0).")
        return tile_type, 0, 0  # Default to the original tile and default frame if no rules match

    def matches_pattern(self, pattern, surrounding):
        for p, s in zip(pattern, surrounding):
            if p != '*' and p is not None and (s is None or p != s):
                return False
        return True

class TilingRenderer(Rectangle2DNode):
    def __init__(self, tile_map, tile_types, tile_rules):
        super().__init__(self)
        self.tile_map = tile_map
        self.tile_types = tile_types
        self.tile_rules = tile_rules
        print("TilingRenderer initialized. Rendering tiles...")
        self.render_tiles()
        self.sprites = []

    def render_tiles(self):
        print("Starting to render tiles...")
        self.destroy_children()
        self.sprites = []
        for y, row in enumerate(self.tile_map):
            for x, tile in enumerate(row):
                print(f"Rendering tile at position ({x}, {y}) with type: {tile}")
                surrounding = self.get_surrounding(x, y)
                final_tile, frame_x, frame_y = self.tile_rules.get_tile(self.tile_types[tile], surrounding)
                self.render_tile(x, y, final_tile, frame_x, frame_y)
        print("Finished rendering tiles.")

    def get_surrounding(self, x, y):
        surrounding = []
        for dy in (-1, 0, 1):
            for dx in (-1, 0, 1):
                if dx == 0 and dy == 0:
                    continue
                nx, ny = x + dx, y + dy
                if 0 <= nx < len(self.tile_map[0]) and 0 <= ny < len(self.tile_map):
                    surrounding.append(self.tile_map[ny][nx])
                else:
                    surrounding.append(None)
        print(f"Surrounding tiles for ({x}, {y}): {surrounding}")
        return surrounding

    def render_tile(self, x, y, tile_type, frame_x, frame_y):
        print(f"Rendering tile at ({x}, {y}) with final tile type: {tile_type.name} and frames ({frame_x}, {frame_y})")
        frame_w = tile_type.texture.width / tile_type.frame_count_x
        frame_h = tile_type.texture.height / tile_type.frame_count_y

        px = x * frame_w
        py = y * frame_h

        print(f"Creating {tile_type.texture} {tile_type.name} sprite for tile ({x}, {y}) at position ({px}, {py}) with size ({frame_w}, {frame_h}) and frame ({frame_x}, {frame_y})")
        sprite = Sprite2DNode(
            position=Vector2(px + frame_w / 2, py + frame_h / 2),
            texture=tile_type.texture,
            transparent_color=255,
            fps=0,
            frame_count_x=tile_type.frame_count_x,
            frame_count_y=tile_type.frame_count_y,
            rotation=0,
            scale=Vector2(1, 1),
            opacity=1.0,
            playing=False
        )
        sprite.frame_current_x = frame_x
        sprite.frame_current_y = frame_y
        sprite.set_layer(0)
        self.add_child(sprite)
        self.sprites.append(sprite)
        print(f"Finished rendering tile at ({x}, {y})")
