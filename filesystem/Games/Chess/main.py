import engine_main
import engine
import engine_draw
import engine_io
import engine_audio
import random
import time
from engine_math import Vector2, Vector3
from engine_nodes import Rectangle2DNode, CameraNode, Sprite2DNode, Text2DNode, Line2DNode
from engine_resources import FontResource, TextureResource, WaveSoundResource
from engine_animation import Tween, ONE_SHOT, EASE_SINE_IN
from engine_draw import Color

from chessengine import openings, piece_values, pstable

random.seed(time.ticks_ms())

font = FontResource("../../assets/outrunner_outline.bmp")

chess_texture = TextureResource("chess.bmp")
move_sound = WaveSoundResource("move.wav")
engine_audio.set_volume(0.25)

# Display dimensions
DISP_WIDTH = 128
DISP_HEIGHT = 128

# Cell dimensions
CELL_WIDTH = 16
CELL_HEIGHT = 16
OFFSET = CELL_WIDTH / 2

class ChessPiece:
    def __init__(self, grid_position, is_white):
        self.grid_position = grid_position
        self.is_white = is_white
        self.has_moved = False

    def valid_moves(self, board):
        return []

class King(ChessPiece):
    def valid_moves(self, board):
        moves = []
        directions = [(-1, -1), (-1, 0), (-1, 1), (0, -1), (0, 1), (1, -1), (1, 0), (1, 1)]
        for direction in directions:
            new_pos = (self.grid_position[0] + direction[0], self.grid_position[1] + direction[1])
            if 0 <= new_pos[0] < 8 and 0 <= new_pos[1] < 8:
                target_piece = board.get_piece_at_position(new_pos)
                if not target_piece or target_piece.is_white != self.is_white:
                    moves.append(new_pos)

        if not self.has_moved:
            # Kingside castling
            rook = board.get_piece_at_position((7, self.grid_position[1]))
            if isinstance(rook, Rook) and not rook.has_moved:
                if not any(board.get_piece_at_position((i, self.grid_position[1])) for i in range(5, 7)):
                    moves.append((6, self.grid_position[1]))

            # Queenside castling
            rook = board.get_piece_at_position((0, self.grid_position[1]))
            if isinstance(rook, Rook) and not rook.has_moved:
                if not any(board.get_piece_at_position((i, self.grid_position[1])) for i in range(1, 4)):
                    moves.append((2, self.grid_position[1]))

        return moves


class Queen(ChessPiece):
    def valid_moves(self, board):
        moves = []
        directions = [(-1, -1), (-1, 0), (-1, 1), (0, -1), (0, 1), (1, -1), (1, 0), (1, 1)]
        for direction in directions:
            for i in range(1, 8):
                new_pos = (self.grid_position[0] + direction[0] * i, self.grid_position[1] + direction[1] * i)
                if 0 <= new_pos[0] < 8 and 0 <= new_pos[1] < 8:
                    target_piece = board.get_piece_at_position(new_pos)
                    if target_piece:
                        if target_piece.is_white != self.is_white:
                            moves.append(new_pos)
                        break
                    moves.append(new_pos)
                else:
                    break
        return moves


class Rook(ChessPiece):
    def valid_moves(self, board):
        moves = []
        directions = [(0, -1), (0, 1), (-1, 0), (1, 0)]
        for direction in directions:
            for i in range(1, 8):
                new_pos = (self.grid_position[0] + direction[0] * i, self.grid_position[1] + direction[1] * i)
                if 0 <= new_pos[0] < 8 and 0 <= new_pos[1] < 8:
                    target_piece = board.get_piece_at_position(new_pos)
                    if target_piece:
                        if target_piece.is_white != self.is_white:
                            moves.append(new_pos)
                        break
                    moves.append(new_pos)
                else:
                    break
        return moves

class Bishop(ChessPiece):
    def valid_moves(self, board):
        moves = []
        directions = [(-1, -1), (-1, 1), (1, -1), (1, 1)]
        for direction in directions:
            for i in range(1, 8):
                new_pos = (self.grid_position[0] + direction[0] * i, self.grid_position[1] + direction[1] * i)
                if 0 <= new_pos[0] < 8 and 0 <= new_pos[1] < 8:
                    target_piece = board.get_piece_at_position(new_pos)
                    if target_piece:
                        if target_piece.is_white != self.is_white:
                            moves.append(new_pos)
                        break
                    moves.append(new_pos)
                else:
                    break
        return moves

class Knight(ChessPiece):
    def valid_moves(self, board):
        moves = []
        knight_moves = [(-2, -1), (-1, -2), (1, -2), (2, -1), (2, 1), (1, 2), (-1, 2), (-2, 1)]
        for move in knight_moves:
            new_pos = (self.grid_position[0] + move[0], self.grid_position[1] + move[1])
            if 0 <= new_pos[0] < 8 and 0 <= new_pos[1] < 8:
                target_piece = board.get_piece_at_position(new_pos)
                if not target_piece or target_piece.is_white != self.is_white:
                    moves.append(new_pos)
        return moves

