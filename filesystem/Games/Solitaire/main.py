import engine_main
import engine
import engine_draw
import engine_io
from engine_nodes import Sprite2DNode, CameraNode, Rectangle2DNode
from engine_resources import TextureResource
from engine_math import Vector2, Vector3
from engine_draw import Color
from engine_animation import Tween, ONE_SHOT, EASE_SINE_IN
import random

# Load card sprite texture
cards_texture = TextureResource("BiggerCards2.bmp")

tweens = []

def tween_card(card, target):
    tw = Tween()
    tw.start(card,'position',card.position, target, 200, 5, ONE_SHOT, EASE_SINE_IN)
    tweens.append(tw)

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
            self.layer = 1
        else:
            self.frame_current_x = 2
            self.frame_current_y = 4
            self.layer = 0
        self.revealed = face_up

# Define a class for the deck
class Deck:
    def __init__(self):
        self.cards = self.create_deck()
        self.placeholder = CardSprite(Vector2(8, 12), 4, 4)
        self.placeholder.flip(face_up=True)
        self.show_placeholder()
        self.shuffle()

    def create_deck(self):
        cards = []
        for suit in range(4):  # 4 suits
            for rank in range(13):  # 13 ranks
                card_position = Vector2(8, 12) 
                card = CardSprite(card_position, rank, suit)
                cards.append(card)
        return cards

    def draw_card(self):
        if self.cards:
            card = self.cards.pop()
            self.show_placeholder()
            return card
        return None
    
    def shuffle(self):
        # Implementing a Fisher-Yates shuffle algorithm
        n = len(self.cards)
        for i in range(n - 1, 0, -1):
            j = random.randint(0, i)
            self.cards[i], self.cards[j] = self.cards[j], self.cards[i]
    
    def is_empty(self):
        return len(self.cards) == 0

    def refill_from_discard(self, discard_pile):
        self.cards = discard_pile.cards[::-1]  # Reverse the order of discard pile
        self.show_placeholder()
        for card in self.cards:
            card.flip(face_up=False)
            card.position = Vector2(8, 12)  # Move card back to deck position
        discard_pile.cards.clear()

    def show_placeholder(self):
        if not self.cards:
            self.placeholder.opacity = 1
        else:
            self.placeholder.opacity = 0

# Define a class for the discard pile
class DiscardPile:
    def __init__(self, position):
        self.position = position
        self.cards = []
        self.placeholder = CardSprite(position, 4, 4)
        self.placeholder.flip(face_up=True)

    def add_card(self, card):
        tween_card(card, self.position)
        card.flip(face_up=True)
        self.cards.append(card)

    def remove_top_card(self):
        if self.cards:
            return self.cards.pop()
        return None
    
    def reveal_top_card(self):
        if self.cards and not self.cards[-1].revealed:
            self.cards[-1].flip(face_up=True)

# Define a class for the columns
class Column:
    def __init__(self, position, index):
        self.position = position
        self.index = index
        self.cards = []
        self.placeholder = CardSprite(position, 4, 4)
        self.placeholder.flip(face_up=True)
        self.show_placeholder()

    def add_card(self, card):
        # Set all existing cards to layer 0
        for c in self.cards:
            c.layer = 0
        tween_card(card, Vector2(self.position.x, self.position.y + len(self.cards) * 5))
        card.layer = 1  # Set the new card to layer 1
        self.cards.append(card)
        self.show_placeholder()

    def add_cards(self, cards):
        for card in cards:
            tween_card(card, Vector2(self.position.x, self.position.y + len(self.cards) * 5))
            card.layer = 1
            self.cards.append(card)
        self.show_placeholder()

    def remove_top_cards(self, start_index):
        removed_cards = self.cards[start_index:]
        self.cards = self.cards[:start_index]
        self.show_placeholder()
        return removed_cards

    def remove_top_card(self):
        if self.cards:
            card = self.cards.pop()
            self.show_placeholder()
            return card
        return None

    def reveal_top_card(self):
        if self.cards and not self.cards[-1].revealed:
            self.cards[-1].flip(face_up=True)

    def show_placeholder(self):
        if not self.cards:
            self.placeholder.opacity = 1
        else:
            self.placeholder.opacity = 0

