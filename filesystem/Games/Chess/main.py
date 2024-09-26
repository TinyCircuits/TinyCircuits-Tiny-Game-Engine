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

font = FontResource("/system/assets/outrunner_outline.bmp")

chess_texture = TextureResource("chess.bmp")
board_texture = TextureResource("board.bmp")
move_sound = WaveSoundResource("move.wav")
take_sound = WaveSoundResource("take.wav")
pawn_sound = WaveSoundResource("pawn.wav")
engine_audio.set_volume(0.5)

# Display dimensions
DISP_WIDTH = 128
DISP_HEIGHT = 128

# Cell dimensions
CELL_WIDTH = 16
CELL_HEIGHT = 16
OFFSET = CELL_WIDTH / 2

MAX_DEPTH=2

# Global dictionary to track timing
timing_data = {}

def log_time(start, description):
    duration = time.ticks_ms() - start
    if description not in timing_data:
        timing_data[description] = {'total_time': 0, 'call_count': 0}
    timing_data[description]['total_time'] += duration/1000
    timing_data[description]['call_count'] += 1

def print_timing_data():
    # Convert the dictionary to a list of tuples and sort by total_time in descending order
    sorted_timing_data = sorted(timing_data.items(), key=lambda x: x[1]['total_time'], reverse=True)
    
    # Iterate through the sorted list and print the timing data
    for description, data in sorted_timing_data:
        total_time = data['total_time']
        call_count = data['call_count']
        avg_time = total_time / call_count if call_count > 0 else 0
        print(f'{description}: total_time = {total_time:.4f} sec, call_count = {call_count}, avg_time = {avg_time:.4f} sec')


class ChessPiece:
    def __init__(self, grid_position, is_white): 
        self.grid_position = grid_position
        self.is_white = is_white
        self.has_moved = False

    def valid_moves(self, board):
        return []
    
    def safe_moves(self, board):
        #start_time = time.ticks_ms()
        valid_moves = self.valid_moves(board)
        safe_moves = []
        for move in valid_moves:
            board.make_move(self.grid_position, move)
            if not board.is_in_check(self.is_white):
                safe_moves.append(move)
            board.undo_move()
        #log_time(start_time, f'safe_moves for {type(self).__name__}')
        return safe_moves
    
    def __str__(self):
        color = "White" if self.is_white else "Black"
        return f"{color} piece at {self.grid_position}, has moved: {self.has_moved}"

    def __repr__(self):
        return f"{type(self).__name__}(grid_position={self.grid_position}, is_white={self.is_white}, has_moved={self.has_moved})"


class King(ChessPiece):
    def valid_moves(self, board):
        #start_time = time.ticks_ms()
        moves = []
        directions = [(-1, -1), (-1, 0), (-1, 1), (0, -1), (0, 1), (1, -1), (1, 0), (1, 1)]
        for direction in directions:
            new_pos = (self.grid_position[0] + direction[0], self.grid_position[1] + direction[1])
            if 0 <= new_pos[0] < 8 and 0 <= new_pos[1] < 8:
                target_piece = board.piece_positions.get(new_pos)
                if not target_piece or target_piece.is_white != self.is_white:
                    moves.append(new_pos)

        if not self.has_moved:
            # Kingside castling
            rook = board.piece_positions.get((7, self.grid_position[1]))
            if isinstance(rook, Rook) and not rook.has_moved:
                if not any(board.piece_positions.get((i, self.grid_position[1])) for i in range(5, 7)):
                    moves.append((6, self.grid_position[1]))

            # Queenside castling
            rook = board.piece_positions.get((0, self.grid_position[1]))
            if isinstance(rook, Rook) and not rook.has_moved:
                if not any(board.piece_positions.get((i, self.grid_position[1])) for i in range(1, 4)):
                    moves.append((2, self.grid_position[1]))

        #log_time(start_time, f'valid_moves for King')
        return moves