class Pawn(ChessPiece):
    def __init__(self, grid_position, is_white):
        super().__init__(grid_position, is_white)
        self.en_passant_target = False

    def valid_moves(self, board):
        moves = []
        direction = -1 if self.is_white else 1
        start_row = 6 if self.is_white else 1

        # Move forward
        forward_pos = (self.grid_position[0], self.grid_position[1] + direction)
        if 0 <= forward_pos[1] < 8 and not board.get_piece_at_position(forward_pos):
            moves.append(forward_pos)
            # Double move from start position
            if self.grid_position[1] == start_row:
                double_forward_pos = (self.grid_position[0], self.grid_position[1] + 2 * direction)
                if not board.get_piece_at_position(double_forward_pos):
                    moves.append(double_forward_pos)

        # Captures
        capture_moves = [(self.grid_position[0] - 1, self.grid_position[1] + direction), 
                         (self.grid_position[0] + 1, self.grid_position[1] + direction)]
        for capture_pos in capture_moves:
            if 0 <= capture_pos[0] < 8 and 0 <= capture_pos[1] < 8:
                target_piece = board.get_piece_at_position(capture_pos)
                if target_piece and target_piece.is_white != self.is_white:
                    moves.append(capture_pos)
                # En passant capture
                elif not target_piece and isinstance(board.get_piece_at_position((capture_pos[0], self.grid_position[1])), Pawn) and \
                     board.get_piece_at_position((capture_pos[0], self.grid_position[1])).en_passant_target:
                    moves.append(capture_pos)

        return moves

