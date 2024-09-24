from engine_nodes import Text2DNode
from engine_math import Vector2
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

    def stop(self):
        if self.file is not None: self.file.close()
    
    def next(self):
        to_return = None

        if self.lines_read == 0:
            to_return = (engine_draw.green, self.header)
        else:
            line = self.file.readline()
            line = line[:line.find(',')]

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
                         Section("Collectors\nEdition", "system/credits_2_collectors.csv"),
                         Section("Special\nEdition", "system/credits_3_special.csv")]

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
y_limit_start = 0   # Point at which rows start off screen
y_limit_end = 0     # Point at which screen should re-position itself (calculated later based on font height)
y_row_space_px = 8  # How much spacing there should be between rows
row_height = 0      # Height of each row (calculated later based on font_height)
lowest_row = None   # The row that is lowest on the screen (not in y)
line_retriever = SectionLineRetriever()


# Holds information about the row as well as
# moves it up the screen and repositions it
class Name(Text2DNode):
    def __init__(self, font, text):
        super().__init__(self)
        self.font = font
        self.text = text
        self.position.x = 128*2
        self.position.y = 0
        self.letter_spacing = 1
        self.line_spacing = 1
    
    def tick(self, dt):
        global lowest_row

        if scroll is False:
            return

        # Move this text node up the screen
        self.position.y -= 0.5

        # If the position of the node as above the top of the screen,
        # re-position just below the lowest row and set self as lowest
        if self.position.y <= y_limit_end:
            color_text_pair = line_retriever.next()
            self.color = color_text_pair[0]
            self.text = color_text_pair[1]
            self.position.y = lowest_row.position.y+lowest_row.height
            lowest_row = self


class CreditsScreen():
    def __init__(self, font):
        global y_limit_start
        global y_limit_end
        global lowest_row
        global row_height
        self.font = font

        # Calculate bounds to just outside the top/bottom of the screen
        y_limit_start = 64 + font.height + 1
        y_limit_end = -64 - font.height - 1
        y_span = y_limit_start - y_limit_end

        # Calculate row height based on font height and then 
        # calculate how many rows can fit in that
        row_height = font.height + y_row_space_px
        self.row_count = int(math.floor(y_span / row_height))+1

        # Create the rows
        self.rows = []

        for irx in range(self.row_count):
            self.rows.append(Name(self.font, "TEST"))
    
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