class Queen(ChessPiece):
    def valid_moves(self, board):
        #start_time = time.ticks_ms()
        moves = []
        directions = [(-1, -1), (-1, 0), (-1, 1), (0, -1), (0, 1), (1, -1), (1, 0), (1, 1)]
        for direction in directions:
            for i in range(1, 8):
                new_pos = (self.grid_position[0] + direction[0] * i, self.grid_position[1] + direction[1] * i)
                if 0 <= new_pos[0] < 8 and 0 <= new_pos[1] < 8:
                    target_piece = board.piece_positions.get(new_pos)
                    if target_piece:
                        if target_piece.is_white != self.is_white:
                            moves.append(new_pos)
                        break
                    moves.append(new_pos)
                else:
                    break
        #log_time(start_time, f'valid_moves for Queen')
        return moves

class Rook(ChessPiece):
    def valid_moves(self, board):
        #start_time = time.ticks_ms()
        moves = []
        directions = [(0, -1), (0, 1), (-1, 0), (1, 0)]
        for direction in directions:
            for i in range(1, 8):
                new_pos = (self.grid_position[0] + direction[0] * i, self.grid_position[1] + direction[1] * i)
                if 0 <= new_pos[0] < 8 and 0 <= new_pos[1] < 8:
                    target_piece = board.piece_positions.get(new_pos)
                    if target_piece:
                        if target_piece.is_white != self.is_white:
                            moves.append(new_pos)
                        break
                    moves.append(new_pos)
                else:
                    break
        #log_time(start_time, f'valid_moves for Rook')
        return moves

class Bishop(ChessPiece):
    def valid_moves(self, board):
        #start_time = time.ticks_ms()
        moves = []
        directions = [(-1, -1), (-1, 1), (1, -1), (1, 1)]
        for direction in directions:
            for i in range(1, 8):
                new_pos = (self.grid_position[0] + direction[0] * i, self.grid_position[1] + direction[1] * i)
                if 0 <= new_pos[0] < 8 and 0 <= new_pos[1] < 8:
                    target_piece = board.piece_positions.get(new_pos)
                    if target_piece:
                        if target_piece.is_white != self.is_white:
                            moves.append(new_pos)
                        break
                    moves.append(new_pos)
                else:
                    break
        #log_time(start_time, f'valid_moves for Bishop')
        return moves

class Knight(ChessPiece):
    def valid_moves(self, board):
        #start_time = time.ticks_ms()
        moves = []
        knight_moves = [(-2, -1), (-1, -2), (1, -2), (2, -1), (2, 1), (1, 2), (-1, 2), (-2, 1)]
        for move in knight_moves:
            new_pos = (self.grid_position[0] + move[0], self.grid_position[1] + move[1])
            if 0 <= new_pos[0] < 8 and 0 <= new_pos[1] < 8:
                target_piece = board.piece_positions.get(new_pos)
                if not target_piece or target_piece.is_white != self.is_white:
                    moves.append(new_pos)
        #log_time(start_time, f'valid_moves for Knight')
        return moves