class ChessBoard(Rectangle2DNode):
    def __init__(self, player_is_white):
        super().__init__(self)
        self.width = DISP_WIDTH
        self.height = DISP_HEIGHT
        self.position = Vector2(0, 0)
        self.color = engine_draw.black
        self.selected_piece = None
        self.selected_grid_position = (0, 0)
        self.highlight_layer = Rectangle2DNode()
        self.add_child(self.highlight_layer)
        self.pieces = []
        self.sprites = []
        self.squares = []
        self.layer = 0
        self.player_is_white = player_is_white
        self.draw_board()
        
        self.highlight_square_node = Rectangle2DNode()
        self.highlight_square_node.width = CELL_WIDTH
        self.highlight_square_node.height = CELL_HEIGHT
        self.highlight_square_node.color = engine_draw.blue
        self.highlight_square_node.layer = 3
        self.add_child(self.highlight_square_node)
        self.highlight_square_node.opacity = 0

        self.selected_square_node = Rectangle2DNode()
        self.selected_square_node.width = CELL_WIDTH
        self.selected_square_node.height = CELL_HEIGHT
        self.selected_square_node.color = engine_draw.yellow
        self.selected_square_node.layer = 2
        self.add_child(self.selected_square_node)
        self.selected_square_node.opacity = 0

        # AI move highlight nodes
        self.ai_move_from_highlight_node = Rectangle2DNode()
        self.ai_move_from_highlight_node.width = CELL_WIDTH
        self.ai_move_from_highlight_node.height = CELL_HEIGHT
        self.ai_move_from_highlight_node.color = engine_draw.red  # Change to desired highlight color
        self.ai_move_from_highlight_node.layer = 3
        self.add_child(self.ai_move_from_highlight_node)
        self.ai_move_from_highlight_node.opacity = 0

        self.ai_move_to_highlight_node = Rectangle2DNode()
        self.ai_move_to_highlight_node.width = CELL_WIDTH
        self.ai_move_to_highlight_node.height = CELL_HEIGHT
        self.ai_move_to_highlight_node.color = engine_draw.green  # Change to desired highlight color
        self.ai_move_to_highlight_node.layer = 3
        self.add_child(self.ai_move_to_highlight_node)
        self.ai_move_to_highlight_node.opacity = 0

    def highlight_ai_move(self, from_position, to_position):
        if self.player_is_white:
            from_pos = Vector2(from_position[0] * CELL_WIDTH + OFFSET, from_position[1] * CELL_HEIGHT + OFFSET)
            to_pos = Vector2(to_position[0] * CELL_WIDTH + OFFSET, to_position[1] * CELL_HEIGHT + OFFSET)
        else:
            from_pos = Vector2((7 - from_position[0]) * CELL_WIDTH + OFFSET, (7 - from_position[1]) * CELL_HEIGHT + OFFSET)
            to_pos = Vector2((7 - to_position[0]) * CELL_WIDTH + OFFSET, (7 - to_position[1]) * CELL_HEIGHT + OFFSET)
        self.ai_move_from_highlight_node.position = from_pos
        self.ai_move_from_highlight_node.opacity = 1
        self.ai_move_to_highlight_node.position = to_pos
        self.ai_move_to_highlight_node.opacity = 1

    def clear_ai_move_highlight(self):
        self.ai_move_from_highlight_node.opacity = 0
        self.ai_move_to_highlight_node.opacity = 0

    def draw_board(self):
        for row in range(8):
            for col in range(8):
                if self.player_is_white:
                    square_color = Color(77/256, 125/256, 210/256) if (row + col) % 2 == 1 else Color(220/256, 220/256, 230/256)
                else:
                    square_color = Color(77/256, 125/256, 210/256) if (row + col) % 2 == 0 else Color(220/256, 220/256, 230/256)
                square = Rectangle2DNode()
                square.position = Vector2(col * CELL_WIDTH + OFFSET, row * CELL_HEIGHT + OFFSET)
                square.width = CELL_WIDTH
                square.height = CELL_HEIGHT
                square.color = square_color
                square.layer = 1
                self.squares.append(square)
                self.add_child(square)

    def render_pieces(self):
        # Clear existing piece sprites
        for sprite in self.sprites:
            sprite.opacity = 0
        self.sprites = []

        # Render piece sprites based on the current board state
        for piece in self.pieces:
            sprite = Sprite2DNode(texture=chess_texture)
            if self.player_is_white:
                sprite.position = Vector2(piece.grid_position[0] * CELL_WIDTH + OFFSET, piece.grid_position[1] * CELL_HEIGHT + OFFSET)
            else:
                sprite.position = Vector2((7 - piece.grid_position[0]) * CELL_WIDTH + OFFSET, (7 - piece.grid_position[1]) * CELL_HEIGHT + OFFSET)
            sprite.layer = 5
            sprite.playing = False
            sprite.frame_count_x = 6
            sprite.frame_count_y = 2
            sprite.frame_current_x = get_piece_frame_x(piece)
            sprite.frame_current_y = 1 if piece.is_white else 0
            sprite.transparent_color = engine_draw.white
            self.add_child(sprite)
            self.sprites.append(sprite)

    def add_piece(self, piece):
        self.pieces.append(piece)

    def setup_pieces(self, is_white):
        base_row = 7 if is_white else 0
        pawn_row = 6 if is_white else 1

        # Rooks
        self.add_piece(Rook((0, base_row), is_white))
        self.add_piece(Rook((7, base_row), is_white))

        # Knights
        self.add_piece(Knight((1, base_row), is_white))
        self.add_piece(Knight((6, base_row), is_white))

        # Bishops
        self.add_piece(Bishop((2, base_row), is_white))
        self.add_piece(Bishop((5, base_row), is_white))

        # Queens
        self.add_piece(Queen((3, base_row), is_white))

        # Kings
        self.add_piece(King((4, base_row), is_white))

        # Pawns
        for i in range(8):
            self.add_piece(Pawn((i, pawn_row), is_white))

        self.render_pieces()  # Render the pieces after setting them up

    def highlight_square(self, grid_position):
        if self.player_is_white:
            position = Vector2(grid_position[0] * CELL_WIDTH + OFFSET, grid_position[1] * CELL_HEIGHT + OFFSET)
        else:
            position = Vector2((7 - grid_position[0]) * CELL_WIDTH + OFFSET, (7 - grid_position[1]) * CELL_HEIGHT + OFFSET)
        self.highlight_square_node.position = position
        self.highlight_square_node.opacity = 1

    def clear_highlight(self):
        self.highlight_square_node.opacity = 0

    def select_square(self, grid_position):
        if self.player_is_white:
            position = Vector2(grid_position[0] * CELL_WIDTH + OFFSET, grid_position[1] * CELL_HEIGHT + OFFSET)
        else:
            position = Vector2((7 - grid_position[0]) * CELL_WIDTH + OFFSET, (7 - grid_position[1]) * CELL_HEIGHT + OFFSET)
        self.selected_square_node.position = position
        self.selected_square_node.opacity = 1

    def deselect_square(self):
        self.selected_square_node.opacity = 0

    def remove_piece(self, piece):
        if piece in self.pieces:
            self.pieces.remove(piece)

    def piece_has_moved(self, piece):
        piece.has_moved = True

    def get_piece_at_position(self, position):
        for piece in self.pieces:
            if piece.grid_position == position:
                return piece
        return None
    
    def promote_pawn(self, pawn):
        # Remove the pawn
        self.remove_piece(pawn)
        # Create a new queen at the same position
        new_queen = Queen(pawn.grid_position, pawn.is_white)
        self.add_piece(new_queen)


