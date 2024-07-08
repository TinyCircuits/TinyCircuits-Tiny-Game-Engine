# tilemapper.py
import random
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
    def __init__(self, surrounding_pattern, output_frames):
        self.surrounding_pattern = surrounding_pattern  # 3x3 List of expected surrounding tiles
        if isinstance(output_frames, tuple):
            self.tile_choices = [(output_frames[0], output_frames[1], 1)]
        elif isinstance(output_frames, list):
            self.tile_choices = output_frames
        else:
            raise ValueError("output_frames must be a tuple or a list of tuples")

    def select_tile(self):
        if len(self.tile_choices) == 1:
            return self.tile_choices[0][:2]
        
        total_weight = sum(weight for _, _, weight in self.tile_choices)
        choice = random.uniform(0, total_weight)
        current = 0
        for frame_x, frame_y, weight in self.tile_choices:
            if current + weight >= choice:
                return frame_x, frame_y
            current += weight
        return self.tile_choices[-1][:2]  # Default to the last tile if none chosen

class TileRules:
    def __init__(self):
        self.rules = []

    def add_rule(self, rule):
        self.rules.append(rule)

    def get_tile(self, surrounding):
        for rule in self.rules:
            if self.matches_pattern(rule.surrounding_pattern, surrounding):
                return rule.select_tile()
        return None

    def matches_pattern(self, pattern, surrounding):
        for p_row, s_row in zip(pattern, surrounding):
            for p, s in zip(p_row, s_row):
                if p != '*' and p is not None and (s is None or p != s):
                    return False
        return True

class TilingRenderer(Rectangle2DNode):
    def __init__(self, tile_map, tile_type, tile_rules):
        super().__init__(self)
        self.tile_map = tile_map
        self.tile_type = tile_type
        self.tile_rules = tile_rules
        self.render_tiles()
        self.sprites = []
        self.opacity=0.0

    def render_tiles(self):
        self.sprites = []
        for y, row in enumerate(self.tile_map):
            for x, tile in enumerate(row):
                if tile == 1:
                    surrounding = self.get_surrounding(x, y)
                    result = self.tile_rules.get_tile(surrounding)
                    if result is None:
                        continue
                    frame_x, frame_y = result
                    self.render_tile(x, y, frame_x, frame_y)

    def get_surrounding(self, x, y):
        surrounding = []
        for dy in (-1, 0, 1):
            row = []
            for dx in (-1, 0, 1):
                nx, ny = x + dx, y + dy
                if 0 <= nx < len(self.tile_map[0]) and 0 <= ny < len(self.tile_map):
                    row.append(self.tile_map[ny][nx])
                else:
                    row.append(None)
            surrounding.append(row)
        return surrounding

    def render_tile(self, x, y, frame_x, frame_y):
        frame_w = self.tile_type.texture.width / self.tile_type.frame_count_x
        frame_h = self.tile_type.texture.height / self.tile_type.frame_count_y

        px = x * frame_w
        py = y * frame_h

        sprite = Sprite2DNode(
            position=Vector2(px + frame_w / 2, py + frame_h / 2),
            texture=self.tile_type.texture,
            transparent_color=engine_draw.green,
            fps=0,
            frame_count_x=self.tile_type.frame_count_x,
            frame_count_y=self.tile_type.frame_count_y,
            rotation=0,
            scale=Vector2(1, 1),
            opacity=1.0,
            playing=False
        )
        sprite.frame_current_x = frame_x
        sprite.frame_current_y = frame_y
        self.add_child(sprite)
        self.sprites.append(sprite)