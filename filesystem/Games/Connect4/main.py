import engine_main

import engine
import engine_draw
import engine_io
import engine_debug
import random
import time
from engine_math import Vector2,Vector3
from engine_nodes import Rectangle2DNode, Text2DNode, CameraNode, Circle2DNode
from engine_resources import FontResource

engine.set_fps_limit(30)

#engine_debug.enable_all()

font = FontResource("outrunner_outline.bmp")

# Display dimensions
DISP_WIDTH = 128
DISP_HEIGHT = 128

# Grid dimensions
GRID_COLS = 7
GRID_ROWS = 6

# Cell dimensions
CELL_WIDTH = DISP_WIDTH / GRID_COLS
CELL_HEIGHT = DISP_HEIGHT  / GRID_ROWS - 2
OFFSET = CELL_WIDTH / 2

HEADER_HEIGHT = GRID_ROWS * 2

class GridNode(Rectangle2DNode):
    def __init__(self, grid):
        super().__init__(self)
        self.grid = grid
        self.width = DISP_WIDTH
        self.height = DISP_HEIGHT
        self.position = Vector2(0, 0)
        self.color = engine_draw.black
        self.selected_col = 0

    def tick(self, dt):
        global game
        if game.winner_message:
            win_text = Text2DNode(position=Vector2(DISP_WIDTH//2, DISP_HEIGHT//2),
                        font=font,
                        text=game.winner_message,
                        rotation=0,
                        scale=Vector2(1, 1),
                        opacity=1.0,
                        letter_spacing=1,
                        line_spacing=1)
            self.add_child(win_text)
            return
        self.draw_grid()

    def draw_piece(self, position, color, ringcolor):
        piece = Circle2DNode()
        piece.radius = CELL_WIDTH / 2 - 2
        piece.position = position
        piece.color = color
        ring = Circle2DNode()
        ring.radius = CELL_WIDTH / 2 - 4
        ring.position = position
        ring.outline = True
        ring.color = ringcolor
        self.add_child(ring)
        return piece

    def draw_grid(self):
        global game
        self.destroy_children()
        rec = Rectangle2DNode()
        rec.width = DISP_WIDTH + 1
        rec.height = DISP_HEIGHT - HEADER_HEIGHT // 2
        rec.position = Vector2(DISP_WIDTH / 2 - 1, HEADER_HEIGHT // 2 + DISP_HEIGHT / 2 - 1)
        rec.color = engine_draw.blue
        self.add_child(rec)

        for row in range(GRID_ROWS):
            for col in range(GRID_COLS):
                x = col * CELL_WIDTH + OFFSET
                y = row * CELL_HEIGHT + HEADER_HEIGHT + OFFSET
                hole = Circle2DNode()
                hole.radius = CELL_WIDTH / 2 - 1
                hole.position = Vector2(x, y)
                hole.color = engine_draw.black
                self.add_child(hole)

                if self.grid[row][col] == 1:
                    piece = self.draw_piece(Vector2(x, y), engine_draw.red, engine_draw.maroon)
                    self.add_child(piece)
                elif self.grid[row][col] == 2:
                    piece = self.draw_piece(Vector2(x, y), engine_draw.yellow, engine_draw.gold)
                    self.add_child(piece)

                if col == self.selected_col:
                    if (game.current_player == 1):
                        indicator = self.draw_piece(Vector2(x, 0), engine_draw.red, engine_draw.maroon)
                    else:
                        indicator = self.draw_piece(Vector2(x, 0), engine_draw.lightgrey, engine_draw.darkgrey)
                    self.add_child(indicator)

class Game(Rectangle2DNode):
    def __init__(self, camera):
        super().__init__(self)
        self.grid = [[0 for _ in range(GRID_COLS)] for _ in range(GRID_ROWS)]
        self.grid_node = GridNode(self.grid)
        self.add_child(self.grid_node)
        self.camera = camera
        self.current_player = 1
        self.selected_col = 3
        self.grid_node.selected_col = self.selected_col
        self.winner_message = None
        self.winner_timer = 0

    def tick(self, dt):
        if self.winner_message:
            self.winner_timer -= dt
            if self.winner_timer <= 0:
                self.reset_game()
            return

        if self.check_win(1):
            self.show_winner("Player Wins!")
            return
        if self.check_win(2):
            self.show_winner("AI Wins!")
            return
        if self.current_player == 1:
            if engine_io.LEFT.is_just_pressed:
                self.selected_col = max(0, self.selected_col - 1)
                self.grid_node.selected_col = self.selected_col
            elif engine_io.RIGHT.is_just_pressed:
                self.selected_col = min(GRID_COLS - 1, self.selected_col + 1)
                self.grid_node.selected_col = self.selected_col
            elif engine_io.A.is_just_pressed:
                if self.make_move(self.selected_col, 1):
                    self.current_player = 2
        elif self.current_player == 2:
            self.ai_move()
            self.current_player = 1

    def show_winner(self, message):
        self.winner_message = message
        self.winner_timer = 3  # 3 seconds delay before resetting

    def reset_game(self):
        self.grid = [[0 for _ in range(GRID_COLS)] for _ in range(GRID_ROWS)]
        self.grid_node.grid = self.grid
        self.grid_node.draw_grid()
        self.current_player = random.choice([1, 2])
        self.selected_col = 3
        self.grid_node.selected_col = self.selected_col
        self.winner_message = None
        self.winner_timer = 0

    def make_move(self, col, player):
        for row in reversed(range(GRID_ROWS)):
            if self.grid[row][col] == 0:
                self.grid[row][col] = player
                return True
        return False

    def check_win(self, player):
        for row in range(GRID_ROWS):
            for col in range(GRID_COLS):
                if col + 3 < GRID_COLS and all(self.grid[row][col+i] == player for i in range(4)):
                    return True
                if row + 3 < GRID_ROWS and all(self.grid[row+i][col] == player for i in range(4)):
                    return True
                if col + 3 < GRID_COLS and row + 3 < GRID_ROWS and all(self.grid[row+i][col+i] == player for i in range(4)):
                    return True
                if col + 3 < GRID_COLS and row - 3 >= 0 and all(self.grid[row-i][col+i] == player for i in range(4)):
                    return True
        return False

    def evaluate_board(self,grid):
        """ Evaluates the board for scoring based on open lines for 4 in a row """
        score = 0
        # Check horizontal locations for potential four in a row
        for row in range(GRID_ROWS):
            row_array = [int(grid[row][col]) for col in range(GRID_COLS)]
            for col in range(GRID_COLS - 3):
                window = row_array[col:col + 4]
                score += self.evaluate_window(window)

        # Check vertical locations for potential four in a row
        for col in range(GRID_COLS):
            col_array = [int(grid[row][col]) for row in range(GRID_ROWS)]
            for row in range(GRID_ROWS - 3):
                window = col_array[row:row + 4]
                score += self.evaluate_window(window)

        # Check positively sloped diagonals
        for row in range(GRID_ROWS - 3):
            for col in range(GRID_COLS - 3):
                window = [grid[row + i][col + i] for i in range(4)]
                score += self.evaluate_window(window)

        # Check negatively sloped diagonals
        for row in range(GRID_ROWS - 3):
            for col in range(3, GRID_COLS):
                window = [grid[row + i][col - i] for i in range(4)]
                score += self.evaluate_window(window)

        return score

    def evaluate_window(self,window):
        score = 0
        if window.count(2) == 4:
            score += 1000
        elif window.count(1) == 4:
            score -= 1000
        elif window.count(2) == 3 and window.count(0) == 1:
            score += 100
        elif window.count(1) == 3 and window.count(0) == 1:
            score -= 200 # Penalise allowing a winning move
        elif window.count(2) == 2 and window.count(0) == 2:
            score += 10  # Reward positions with two AI pieces and two open slots

        return score

    def ai_move(self):
        filled_cells = sum(self.grid[row][col] != 0 for row in range(GRID_ROWS) for col in range(GRID_COLS))
        # Depth settings based on difficulty level and game stage
        depth_settings = {
            0: (0, 0, 0),  # Very Easy: (Early, Mid, Late)
            1: (1, 1, 1),  # Easy: (Early, Mid, Late)
            2: (1, 2, 2),  # Medium: (Early, Mid, Late)
            3: (2, 3, 4),   # Hard: (Early, Mid, Late)
            4: (3, 4, 4)   # Ultra
        }
        difficulty_level=4
        # Select depth based on the number of filled cells
        if filled_cells < 5:
            depth = depth_settings[difficulty_level][0]  # Early game depth
        elif filled_cells < 10:
            depth = depth_settings[difficulty_level][1]  # Mid game depth
        else:
            depth = depth_settings[difficulty_level][2]  # Late game depth

        best_score = float('-inf')
        best_cols = []
        for col in range(GRID_COLS):
            row = self.get_next_open_row(self.grid, col)
            if row is not None:
                self.grid[row][col] = 2  # Temporarily make the move
                score = self.minimax(self.grid, depth, False, float('-inf'), float('inf'))
                self.grid[row][col] = 0  # Undo the move
                print("Col "+str(col)+ " score: "+str(score))
                if score > best_score:
                    best_score = score
                    best_cols = [col]  # Reset the list with the current column
                elif score == best_score:
                    best_cols.append(col)

        best_col = random.choice(best_cols) if best_cols else random.choice(self.valid_moves())
        self.make_move(best_col, 2)

    def valid_moves(self):
        return [c for c in range(GRID_COLS) if self.grid[0][c] == 0]
    
    def get_next_open_row(self, grid, col):
        for r in range(GRID_ROWS-1, -1, -1):
            if grid[r][col] == 0:
                return r
        return None
    
    def minimax(self,grid, depth, isMaximizing, alpha, beta):
        if depth == 0 or self.check_win(1) or self.check_win(2):
            return self.evaluate_board(grid)
        
        if isMaximizing:
            maxEval = float('-inf')
            for col in range(GRID_COLS):
                row = self.get_next_open_row(grid, col)
                if row is not None:
                    grid[row][col] = 2
                    eval = self.minimax(grid, depth - 1, False, alpha, beta)
                    grid[row][col] = 0
                    maxEval = max(maxEval, eval)
                    alpha = max(alpha, eval)
                    if beta <= alpha:
                        break
            return maxEval
        else:
            minEval = float('inf')
            for col in range(GRID_COLS):
                row = self.get_next_open_row(grid, col)
                if row is not None:
                    grid[row][col] = 1
                    eval = self.minimax(grid, depth - 1, True, alpha, beta)
                    grid[row][col] = 0
                    minEval = min(minEval, eval)
                    beta = min(beta, eval)
                    if beta <= alpha:
                        break
            return minEval

camera = CameraNode()
camera.position = Vector3(DISP_WIDTH/2, DISP_WIDTH/2,1)
game = Game(camera)

engine.start()