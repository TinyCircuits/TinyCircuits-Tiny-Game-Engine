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

CHECKMATE_THRESHOLD = 100000

class ChessPiece(Sprite2DNode):
    def __init__(self, grid_position, texture, frame_x, frame_y, is_white):
        super().__init__(self)
        self.grid_position = grid_position
        self.texture = texture
        self.layer = 5
        self.frame_count_x = 6
        self.frame_count_y = 2
        self.frame_current_x = frame_x
        self.frame_current_y = frame_y
        self.is_white = is_white
        self.playing = False
        self.transparent_color = engine_draw.white
        self.update_position()

    def update_position(self):
        self.position = Vector2(self.grid_position[0] * CELL_WIDTH + OFFSET, self.grid_position[1] * CELL_HEIGHT + OFFSET)

    def valid_moves(self, board):
        return []

class King(ChessPiece):
    def __init__(self, grid_position, texture, is_white):
        frame_x = 3
        frame_y = 1 if is_white else 0
        super().__init__(grid_position, texture, frame_x, frame_y, is_white)
        self.has_moved = False

    def valid_moves(self, board):
        moves = []
        directions = [(-1, -1), (-1, 0), (-1, 1), (0, -1), (0, 1), (1, -1), (1, 0), (1, 1)]
        for direction in directions:
            new_pos = (self.grid_position[0] + direction[0], self.grid_position[1] + direction[1])
            if 0 <= new_pos[0] < 8 and 0 <= new_pos[1] < 8:
                target_piece = board.get_piece_at_position(new_pos)
                if not target_piece or target_piece.is_white != self.is_white:
                    moves.append(new_pos)
        
        # Castling
        if not self.has_moved:
            # Kingside castling
            if isinstance(board.get_piece_at_position((7, self.grid_position[1])), Rook) and \
               not board.get_piece_at_position((5, self.grid_position[1])) and \
               not board.get_piece_at_position((6, self.grid_position[1])) and \
               not board.get_piece_at_position((7, self.grid_position[1])).has_moved:
                moves.append((6, self.grid_position[1]))

            # Queenside castling
            if isinstance(board.get_piece_at_position((0, self.grid_position[1])), Rook) and \
               not board.get_piece_at_position((1, self.grid_position[1])) and \
               not board.get_piece_at_position((2, self.grid_position[1])) and \
               not board.get_piece_at_position((3, self.grid_position[1])) and \
               not board.get_piece_at_position((0, self.grid_position[1])).has_moved:
                moves.append((2, self.grid_position[1]))

        return moves

class Queen(ChessPiece):
    def __init__(self, grid_position, texture, is_white):
        frame_x = 4
        frame_y = 1 if is_white else 0
        super().__init__(grid_position, texture, frame_x, frame_y, is_white)

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
    def __init__(self, grid_position, texture, is_white):
        frame_x = 0
        frame_y = 1 if is_white else 0
        super().__init__(grid_position, texture, frame_x, frame_y, is_white)
        self.has_moved = False

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
    def __init__(self, grid_position, texture, is_white):
        frame_x = 2
        frame_y = 1 if is_white else 0
        super().__init__(grid_position, texture, frame_x, frame_y, is_white)

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
    def __init__(self, grid_position, texture, is_white):
        frame_x = 1
        frame_y = 1 if is_white else 0
        super().__init__(grid_position, texture, frame_x, frame_y, is_white)

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
    def __init__(self, grid_position, texture, is_white):
        frame_x = 5
        frame_y = 1 if is_white else 0
        super().__init__(grid_position, texture, frame_x, frame_y, is_white)
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
    def __init__(self):
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
        self.squares = []
        self.layer = 0
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
        self.ai_move_from_highlight_node.position = Vector2(from_position[0] * CELL_WIDTH + OFFSET, from_position[1] * CELL_HEIGHT + OFFSET)
        self.ai_move_from_highlight_node.opacity = 1

        self.ai_move_to_highlight_node.position = Vector2(to_position[0] * CELL_WIDTH + OFFSET, to_position[1] * CELL_HEIGHT + OFFSET)
        self.ai_move_to_highlight_node.opacity = 1

    def clear_ai_move_highlight(self):
        self.ai_move_from_highlight_node.opacity = 0
        self.ai_move_to_highlight_node.opacity = 0

    def draw_board(self):
        for row in range(8):
            for col in range(8):
                square_color = Color(77/256,125/256,210/256) if (row + col) % 2 == 1 else Color(220/256,220/256,230/256)
                square = Rectangle2DNode()
                square.position = Vector2(col * CELL_WIDTH + OFFSET, row * CELL_HEIGHT + OFFSET)
                square.width = CELL_WIDTH
                square.height = CELL_HEIGHT
                square.color = square_color
                square.layer = 1
                self.squares.append(square)
                self.add_child(square)

    def add_piece(self, piece):
        self.pieces.append(piece)
        self.add_child(piece)

    def setup_pieces(self, is_white):
        base_row = 7 if is_white else 0
        pawn_row = 6 if is_white else 1

        # Rooks
        self.add_piece(Rook((0, base_row), chess_texture, is_white))
        self.add_piece(Rook((7, base_row), chess_texture, is_white))

        # Knights
        self.add_piece(Knight((1, base_row), chess_texture, is_white))
        self.add_piece(Knight((6, base_row), chess_texture, is_white))

        # Bishops
        self.add_piece(Bishop((2, base_row), chess_texture, is_white))
        self.add_piece(Bishop((5, base_row), chess_texture, is_white))

        # Queens
        self.add_piece(Queen((3, base_row), chess_texture, is_white))

        # Kings
        self.add_piece(King((4, base_row), chess_texture, is_white))

        # Pawns
        for i in range(8):
            self.add_piece(Pawn((i, pawn_row), chess_texture, is_white))

    def highlight_square(self, grid_position):
        self.highlight_square_node.position = Vector2(grid_position[0] * CELL_WIDTH + OFFSET, grid_position[1] * CELL_HEIGHT + OFFSET)
        self.highlight_square_node.opacity = 1

    def clear_highlight(self):
        self.highlight_square_node.opacity = 0

    def select_square(self, grid_position):
        self.selected_square_node.position = Vector2(grid_position[0] * CELL_WIDTH + OFFSET, grid_position[1] * CELL_HEIGHT + OFFSET)
        self.selected_square_node.opacity = 1

    def deselect_square(self):
        self.selected_square_node.opacity = 0

    def remove_piece(self, piece):
        if piece in self.pieces:
            piece.opacity = 0
            self.pieces.remove(piece)
            piece = None

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
        new_queen = Queen(pawn.grid_position, chess_texture, pawn.is_white)
        self.add_piece(new_queen)