class Pawn(ChessPiece):
    def __init__(self, grid_position, is_white):
        super().__init__(grid_position, is_white)
        self.en_passant_target = False

    def valid_moves(self, board):
        #start_time = time.ticks_ms()
        moves = []
        direction = -1 if self.is_white else 1
        start_row = 6 if self.is_white else 1

        # Move forward
        forward_pos = (self.grid_position[0], self.grid_position[1] + direction)
        if 0 <= forward_pos[1] < 8 and not board.piece_positions.get(forward_pos):
            moves.append(forward_pos)
            # Double move from start position
            if self.grid_position[1] == start_row:
                double_forward_pos = (self.grid_position[0], self.grid_position[1] + 2 * direction)
                if not board.piece_positions.get(double_forward_pos):
                    moves.append(double_forward_pos)

        # Captures
        capture_moves = [(self.grid_position[0] - 1, self.grid_position[1] + direction), 
                         (self.grid_position[0] + 1, self.grid_position[1] + direction)]
        for capture_pos in capture_moves:
            if 0 <= capture_pos[0] < 8 and 0 <= capture_pos[1] < 8:
                target_piece = board.piece_positions.get(capture_pos)
                if target_piece and target_piece.is_white != self.is_white:
                    moves.append(capture_pos)
                # En passant capture
                elif not target_piece:
                    adjacent_pos = (capture_pos[0], self.grid_position[1])
                    adjacent_piece = board.piece_positions.get(adjacent_pos)
                    if isinstance(adjacent_piece, Pawn) and adjacent_piece.en_passant_target and adjacent_piece.is_white != self.is_white:
                        moves.append(capture_pos)
        #log_time(start_time, f'valid_moves for Pawn')
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
        self.sprites = []
        self.squares = []
        self.layer = 0
        self.player_is_white = player_is_white
        self.board = SimulatedChessBoard()
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
                square = Sprite2DNode(texture=board_texture)
                square.position = Vector2(col * CELL_WIDTH + OFFSET, row * CELL_HEIGHT + OFFSET)
                square.playing = False
                square.frame_count_x = 2
                square.frame_count_y = 1
                square.frame_current_x = (row + col + 1) % 2
                square.frame_current_y = 1
                square.layer = 1
                self.squares.append(square)
                self.add_child(square)

    def render_pieces(self):
        # Clear existing piece sprites
        for sprite in self.sprites:
            sprite.opacity = 0
        self.sprites = []

        # Render piece sprites based on the current board state
        for piece in self.board.pieces:
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

    def setup_pieces(self, is_white):
        base_row = 7 if is_white else 0
        pawn_row = 6 if is_white else 1

        # Rooks
        self.board.add_piece(Rook((0, base_row), is_white))
        self.board.add_piece(Rook((7, base_row), is_white))

        # Knights
        self.board.add_piece(Knight((1, base_row), is_white))
        self.board.add_piece(Knight((6, base_row), is_white))

        # Bishops
        self.board.add_piece(Bishop((2, base_row), is_white))
        self.board.add_piece(Bishop((5, base_row), is_white))

        # Queens
        self.board.add_piece(Queen((3, base_row), is_white))

        # Kings
        self.board.add_piece(King((4, base_row), is_white))

        # Pawns
        for i in range(8):
            self.board.add_piece(Pawn((i, pawn_row), is_white))

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

