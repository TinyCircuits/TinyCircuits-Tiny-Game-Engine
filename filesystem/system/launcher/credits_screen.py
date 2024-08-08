from engine_nodes import Text2DNode
from engine_math import Vector2
import math

files = ["system/credits_1_testers.csv", "system/credits_2_collectors.csv", "system/credits_3_special.csv"]

y_limit_end = 0     # Point at which screen should re-position itself (calculated later based on font height)
y_row_space_px = 8  # How much spacing there should be between rows
row_height = 0      # Height of each row (calculated later based on font_height)

# The row that is lowest on the screen (not in y)
lowest_row = None

class Name(Text2DNode):
    def __init__(self, font, name, y):
        super().__init__(self)
        self.font = font
        self.text = name
        self.position.x = 128*2
        self.position.y = y
        self.letter_spacing = 1
    
    def tick(self, dt):
        global lowest_row

        # Move this text node up the screen
        self.position.y -= 0.5

        # If the position of the node as above the top of the screen,
        # re-position just below the lowest row and set self as lowest
        if self.position.y <= y_limit_end:
            self.position.y = lowest_row.position.y+row_height
            lowest_row = self


class CreditsScreen():
    def __init__(self, font):
        global y_limit_end
        global lowest_row
        global row_height

        # Calculate bounds to just outside the top/bottom of the screen
        y_limit_start = 64 + font.height + 1
        y_limit_end = -64 - font.height - 1
        y_span = y_limit_start - y_limit_end

        # Calculate row height based on font height and then 
        # calculate how many rows can fit in that
        row_height = font.height + y_row_space_px
        row_count = int(math.floor(y_span / row_height))+1

        # Create the rows
        self.rows = []

        for irx in range(row_count):
            self.rows.append(Name(font, "TEST", y_limit_start+irx*row_height))
        lowest_row = self.rows[-1]