class ChessGame(Rectangle2DNode):
    def __init__(self, camera):
        super().__init__(self)
        self.camera = camera
        self.board = ChessBoard()
        self.add_child(self.board)
        self.layer = 0
        self.current_player_is_white = True
        self.selected_piece = None
        self.winner_message = None
        self.board.setup_pieces(is_white=False)  # Black pieces
        self.board.setup_pieces(is_white=True)   # White pieces
        self.selected_grid_position = (4, 6)
        self.print_board_state()
        self.move_mode = False
        self.process_ai_move = False
        self.last_move = None
        self.moves = []

        # Initialize evaluation lines
        self.white_evaluation_line = Line2DNode(start=Vector2(0, DISP_HEIGHT), end=Vector2(0, DISP_HEIGHT), thickness=2, color=engine_draw.white, opacity=1.0, outline=False)
        self.black_evaluation_line = Line2DNode(start=Vector2(0, 0), end=Vector2(0, 0), thickness=2, color=engine_draw.black, opacity=1.0, outline=False)
        self.white_evaluation_line.layer = 6
        self.black_evaluation_line.layer = 6
        self.add_child(self.white_evaluation_line)
        self.add_child(self.black_evaluation_line)

    def update_evaluation_line(self, evaluation_score):
        max_score = 1000
        min_score = -1000

        # Clamp the evaluation score
        clamped_score = max(min(evaluation_score, max_score), min_score)

        normalized_score = (clamped_score - min_score) / (max_score - min_score)
        y_pos = DISP_HEIGHT * (1 - normalized_score)  # Invert to match coordinate system

        white_line_end_pos = Vector2(0, y_pos)
        black_line_end_pos = Vector2(0, y_pos)


        # Update the positions of the evaluation lines
        self.white_evaluation_line.end = white_line_end_pos
        self.black_evaluation_line.end = black_line_end_pos

    def tick(self, dt):
        if self.winner_message:
            win_text = Text2DNode(position=Vector2(DISP_WIDTH/2, DISP_HEIGHT/2),
                        font=font,
                        text=game.winner_message,
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

        if engine_io.LEFT.is_just_pressed:
            self.move_cursor((-1, 0))
        elif engine_io.RIGHT.is_just_pressed:
            self.move_cursor((1, 0))
        elif engine_io.UP.is_just_pressed:
            self.move_cursor((0, -1))
        elif engine_io.DOWN.is_just_pressed:
            self.move_cursor((0, 1))
        elif engine_io.A.is_just_pressed:
            self.select_or_move_piece()
        elif engine_io.B.is_just_pressed:
            self.deselect_piece()

        # Process the AI move if the flag is set
        if self.process_ai_move:
            self.process_ai_move = False  # Reset the flag
            self.make_ai_move()

        # Set the flag if it's not the player's turn and no piece is selected
        if not self.current_player_is_white and not self.selected_piece and not self.process_ai_move:
            self.process_ai_move = True  # Set the flag

    def move_cursor(self, direction):
        self.board.clear_ai_move_highlight()
        new_col = self.selected_grid_position[0] + direction[0]
        new_row = self.selected_grid_position[1] + direction[1]
        if 0 <= new_col < 8 and 0 <= new_row < 8:
            self.selected_grid_position = (new_col, new_row)

    def select_or_move_piece(self):
            col, row = self.selected_grid_position

            if self.selected_piece:
                new_col, new_row = self.selected_grid_position
                if (new_col, new_row) != self.selected_piece.grid_position:
                    if (new_col, new_row) in self.selected_piece.valid_moves(self.board):
                        original_position = self.selected_piece.grid_position
                        captured_piece = self.board.get_piece_at_position((new_col, new_row))
                        
                        # Generate move notation before updating the board state
                        move_notation = generate_move_notation(self.selected_piece, original_position, (new_col, new_row), self.board)
                        
                        if captured_piece and captured_piece.is_white != self.selected_piece.is_white:
                            self.board.remove_piece(captured_piece)
                        
                        # Special handling for castling
                        if isinstance(self.selected_piece, King):
                            if new_col == 6 and not self.selected_piece.has_moved:  # Kingside castling
                                rook = self.board.get_piece_at_position((7, row))
                                rook.grid_position = (5, row)
                                rook.update_position()
                            elif new_col == 2 and not self.selected_piece.has_moved:  # Queenside castling
                                rook = self.board.get_piece_at_position((0, row))
                                rook.grid_position = (3, row)
                                rook.update_position()
                        
                        # Special handling for en passant
                        if isinstance(self.selected_piece, Pawn):
                            if abs(new_row - self.selected_piece.grid_position[1]) == 2:
                                self.selected_piece.en_passant_target = True
                            else:
                                self.selected_piece.en_passant_target = False
                            # Check and remove en passant captured pawn
                            if (new_col, new_row) != self.selected_piece.grid_position:
                                if not captured_piece and abs(new_col - self.selected_piece.grid_position[0]) == 1:
                                    captured_pawn = self.board.get_piece_at_position((new_col, row))
                                    if captured_pawn and isinstance(captured_pawn, Pawn) and captured_pawn.en_passant_target:
                                        self.board.remove_piece(captured_pawn)

                        self.selected_piece.grid_position = (new_col, new_row)
                        self.selected_piece.update_position()
                        self.board.piece_has_moved(self.selected_piece)

                        # Check for check
                        if self.is_in_check(self.current_player_is_white):
                            self.selected_piece.grid_position = original_position
                            self.selected_piece.update_position()
                            if captured_piece:
                                captured_piece.opacity = 1
                                self.board.add_piece(captured_piece)
                        else:
                            # Handle pawn promotion
                            if isinstance(self.selected_piece, Pawn) and (new_row == 0 or new_row == 7):
                                self.board.promote_pawn(self.selected_piece)

                            # Track and print move
                            self.moves.append(move_notation)
                            print(self.moves)

                            # Check for opening
                            self.opening_name = check_opening(self.moves)
                            if self.opening_name:
                                print(self.opening_name)

                            # Evaluate board and update evaluation line
                            board_str = board_to_string(self.board)
                            evaluation_score = evaluate_board(board_str)
                            self.update_evaluation_line(evaluation_score)

                            self.selected_piece = None
                            self.move_mode = False
                            self.current_player_is_white = not self.current_player_is_white
                            engine_audio.play(move_sound, 0, False)
                            self.print_board_state()
                            self.last_move = ((col, row), (new_col, new_row))

                            # Check for checkmate
                            white_checkmate, black_checkmate = is_checkmate(self.board)
                            if white_checkmate:
                                self.winner_message = "Checkmate AI!"
                                return
                            if black_checkmate:
                                self.winner_message = "Checkmate Win!"
                                return
            else:
                selected_piece = self.board.get_piece_at_position((col, row))
                if selected_piece and selected_piece.is_white == self.current_player_is_white:
                    self.selected_piece = selected_piece
                    self.move_mode = True

    def is_in_check(self, is_white):
        king_position = None
        for piece in self.board.pieces:
            if isinstance(piece, King) and piece.is_white == is_white:
                king_position = piece.grid_position
                break
        
        for piece in self.board.pieces:
            if piece.is_white != is_white:
                if king_position in piece.valid_moves(self.board):
                    return True
        return False
    

    def deselect_piece(self):
        if self.selected_piece:
            self.selected_piece = None
            self.move_mode = False

    def print_board_state(self):
        ##print(board_to_string(self.board))
        return
    
    def make_ai_move(self):
        board_str = board_to_string(self.board)
        
        # Check if the current moves match any opening
        matched_openings = [(name, moves) for name, moves in openings.items() if self.moves == moves[:len(self.moves)]]
        
        if matched_openings:
            # Select a random opening from the matched openings
            opening_name, opening_moves = random.choice(matched_openings)
            opening_move = opening_moves[len(self.moves)]
            print(opening_name)
        else:
            opening_move = None

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
            # Use minimax if no opening is tracked or opening moves are exhausted
            _, best_move = minimax(board_str, depth=2, is_maximizing_player=False, alpha=float('-inf'), beta=float('inf'))
            p, to_pos = best_move
            from_pos = p.grid_position

        # Generate the move notation before the move is executed
        piece = self.board.get_piece_at_position(from_pos)
        move_notation = generate_move_notation(piece, from_pos, to_pos, self.board)
        
        self.execute_move(from_pos, to_pos)

        # Track and print move
        self.moves.append(move_notation)
        print(self.moves)


    def execute_move(self, from_pos, to_pos):
        piece = self.board.get_piece_at_position(from_pos)
        if piece:
            self.board.highlight_ai_move(from_pos, to_pos)
            captured_piece = self.board.get_piece_at_position(to_pos)
            if captured_piece and captured_piece.is_white != piece.is_white:
                self.board.remove_piece(captured_piece)

            # Special handling for castling
            if isinstance(piece, King):
                if to_pos[0] == 6 and not piece.has_moved:  # Kingside castling
                    rook = self.board.get_piece_at_position((7, from_pos[1]))
                    if rook and isinstance(rook, Rook) and not rook.has_moved:
                        rook.grid_position = (5, from_pos[1])
                        rook.update_position()
                        self.board.piece_has_moved(rook)
                elif to_pos[0] == 2 and not piece.has_moved:  # Queenside castling
                    rook = self.board.get_piece_at_position((0, from_pos[1]))
                    if rook and isinstance(rook, Rook) and not rook.has_moved:
                        rook.grid_position = (3, from_pos[1])
                        rook.update_position()
                        self.board.piece_has_moved(rook)

            piece.grid_position = to_pos
            piece.update_position()
            self.board.piece_has_moved(piece)

            # Handle pawn promotion
            if isinstance(piece, Pawn) and (to_pos[1] == 0 or to_pos[1] == 7):
                self.board.promote_pawn(piece)

            engine_audio.play(move_sound, 0, False)
            self.current_player_is_white = not self.current_player_is_white
            self.print_board_state()

            # Evaluate board and update evaluation line
            board_str = board_to_string(self.board)
            evaluation_score = evaluate_board(board_str)
            self.update_evaluation_line(evaluation_score)

            # Check for checkmate
            white_checkmate, black_checkmate = is_checkmate(self.board)
            if white_checkmate:
                self.winner_message = "Checkmate Lose!"
                return
            if black_checkmate:
                self.winner_message = "Checkmate Win!"
                return


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
    
    return piece_notation + capture_notation + position_to_algebraic(to_pos)


def position_to_algebraic(pos):
    return chr(pos[0] + ord('a')) + str(8 - pos[1])

def board_to_string(board):
    board_state = [["." for _ in range(8)] for _ in range(8)]
    for piece in board.pieces:
        col, row = piece.grid_position
        piece_char = get_piece_char(piece)
        board_state[row][col] = piece_char

    return "\n".join(["".join(row) for row in board_state])

def string_to_board(board_str):
    board = ChessBoard()
    rows = board_str.split("\n")
    for row in range(8):
        for col in range(8):
            char = rows[row][col]
            if char != '.':
                piece = char_to_piece(char, (col, row))
                if piece:
                    piece.opacity = 0  # Ensure the piece is not rendered
                    board.add_piece(piece)
    return board

def char_to_piece(char, grid_position):
    if char in 'KQRBNP':
        is_white = True
    elif char in 'kqrbnp':
        is_white = False
    else:
        return None
    
    piece_map = {
        'K': King,
        'Q': Queen,
        'R': Rook,
        'B': Bishop,
        'N': Knight,
        'P': Pawn
    }

    piece_type = piece_map[char.upper()]
    return piece_type(grid_position, chess_texture, is_white)

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


piece_values = {"P": 100, "N": 280, "B": 320, "R": 479, "Q": 929, "K": 60000}
pst = {
    'P': (  829, 829, 829, 829,829, 829, 829, 829,
            78,  83,  86,  73, 102,  82,  85,  90,
             7,  29,  21,  44,  40,  31,  44,   7,
           -17,  16,  -2,  15,  14,   0,  15, -13,
           -26,   3,  10,   9,   6,   1,   0, -23,
           -22,   9,   5, -11, -10,  -2,   3, -19,
           -31,   8,  -7, -37, -36, -14,   3, -31,
             0,   0,   0,   0,   0,   0,   0,   0),
    'N': ( -66, -53, -75, -75, -10, -55, -58, -70,
            -3,  -6, 100, -36,   4,  62,  -4, -14,
            10,  67,   1,  74,  73,  27,  62,  -2,
            24,  24,  45,  37,  33,  41,  25,  17,
            -1,   5,  31,  21,  22,  35,   2,   0,
           -18,  10,  13,  22,  18,  15,  11, -14,
           -23, -15,   2,   0,   2,   0, -23, -20,
           -74, -23, -26, -24, -19, -35, -22, -69),
    'B': ( -59, -78, -82, -76, -23,-107, -37, -50,
           -11,  20,  35, -42, -39,  31,   2, -22,
            -9,  39, -32,  41,  52, -10,  28, -14,
            25,  17,  20,  34,  26,  25,  15,  10,
            13,  10,  17,  23,  17,  16,   0,   7,
            14,  25,  24,  15,   8,  25,  20,  15,
            19,  20,  11,   6,   7,   6,  20,  16,
            -7,   2, -15, -12, -14, -15, -10, -10),
    'R': (  35,  29,  33,   4,  37,  33,  56,  50,
            55,  29,  56,  67,  55,  62,  34,  60,
            19,  35,  28,  33,  45,  27,  25,  15,
             0,   5,  16,  13,  18,  -4,  -9,  -6,
           -28, -35, -16, -21, -13, -29, -46, -30,
           -42, -28, -42, -25, -25, -35, -26, -46,
           -53, -38, -31, -26, -29, -43, -44, -53,
           -30, -24, -18,   5,  -2, -18, -31, -32),
    'Q': (   6,   1,  -8,-104,  69,  24,  88,  26,
            14,  32,  60, -10,  20,  76,  57,  24,
            -2,  43,  32,  60,  72,  63,  43,   2,
             1, -16,  22,  17,  25,  20, -13,  -6,
           -14, -15,  -2,  -5,  -1, -10, -20, -22,
           -30,  -6, -13, -11, -16, -11, -16, -27,
           -36, -18,   0, -19, -15, -15, -21, -38,
           -39, -30, -31, -13, -31, -36, -34, -42),
    'K': (   4,  54,  47, -99, -99,  60,  83, -62,
           -32,  10,  55,  56,  56,  55,  10,   3,
           -62,  12, -57,  44, -67,  28,  37, -31,
           -55,  50,  11,  -4, -19,  13,   0, -49,
           -55, -43, -52, -28, -51, -47,  -8, -50,
           -47, -42, -43, -79, -64, -32, -29, -32,
            -4,   3, -14, -50, -57, -18,  13,   4,
            17,  30,  -3, -14,   6,  -1,  40,  18),
}

def check_opening(moves):
    for opening_name, opening_moves in openings.items():
        if moves == opening_moves[:len(moves)]:
            return opening_name
    return None

openings = {
    "Ruy Lopez Main Line": ["e4", "e5", "Nf3", "Nc6", "Bb5", "a6", "Ba4", "Nf6", "O-O", "Be7", "Re1", "b5", "Bb3", "d6", "c3", "O-O"],
    "Ruy Lopez Closed Ruy Lopez": ["e4", "e5", "Nf3", "Nc6", "Bb5", "a6", "Ba4", "Nf6", "O-O", "Be7", "Re1", "b5", "Bb3", "d6", "c3", "O-O", "h3", "Nb8", "d4", "Nbd7"],
    "Ruy Lopez Open Ruy Lopez": ["e4", "e5", "Nf3", "Nc6", "Bb5", "a6", "Ba4", "Nf6", "O-O", "Be7", "Re1", "b5", "Bb3", "d6", "c3", "Na5", "Bc2", "c5"],
    "Ruy Lopez Breyer Defense": ["e4", "e5", "Nf3", "Nc6", "Bb5", "a6", "Ba4", "Nf6", "O-O", "Be7", "Re1", "b5", "Bb3", "d6", "c3", "O-O", "h3", "Nb8", "d4", "Nbd7", "Nbd2", "Bb7"],
    "Ruy Lopez Marshall Attack": ["e4", "e5", "Nf3", "Nc6", "Bb5", "a6", "Ba4", "Nf6", "O-O", "Be7", "Re1", "b5", "Bb3", "d6", "c3", "d5", "exd5", "Nxd5", "Nxe5", "Nxe5", "Rxe5", "c6"],
    
    "Italian Game Main Line": ["e4", "e5", "Nf3", "Nc6", "Bc4", "Bc5", "c3", "Nf6", "d3", "d6", "O-O", "O-O", "Re1", "a6", "a4", "Ba7"],
    "Italian Game Evans Gambit": ["e4", "e5", "Nf3", "Nc6", "Bc4", "Bc5", "b4", "Bxb4", "c3", "Ba5"],
    "Italian Game Giuoco Pianissimo": ["e4", "e5", "Nf3", "Nc6", "Bc4", "Bc5", "d3", "d6", "c3", "a6"],
    "Italian Game Greco Attack": ["e4", "e5", "Nf3", "Nc6", "Bc4", "Bc5", "c3", "Nf6", "d4", "exd4", "cxd4", "Bb4+"],
    "Italian Game Two Knights Defense": ["e4", "e5", "Nf3", "Nc6", "Bc4", "Nf6", "Ng5", "d5", "exd5", "Na5"],
    
    "Sicilian Defense Main Line": ["e4", "c5", "Nf3", "d6", "d4", "cxd4", "Nxd4", "Nf6", "Nc3", "a6", "Be3", "e6", "f3", "Be7", "Qd2", "O-O"],
    "Sicilian Defense Najdorf": ["e4", "c5", "Nf3", "d6", "d4", "cxd4", "Nxd4", "Nf6", "Nc3", "a6", "Be3", "e5", "Nb3", "Be6"],
    "Sicilian Defense Dragon": ["e4", "c5", "Nf3", "d6", "d4", "cxd4", "Nxd4", "Nf6", "Nc3", "g6", "Be3", "Bg7", "f3", "O-O", "Qd2", "Nc6"],
    "Sicilian Defense Scheveningen": ["e4", "c5", "Nf3", "d6", "d4", "cxd4", "Nxd4", "Nf6", "Nc3", "a6", "Be3", "e6", "f3", "Be7"],
    "Sicilian Defense Sveshnikov": ["e4", "c5", "Nf3", "d6", "d4", "cxd4", "Nxd4", "Nf6", "Nc3", "e5", "Ndb5", "d6", "Na3", "b5"],
    
    "French Defense Main Line": ["e4", "e6", "d4", "d5", "Nc3", "Nf6", "Bg5", "Be7", "e5", "Nfd7", "Bxe7", "Qxe7", "f4", "O-O", "Nf3", "c5"],
    "French Defense Classical": ["e4", "e6", "d4", "d5", "Nc3", "Nf6", "Bg5", "dxe4", "Nxe4", "Be7"],
    "French Defense Winawer": ["e4", "e6", "d4", "d5", "Nc3", "Bb4", "e5", "c5", "a3", "Bxc3+", "bxc3", "Ne7"],
    "French Defense Tarrasch": ["e4", "e6", "d4", "d5", "Nd2", "Nf6", "e5", "Nfd7", "Bd3", "c5"],
    "French Defense Advance": ["e4", "e6", "d4", "d5", "e5", "c5", "c3", "Nc6", "Nf3", "Qb6"],
    
    "Caro-Kann Defense Main Line": ["e4", "c6", "d4", "d5", "Nc3", "dxe4", "Nxe4", "Bf5", "Ng3", "Bg6", "h4", "h6", "Nf3", "Nd7", "h5", "Bh7", "Bd3", "Bxd3"],
    "Caro-Kann Defense Advance": ["e4", "c6", "d4", "d5", "e5", "Bf5", "Nc3", "e6", "g4", "Bg6"],
    "Caro-Kann Defense Classical": ["e4", "c6", "d4", "d5", "Nc3", "dxe4", "Nxe4", "Bf5", "Ng3", "Bg6", "h4", "h6", "Nf3", "Nd7"],
    "Caro-Kann Defense Panov-Botvinnik Attack": ["e4", "c6", "d4", "d5", "exd5", "cxd5", "c4", "Nf6", "Nc3", "e6", "Nf3", "Bb4"],
    "Caro-Kann Defense Two Knights": ["e4", "c6", "Nf3", "d5", "Nc3", "Bg4"],
    
    "Queen's Gambit Main Line": ["d4", "d5", "c4", "e6", "Nc3", "Nf6", "Bg5", "Be7", "e3", "O-O", "Nf3", "h6", "Bh4", "b6"],
    "Queen's Gambit Accepted": ["d4", "d5", "c4", "dxc4", "Nf3", "Nf6", "e3", "e6", "Bxc4", "c5", "O-O", "a6", "dxc5", "Bxc5", "Qe2", "b5"],
    "Queen's Gambit Declined": ["d4", "d5", "c4", "e6", "Nc3", "Nf6", "Bg5", "Be7"],
    "Slav Defense": ["d4", "d5", "c4", "c6"],
    "Chigorin Defense": ["d4", "d5", "c4", "Nc6"],
    
    "Queen's Gambit Accepted Classical": ["d4", "d5", "c4", "dxc4", "Nf3", "Nf6", "e3", "e6", "Bxc4", "c5", "O-O", "a6"],
    "Queen's Gambit Accepted Modern": ["d4", "d5", "c4", "dxc4", "e4"],
    "Queen's Gambit Accepted Alekhine": ["d4", "d5", "c4", "dxc4", "e4"],
    "Queen's Gambit Accepted Lasker Defense": ["d4", "d5", "c4", "dxc4", "Nc3", "e6", "e4", "c5"],
    
    "King's Indian Defense Main Line": ["d4", "Nf6", "c4", "g6", "Nc3", "Bg7", "e4", "d6", "Nf3", "O-O", "Be2", "e5", "O-O", "Nc6", "d5", "Ne7"],
    "King's Indian Defense Classical": ["d4", "Nf6", "c4", "g6", "Nc3", "Bg7", "e4", "d6", "Nf3", "O-O", "Be2", "Nc6", "d5", "Ne7"],
    "King's Indian Defense Fianchetto": ["d4", "Nf6", "c4", "g6", "Nf3", "Bg7", "g3", "O-O", "Bg2", "d6"],
    "King's Indian Defense Four Pawns Attack": ["d4", "Nf6", "c4", "g6", "e4", "d6", "f4"],
    "King's Indian Defense Saemisch": ["d4", "Nf6", "c4", "g6", "Nc3", "Bg7", "f3"],
    
    "Nimzo-Indian Defense Main Line": ["d4", "Nf6", "c4", "e6", "Nc3", "Bb4", "e3", "O-O", "Bd3", "d5", "Nf3", "c5", "O-O", "Nc6", "a3", "Bxc3"],
    "Nimzo-Indian Defense Rubinstein": ["d4", "Nf6", "c4", "e6", "Nc3", "Bb4", "e3", "O-O", "Bd3", "d5", "Nf3"],
    "Nimzo-Indian Defense Classical": ["d4", "Nf6", "c4", "e6", "Nc3", "Bb4", "Qc2", "O-O", "a3", "Bxc3", "Qxc3", "d5"],
    "Nimzo-Indian Defense Leningrad": ["d4", "Nf6", "c4", "e6", "Nc3", "Bb4", "Bg5", "h6", "Bh4", "c5"],
    "Nimzo-Indian Defense Fischer Variation": ["d4", "Nf6", "c4", "e6", "Nc3", "Bb4", "Qc2", "O-O", "a3", "Bxc3", "Qxc3", "d5"],
    
    "Slav Defense Main Line": ["d4", "d5", "c4", "c6", "Nf3", "Nf6", "Nc3", "dxc4", "a4", "Bf5", "e3", "e6", "Bxc4", "Bb4", "O-O", "O-O"],
    "Slav Defense Exchange": ["d4", "d5", "c4", "c6", "cxd5", "cxd5"],
    "Slav Defense Chebanenko": ["d4", "d5", "c4", "c6", "Nf3", "a6"],
    "Slav Defense Moran Defense": ["d4", "d5", "c4", "c6", "Nf3"],
    "Slav Defense Schlechter": ["d4", "d5", "c4", "c6", "Nf3", "e6", "Nbd7"],
    
    "English Opening Main Line": ["c4", "e5", "Nc3", "Nc6", "g3", "g6", "Bg2", "Bg7", "d3", "d6", "e4", "Be6", "Nge2", "Qd7"],
    "English Opening Symmetrical": ["c4", "c5", "g3", "g6"],
    "English Opening Reversed Sicilian": ["c4", "e5", "Nf3", "Nc6"],
    "English Opening Botvinnik": ["c4", "g3"],
    "English Opening Four Knights": ["c4", "e5", "Nc3", "Nc6", "Nf3"],
    
    "Reti Opening Main Line": ["Nf3", "d5", "c4", "c6", "g3", "Nf6", "Bg2", "Bf5", "O-O", "e6", "d3", "h6", "Nc3", "Be7", "Re1", "O-O"],
    "Reti Opening King's Indian Attack": ["Nf3", "d5", "g3", "Bg4", "d3", "Nd7", "Nbd2", "Ngf6"],
    "Reti Opening Closed": ["Nf3", "c4", "c6", "d4"],
    "Reti Opening English Variation": ["c4", "Nf6", "Nf3", "c5"],
    "Reti Opening Catalan": ["Nf3", "d5", "g3", "e6", "Bg2"],
    
    "London System Main Line": ["d4", "d5", "Nf3", "Nf6", "Bf4", "e6", "e3", "c5", "c3", "Nc6", "Nbd2", "Bd6", "Bg3", "O-O", "Bd3", "b6"],
    "London System Accelerated": ["d4", "Bf4"],
    "London System Jobava": ["d4", "d5", "Nc3", "Nf6", "Bf4"],
    "London System Barry Attack": ["d4", "Nf6", "Nc3", "e6", "Bf4"],
    "London System Colle System": ["d4", "Nf3", "e6", "e3"],
    
    "Scandinavian Defense Main Line": ["e4", "d5", "exd5", "Qxd5", "Nc3", "Qa5", "d4", "c6", "Nf3", "Bg4", "Be2", "e6", "O-O", "Nd7", "h3", "Bh5"],
    "Scandinavian Defense Modern": ["e4", "d5", "d4"],
    "Scandinavian Defense Portuguese": ["e4", "d5", "Nf3"],
    "Scandinavian Defense Classical": ["e4", "d5", "exd5", "Qxd5", "Nc3", "Qa5", "d4", "Nf6"],
    "Scandinavian Defense Patzer": ["e4", "d5", "Qe2"],
    
    "Pirc Defense Main Line": ["e4", "d6", "d4", "Nf6", "Nc3", "g6", "Be2", "Bg7", "Be3", "O-O", "Qd2", "c6", "Bh6", "b5"],
    "Pirc Defense Austrian Attack": ["e4", "d6", "d4", "Nf6", "Nc3", "g6", "f4"],
    "Pirc Defense Classical": ["e4", "d6", "d4", "Nf6", "Nc3", "g6", "Nf3", "Bg7", "Be2", "O-O", "O-O", "c6"],
    "Pirc Defense 150 Attack": ["e4", "d6", "d4", "Nf6", "Nc3", "g6", "Be3", "Bg7", "Qd2", "O-O", "O-O-O"],
    "Pirc Defense Byrne": ["e4", "d6", "d4", "Nf6", "Nc3", "g6", "Bg5", "Bg7", "Qd2", "h6", "Be3"]
}


def evaluate_board(board_str):
    score = 0
    board = string_to_board(board_str)
    
    # Check for checkmate
    white_checkmate, black_checkmate = is_checkmate(board)
    if white_checkmate:
        return -CHECKMATE_THRESHOLD  # Black wins
    if black_checkmate:
        return CHECKMATE_THRESHOLD  # White wins

    rows = board_str.split("\n")
    for y, row in enumerate(rows):
        for x, char in enumerate(row):
            if char != '.':
                piece_value = piece_values[char.upper()]
                # Flip the PST index for black pieces
                if char.islower():
                    pst_value = pst[char.upper()][(7 - y) * 8 + (7 - x)]
                    score -= piece_value + pst_value
                else:
                    pst_value = pst[char.upper()][y * 8 + x]
                    score += piece_value + pst_value
    return score

def is_checkmate(board):
    white_checkmate = checkmate_for_color(board, True)
    black_checkmate = checkmate_for_color(board, False)
    return white_checkmate, black_checkmate

def checkmate_for_color(board, is_white):
    if not is_in_check(board, is_white):
        return False
    for piece in board.pieces:
        if piece.is_white == is_white:
            for move in piece.valid_moves(board):
                board_copy_str = simulate_move(board_to_string(board), piece.grid_position, move)
                if not leaves_king_in_check(board_copy_str, is_white):
                    return False
    return True

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

def minimax(board_str, depth, is_maximizing_player, alpha, beta, indent=""):
    if depth == 0:
        evaluation = evaluate_board(board_str)
        #print(f"{indent}Depth: {depth}, Evaluation: {evaluation}")
        return evaluation, None

    best_move = None
    board = string_to_board(board_str)
    is_white = is_maximizing_player
    all_moves = get_all_valid_moves(board, is_white)

    if is_maximizing_player:
        max_eval = float('-inf')
        #print(f"{indent}Maximizing player at depth {depth}:")
        for piece, move in all_moves:
            board_copy_str = simulate_move(board_str, piece.grid_position, move)
            if not leaves_king_in_check(board_copy_str, is_white):
                eval, _ = minimax(board_copy_str, depth - 1, False, alpha, beta, indent + "    ")
                #move_notation = generate_move_notation(piece, piece.grid_position, move, board)
                #print(f"{indent}  Move: {move_notation}, Eval: {eval}")
                if eval > max_eval:
                    max_eval = eval
                    best_move = (piece, move)
                alpha = max(alpha, eval)
                if beta <= alpha:
                    #print(f"{indent}  Alpha-beta cutoff")
                    break
        #if best_move:
        #    best_move_notation = generate_move_notation(best_move[0], best_move[0].grid_position, best_move[1], board)
        #else:
        #    best_move_notation = "No valid move"
        #print(f"{indent}Best move at depth {depth}: {best_move_notation}, Max eval: {max_eval}")
        return max_eval, best_move
    else:
        min_eval = float('inf')
        #print(f"{indent}Minimizing player at depth {depth}:")
        for piece, move in all_moves:
            board_copy_str = simulate_move(board_str, piece.grid_position, move)
            if not leaves_king_in_check(board_copy_str, is_white):
                eval, _ = minimax(board_copy_str, depth - 1, True, alpha, beta, indent + "    ")
                #move_notation = generate_move_notation(piece, piece.grid_position, move, board)
                #print(f"{indent}  Move: {move_notation}, Eval: {eval}")
                if eval < min_eval:
                    min_eval = eval
                    best_move = (piece, move)
                beta = min(beta, eval)
                if beta <= alpha:
                    #print(f"{indent}  Alpha-beta cutoff")
                    break
        #if best_move:
        #    best_move_notation = generate_move_notation(best_move[0], best_move[0].grid_position, best_move[1], board)
        #else:
        #    best_move_notation = "No valid move"
        #print(f"{indent}Best move at depth {depth}: {best_move_notation}, Min eval: {min_eval}")
        return min_eval, best_move


def leaves_king_in_check(board_str, is_white):
    board = string_to_board(board_str)
    return is_in_check(board, is_white)

def is_in_check(board, is_white):
    king_position = None
    for piece in board.pieces:
        if isinstance(piece, King) and piece.is_white == is_white:
            king_position = piece.grid_position
            break
    
    for piece in board.pieces:
        if piece.is_white != is_white:
            if king_position in piece.valid_moves(board):
                return True
    return False




def simulate_move(board_str, from_pos, to_pos):
    board = string_to_board(board_str)
    piece = board.get_piece_at_position(from_pos)
    if piece:
        captured_piece = board.get_piece_at_position(to_pos)
        if captured_piece:
            board.remove_piece(captured_piece)
        piece.grid_position = to_pos
        piece.update_position()
    new_board_str = board_to_string(board)
    return new_board_str


def start_chess_game():
    global game, camera
    camera = CameraNode()
    camera.position = Vector3(DISP_WIDTH / 2, DISP_WIDTH / 2, 1)
    game = ChessGame(camera)

camera = CameraNode()
camera.position = Vector3(DISP_WIDTH / 2, DISP_WIDTH / 2, 1)
game = None
start_chess_game()

engine.start()