class SimulatedChessBoard:
    def __init__(self):
        self.pieces = []
        self.piece_positions = {}  # Cache for piece positions
        self.piece_scores = {}  # Cache for piece evaluation scores

    def add_piece(self, piece):
        self.pieces.append(piece)
        self.piece_positions[piece.grid_position] = piece
        self.update_piece_score(piece)

    def remove_piece(self, piece):
        if piece in self.pieces:
            self.pieces.remove(piece)
            del self.piece_positions[piece.grid_position]
            del self.piece_scores[piece]

    def get_piece_at_position(self, position):
        return self.piece_positions.get(position, None)

    def piece_has_moved(self, piece):
        piece.has_moved = True

    def copy_from_board(self, board):
        self.pieces = [piece.__class__(piece.grid_position, piece.is_white) for piece in board.pieces]
        self.piece_positions = {piece.grid_position: piece for piece in self.pieces}
        self.piece_scores = {}
        for piece, original_piece in zip(self.pieces, board.pieces):
            piece.has_moved = original_piece.has_moved
            if isinstance(piece, Pawn):
                piece.en_passant_target = original_piece.en_passant_target
            self.update_piece_score(piece)

    def make_move(self, from_pos, to_pos):
        piece = self.get_piece_at_position(from_pos)
        captured_piece = self.get_piece_at_position(to_pos)
        if piece:
            if captured_piece:
                self.remove_piece(captured_piece)
            piece.grid_position = to_pos
            self.piece_positions[to_pos] = piece
            del self.piece_positions[from_pos]
            self.update_piece_score(piece)
            self.piece_has_moved(piece)
        return piece, captured_piece

    def undo_move(self, piece, captured_piece, from_pos, to_pos):
        if piece:
            piece.grid_position = from_pos
            piece.has_moved = False  # TODO: - should reset this to what it was
            self.piece_positions[from_pos] = piece
            del self.piece_positions[to_pos]
            self.update_piece_score(piece)
        if captured_piece:
            self.add_piece(captured_piece)

    def update_piece_score(self, piece):
        piece_char = get_piece_char(piece)
        piece_value = piece_values[piece_char.upper()]
        pos_idx = piece.grid_position[1] * 8 + piece.grid_position[0]
        if piece.is_white:
            pstable_value = pstable[piece_char.upper()][pos_idx]
            self.piece_scores[piece] = piece_value + pstable_value
        else:
            pstable_value = pstable[piece_char.upper()][(7 - piece.grid_position[1]) * 8 + (7 - piece.grid_position[0])]
            self.piece_scores[piece] = -(piece_value + pstable_value)



