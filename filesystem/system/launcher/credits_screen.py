from engine_nodes import Text2DNode, EmptyNode
from engine_math import Vector2
from engine_resources import FontResource
import engine_io
import engine_draw
import math

# Holds information about each section
class Section:
    def __init__(self, header, path):
        self.header = header
        self.path = path
        self.lines_read = 0
        self.file = None
    
    def start(self):
        self.lines_read = 0
        self.file = open(self.path, "r")
        self.file.read(1)   # CSV encoding

    def stop(self):
        if self.file is not None: self.file.close()
    
    def next(self):
        to_return = None

        if self.lines_read == 0:
            to_return = (engine_draw.green, self.header)
        else:
            line = self.file.readline()
            line = line.replace("\r", " ") # Don't care about /r since it is either \r\n or \n

            new_line_index = line.find('\n')
            if new_line_index > -1:
                line = line[:line.find('\n')]
            

            if line == "":
                to_return = None
                self.stop()
            else:
                to_return = (engine_draw.white, line)
        
        self.lines_read += 1
        return to_return


class SectionLineRetriever():
    def __init__(self):
        self.sections = [Section("Early Testers", "system/credits_1_testers.csv"),
                         Section("Special Edition", "system/credits_2_special.csv")]

    def start(self):
        self.index = 0
        self.sections[self.index].start()

    def stop(self):
        for section in self.sections:
            section.stop()
    
    def next(self):
        # Return none if out of sections
        if self.index >= len(self.sections):
            return (engine_draw.white, "")

        # Get next line result from current section, if none,
        # go to next section and try this function again
        returned = self.sections[self.index].next()
        if returned == None:
            self.index += 1
            if self.index >= len(self.sections):
                return (engine_draw.white, "")
            else:
                self.sections[self.index].start()
                return self.next()
        else:
            return returned




scroll = False      # Flag for if credits should be scroll, or not
scrollSpeed = 35.0  # Speed to scroll the names at
y_limit_start = 0   # Point at which rows start off screen
y_limit_end = 0     # Point at which screen should re-position itself (calculated later based on font height)
y_row_space_px = 8  # How much spacing there should be between rows
row_height = 0      # Height of each row (calculated later based on font_height)
lowest_row = None   # The row that is lowest on the screen (not in y)
line_retriever = SectionLineRetriever()


# Holds information about the row as well as
# moves it up the screen and repositions it
class Name(Text2DNode):
    def __init__(self, text, font5x7):
        super().__init__(self)
        self.font = font5x7
        self.text = text
        self.position.x = 128*2
        self.position.y = 0
        self.letter_spacing = 1
        self.line_spacing = 1
    
    def tick(self, dt):
        global scrollSpeed
        global lowest_row
        global row_height

        # Sanitize newlines every tick (brute-force, but whatever)
        self.text = self.text.replace("\n", "")

        if scroll is False:
            return
        
        if engine_io.DOWN.is_pressed:
            scrollSpeed = 130.0
        else:
            scrollSpeed = 35.0

        # Move this text node up the screen
        self.position.y -= scrollSpeed * dt

        # If the position of the node is above the top of the screen,
        # re-position just below the lowest row and set self as lowest
        if self.position.y <= y_limit_end:
            color_text_pair = line_retriever.next()
            self.color = color_text_pair[0]
            self.text = color_text_pair[1]
            self.position.y = lowest_row.position.y+row_height
            lowest_row = self


class CreditsScreen():
    def __init__(self, font5x7):
        global y_limit_start
        global y_limit_end
        global lowest_row
        global row_height

        self.font5x7 = font5x7

        # Calculate bounds to just outside the top/bottom of the screen
        y_limit_start = 64 + self.font5x7.height + 1
        y_limit_end = -64 - self.font5x7.height - 1
        y_span = y_limit_start - y_limit_end

        # Calculate row height based on font height and then 
        # calculate how many rows can fit in that
        row_height = self.font5x7.height + y_row_space_px
        self.row_count = int(math.floor(y_span / row_height))+1

        # Create the rows
        self.rows = []

        for irx in range(self.row_count):
            self.rows.append(Name("TEST", self.font5x7 ))
    
    def reset_rows(self):
        global lowest_row
        for irx in range(self.row_count):
            self.rows[irx].position.y = y_limit_start+irx*row_height
        lowest_row = self.rows[-1]

        for row in self.rows:
            color_text_pair = line_retriever.next()
            row.color = color_text_pair[0]
            row.text = color_text_pair[1]
    
    def tell_page(self, page_index):
        global scroll

        # If on credits screen, open file
        if page_index == 2:
            line_retriever.start()
            scroll = True
            self.reset_rows()
        else:
            scroll = False
            line_retriever.stop()