class SimulatedChessBoard:
    def __init__(self):
        self.pieces = []
        self.piece_positions = {}  # Cache for piece positions
        self.piece_scores = {}  # Cache for piece evaluation scores
        self.move_history = []
        self.directions = [(-1, 0), (1, 0), (0, -1), (0, 1), (-1, -1), (-1, 1), (1, -1), (1, 1)]
        self.knight_moves = [(-2, -1), (-1, -2), (1, -2), (2, -1), (2, 1), (1, 2), (-1, 2), (-2, 1)]


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

    def reset_en_passant_status(self):
        for piece in self.pieces:
            if isinstance(piece, Pawn):
                piece.en_passant_target = False

    def make_move(self, from_pos, to_pos):
        piece = self.piece_positions.get(from_pos)
        if not piece:
            return
        captured_piece = self.piece_positions.get(to_pos)
        en_passant_capture = None
        castling_rook_from = None
        castling_rook_to = None
        promotion = None
        has_moved_before = piece.has_moved

        if piece:
            # Handle special moves
            if isinstance(piece, King):
                if to_pos[0] == 6 and not piece.has_moved:  # Kingside castling
                    castling_rook_from = (7, from_pos[1])
                    castling_rook_to = (5, from_pos[1])
                elif to_pos[0] == 2 and not piece.has_moved:  # Queenside castling
                    castling_rook_from = (0, from_pos[1])
                    castling_rook_to = (3, from_pos[1])

            if isinstance(piece, Pawn):
                if not captured_piece and abs(to_pos[0] - piece.grid_position[0]) == 1:
                    en_passant_capture = self.piece_positions.get((to_pos[0], from_pos[1]))
                    if en_passant_capture and isinstance(en_passant_capture, Pawn) and en_passant_capture.en_passant_target:
                        captured_piece = en_passant_capture

                if abs(to_pos[1] - piece.grid_position[1]) == 2:
                    self.reset_en_passant_status()
                    piece.en_passant_target = True
                else:
                    self.reset_en_passant_status()
                    piece.en_passant_target = False

                if to_pos[1] == 0 or to_pos[1] == 7:
                    promotion = piece
                    piece = self.promote_pawn(piece)
            else:
                self.reset_en_passant_status()

            if castling_rook_from and castling_rook_to:
                rook = self.piece_positions.get(castling_rook_from)
                if rook and isinstance(rook, Rook) and not rook.has_moved:
                    rook.grid_position = castling_rook_to
                    self.piece_positions[castling_rook_to] = rook
                    del self.piece_positions[castling_rook_from]
                    self.piece_has_moved(rook)

            if captured_piece:
                self.remove_piece(captured_piece)

            piece.grid_position = to_pos
            self.piece_positions[to_pos] = piece
            del self.piece_positions[from_pos]
            self.update_piece_score(piece)
            self.piece_has_moved(piece)

            # Cache move details
            self.move_history.append({
                'piece': piece,
                'from_pos': from_pos,
                'to_pos': to_pos,
                'captured_piece': captured_piece,
                'en_passant_capture': en_passant_capture,
                'castling_rook_from': castling_rook_from,
                'castling_rook_to': castling_rook_to,
                'promotion': promotion,
                'has_moved_before': has_moved_before
            })
        return piece, captured_piece

    def undo_move(self):
        if not self.move_history:
            return

        last_move = self.move_history.pop()
        piece = last_move['piece']
        from_pos = last_move['from_pos']
        to_pos = last_move['to_pos']
        captured_piece = last_move['captured_piece']
        en_passant_capture = last_move['en_passant_capture']
        castling_rook_from = last_move['castling_rook_from']
        castling_rook_to = last_move['castling_rook_to']
        promotion = last_move['promotion']
        has_moved_before = last_move['has_moved_before']

        # Handle castling
        if castling_rook_to and castling_rook_from:
            rook = self.piece_positions.get(castling_rook_to)
            if rook and isinstance(rook, Rook):
                rook.grid_position = castling_rook_from
                self.piece_positions[castling_rook_from] = rook
                self.piece_positions.pop(castling_rook_to, None)
                rook.has_moved = False

        # Handle promotion
        if promotion:
            # Remove the promoted piece
            promoted_piece = self.piece_positions.get(to_pos)
            self.remove_piece(promoted_piece)
            # Re-add the pawn
            pawn = Pawn(from_pos, piece.is_white)
            self.add_piece(pawn)
            piece = pawn  # Update piece reference to the pawn

        piece.grid_position = from_pos
        self.piece_positions[from_pos] = piece
        try:
            del self.piece_positions[to_pos]
        except:
            pass

        # Restore the captured piece if there was one
        if captured_piece:
            self.add_piece(captured_piece)

        # Restore the en passant captured piece if there was one
        if en_passant_capture:
            self.add_piece(en_passant_capture)

        piece.has_moved = has_moved_before
        self.update_piece_score(piece)

        # Restore the en passant status for the pawn that moved two squares
        if isinstance(piece, Pawn) and abs(to_pos[1] - from_pos[1]) == 2:
            piece.en_passant_target = True

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

    def piece_has_moved(self, piece):
        piece.has_moved = True
    
    def promote_pawn(self, pawn):
        # Remove the pawn
        self.remove_piece(pawn)
        # Create a new queen at the same position
        new_queen = Queen(pawn.grid_position, pawn.is_white)
        self.add_piece(new_queen)
        return new_queen

    def is_on_board(self, position):
        return 0 <= position[0] < 8 and 0 <= position[1] < 8

    def is_in_check(self, is_white):
        #start_time = time.ticks_ms()
        king_position = None

        # Find the king's position
        for piece in self.pieces:
            if isinstance(piece, King) and piece.is_white == is_white:
                king_position = piece.grid_position
                break

        # If no king is found, return False (should not happen in a standard game)
        if king_position is None:
            return False

        enemy_color = not is_white

        # Directions for rooks, bishops, and queens
        for i, d in enumerate(self.directions):
            for j in range(1, 8):  # Check the entire row/column/diagonal in that direction
                end_square = (king_position[0] + d[0] * j, king_position[1] + d[1] * j)
                if not self.is_on_board(end_square):
                    break
                piece = self.piece_positions.get(end_square)
                if piece is None:
                    continue
                if piece.is_white == is_white:
                    break
                if piece.is_white == enemy_color:
                    piece_type = type(piece)
                    if (i < 4 and piece_type == Rook) or \
                    (i >= 4 and piece_type == Bishop) or \
                    (piece_type == Queen) or \
                    (j == 1 and piece_type == King):
                        return True
                    # Encountering any piece (enemy or allied) that cannot put the king in check should stop further checks in this direction
                    break

        # Check for knight checks
        for d in self.knight_moves:
            end_square = (king_position[0] + d[0], king_position[1] + d[1])
            if self.is_on_board(end_square):
                piece = self.piece_positions.get(end_square)
                if piece and isinstance(piece, Knight) and piece.is_white == enemy_color:
                    return True

        # Check for pawn checks
        pawn_attack_directions = [(-1, -1), (1, -1)] if is_white else [(-1, 1), (1, 1)]
        for d in pawn_attack_directions:
            end_square = (king_position[0] + d[0], king_position[1] + d[1])
            if self.is_on_board(end_square):
                piece = self.piece_positions.get(end_square)
                if piece and isinstance(piece, Pawn) and piece.is_white == enemy_color:
                    return True

        #Uncomment the following line if logging time is necessary
        #log_time(start_time, f'is_in_check for {"white" if is_white else "black"}')
        
        return False

    def get_all_safe_moves(self, is_white, sort=False):
        #start_time = time.ticks_ms()
        safe_moves = []

        for piece in self.pieces:
            if piece.is_white == is_white:
                moves = piece.safe_moves(self)
                safe_moves.extend((piece, move) for move in moves)

        if sort:
            safe_moves.sort(key=lambda move: self.piece_positions.get(move[1]) is not None, reverse=True)

        #log_time(start_time, f'get_all_safe_moves for {"white" if is_white else "black"}')
        return safe_moves

    
    def get_all_valid_moves(self, is_white, sort=False):
        #start_time = time.ticks_ms()
        valid_moves = []

        for piece in self.pieces:
            if piece.is_white == is_white:
                moves = piece.valid_moves(self)
                valid_moves.extend((piece, move) for move in moves)

        if sort:
            valid_moves.sort(key=lambda move: self.get_piece_at_position(move[1]) is not None, reverse=True)

        #log_time(start_time, f'get_all_valid_moves for {"white" if is_white else "black"}')
        return valid_moves
    
    def check_for_checkmate_or_stalemate(self, current_player_is_white):
        #start_time = time.ticks_ms()
        is_checkmate = True
        is_stalemate = True

        for piece in self.pieces:
            if piece.is_white == current_player_is_white:
                safe_moves = piece.safe_moves(self)
                if safe_moves:
                    is_checkmate = False
                    is_stalemate = False
                    break
        
        #double check
        if is_checkmate:
            if self.get_all_safe_moves(current_player_is_white):
                is_checkmate = False
                is_stalemate = False

        if self.is_in_check(current_player_is_white):
            is_stalemate = False
        else:
            is_checkmate = False
        #log_time(start_time, 'check_for_checkmate_or_stalemate')
        return is_checkmate, is_stalemate