# Define a class for the foundation piles
class FoundationPile:
    def __init__(self, position):
        self.position = position
        self.suit = None
        self.cards = []
        self.placeholder = CardSprite(position, 4, 4)
        self.placeholder.flip(face_up=True)

    def add_card(self, card):
        for c in self.cards:
            c.layer = 0
        card.layer = 1
        tween_card(card, self.position)
        self.cards.append(card)
        self.show_placeholder()

    def can_add_card(self, card):
        if not self.cards and card.original_frame_x == 0:  # Must be an Ace to start
            self.suit = card.original_frame_y
            return True
        if self.cards and card.original_frame_y == self.suit and card.original_frame_x == self.cards[-1].original_frame_x + 1:
            return True
        return False
    
    def show_placeholder(self):
        if not self.cards:
            self.placeholder.opacity = 1
        else:
            self.placeholder.opacity = 0

# Define a class for the game logic
class SolitaireGame(Rectangle2DNode):
    def __init__(self, position, width, height):
        super().__init__(self, position, width, height)
        self.deck = Deck()
        self.discard_pile = DiscardPile(Vector2(27, 12))
        self.columns = [Column(Vector2(8 + i * 18 + (1 if i != 0 else 0), 40), i) for i in range(7)]
        self.foundation_piles = [FoundationPile(Vector2(63 + (i % 4) * 18, 12)) for i in range(4)]
        self.hand_indicator = None
        self.color = Color(0x0400)
        self.selected_cards = []
        self.selected_card_origin = None
        self.current_position_index = 0
        self.selected_column_index = -1  # Initialize selected_column_index
        self.selected_card_index = 0  # Initialize selected_card_index
        self.indicator_moved = False
        self.current_row = 'top'
        self.setup_board()
        self.positions_top = [
            Vector2(8, 12),  # Deck
            Vector2(26, 12)   # Discard
        ] + [pile.position for pile in self.foundation_piles]  # Foundation piles 1-4
        self.positions_bottom = [col.position for col in self.columns]  # Columns 1-7
        self.previous_highlighted_card = None
        # New attributes for end-game animation
        self.end_game_timer = 0
        self.end_game_active = False
        self.end_game_card_queue = []

    def setup_board(self):
        # Set up initial card layout
        for i, column in enumerate(self.columns):
            for j in range(i + 1):
                card = self.deck.draw_card()
                card.flip(face_up=(j == i))  # Only the top card is revealed
                column.add_card(card)
                self.add_child(card)

        self.hand_indicator = Sprite2DNode(Vector2(8, 26))
        self.hand_indicator.texture = cards_texture
        self.hand_indicator.frame_count_x = 13
        self.hand_indicator.frame_count_y = 5
        self.hand_indicator.frame_current_x = 5
        self.hand_indicator.frame_current_y = 4
        self.hand_indicator.playing = False
        self.hand_indicator.layer = 2
        self.hand_indicator.transparent_color = Color(0x0400)
        self.add_child(self.hand_indicator)

        # Add placeholders for deck, discard, and foundation piles
        self.add_child(self.deck.placeholder)
        self.add_child(self.discard_pile.placeholder)
        for pile in self.foundation_piles:
            self.add_child(pile.placeholder)

    def tick(self, dt):
        # Handle tweens
        for tween in tweens[:]:
            tween.tick(dt)
            if tween.finished:
                tweens.remove(tween)
                
        # Handle end-game card movements every 100ms
        if self.end_game_active and self.end_game_card_queue:
            self.end_game_timer += dt
            if self.end_game_timer >= 0.2:
                self.end_game_timer = 0
                card, pile = self.end_game_card_queue.pop(0)
                pile.add_card(card)
                for column in self.columns:
                    column.show_placeholder()
                if not self.end_game_card_queue:
                    self.end_game_active = False
        
        # Check and trigger end-game condition
        if not self.end_game_active:
            self.move_cards_to_foundation_automatically()
        
        # Ignore key inputs if tweens are active
        if tweens or self.end_game_active:
            return

        # Handle user input and game logic here
        if engine_io.UP.is_just_pressed:
            self.move_up()
        elif engine_io.DOWN.is_just_pressed:
            self.move_down()
        if engine_io.LEFT.is_just_pressed:
            self.move_left()
        elif engine_io.RIGHT.is_just_pressed:
            self.move_right()
        if engine_io.A.is_just_pressed:
            self.handle_selection()
        if engine_io.B.is_just_pressed:
            self.undo_selection()
        
        self.update_hand_indicator_position()

    def move_up(self):
        self.clear_highlight()
        self.indicator_moved = True
        if self.current_row == 'bottom':
            column_index = self.current_position_index
            column = self.columns[column_index]
            if column.cards and self.selected_column_index == column_index:
                if self.selected_card_index > 0:
                    if column.cards[self.selected_card_index - 1].revealed:
                        self.selected_card_index -= 1
                        self.highlight_card()
                    else:
                        self.switch_row()
                else:
                    self.switch_row()
            else:
                self.switch_row()
        else:
            self.switch_row()

    def move_down(self):
        self.clear_highlight()
        self.indicator_moved = True
        if self.current_row == 'bottom':
            column_index = self.current_position_index
            column = self.columns[column_index]
            if column.cards and self.selected_column_index == column_index:
                if self.selected_card_index < len(column.cards) - 1:
                    if column.cards[self.selected_card_index + 1].revealed:
                        self.selected_card_index += 1
                        if(self.selected_card_index < len(column.cards) - 1):
                            self.highlight_card()
                    else:
                        self.switch_row()
                else:
                    self.switch_row()
            else:
                self.switch_row()
        else:
            self.switch_row()


    def move_left(self):
        self.clear_highlight()
        self.indicator_moved = True
        if self.current_row == 'top':
            self.current_position_index = (self.current_position_index - 1) % len(self.positions_top)
        else:
            self.current_position_index = (self.current_position_index - 1) % len(self.positions_bottom)
        self.update_selected_column_index()

    def move_right(self):
        self.clear_highlight()
        self.indicator_moved = True
        if self.current_row == 'top':
            self.current_position_index = (self.current_position_index + 1) % len(self.positions_top)
        else:
            self.current_position_index = (self.current_position_index + 1) % len(self.positions_bottom)
        self.update_selected_column_index()

    def highlight_card(self):
        self.previous_highlighted_card = self.columns[self.current_position_index].cards[self.selected_card_index]
        self.previous_highlighted_card.position.y -= 5

    def clear_highlight(self):
        if(self.previous_highlighted_card):
            self.previous_highlighted_card.position.y += 5
            self.previous_highlighted_card = None

    def update_selected_column_index(self):
        if self.current_row == 'bottom':
            self.selected_column_index = self.current_position_index
            column = self.columns[self.selected_column_index]
            if column.cards:
                # Select the top face-up card
                self.selected_card_index = len(column.cards) - 1
                for i in reversed(range(len(column.cards))):
                    if column.cards[i].revealed:
                        self.selected_card_index = i
                        break
        else:
            self.selected_column_index = -1
            self.selected_card_index = 0

    def switch_row(self):
        index_mapping_bottom_to_top = [0, 1, 1, 2, 3, 4, 5]
        index_mapping_top_to_bottom = [0, 1, 3, 4, 5, 6]

        if self.current_row == 'top':
            self.current_row = 'bottom'
            self.current_position_index = index_mapping_top_to_bottom[self.current_position_index]
        else:
            self.current_row = 'top'
            self.current_position_index = index_mapping_bottom_to_top[self.current_position_index]

        self.update_selected_column_index()

    def update_hand_indicator_position(self):
        if self.current_row == 'top':
            new_position = self.positions_top[self.current_position_index]
        else:
            new_position = self.positions_bottom[self.current_position_index]
            column = self.columns[self.current_position_index]
            if column.cards:
                if self.selected_column_index == self.current_position_index:
                    new_position = Vector2(new_position.x, new_position.y + self.selected_card_index * 5)
                else:
                    # Select the top face-up card
                    face_up_index = len(column.cards) - 1
                    for i in reversed(range(len(column.cards))):
                        if column.cards[i].revealed:
                            face_up_index = i
                            break
                    new_position = Vector2(new_position.x, new_position.y + face_up_index * 5)

        new_indicator_position = Vector2(new_position.x, new_position.y + 26)
        if(self.hand_indicator.position.x != new_indicator_position.x or self.hand_indicator.position.y != new_indicator_position.y):
            tween_card(self.hand_indicator, new_indicator_position)

        # Update the selected card's position to match the hand indicator's position
        if self.selected_cards:
            for i, card in enumerate(self.selected_cards):
                new_card_position = Vector2(new_indicator_position.x, new_indicator_position.y + i * 5)
                if(card.position.x != new_card_position.x or card.position.y != new_card_position.y):
                    tween_card(card, new_card_position)
            self.hand_indicator.opacity = 0  # Hide the hand indicator
        else:
            self.hand_indicator.opacity = 1  # Show the hand indicator


    def handle_selection(self):
        self.clear_highlight()
        # New code to automatically move selected card to foundation if 'A' is pressed again
        if self.selected_cards and len(self.selected_cards) == 1 and not self.indicator_moved:
            for pile in self.foundation_piles:
                if pile.can_add_card(self.selected_cards[0]):
                    self.move_card_to_foundation(pile)
                    self.selected_cards = []
                    self.hand_indicator.opacity = 1  # Show the hand indicator
                    self.update_selected_column_index()
                    return
        if self.selected_cards:
            if self.current_row == 'bottom':
                if self.move_cards_to_column(self.columns[self.current_position_index]):
                    self.selected_cards = []
            else:
                if self.move_card_to_foundation(self.foundation_piles[self.current_position_index - 2]):
                    self.selected_cards = []
            if not self.selected_cards:
                self.hand_indicator.opacity = 1  # Show the hand indicator
        else:
            if self.current_row == 'top':
                if self.current_position_index == 0:
                    self.draw_from_deck()
                elif self.current_position_index == 1:
                    self.select_from_discard_pile()
                else:
                    self.select_from_foundation(self.foundation_piles[self.current_position_index - 2])
            else:
                self.select_from_column(self.columns[self.current_position_index])

    def draw_from_deck(self):
        if self.deck.is_empty():
            if self.discard_pile.cards:
                self.deck.refill_from_discard(self.discard_pile)
        else:
            card = self.deck.draw_card()
            if card:
                self.discard_pile.add_card(card)
                self.add_child(card)

    def select_from_discard_pile(self):
        if self.discard_pile.cards:
            card = self.discard_pile.remove_top_card()
            self.selected_cards = [card]
            self.selected_card_origin = self.discard_pile
            self.indicator_moved = False
            for c in self.selected_cards:
                c.layer = 3
            self.add_child(card)

    def select_from_column(self, column):
        if column.cards:
            face_up_index = len(column.cards) - 1
            for i, card in enumerate(column.cards):
                if card.revealed:
                    face_up_index = i
                    break
            if face_up_index == -1:
                self.selected_column_index = -1
                self.selected_card_index = 0
                return

            if self.selected_column_index == column.index:
                # Picking up cards from a selected column
                self.selected_cards = column.remove_top_cards(self.selected_card_index)
                for c in self.selected_cards:
                    c.layer = 3
                self.selected_card_origin = column
                self.indicator_moved = False
                self.selected_column_index = -1  # Reset column selection after picking up cards
            else:
                # Select the column and initialize the selected card index
                self.selected_column_index = column.index
                self.selected_card_index = face_up_index
        else:
            self.selected_column_index = -1
            self.selected_card_index = 0

    def move_cards_to_column(self, column):
        bottom_card = self.selected_cards[0]
        if not column.cards and bottom_card.original_frame_x == 12:  # Must be a King to place in empty column
            column.add_cards(self.selected_cards)
            if self.selected_card_origin:
                self.selected_card_origin.reveal_top_card()
            self.selected_cards = []
            self.selected_card_origin = None
            # Reset the selected card and column indices
            self.selected_column_index = column.index
            self.selected_card_index = len(column.cards) - 1
            self.update_hand_indicator_position()
            return True
        elif column.cards:
            target_card = column.cards[-1]
            if bottom_card.original_frame_x == target_card.original_frame_x - 1 and ((bottom_card.original_frame_y % 2 == 0 and target_card.original_frame_y % 2 == 1) or (bottom_card.original_frame_y % 2 == 1 and target_card.original_frame_y % 2 == 0)):
                column.add_cards(self.selected_cards)
                if self.selected_card_origin:
                    self.selected_card_origin.reveal_top_card()
                self.selected_cards = []
                self.selected_card_origin = None
                # Reset the selected card and column indices
                self.selected_column_index = column.index
                self.selected_card_index = len(column.cards) - 1
                self.update_hand_indicator_position()
                return True
        return False


    def select_from_foundation(self, pile):
        # Typically you don't move cards back from foundation in solitaire, so we'll keep this empty
        pass

    def move_card_to_foundation(self, pile):
        if len(self.selected_cards) == 1 and pile.can_add_card(self.selected_cards[0]):
            pile.add_card(self.selected_cards[0])
            if self.selected_card_origin:
                self.selected_card_origin.reveal_top_card()
            return True
        return False

    def undo_selection(self):
        self.clear_highlight()
        if self.selected_cards:
            if isinstance(self.selected_card_origin, Column):
                self.selected_card_origin.add_cards(self.selected_cards)
                # Reset the selected card and column indices
                self.selected_column_index = self.selected_card_origin.index
                self.selected_card_index = len(self.selected_card_origin.cards) - len(self.selected_cards)
                self.update_selected_column_index()
            elif isinstance(self.selected_card_origin, DiscardPile):
                self.selected_card_origin.add_card(self.selected_cards[0])
                self.selected_column_index = -1
                self.selected_card_index = 0
            self.selected_cards = []
            self.selected_card_origin = None
            self.update_hand_indicator_position()
            self.hand_indicator.opacity = 1  # Show the hand indicator
        else:
            self.current_row = 'top'
            self.current_position_index = 0

    def check_end_game_condition(self):

        # Check if the deck and discard pile are empty
        if not self.deck.cards and not self.discard_pile.cards and not self.selected_cards:
            # Check if all cards in all columns are face up
            for column in self.columns:
                for card in column.cards:
                    if not card.revealed:
                        return False
            return True
        return False

    def move_cards_to_foundation_automatically(self):
        if self.check_end_game_condition():
            self.end_game_active = True
            scheduled_cards = set()

            def can_schedule_card(card):
                for pile in self.foundation_piles:
                    if pile.can_add_card(card):
                        return True
                return False

            def schedule_card(card):
                for pile in self.foundation_piles:
                    if pile.can_add_card(card):
                        self.end_game_card_queue.append((card, pile))
                        scheduled_cards.add(card)
                        return

            cards_to_check = []

            # Add all cards from columns to the list to check
            for column in self.columns:
                cards_to_check.extend(column.cards)

            while cards_to_check:
                any_card_scheduled = False
                for card in cards_to_check[:]:
                    if card not in scheduled_cards and can_schedule_card(card):
                        schedule_card(card)
                        cards_to_check.remove(card)
                        any_card_scheduled = True
                
                if not any_card_scheduled:
                    # If no cards were scheduled in this pass, break to avoid infinite loop
                    break


# Make an instance of our game
game = SolitaireGame(Vector2(0, 0), 256, 256)

# Make a camera to render the scene
camera = CameraNode()
camera.position = Vector3(128 / 2, 128 / 2, 1)

# Start rendering the scene
engine.start()