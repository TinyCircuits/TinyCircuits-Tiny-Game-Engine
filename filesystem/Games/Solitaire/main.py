import engine_main
import engine
import engine_draw
import engine_io
from engine_nodes import Sprite2DNode, CameraNode, Rectangle2DNode
from engine_resources import TextureResource
from engine_math import Vector2, Vector3
from engine_draw import Color
import random

# Load card sprite texture
cards_texture = TextureResource("Cards.bmp")

# Define a class for the card sprite
class CardSprite(Sprite2DNode):
    def __init__(self, position, frame_x, frame_y):
        super().__init__(self, position)
        self.texture = cards_texture
        self.frame_count_x = 13  # 13 ranks in a suit
        self.frame_count_y = 5   # 4 suits + extras
        self.playing = False
        self.original_frame_x = frame_x
        self.original_frame_y = frame_y
        self.frame_current_x = 2  # Start with the card back
        self.frame_current_y = 4  # Start with the card back
        self.transparent_color = Color(0x0400)
        self.revealed = False

    def flip(self, face_up=True):
        if face_up:
            self.frame_current_x = self.original_frame_x
            self.frame_current_y = self.original_frame_y
            self.set_layer(1)
        else:
            self.frame_current_x = 2
            self.frame_current_y = 4
            self.set_layer(0)
        self.revealed = face_up

# Define a class for the deck
class Deck:
    def __init__(self):
        self.cards = self.create_deck()
        self.shuffle()

    def create_deck(self):
        cards = []
        for suit in range(4):  # 4 suits
            for rank in range(13):  # 13 ranks
                card_position = Vector2(10, 12)  # Off-screen initially
                card = CardSprite(card_position, rank, suit)
                cards.append(card)
        return cards

    def draw_card(self):
        if self.cards:
            return self.cards.pop()
        return None
    
    def shuffle(self):
        # Implementing a Fisher-Yates shuffle algorithm
        n = len(self.cards)
        for i in range(n - 1, 0, -1):
            j = random.randint(0, i)
            self.cards[i], self.cards[j] = self.cards[j], self.cards[i]

# Define a class for the discard pile
class DiscardPile:
    def __init__(self, position):
        self.position = position
        self.cards = []
        self.placeholder = CardSprite(position, 4, 4)
        self.placeholder.flip(face_up=True)

    def add_card(self, card):
        card.position = self.position
        card.flip(face_up=True)
        self.cards.append(card)

# Define a class for the columns
class Column:
    def __init__(self, position, index):
        self.position = position
        self.index = index
        self.cards = []

    def add_card(self, card):
        card.position = Vector2(self.position.x, self.position.y + len(self.cards) * 4)
        self.cards.append(card)

    def remove_card(self):
        if self.cards:
            return self.cards.pop()
        return None

# Define a class for the foundation piles
class FoundationPile:
    def __init__(self, position, suit):
        self.position = position
        self.suit = suit
        self.cards = []
        self.placeholder = CardSprite(position, 4, 4)
        self.placeholder.flip(face_up=True)

    def add_card(self, card):
        card.position = self.position
        self.cards.append(card)

# Define a class for the game logic
class SolitaireGame(Rectangle2DNode):
    def __init__(self, position, width, height):
        super().__init__(self, position, width, height)
        self.deck = Deck()
        self.discard_pile = DiscardPile(Vector2(22, 12))
        self.columns = [Column(Vector2(46 + i * 12, 12), i) for i in range(7)]
        self.foundation_piles = [FoundationPile(Vector2(10 + (i % 4) * 12, 101 + (i // 4) * 16), i) for i in range(4)]
        self.hand_indicator = None
        self.color = Color(0x0400)
        self.selected_card = None
        self.current_position_index = 0
        self.current_row = 'top'
        self.setup_board()
        self.positions_top = [
            Vector2(10, 12),  # Deck
            Vector2(22, 12)   # Discard
        ] + [col.position for col in self.columns]  # Columns 1-7
        self.positions_bottom = [pile.position for pile in self.foundation_piles]  # Foundation piles 1-4

    def setup_board(self):
        # Set up initial card layout
        for i, column in enumerate(self.columns):
            for j in range(i + 1):
                card = self.deck.draw_card()
                card.flip(face_up=(j == i))  # Only the top card is revealed
                column.add_card(card)
                self.add_child(card)

        self.hand_indicator = Sprite2DNode(Vector2(10, 24))
        self.hand_indicator.texture = cards_texture
        self.hand_indicator.frame_count_x = 13
        self.hand_indicator.frame_count_y = 5
        self.hand_indicator.frame_current_x = 5
        self.hand_indicator.frame_current_y = 4
        self.hand_indicator.playing = False
        self.hand_indicator.set_layer(2)
        self.hand_indicator.transparent_color = Color(0x0400)
        self.add_child(self.hand_indicator)

        # Add placeholders for discard and foundation piles
        self.add_child(self.discard_pile.placeholder)
        for pile in self.foundation_piles:
            self.add_child(pile.placeholder)

    def tick(self, dt):
        # Handle user input and game logic here
        if engine_io.check_just_pressed(engine_io.DPAD_UP):
            self.switch_row()
        elif engine_io.check_just_pressed(engine_io.DPAD_DOWN):
            self.switch_row()
        if engine_io.check_just_pressed(engine_io.DPAD_LEFT):
            self.move_left()
        elif engine_io.check_just_pressed(engine_io.DPAD_RIGHT):
            self.move_right()
        if engine_io.check_just_pressed(engine_io.A):
            self.handle_selection()
        
        self.update_hand_indicator_position()

    def move_left(self):
        if self.current_row == 'top':
            self.current_position_index = (self.current_position_index - 1) % len(self.positions_top)
        else:
            self.current_position_index = (self.current_position_index - 1) % len(self.positions_bottom)

    def move_right(self):
        if self.current_row == 'top':
            self.current_position_index = (self.current_position_index + 1) % len(self.positions_top)
        else:
            self.current_position_index = (self.current_position_index + 1) % len(self.positions_bottom)

    def switch_row(self):
        if self.current_row == 'top':
            self.current_row = 'bottom'
            self.current_position_index = self.current_position_index % len(self.positions_bottom)
        else:
            self.current_row = 'top'
            self.current_position_index = self.current_position_index % len(self.positions_top)

    def update_hand_indicator_position(self):
        if self.current_row == 'top':
            new_position = self.positions_top[self.current_position_index]
            if self.current_position_index > 1:  # If it's a column, adjust the y position
                column = self.columns[self.current_position_index - 2]
                if column.cards:
                    new_position = Vector2(new_position.x, new_position.y + (len(column.cards) - 1) * 4 )
                else:
                    new_position = Vector2(new_position.x, new_position.y )
        else:
            new_position = self.positions_bottom[self.current_position_index]
        new_position = Vector2(new_position.x, new_position.y + 12)
        self.hand_indicator.position = new_position

    def handle_selection(self):
        # Implement card selection and movement logic here
        # This is a placeholder for the logic to move cards between columns, foundation piles, and the discard pile
        pass

# Make an instance of our game
game = SolitaireGame(Vector2(0, 0), 256, 256)

# Make a camera to render the scene
camera = CameraNode()
camera.position = Vector3(128 / 2, 128 / 2, 1)

# Start rendering the scene
engine.start()