class ChessGame(Rectangle2DNode):
    def __init__(self, camera, player_is_white):
        super().__init__(self)
        self.camera = camera
        self.chessboard = ChessBoard(player_is_white)
        self.add_child(self.chessboard)
        self.layer = 0
        self.current_player_is_white = True
        self.selected_piece = None
        self.winner_message = None
        self.chessboard.setup_pieces(is_white=False)  # Black pieces
        self.chessboard.setup_pieces(is_white=True)   # White pieces
        self.player_is_white = player_is_white
        self.print_board_state()
        self.move_mode = False
        self.process_ai_move = False
        self.post_ai_check = False
        self.last_move = None
        self.moves = []
        self.endgame = False
        # Choose a random opening if the AI is white
        self.ai_opening_moves = None
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
        white_material_value = sum(piece_values[get_piece_char(piece).upper()] for piece in self.chessboard.board.pieces if piece.is_white and not isinstance(piece, King))
        black_material_value = sum(piece_values[get_piece_char(piece).upper()] for piece in self.chessboard.board.pieces if not piece.is_white and not isinstance(piece, King))
        
        endgame_threshold = 800  # Adjust this threshold based on testing and observations
        self.endgame = white_material_value <= endgame_threshold or black_material_value <= endgame_threshold


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

        self.chessboard.clear_highlight()
        if self.move_mode and self.selected_piece:
            self.chessboard.highlight_square(self.selected_piece.grid_position)
        self.chessboard.select_square(self.selected_grid_position)

        if not ai_turn and self.post_ai_check:
            self.post_ai_check=False
            # Check for checkmate or stalemate after making a move
            is_checkmate, is_stalemate = self.chessboard.board.check_for_checkmate_or_stalemate(self.current_player_is_white)
            if is_checkmate:
                self.winner_message = ("Black wins" if self.current_player_is_white else "White wins")
                return
            elif is_stalemate:
                self.winner_message = "Stalemate!"
                return

        if engine_io.LEFT.is_pressed_autorepeat:
            self.move_cursor((-1, 0))
        elif engine_io.RIGHT.is_pressed_autorepeat:
            self.move_cursor((1, 0))
        elif engine_io.UP.is_pressed_autorepeat:
            self.move_cursor((0, -1))
        elif engine_io.DOWN.is_pressed_autorepeat:
            self.move_cursor((0, 1))
        elif engine_io.A.is_just_pressed:
            self.select_or_move_piece()
        elif engine_io.B.is_just_pressed:
            self.deselect_piece()

        # Process the AI move if the flag is set
        if ai_turn and self.process_ai_move:
            self.process_ai_move = False  # Reset the flag
            self.make_ai_move()
            return

        # Set the flag if it's not the player's turn and no piece is selected
        if ai_turn and not self.selected_piece and not self.process_ai_move:
            self.process_ai_move = True  # Set the flag

    def move_cursor(self, direction):
        if not self.player_is_white:
            direction=(direction[0]*-1,direction[1]*-1)
        new_col = self.selected_grid_position[0] + direction[0]
        new_row = self.selected_grid_position[1] + direction[1]
        if 0 <= new_col < 8 and 0 <= new_row < 8:
            self.selected_grid_position = (new_col, new_row)

    def select_or_move_piece(self):
        self.chessboard.clear_ai_move_highlight()
        col, row = self.selected_grid_position
        if self.selected_piece:
            new_col, new_row = self.selected_grid_position
            if (new_col, new_row) != self.selected_piece.grid_position:
                if (new_col, new_row) in self.selected_piece.safe_moves(self.chessboard.board):
                    if isinstance(self.selected_piece, King) and abs(new_col - self.selected_piece.grid_position[0]) > 1:
                        if self.chessboard.board.is_in_check(self.current_player_is_white):
                            return  # Cannot castle out of check
                    self.make_move(self.selected_piece, (new_col, new_row))
                    is_checkmate, is_stalemate = self.chessboard.board.check_for_checkmate_or_stalemate(self.current_player_is_white)
                    if is_checkmate:
                        self.winner_message = ("Black wins" if self.current_player_is_white else "White wins")
                    elif is_stalemate:
                        self.winner_message = "Stalemate!"
        else:
            selected_piece = self.chessboard.board.piece_positions.get((col, row))
            if selected_piece and selected_piece.is_white == self.current_player_is_white:
                self.selected_piece = selected_piece
                self.move_mode = True

    def deselect_piece(self):
        if self.selected_piece:
            self.selected_piece = None
            self.move_mode = False

    def print_board_state(self):
        #print(board_to_string(self.chessboard.board, self.player_is_white))
        return

    def make_ai_move(self):
        #start_time = time.ticks_ms()
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
                try:
                    opening_move = opening_moves[len(self.moves)]
                    print(opening_name)
                except:
                    opening_move = None

        if opening_move:
            # Play the next move in the opening
            piece_type, from_pos, to_pos = self.algebraic_to_positions(opening_move, self.current_player_is_white)
            if from_pos is None:
                # If from_pos is not determined, find the piece based on the to_pos and type
                for p in self.chessboard.board.pieces:
                    if p.is_white != self.current_player_is_white:
                        continue
                    if piece_type and get_piece_notation(p) != piece_type:
                        continue
                    if to_pos in p.valid_moves(self.chessboard.board):
                        from_pos = p.grid_position
                        break
        else:
            # Update endgame flag
            self.update_endgame_flag()
            # Use minimax if no opening is tracked or opening moves are exhausted
            global MAX_DEPTH
            if self.endgame:
                MAX_DEPTH = 3
     
            engine.freq(250 * 1000 * 1000)

            eval_score, best_move = minimax(self.chessboard.board, depth=MAX_DEPTH, is_white=not self.player_is_white, alpha=float('-inf'), beta=float('inf'))

            engine.freq(150 * 1000 * 1000)

            if not best_move:
                return
            p, to_pos = best_move
            from_pos = p.grid_position

        # Execute the AI move
        self.execute_move(from_pos, to_pos)
        self.chessboard.render_pieces()
        self.post_ai_check = True
        #log_time(start_time, 'make_ai_move')
        #print_timing_data()

    def execute_move(self, from_pos, to_pos):
        piece = self.chessboard.board.piece_positions.get(from_pos)
        if piece:
            self.chessboard.highlight_ai_move(from_pos, to_pos)
            self.make_move(piece, to_pos)
            
    def make_move(self, piece, to_pos):
        from_pos = piece.grid_position

        # Generate move notation before updating the board state
        move_notation = generate_move_notation(piece, piece.grid_position, to_pos, self.chessboard.board)

        # Determine the move type and play the appropriate sound
        captured_piece = self.chessboard.board.piece_positions.get(to_pos)
        if captured_piece:
            engine_audio.play(take_sound, 0, False)
        elif isinstance(piece, Pawn):
            engine_audio.play(pawn_sound, 0, False)
        else:
            engine_audio.play(move_sound, 0, False)

        # Make the move
        self.chessboard.board.make_move(from_pos, to_pos)

        # Track and print move
        self.moves.append(move_notation)
        #print(generate_pgn_moves_list(self.moves))

        # Check for opening
        self.opening_name = check_opening(self.moves)
        #if self.opening_name:
            #print(self.opening_name)

        # Evaluate board and update evaluation line
        evaluation_score = evaluate_board(self.chessboard.board)
        self.update_evaluation_line(evaluation_score)

        self.selected_piece = None
        self.move_mode = False
        self.current_player_is_white = not self.current_player_is_white

        self.print_board_state()
        self.chessboard.render_pieces()

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
            for piece in self.chessboard.board.pieces:
                if isinstance(piece, Pawn) and piece.is_white == is_white:
                    valid_moves = piece.valid_moves(self.chessboard.board)
                    if to_pos in valid_moves:
                        if not from_file or piece.grid_position[0] == (ord(from_file) - ord('a')):
                            possible_pieces.append(piece)
        else:
            for piece in self.chessboard.board.pieces:
                if piece.is_white == is_white and get_piece_notation(piece) == piece_type:
                    valid_moves = piece.valid_moves(self.chessboard.board)
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
    target_piece = board.piece_positions.get(to_pos)
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