class ChessGame(Rectangle2DNode):
    def __init__(self, camera, player_is_white):
        super().__init__(self)
        self.camera = camera
        self.board = ChessBoard(player_is_white)
        self.add_child(self.board)
        self.layer = 0
        self.current_player_is_white = True
        self.selected_piece = None
        self.winner_message = None
        self.board.setup_pieces(is_white=False)  # Black pieces
        self.board.setup_pieces(is_white=True)   # White pieces
        self.print_board_state()
        self.move_mode = False
        self.process_ai_move = False
        self.post_ai_check = False
        self.last_move = None
        self.moves = []
        self.endgame = False
        self.ai_in_check_cant_castle = None
        # Choose a random opening if the AI is white
        self.ai_opening_moves = None
        self.player_is_white = player_is_white
        if self.player_is_white:
            self.selected_grid_position = (4, 6)
        else:
            self.selected_grid_position = (7 - 4, 7 - 6)
        if not self.player_is_white:
            self.ai_opening_moves =random.choice(list(openings.values()))

        # Initialize evaluation lines
        self.white_evaluation_line = Line2DNode(start=Vector2(0, DISP_HEIGHT), end=Vector2(0, DISP_HEIGHT), thickness=2, color=engine_draw.white, opacity=1.0, outline=False)
        self.black_evaluation_line = Line2DNode(start=Vector2(0, 0), end=Vector2(0, 0), thickness=2, color=engine_draw.black, opacity=1.0, outline=False)
        self.white_evaluation_line.layer = 6
        self.black_evaluation_line.layer = 6
        self.add_child(self.white_evaluation_line)
        self.add_child(self.black_evaluation_line)

    def update_endgame_flag(self):
        white_material_value = sum(piece_values[get_piece_char(piece).upper()] for piece in self.board.pieces if piece.is_white and not isinstance(piece, King))
        black_material_value = sum(piece_values[get_piece_char(piece).upper()] for piece in self.board.pieces if not piece.is_white and not isinstance(piece, King))
        
        endgame_threshold = 800  # Adjust this threshold based on testing and observations
        self.endgame = white_material_value <= endgame_threshold and black_material_value > white_material_value


    def update_evaluation_line(self, evaluation_score):
        max_score = 1000
        min_score = -1000

        # Clamp the evaluation score
        clamped_score = max(min(evaluation_score, max_score), min_score)

        normalized_score = (clamped_score - min_score) / (max_score - min_score)
        y_pos = DISP_HEIGHT * (1 - normalized_score)  # Invert to match coordinate system

        if self.player_is_white:
            white_line_start_pos = Vector2(0, DISP_HEIGHT)
            black_line_start_pos = Vector2(0, 0)
            white_line_end_pos = Vector2(0, y_pos)
            black_line_end_pos = Vector2(0, y_pos)
        else:
            white_line_start_pos = Vector2(0, 0)
            black_line_start_pos = Vector2(0, DISP_HEIGHT)
            white_line_end_pos = Vector2(0, DISP_HEIGHT - y_pos)
            black_line_end_pos = Vector2(0, DISP_HEIGHT - y_pos)

        # Update the positions of the evaluation lines
        self.white_evaluation_line.start = white_line_start_pos
        self.white_evaluation_line.end = white_line_end_pos
        self.black_evaluation_line.start = black_line_start_pos
        self.black_evaluation_line.end = black_line_end_pos

    def tick(self, dt):
        ai_turn = self.current_player_is_white != self.player_is_white

        if self.winner_message:
            win_text = Text2DNode(position=Vector2(DISP_WIDTH/2, DISP_HEIGHT/2),
                        font=font,
                        text=self.winner_message,
                        rotation=0,
                        scale=Vector2(1, 1),
                        opacity=1.0,
                        letter_spacing=1,
                        line_spacing=1)
            win_text.layer = 7
            self.add_child(win_text)
            return

        self.board.clear_highlight()
        if self.move_mode and self.selected_piece:
            self.board.highlight_square(self.selected_piece.grid_position)
        self.board.select_square(self.selected_grid_position)

        if not ai_turn and self.post_ai_check:
            simulated_board = SimulatedChessBoard()
            simulated_board.copy_from_board(self.board)
            is_in_check = minimax_check(simulated_board, self.current_player_is_white)
            all_moves = get_all_valid_moves(simulated_board, self.current_player_is_white)
            can_move = False
            for piece, move in all_moves:
                if not self.move_puts_king_in_check(piece, move):
                    can_move = True
                    break
            if not can_move:
                if is_in_check:
                    self.winner_message=("AI Wins")
                else:
                    self.winner_message=("Stalemate")
            self.post_ai_check = False

        if engine_io.LEFT.is_just_pressed:
            self.move_cursor((-1, 0))
        elif engine_io.RIGHT.is_just_pressed:
            self.move_cursor((1, 0))
        elif engine_io.UP.is_just_pressed:
            self.move_cursor((0, -1))
        elif engine_io.DOWN.is_just_pressed:
            self.move_cursor((0, 1))
        elif engine_io.A.is_just_pressed:
            print(f"Move AI Turn: {ai_turn}")
            self.select_or_move_piece()
        elif engine_io.B.is_just_pressed:
            self.deselect_piece()

        # Process the AI move if the flag is set
        if ai_turn and self.process_ai_move:
            print("AI going to make move")
            print(f"Move AI Turn: {ai_turn}")
            self.process_ai_move = False  # Reset the flag
            self.make_ai_move()
            return

        # Set the flag if it's not the player's turn and no piece is selected
        if ai_turn and not self.selected_piece and not self.process_ai_move:
            print("Preparing for AI move next turn")
            self.process_ai_move = True  # Set the flag

    def move_cursor(self, direction):
        if not self.player_is_white:
            direction=(direction[0]*-1,direction[1]*-1)
        new_col = self.selected_grid_position[0] + direction[0]
        new_row = self.selected_grid_position[1] + direction[1]
        if 0 <= new_col < 8 and 0 <= new_row < 8:
            self.selected_grid_position = (new_col, new_row)

    def select_or_move_piece(self):
        self.board.clear_ai_move_highlight()
        col, row = self.selected_grid_position
        if self.selected_piece:
            new_col, new_row = self.selected_grid_position
            if (new_col, new_row) != self.selected_piece.grid_position:
                if (new_col, new_row) in self.selected_piece.valid_moves(self.board):
                    if not self.move_puts_king_in_check(self.selected_piece, (new_col, new_row)):
                        self.make_move(self.selected_piece, (new_col, new_row))
        else:
            selected_piece = self.board.get_piece_at_position((col, row))
            if selected_piece and selected_piece.is_white == self.current_player_is_white:
                self.selected_piece = selected_piece
                self.move_mode = True

    def move_puts_king_in_check(self, piece, to_pos):
        simulated_board = SimulatedChessBoard()
        simulated_board.copy_from_board(self.board)
        from_pos = piece.grid_position
        moved_piece, captured_piece = simulated_board.make_move(from_pos, to_pos)
        is_in_check = minimax_check(simulated_board, piece.is_white)
        simulated_board.undo_move(moved_piece, captured_piece, from_pos, to_pos)
        return is_in_check

    def deselect_piece(self):
        if self.selected_piece:
            self.selected_piece = None
            self.move_mode = False

    def print_board_state(self):
        #print(board_to_string(self.board))
        return

    def make_ai_move(self):
        opening_move = None
        if self.current_player_is_white and self.ai_opening_moves and len(self.moves) < 4:
            # start with a book opening
            opening_move_index = len(self.moves)
            if opening_move_index < len(self.ai_opening_moves):
                opening_move = self.ai_opening_moves[opening_move_index]
        else:
            # Check if the current moves match any opening
            matched_openings = [(name, moves) for name, moves in openings.items() if self.moves == moves[:len(self.moves)]]
            
            if matched_openings:
                # Select a random opening from the matched openings
                opening_name, opening_moves = random.choice(matched_openings)
                opening_move = opening_moves[len(self.moves)]
                print(opening_name)

        if opening_move:
            # Play the next move in the opening
            piece_type, from_pos, to_pos = self.algebraic_to_positions(opening_move, self.current_player_is_white)
            if from_pos is None:
                # If from_pos is not determined, find the piece based on the to_pos and type
                for p in self.board.pieces:
                    if p.is_white != self.current_player_is_white:
                        continue
                    if piece_type and get_piece_notation(p) != piece_type:
                        continue
                    if to_pos in p.valid_moves(self.board):
                        from_pos = p.grid_position
                        break
        else:
            simulated_board = SimulatedChessBoard()
            simulated_board.copy_from_board(self.board)
            # Update endgame flag
            self.update_endgame_flag()
            # Use minimax if no opening is tracked or opening moves are exhausted
            depth = 2
            if self.endgame:
                depth = 3
            eval_score, best_move = minimax(simulated_board, depth=depth, is_white=not self.player_is_white, alpha=float('-inf'), beta=float('inf'), endgame=self.endgame, cant_castle=self.ai_in_check_cant_castle)
            if eval_score > CHECKMATE_SCORE:
                is_in_check = minimax_check(simulated_board, self.current_player_is_white)
                if is_in_check:
                    self.winner_message = "Player Wins"
                else:
                    self.winner_message = "Stalemate"
                return
            p, to_pos = best_move
            from_pos = p.grid_position

        # Execute the AI move
        self.execute_move(from_pos, to_pos)
        self.board.render_pieces()
        self.post_ai_check = True

    def execute_move(self, from_pos, to_pos):
        piece = self.board.get_piece_at_position(from_pos)
        if piece:
            if isinstance(piece, King):
                if to_pos[0] == 6 and not piece.has_moved or to_pos[0] == 2 and not piece.has_moved:  # castling
                    simulated_board = SimulatedChessBoard()
                    simulated_board.copy_from_board(self.board)
                    is_in_check = minimax_check(simulated_board, self.current_player_is_white)
                    if is_in_check:
                        self.ai_in_check_cant_castle = (from_pos, to_pos)
                        return
            self.ai_in_check_cant_castle = None
            self.board.highlight_ai_move(from_pos, to_pos)
            self.make_move(piece, to_pos)
            
    def make_move(self, piece, to_pos):
        print(f"Make Move current_player_is_white: {self.current_player_is_white}")
        from_pos = piece.grid_position
        captured_piece = self.board.get_piece_at_position(to_pos)
        original_position = piece.grid_position

        # Generate move notation before updating the board state
        move_notation = generate_move_notation(piece, original_position, to_pos, self.board)

        if captured_piece and captured_piece.is_white != piece.is_white:
            self.board.remove_piece(captured_piece)

        # Special handling for castling
        if isinstance(piece, King):
            if to_pos[0] == 6 and not piece.has_moved:  # Kingside castling
                rook = self.board.get_piece_at_position((7, from_pos[1]))
                if rook and isinstance(rook, Rook) and not rook.has_moved:
                    rook.grid_position = (5, from_pos[1])
                    self.board.piece_has_moved(rook)
            elif to_pos[0] == 2 and not piece.has_moved:  # Queenside castling
                rook = self.board.get_piece_at_position((0, from_pos[1]))
                if rook and isinstance(rook, Rook) and not rook.has_moved:
                    rook.grid_position = (3, from_pos[1])
                    self.board.piece_has_moved(rook)

        # Special handling for en passant
        if isinstance(piece, Pawn):
            if abs(to_pos[1] - piece.grid_position[1]) == 2:
                piece.en_passant_target = True
            else:
                piece.en_passant_target = False
            # Check and remove en passant captured pawn
            if (to_pos[0], to_pos[1]) != piece.grid_position:
                if not captured_piece and abs(to_pos[0] - piece.grid_position[0]) == 1:
                    captured_pawn = self.board.get_piece_at_position((to_pos[0], from_pos[1]))
                    if captured_pawn and isinstance(captured_pawn, Pawn) and captured_pawn.en_passant_target:
                        self.board.remove_piece(captured_pawn)

        piece.grid_position = to_pos
        self.board.piece_has_moved(piece)

        # Handle pawn promotion
        if isinstance(piece, Pawn) and (to_pos[1] == 0 or to_pos[1] == 7):
            self.board.promote_pawn(piece)
            move_notation = f"{move_notation}=Q"

        # Track and print move
        self.moves.append(move_notation)
        print(generate_pgn_moves_list(self.moves))

        # Check for opening
        self.opening_name = check_opening(self.moves)
        if self.opening_name:
            print(self.opening_name)

        # Evaluate board and update evaluation line
        simulated_board = SimulatedChessBoard()
        simulated_board.copy_from_board(self.board)
        evaluation_score = evaluate_board(simulated_board)
        self.update_evaluation_line(evaluation_score)

        self.selected_piece = None
        self.move_mode = False
        self.current_player_is_white = not self.current_player_is_white
        engine_audio.play(move_sound, 0, False)
        self.print_board_state()
        self.board.render_pieces()
        self.last_move = (from_pos, to_pos)



    def algebraic_to_positions(self, move, is_white):
        # Handle castling
        if move == 'O-O':
            # Kingside castling
            return 'K', (4, 7 if is_white else 0), (6, 7 if is_white else 0)
        elif move == 'O-O-O':
            # Queenside castling
            return 'K', (4, 7 if is_white else 0), (2, 7 if is_white else 0)

        piece_type = ''
        if move[0].isupper() and move[0] != 'x':
            piece_type = move[0]
            move = move[1:]

        capture = 'x' in move
        if capture:
            parts = move.split('x')
            if len(parts) != 2:
                raise ValueError("Invalid move notation")
            from_file = parts[0] if piece_type == '' else ''
            to_pos = parts[1]
        else:
            from_file = ''
            to_pos = move

        to_pos = (ord(to_pos[0]) - ord('a'), 8 - int(to_pos[1]))
        from_pos = None
        possible_pieces = []

        if piece_type == '':  # Handle pawn moves
            for piece in self.board.pieces:
                if isinstance(piece, Pawn) and piece.is_white == is_white:
                    valid_moves = piece.valid_moves(self.board)
                    if to_pos in valid_moves:
                        if not from_file or piece.grid_position[0] == (ord(from_file) - ord('a')):
                            possible_pieces.append(piece)
        else:
            for piece in self.board.pieces:
                if piece.is_white == is_white and get_piece_notation(piece) == piece_type:
                    valid_moves = piece.valid_moves(self.board)
                    if to_pos in valid_moves:
                        if not from_file or piece.grid_position[0] == (ord(from_file) - ord('a')):
                            possible_pieces.append(piece)

        if len(possible_pieces) == 1:
            from_pos = possible_pieces[0].grid_position
        elif len(possible_pieces) > 1:
            for piece in possible_pieces:
                if piece.grid_position[0] == (ord(from_file) - ord('a')):
                    from_pos = piece.grid_position
                    break
            if from_pos is None:
                raise ValueError("Ambiguous move notation")

        return piece_type, from_pos, to_pos

