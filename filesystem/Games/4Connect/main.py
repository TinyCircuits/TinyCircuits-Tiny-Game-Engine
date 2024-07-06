import engine_main
import engine
import engine_draw
import engine_io
import engine_debug
import engine_physics
import random
import time
from engine_math import Vector2, Vector3
from engine_nodes import Rectangle2DNode, CameraNode, PhysicsRectangle2DNode, PhysicsCircle2DNode, Circle2DNode, Text2DNode, Sprite2DNode
from engine_resources import FontResource, TextureResource

from simpletextmenu import SimpleTextMenu

#engine.set_fps_limit(30)
random.seed(time.ticks_ms())

# engine_debug.enable_all()

font = FontResource("../../assets/outrunner_outline.bmp")

red_texture = TextureResource("red.bmp")
yellow_texture = TextureResource("yellow.bmp")

# Display dimensions
DISP_WIDTH = 128
DISP_HEIGHT = 128

# Grid dimensions
GRID_COLS = 7
GRID_ROWS = 6

# Cell dimensions
CELL_WIDTH = 17
CELL_HEIGHT = 17
OFFSET = CELL_WIDTH / 2
COLSHIFT = 14

HEADER_HEIGHT = GRID_ROWS * 2

planned_ai_move = None
ai_thinking = False

def deepcopy_2d_list(original):
    return [row[:] for row in original]

class PieceNode(PhysicsCircle2DNode):
    def __init__(self, position, texture):
        super().__init__(self)
        self.radius = CELL_WIDTH / 2
        self.position = position
        self.bounciness= 0.1
        self.gravity_scale=Vector2(0,2)
        self.sprite = Sprite2DNode()
        self.sprite.texture = texture
        self.sprite.transparent_color = engine_draw.black
        self.sprite.scale = Vector2(1, 1)
        self.sprite.position = Vector2(0, 0)
        self.add_child(self.sprite)

    def update(self, position=None, texture=None):
        if position:
            self.position = position
        if texture:
            self.sprite.texture = texture

pieces = []