def board_to_string(board, player_is_white):
    # Create a 2D array to represent the board
    board_state = [[' ' for _ in range(8)] for _ in range(8)]
    
    # Place pieces on the board
    for piece in board.pieces:
        col, row = piece.grid_position
        piece_char = get_piece_char(piece)
        board_state[row][col] = piece_char

    # Generate the board string
    
    if player_is_white:
        board_str = "  a b c d e f g h\n"
        board_str += " +---------------+\n"
        for i in range(8):
            row = i
            board_str += f"{8-i}|{' '.join(board_state[row])}|{8-i}\n"
        board_str += " +---------------+\n"
        board_str += "  a b c d e f g h"
    else:
        board_str = "  h g f e d c b a\n"
        board_str += " +---------------+\n"
        for i in range(8):
            row = 7-i  # Print from the bottom (1) to top (8)
            board_str += f"{i+1}|{' '.join(board_state[row][::-1])}|{i+1}\n"
        board_str += " +---------------+\n"
        board_str += "  h g f e d c b a"
    
    
    return board_str

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
    return score

CHECKMATE_SCORE = 60000
STALEMATE_SCORE = 0

def minimax(board, depth, is_white, alpha, beta):
    if depth == 0:
        return evaluate_board(board), None

    best_move = None
    all_moves = board.get_all_safe_moves(is_white, sort=True)

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
            if isinstance(piece, King) and abs(from_pos[0] - to_pos[0]) > 1:
                if board.is_in_check(is_white):
                    continue
            board.make_move(from_pos, to_pos)
            eval, _ = minimax(board, depth - 1, False, alpha, beta)
            board.undo_move()
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
            if isinstance(piece, King) and abs(from_pos[0] - to_pos[0]) > 1:
                if board.is_in_check(is_white):
                    continue
            board.make_move(from_pos, to_pos)
            eval, _ = minimax(board, depth - 1, True, alpha, beta)
            board.undo_move()
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