def generate_move_notation(piece, from_pos, to_pos, board):
    piece_notation = get_piece_notation(piece)
    target_piece = board.get_piece_at_position(to_pos)
    capture_notation = 'x' if target_piece else ''

    # Handle castling
    if isinstance(piece, King) and abs(to_pos[0] - from_pos[0]) == 2:
        if to_pos[0] == 6:
            return 'O-O'
        elif to_pos[0] == 2:
            return 'O-O-O'

    # Handle pawn moves
    if isinstance(piece, Pawn):
        if capture_notation:
            return position_to_algebraic(from_pos)[0] + 'x' + position_to_algebraic(to_pos)
        else:
            return position_to_algebraic(to_pos)

    # Disambiguation
    disambiguation = ''
    for other_piece in board.pieces:
        if other_piece != piece and type(other_piece) == type(piece) and other_piece.is_white == piece.is_white:
            if to_pos in other_piece.valid_moves(board):
                if from_pos[0] != other_piece.grid_position[0]:
                    disambiguation = position_to_algebraic(from_pos)[0]
                else:
                    disambiguation = str(8 - from_pos[1])

    return piece_notation + disambiguation + capture_notation + position_to_algebraic(to_pos)

def generate_pgn_moves_list(moves):
    pgn_moves = []
    for i in range(0, len(moves), 2):
        move_pair = str(i // 2 + 1) + '. ' + moves[i]
        if i + 1 < len(moves):
            move_pair += ' ' + moves[i + 1]
        pgn_moves.append(move_pair)
    return ' '.join(pgn_moves)



def position_to_algebraic(pos):
    return chr(pos[0] + ord('a')) + str(8 - pos[1])

def board_to_string(board):
    board_state = [["." for _ in range(8)] for _ in range(8)]
    for piece in board.pieces:
        col, row = piece.grid_position
        piece_char = get_piece_char(piece)
        board_state[row][col] = piece_char

    return "\n".join(["".join(row) for row in board_state])

def get_piece_notation(piece):
    if isinstance(piece, King):
        return 'K'
    elif isinstance(piece, Queen):
        return 'Q'
    elif isinstance(piece, Rook):
        return 'R'
    elif isinstance(piece, Bishop):
        return 'B'
    elif isinstance(piece, Knight):
        return 'N'
    elif isinstance(piece, Pawn):
        return ''
    return ''

def get_piece_char(piece):
    piece_type = type(piece)
    if piece_type == King:
        return 'K' if piece.is_white else 'k'
    elif piece_type == Queen:
        return 'Q' if piece.is_white else 'q'
    elif piece_type == Rook:
        return 'R' if piece.is_white else 'r'
    elif piece_type == Bishop:
        return 'B' if piece.is_white else 'b'
    elif piece_type == Knight:
        return 'N' if piece.is_white else 'n'
    elif piece_type == Pawn:
        return 'P' if piece.is_white else 'p'
    return '.'


def check_opening(moves):
    for opening_name, opening_moves in openings.items():
        if moves == opening_moves[:len(moves)]:
            return opening_name
    return None


def evaluate_board(board, endgame=False):
    score = 0
    for piece_score in board.piece_scores.values():
        score += piece_score
    
    if endgame:
        # Add endgame-specific heuristics
        score += king_proximity_score(board, True)

    return score

def king_proximity_score(board, is_white):
    opponent_king = None
    for piece in board.pieces:
        if isinstance(piece, King) and piece.is_white != is_white:
            opponent_king = piece
            break
    if not opponent_king:
        return 0
    
    score = 0
    for piece in board.pieces:
        if piece.is_white == is_white:
            distance = abs(piece.grid_position[0] - opponent_king.grid_position[0]) + abs(piece.grid_position[1] - opponent_king.grid_position[1])
            score -= distance  # Closer pieces get higher scores (negative distance)
    
    return score

def get_all_valid_moves(board, is_white):
    moves = []
    for piece in board.pieces:
        if piece.is_white == is_white:
            valid_moves = piece.valid_moves(board)
            for move in valid_moves:
                moves.append((piece, move))
    # Sort moves to prioritize captures
    moves.sort(key=lambda move: board.get_piece_at_position(move[1]) is not None, reverse=True)
    return moves

CHECKMATE_SCORE = 40000
STALEMATE_SCORE = 0

def minimax_check(board, is_white):
    eval_score, _ = minimax(board, depth=1, is_white=not is_white, alpha=float('-inf'), beta=float('inf'))
    return abs(eval_score) >= CHECKMATE_SCORE

def minimax(board, depth, is_white, alpha, beta, endgame=False, cant_castle=None):
    if depth == 0:
        return evaluate_board(board, endgame), None

    best_move = None
    all_moves = get_all_valid_moves(board, is_white)

    if not all_moves:
        eval_score = evaluate_board(board)
        if abs(eval_score) >= CHECKMATE_SCORE:
            return eval_score, None
        return STALEMATE_SCORE, None

    if is_white:
        max_eval = float('-inf')
        for piece, move in all_moves:
            from_pos = piece.grid_position
            to_pos = move
            moved_piece, captured_piece = board.make_move(from_pos, to_pos)
            eval, _ = minimax(board, depth - 1, False, alpha, beta, endgame)
            board.undo_move(moved_piece, captured_piece, from_pos, to_pos)
            if eval > max_eval:
                max_eval = eval
                best_move = (piece, move)
            alpha = max(alpha, eval)
            if beta <= alpha:
                break
        return max_eval, best_move
    else:
        min_eval = float('inf')
        for piece, move in all_moves:
            from_pos = piece.grid_position
            to_pos = move
            if (from_pos, to_pos) == cant_castle:
                continue
            moved_piece, captured_piece = board.make_move(from_pos, to_pos)
            eval, _ = minimax(board, depth - 1, True, alpha, beta, endgame)
            board.undo_move(moved_piece, captured_piece, from_pos, to_pos)
            if eval < min_eval:
                min_eval = eval
                best_move = (piece, move)
            beta = min(beta, eval)
            if beta <= alpha:
                break
        return min_eval, best_move

def get_piece_frame_x(piece):
    if isinstance(piece, King):
        return 3
    elif isinstance(piece, Queen):
        return 4
    elif isinstance(piece, Rook):
        return 0
    elif isinstance(piece, Bishop):
        return 2
    elif isinstance(piece, Knight):
        return 1
    elif isinstance(piece, Pawn):
        return 5
    return 0

def start_chess_game():
    global game, camera
    camera = CameraNode()
    camera.position = Vector3(DISP_WIDTH / 2, DISP_WIDTH / 2, 1)
    player_is_white = random.choice([True, False])
    game = ChessGame(camera, player_is_white)

camera = CameraNode()
camera.position = Vector3(DISP_WIDTH / 2, DISP_WIDTH / 2, 1)
game = None
start_chess_game()

engine.start()