class GridNode(Rectangle2DNode):
    def __init__(self, grid):
        super().__init__(self)
        self.grid = grid
        self.width = DISP_WIDTH
        self.height = DISP_HEIGHT
        self.position = Vector2(0, 0)
        self.color = engine_draw.black
        self.selected_col = 0
        self.indicator = None
        texture = TextureResource("connect4.bmp")
        self.rec = Sprite2DNode()
        self.rec.scale = Vector2(1,1)
        self.rec.position = Vector2(DISP_WIDTH / 2 - 1, HEADER_HEIGHT / 2 + DISP_HEIGHT / 2 - 1)
        self.rec.texture = texture
        self.rec.transparent_color = engine_draw.black
        self.rec.set_layer(3)
        self.add_child(self.rec)
        self.holes = []
        self.floor = PhysicsRectangle2DNode(position=Vector2(DISP_WIDTH // 2,DISP_WIDTH-1.5),width=DISP_WIDTH, height=2, dynamic=False, solid=True, gravity_scale=Vector2(0,0))


    def tick(self, dt):
        global game
        if game.winner_message:
            win_text = Text2DNode(position=Vector2(DISP_WIDTH/2, DISP_HEIGHT/2),
                        font=font,
                        text=game.winner_message,
                        rotation=0,
                        scale=Vector2(1, 1),
                        opacity=1.0,
                        letter_spacing=1,
                        line_spacing=1)
            win_text.set_layer(7)
            self.add_child(win_text)
            return

    def draw_piece(self, position, texture):
        global pieces
        piece = PieceNode(position, texture)
        pieces.append(piece)
        return piece

    def draw_indicator(self, position, texture):
        if self.indicator:
            self.indicator.update(position, texture)
            self.indicator.dynamic=False
        else:
            self.indicator = self.draw_piece(position, texture)
            self.indicator.dynamic=False

        return self.indicator

    def add_piece(self, col, row, player):
        x = col * CELL_WIDTH + COLSHIFT
        y = row * CELL_HEIGHT + HEADER_HEIGHT + OFFSET
        if player == 1:
            self.draw_piece(Vector2(x, OFFSET), red_texture )
        else:
            self.draw_piece(Vector2(x, OFFSET), yellow_texture )

        

    def update_indicator(self):
        if game.current_player == 1:
            self.draw_indicator(Vector2(self.selected_col * CELL_WIDTH + COLSHIFT, OFFSET), red_texture )
        else:
            self.draw_indicator(Vector2(self.selected_col * CELL_WIDTH + COLSHIFT, OFFSET), yellow_texture )

class Game(Rectangle2DNode):
    def __init__(self, camera, selected_difficulty):
        super().__init__(self)
        self.grid = [[0 for _ in range(GRID_COLS)] for _ in range(GRID_ROWS)]
        self.grid_node = GridNode(self.grid)
        self.add_child(self.grid_node)
        self.camera = camera
        self.current_player = random.choice([1, 2])
        self.selected_col = 3
        self.grid_node.selected_col = self.selected_col
        self.winner_message = None
        self.winner_timer = 0
        self.selected_difficulty = selected_difficulty
        self.last_move_time = None

    def setCurrentPlayer(self,player):
        self.current_player=player

    def enough_time_passed(self):
        if self.last_move_time is None:
            return True
        return time.ticks_diff(time.ticks_ms(), self.last_move_time) >= 2500
    
    def tick(self, dt):
        global planned_ai_move, ai_thinking

        if self.winner_message:
            self.winner_timer -= dt
            if self.winner_timer <= 0:
                self.reset_game()
            return

        if self.check_win(1, self.grid):
            self.show_winner("Player Wins!")
            return
        if self.check_win(2, self.grid):
            self.show_winner("AI Wins!")
            return

        if self.current_player == 1:
            if engine_io.LEFT.is_just_pressed:
                self.selected_col = max(0, self.selected_col - 1)
                self.grid_node.selected_col = self.selected_col
                self.grid_node.update_indicator()
            elif engine_io.RIGHT.is_just_pressed:
                self.selected_col = min(GRID_COLS - 1, self.selected_col + 1)
                self.grid_node.selected_col = self.selected_col
                self.grid_node.update_indicator()
            elif engine_io.A.is_just_pressed:
                if self.make_move(self.selected_col, 1):
                    self.setCurrentPlayer(2)
                    self.last_move_time = time.ticks_ms() 
                    self.grid_node.update_indicator()
        elif self.current_player == 2:
            if self.enough_time_passed():
                if not ai_thinking and planned_ai_move is not None:
                    if self.make_move(planned_ai_move, 2):
                        self.setCurrentPlayer(1)
                        self.grid_node.update_indicator()
                        planned_ai_move = None
                        self.last_move_time = time.ticks_ms() 
                elif not ai_thinking and planned_ai_move is None:
                    self.ai_move()

    def show_winner(self, message):
        self.winner_message = message
        self.winner_timer = 3  # 3 seconds delay before resetting

    def reset_game(self):
        global pieces
        self.grid = [[0 for _ in range(GRID_COLS)] for _ in range(GRID_ROWS)]
        self.grid_node = GridNode(self.grid)
        pieces = []
        self.current_player = random.choice([1, 2])
        self.selected_col = 3
        self.grid_node.selected_col = self.selected_col
        self.winner_message = None
        self.winner_timer = 0
        self.grid_node.update_indicator()

    def make_move(self, col, player):
        for row in reversed(range(GRID_ROWS)):
            if self.grid[row][col] == 0:
                self.grid[row][col] = player
                self.grid_node.add_piece(col, row, player)
                return True
        return False
    
    def print_grid(self):
        for row in self.grid:
            print(' '.join(str(cell) for cell in row))
        print()

    def check_win(self, player, grid):
        for row in range(GRID_ROWS):
            for col in range(GRID_COLS):
                if col + 3 < GRID_COLS and all(grid[row][col + i] == player for i in range(4)):
                    return True
                if row + 3 < GRID_ROWS and all(grid[row + i][col] == player for i in range(4)):
                    return True
                if col + 3 < GRID_COLS and row + 3 < GRID_ROWS and all(grid[row + i][col + i] == player for i in range(4)):
                    return True
                if col + 3 < GRID_COLS and row - 3 >= 0 and all(grid[row - i][col + i] == player for i in range(4)):
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
        global planned_ai_move, ai_thinking
        ai_thinking = True

        start_time = time.ticks_ms()

        grid_copy = deepcopy_2d_list(self.grid)
        filled_cells = sum(grid_copy[row][col] != 0 for row in range(GRID_ROWS) for col in range(GRID_COLS))
        # Depth settings based on difficulty level and game stage
        depth_settings = {
            0: (0, 0, 0),  # Very Easy: (Early, Mid, Late)
            1: (1, 1, 1),  # Easy: (Early, Mid, Late)
            2: (1, 2, 2),  # Medium: (Early, Mid, Late)
            3: (2, 3, 4),   # Hard: (Early, Mid, Late)
            4: (4, 5, 5)   # Ultra
        }
        # Select depth based on the number of filled cells
        if filled_cells < 5:
            depth = depth_settings[self.selected_difficulty][0]  # Early game depth
        elif filled_cells < 10:
            depth = depth_settings[self.selected_difficulty][1]  # Mid game depth
        else:
            depth = depth_settings[self.selected_difficulty][2]  # Late game depth

        best_score = float('-inf')
        best_cols = []
        for col in range(GRID_COLS):
            row = self.get_next_open_row(grid_copy, col)
            if row is not None:
                grid_copy[row][col] = 2  # Temporarily make the move
                score = self.minimax(grid_copy, depth, False, float('-inf'), float('inf'))
                grid_copy[row][col] = 0  # Undo the move
                if score > best_score:
                    best_score = score
                    best_cols = [col]  # Reset the list with the current column
                elif score == best_score:
                    best_cols.append(col)

        planned_ai_move = random.choice(best_cols) if best_cols else random.choice(self.valid_moves(grid_copy))

        # Calculate the elapsed time
        elapsed_time = time.time() - start_time

        # Calculate the elapsed time in milliseconds
        elapsed_time = time.ticks_diff(time.ticks_ms(), start_time)

        # Sleep for the remaining time to ensure a minimum of 1000 ms (1 second)
        if elapsed_time < 1000:
            time.sleep_ms(1000 - elapsed_time)

        ai_thinking = False

    def valid_moves(self, grid):
        return [c for c in range(GRID_COLS) if grid[0][c] == 0]
    
    def get_next_open_row(self, grid, col):
        for r in range(GRID_ROWS-1, -1, -1):
            if grid[r][col] == 0:
                return r
        return None
    
    def minimax(self,grid, depth, isMaximizing, alpha, beta):
        if depth == 0 or self.check_win(1,grid) or self.check_win(2,grid):
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

def start_game(selected_difficulty):
    global game, menu, camera
    camera = CameraNode()
    camera.position = Vector3(DISP_WIDTH / 2, DISP_WIDTH / 2, 1)
    game = Game(camera,selected_difficulty)
    game.grid_node.update_indicator()
    menu = None


camera = CameraNode()
camera.position = Vector3(DISP_WIDTH / 2, DISP_WIDTH / 2, 1)
difficulty_levels = ["Very Easy", "Easy", "Medium", "Hard", "Ultra"]
menu = SimpleTextMenu("Select\nDifficulty", difficulty_levels, start_game, font, DISP_WIDTH, DISP_HEIGHT, engine_draw.white, engine_draw.yellow )
game = None

engine.start()