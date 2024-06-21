import engine_main
import engine
import engine_draw
import engine_io
from engine_nodes import Sprite2DNode, CameraNode, Rectangle2DNode, Text2DNode
from engine_resources import TextureResource, FontResource
from engine_math import Vector2, Vector3
from engine_draw import Color
from engine_animation import Tween, ONE_SHOT, EASE_SINE_IN
import random
import time

# Load card sprite texture

font = FontResource("munro-narrow_10.bmp")
cards_texture = TextureResource("BiggerCards2ext.bmp")
background = TextureResource("thumbatrobackground.bmp")

tweens = []

def tween_card(card, target, duration=500, speed=2):
    tw = Tween()
    tw.start(card, 'position', card.position, target, duration, speed, ONE_SHOT, EASE_SINE_IN)
    tweens.append(tw)

def tween_opacity(node, start_opacity, end_opacity, duration=1000):
    tw = Tween()
    tw.start(node, 'opacity', start_opacity, end_opacity, duration, 1, ONE_SHOT, EASE_SINE_IN)
    tweens.append(tw)

class CardSprite(Sprite2DNode):
    def __init__(self, position, frame_x, frame_y, base_score_bonus=0, multiplier_bonus=0, is_wildcard_suit=False):
        super().__init__(self, position)
        self.texture = cards_texture
        self.frame_count_x = 13
        self.frame_count_y = 5
        self.original_frame_x = frame_x
        self.original_frame_y = frame_y
        self.frame_current_x = frame_x
        self.frame_current_y = frame_y
        self.playing = False
        self.transparent_color = Color(0x0400)
        self.selected = False
        self.in_hand = True
        self.played = False
        self.discarded = False
        self.set_layer(4)
        self.base_score_bonus = base_score_bonus
        self.multiplier_bonus = multiplier_bonus
        self.is_wildcard_suit = is_wildcard_suit

        # Base layer - card background
        self.background = Sprite2DNode(Vector2(0, 0))
        self.background.frame_count_x = 13
        self.background.frame_count_y = 5
        self.background.frame_current_x = 6
        self.background.frame_current_y = 4
        self.background.texture = cards_texture
        self.background.playing = False
        self.background.set_layer(2)
        self.background.transparent_color = Color(0x0400)
        self.add_child(self.background)

        self.bonus_overlay = None
        if self.base_score_bonus > 0 and self.multiplier_bonus > 0:
            self.bonus_overlay = Sprite2DNode(Vector2(0, 0))
            self.bonus_overlay.frame_count_x = 13
            self.bonus_overlay.frame_count_y = 5
            self.bonus_overlay.frame_current_x = 9
            self.bonus_overlay.frame_current_y = 4
            self.bonus_overlay.texture = cards_texture
            self.bonus_overlay.playing = False
            self.bonus_overlay.set_layer(3)
            self.bonus_overlay.opacity = 0.4
            self.bonus_overlay.transparent_color = Color(0x0400)
            self.add_child(self.bonus_overlay)
        # Overlay sprite for base score bonus
        elif self.base_score_bonus > 0:
            self.bonus_overlay = Sprite2DNode(Vector2(0, 0))
            self.bonus_overlay.frame_count_x = 13
            self.bonus_overlay.frame_count_y = 5
            self.bonus_overlay.frame_current_x = 7
            self.bonus_overlay.frame_current_y = 4
            self.bonus_overlay.texture = cards_texture
            self.bonus_overlay.playing = False
            self.bonus_overlay.set_layer(3)
            self.bonus_overlay.transparent_color = Color(0x0400)
            self.add_child(self.bonus_overlay)
        
        elif self.is_wildcard_suit == True:
            self.bonus_overlay = Sprite2DNode(Vector2(0, 0))
            self.bonus_overlay.frame_count_x = 13
            self.bonus_overlay.frame_count_y = 5
            self.bonus_overlay.frame_current_x = 10
            self.bonus_overlay.frame_current_y = 4
            self.bonus_overlay.texture = cards_texture
            self.bonus_overlay.playing = False
            self.bonus_overlay.set_layer(3)
            self.bonus_overlay.transparent_color = Color(0x0400)
            self.add_child(self.bonus_overlay)

        # Overlay sprite for multiplier bonus
        elif self.multiplier_bonus > 0:
            self.bonus_overlay = Sprite2DNode(Vector2(0, 0))
            self.bonus_overlay.frame_count_x = 13
            self.bonus_overlay.frame_count_y = 5
            self.bonus_overlay.frame_current_x = 8
            self.bonus_overlay.frame_current_y = 4
            self.bonus_overlay.texture = cards_texture
            self.bonus_overlay.playing = False
            self.bonus_overlay.set_layer(3)
            self.bonus_overlay.transparent_color = Color(0x0400)
            self.add_child(self.bonus_overlay)

    @property
    def rank_value(self):
        return 14 if self.original_frame_x == 0 else self.original_frame_x + 1

    def select(self, is_selected):
        self.selected = is_selected
        if self.selected:
            self.position.y -= 5
        else:
            self.position.y += 5

    def mark_played(self):
        self.in_hand = False
        self.played = True

    def mark_discarded(self):
        self.hide()
        self.in_hand = False
        self.discarded = True

    def hide(self):
        self.opacity = 0
        self.background.opacity = 0
        if(self.bonus_overlay):
            self.bonus_overlay.opacity = 0

    def __str__(self):
        return (f"Card(rank={self.rank_value}, suit={self.original_frame_y},position=({self.position.x}, {self.position.y})),is_wildcard_suit={self.is_wildcard_suit}")

    def __repr__(self):
        return self.__str__()
    
    def print_rules(self):
        suits = ["Hearts", "Spades", "Diamonds", "Clubs"]
        short_suits = ["Hrt", "Spd", "Dia", "Clb"]
        rank_names = ["Ace", "2", "3", "4", "5", "6", "7", "8", "9", "10", "Jack", "Queen", "King"]
        short_rank_names = ["A", "2", "3", "4", "5", "6", "7", "8", "9", "10", "J", "Q", "K"]
        
        if self.original_frame_y == 4:
            if self.base_score_bonus > 0 and self.multiplier_bonus > 0:
                rule = f"Joker +{self.base_score_bonus} x{self.multiplier_bonus}"
            elif self.base_score_bonus > 0:
                rule = f"Joker +{self.base_score_bonus}"
            elif self.multiplier_bonus > 0:
                rule = f"Joker x{self.multiplier_bonus}"
            else:
                rule = "Joker"
        else:
            suit = suits[self.original_frame_y]
            short_suit = short_suits[self.original_frame_y]
            rank = rank_names[self.original_frame_x]
            short_rank = short_rank_names[self.original_frame_x]
            if self.is_wildcard_suit:
                rule = f"{rank} Wildcard"
            elif self.base_score_bonus > 0 and self.multiplier_bonus > 0:
                rule = f"{rank} of {suit} +{self.base_score_bonus} x{self.multiplier_bonus}"
            elif self.base_score_bonus > 0:
                rule = f"{rank} of {suit} +{self.base_score_bonus}"
            elif self.multiplier_bonus > 0:
                rule = f"{rank} of {suit} x{self.multiplier_bonus}"
            else:
                rule = f"{rank} of {suit}"
        
            if len(rule) > 20:
                if self.is_wildcard_suit:
                    rule = f"{short_rank} Wildcard"
                elif self.base_score_bonus > 0 and self.multiplier_bonus > 0:
                    rule = f"{short_rank} of {short_suit} +{self.base_score_bonus} x{self.multiplier_bonus}"
                elif self.base_score_bonus > 0:
                    rule = f"{short_rank} of {short_suit} +{self.base_score_bonus}"
                elif self.multiplier_bonus > 0:
                    rule = f"{short_rank} of {short_suit} x{self.multiplier_bonus}"
                else:
                    rule = f"{short_rank} of {short_suit}"
        
        return rule



class Deck:
    def __init__(self, collection):
        self.collection = collection
        self.cards = self.create_deck()
        self.shuffle()

    def create_deck(self):
        return [CardSprite(Vector2(150, 80), card.original_frame_x, card.original_frame_y, card.base_score_bonus, card.multiplier_bonus, card.is_wildcard_suit) for card in self.collection]

    def draw_cards(self, num):
        drawn_cards = []
        for _ in range(num):
            if self.cards:
                drawn_cards.append(self.cards.pop())
        return drawn_cards

    def shuffle(self):
        n = len(self.cards)
        for i in range(n - 1, 0, -1):
            j = random.randint(0, i)
            self.cards[i], self.cards[j] = self.cards[j], self.cards[i]

class PokerGame(Rectangle2DNode):
    def __init__(self, position, width, height):
        super().__init__(self, position, width, height)
        self.player_collection = self.create_initial_collection()
        self.jokers = []
        self.deck = Deck(self.player_collection)
        self.hand = []
        self.selected_cards = []
        self.played_cards = []
        self.current_card_index = 0
        self.hand_indicator = self.create_hand_indicator()
        self.draw_hand()
        self.discard_limit = 4
        self.hands_played = 0
        self.current_game = 1
        self.highest_individual_hand_score = 0
        self.setup_board()
        self.score = 0
        self.target_score = 300
        self.booster_cards = []
        self.is_booster_selection = False
        self.is_booster_wait = False
        self.hand_display_time = None
        self.game_over = False
        self.background = Sprite2DNode(Vector2(63, 63))
        self.background.frame_count_x = 1
        self.background.frame_count_y = 1
        self.background.texture = background
        self.background.playing = False
        self.background.set_layer(0)
        self.add_child(self.background)

        # Text nodes for displaying scores
        self.base_score_text = Text2DNode(Vector2(51, 24), font, "", 0, Vector2(1, 1), 1.0, 0, 0)
        self.base_score_text.set_layer(1)
        self.mult_score_text = Text2DNode(Vector2(85, 24), font, "", 0, Vector2(1, 1), 1.0, 0, 0)
        self.mult_score_text.set_layer(1)
        self.round_text = Text2DNode(Vector2(110, 75), font, "", 0, Vector2(1, 1), 1.0, 0, 0)
        self.round_text.set_layer(1)
        self.hand_type_text = Text2DNode(Vector2(82, 37), font, "", 0, Vector2(1, 1), 1.0, 0, 0)
        self.hand_type_text.set_layer(1)
        self.total_score_text = Text2DNode(Vector2(69, 10), font, "", 0, Vector2(1, 1), 1.0, 0, 0)
        self.total_score_text.set_layer(1)
        self.best_hand_text = Text2DNode(Vector2(44, 57), font, "", 0, Vector2(1, 1), 1.0, 0, 0)
        self.best_hand_text.set_layer(1)
        self.remaining_hands_text = Text2DNode(Vector2(104, 59), font, "", 0, Vector2(1, 1), 1.0, 0, 0)
        self.remaining_hands_text.set_layer(1)
        self.remaining_discard_text = Text2DNode(Vector2(118, 59), font, "", 0, Vector2(1, 1), 1.0, 0, 0)
        self.remaining_discard_text.set_layer(1)

        self.total_score_text.text = f"{self.score}/{self.target_score}"
        self.hand_type_text.text = f"Play/Discard 5 cards"

        self.add_child(self.base_score_text)
        self.add_child(self.round_text)
        self.add_child(self.total_score_text)
        self.add_child(self.remaining_hands_text)
        self.add_child(self.remaining_discard_text)
        self.add_child(self.mult_score_text)
        self.add_child(self.hand_type_text)
        self.add_child(self.best_hand_text)

        self.update_joker_display()
        self.update_game_info()

    def update_game_info(self):  # Add this method
        self.remaining_hands_text.text = f"{4 - self.hands_played}"
        self.remaining_discard_text.text = f"{self.discard_limit}"
        self.round_text.text = f"{self.current_game}"

    def create_initial_collection(self):
        collection = []
        for suit in range(4):
            for rank in range(13):
                card = CardSprite(Vector2(150, 80), rank, suit)
                collection.append(card)
        return collection
        
    def sort_cards(self, cards):
        def card_key(card):
            # Handle Ace as the highest rank
            rank_priority = 14 if card.original_frame_x == 0 else card.original_frame_x + 1
            return (rank_priority, card.original_frame_y)
        
        return sorted(cards, key=card_key)

    def draw_hand(self):
        self.hand = self.deck.draw_cards(8)
        self.update_hand_positions()
        self.update_hand_indicator_position()

    def update_hand_positions(self):
        start_x = 7
        y_position = 103
        for i, card in enumerate(self.hand):
            tween_card(card, Vector2(start_x + i * 16, y_position))
            card.select(False)
            self.add_child(card)

    def create_hand_indicator(self):
        indicator = Sprite2DNode(Vector2(7, 80))
        indicator.texture = cards_texture
        indicator.frame_count_x = 13
        indicator.frame_count_y = 5
        indicator.frame_current_x = 5
        indicator.frame_current_y = 4
        indicator.playing = False
        indicator.transparent_color = Color(0x0400)
        self.add_child(indicator)
        return indicator

    def setup_board(self):
        self.color = Color(0x0400)

    def tick(self, dt):
        for tween in tweens[:]:
            if tween.finished:
                tweens.remove(tween)
                self.update_hand_indicator_position()

        if self.game_over:
            return  # Disable inputs when the game is over

        if self.hand_display_time and time.time() - self.hand_display_time >= 2:
            # Tween played cards off to the left
            for card in self.played_cards:
                target_position = Vector2(card.position.x - 100, card.position.y)
                tween_card(card, target_position)
            self.played_cards.clear()  # Clear played cards after tweening
            self.hand_display_time = None  # Reset the display time after tweening

        if self.is_booster_wait:
            if not self.hand_display_time and not tweens:  # Only proceed if all tweens are finished
                self.open_booster_packs()
        elif self.is_booster_selection:
            self.handle_booster_selection()
            self.update_booster_indicator_position()
        else:
            if engine_io.check_just_pressed(engine_io.DPAD_LEFT):
                self.move_left()
            elif engine_io.check_just_pressed(engine_io.DPAD_RIGHT):
                self.move_right()
            if engine_io.check_just_pressed(engine_io.DPAD_UP):
                self.select_card()
            elif engine_io.check_just_pressed(engine_io.DPAD_DOWN):
                self.deselect_card()
            if engine_io.check_just_pressed(engine_io.A):
                self.play_hand()
            if engine_io.check_just_pressed(engine_io.B):
                self.discard_and_draw()

    def move_left(self):
        self.current_card_index = (self.current_card_index - 1) % len(self.hand)
        self.update_hand_indicator_position()
        self.update_selected_card_rules()

    def move_right(self):
        self.current_card_index = (self.current_card_index + 1) % len(self.hand)
        self.update_hand_indicator_position()
        self.update_selected_card_rules()

    def update_hand_indicator_position(self):
        if self.hand and 0 <= self.current_card_index < len(self.hand):
            card_position = self.hand[self.current_card_index].position
            self.hand_indicator.position = Vector2(card_position.x, card_position.y + 20)
            self.hand_indicator.set_layer(7)

    def update_selected_card_rules(self):
        self.hand_type_text.text = self.hand[self.current_card_index].print_rules()

    def select_card(self):
        card = self.hand[self.current_card_index]
        if card not in self.selected_cards and len(self.selected_cards) < 5:
            card.select(True)
            self.selected_cards.append(card)

    def deselect_card(self):
        card = self.hand[self.current_card_index]
        if card in self.selected_cards:
            card.select(False)
            self.selected_cards.remove(card)

    def play_hand(self):
        if self.selected_cards:
            self.selected_cards = self.sort_cards(self.selected_cards)
            self.evaluate_hand()
            self.hands_played += 1
            self.display_played_hand()
            for card in self.selected_cards:
                card.mark_played()
                self.hand.remove(card)
                self.played_cards.append(card) 
            if self.hands_played >= 4 or self.score >= self.target_score:
                hand_copy = self.hand[:]  # Create a copy of the hand
                for card in hand_copy:
                    self.hand.remove(card)
                    card.mark_discarded()
                    tween_card(card, Vector2(-100, card.position.y))
                if self.score >= self.target_score:
                    self.selected_cards = []
                    self.is_booster_wait = True
                else:
                    self.end_game()
            else:
                new_cards = self.deck.draw_cards(len(self.selected_cards))
                self.hand.extend(new_cards)
                self.update_hand_positions()
                self.selected_cards = []
        self.update_game_info()

    def discard_and_draw(self):
        if not self.selected_cards:
            return
        if self.discard_limit > 0:
            for card in self.selected_cards:
                self.hand.remove(card)
                card.mark_destroy()
                card.mark_discarded()
            new_cards = self.deck.draw_cards(len(self.selected_cards))
            self.hand.extend(new_cards)
            self.update_hand_positions()
            self.selected_cards = []
            self.discard_limit -= 1

        self.update_game_info()

    def evaluate_hand(self):
        hand_ranks = [card.rank_value for card in self.selected_cards]
        hand_suits = [card.original_frame_y for card in self.selected_cards]

        rank_count = {}
        for rank in hand_ranks:
            if rank in rank_count:
                rank_count[rank] += 1
            else:
                rank_count[rank] = 1

        suit_count = {}
        for suit in hand_suits:
            if suit in suit_count:
                suit_count[suit] += 1
            else:
                suit_count[suit] = 1

        is_flush = self.check_flush()
        is_straight = self.is_straight(hand_ranks)
        base_score = 0
        multiplier = 1
        hand_name = None

        if is_flush and 5 in rank_count.values():
            base_score = 160  # Flush five
            multiplier = 16
            hand_name="Flush five"
        elif is_flush and 3 in rank_count.values() and 2 in rank_count.values():
            base_score = 140  # Flush house
            multiplier = 14
            hand_name="Flush house"
        elif 5 in rank_count.values():
            base_score = 120  # Five of a Kind
            multiplier = 12
            hand_name="5 of a Kind"
        elif is_flush and is_straight and (12 in hand_ranks or 14 in hand_ranks):
            base_score = 100  # Royal Flush
            multiplier = 10
            hand_name="Royal Flush"
        elif is_flush and is_straight:
            base_score = 100  # Straight Flush
            multiplier = 8
            hand_name="Straight Flush"
        elif 4 in rank_count.values():
            base_score = 60  # Four of a Kind
            multiplier = 7
            hand_name="4 of a Kind"
        elif 3 in rank_count.values() and 2 in rank_count.values():
            base_score = 40  # Full House
            multiplier = 4
            hand_name="Full House"
        elif is_flush:
            base_score = 35  # Flush
            multiplier = 4
            hand_name="Flush"
        elif is_straight:
            base_score = 30  # Straight
            multiplier = 4
            hand_name="Straight"
        elif 3 in rank_count.values():
            base_score = 30  # Three of a Kind
            multiplier = 3
            hand_name="3 of a Kind"
        elif list(rank_count.values()).count(2) == 2:
            base_score = 20  # Two Pair
            multiplier = 2
            hand_name="Two Pair"
        elif 2 in rank_count.values():
            base_score = 10  # One Pair
            multiplier = 2
            hand_name="One Pair"
        else:
            base_score = 5  # High Card
            multiplier = 1
            hand_name="High Card"

        base_score_text = f"+{base_score}"
        multiplier_text = f"x{multiplier}"
        

        self.display_score_animation(base_score_text, Vector2(54,0), self.base_score_text.position, Color(0x001F))
        self.display_score_animation(multiplier_text, Vector2(74,0), self.mult_score_text.position, Color(0xF800))

        # Add the rank of each card to the base score
        base_score += sum(hand_ranks)

        # Add card-specific bonuses and show score animations
        for card in self.selected_cards:
            if card.base_score_bonus > 0:
                base_score += card.base_score_bonus
                base_score_text = f"+{card.base_score_bonus + card.rank_value}"
                # Display base score animation in blue
                base_start_position = Vector2(card.position.x, card.position.y - 10)  # Shift up by 20 pixels
                self.display_score_animation(base_score_text, base_start_position, self.base_score_text.position, Color(0x001F))
            else:
                base_score_text = f"+{card.rank_value}"
                # Display base score animation in blue
                base_start_position = Vector2(card.position.x, card.position.y - 10)  # Shift up by 20 pixels
                self.display_score_animation(base_score_text, base_start_position, self.base_score_text.position, Color(0x001F))
            
            if card.multiplier_bonus > 0:
                multiplier += card.multiplier_bonus
                multiplier_text = f"x{card.multiplier_bonus}"
                # Display multiplier animation in red
                multiplier_start_position = Vector2(card.position.x, card.position.y + 10)  # Shift down by 20 pixels
                self.display_score_animation(multiplier_text, multiplier_start_position, self.mult_score_text.position, Color(0xF800))

        # Add joker bonuses and show score animations
        for joker in self.jokers:
            if joker.base_score_bonus > 0:
                base_score += joker.base_score_bonus
                base_score_text = f"+{joker.base_score_bonus}"
                # Display base score animation in blue
                base_start_position = Vector2(joker.position.x, joker.position.y - 10)  # Shift up by 20 pixels
                self.display_score_animation(base_score_text, base_start_position, self.base_score_text.position, Color(0x001F))
            
            if joker.multiplier_bonus > 0:
                multiplier += joker.multiplier_bonus
                multiplier_text = f"x{joker.multiplier_bonus}"
                # Display multiplier animation in red
                multiplier_start_position = Vector2(joker.position.x, joker.position.y + 10)  # Shift down by 20 pixels
                self.display_score_animation(multiplier_text, multiplier_start_position,self.mult_score_text.position, Color(0xF800))



        # Calculate the final score
        final_score = base_score * multiplier

        # Update highest individual hand score
        if final_score > self.highest_individual_hand_score:
            self.highest_individual_hand_score = final_score

        # Update text nodes
        self.base_score_text.text = f"{base_score}"
        self.mult_score_text.text = f"{multiplier}"
        self.hand_type_text.text = f"{hand_name}"
        self.score += final_score
        self.total_score_text.text = f"{self.score}/{self.target_score}"

    def is_straight(self, ranks):
        ranks = sorted(set(ranks))
        if len(ranks) != 5:
            return False
        if ranks[-1] - ranks[0] == 4:
            return True
        if set(ranks) == {2, 3, 4, 5, 14}:
            return True
        return False
    
    def check_flush(self):
        # Count the number of wildcard cards
        wildcard_count = sum(1 for card in self.selected_cards if card.is_wildcard_suit)
        
        # Get the suits of the non-wildcard cards
        non_wildcard_suits = [card.original_frame_y for card in self.selected_cards if not card.is_wildcard_suit]
        
        # If there are no non-wildcard cards, we need at least one wildcard card to form a flush
        if not non_wildcard_suits and wildcard_count > 0:
            return len(self.selected_cards) == 5
        
        # Check if all non-wildcard cards have the same suit
        is_non_wildcard_flush = len(set(non_wildcard_suits)) == 1
        
        # The flush is valid if the number of non-wildcard cards plus wildcards is 5
        is_flush = is_non_wildcard_flush and (len(non_wildcard_suits) + wildcard_count == 5)
        
        return is_flush
    
    def display_score_animation(self, score_text, start_position, end_position, color=Color(0x0400)):
        score_node = Text2DNode(start_position, font, score_text, 0, Vector2(1, 1), 1.0, 0, 0)
        score_node.color = color
        score_node.set_layer(6)
        self.add_child(score_node)
        tween_card(score_node, end_position, duration=1200, speed=1)
        tween_opacity(score_node, 1.0, 0.0, duration=1200)

    def display_played_hand(self):
        start_x = 10
        y_position = 65
        for i, card in enumerate(self.selected_cards):
            tween_card(card, Vector2(start_x + i * 18 + (1 if i != 0 else 0), y_position))
            card.mark_played()
            self.add_child(card)
        self.hand_display_time = time.time()

    def end_game(self):
        self.game_over = True
        for card in self.hand:
            card.hide()
        for card in self.played_cards:
            card.hide()
        self.hand_indicator.opacity = 0
        self.hand_type_text.text = f"Game Over!"
        self.best_hand_text.text = f"Best Hand: {self.highest_individual_hand_score}"


    def open_booster_packs(self):
        self.is_booster_selection = True
        self.is_booster_wait = False
        self.booster_cards = self.draw_booster_pack()
        self.show_booster_pack()

    def weighted_choice(self, choices, weights):
        total = sum(weights)
        r = random.uniform(0, total)
        upto = 0
        for choice, weight in zip(choices, weights):
            if upto + weight >= r:
                return choice
            upto += weight
        return choices[-1]

    def draw_booster_pack(self):
        new_cards = []
        size = 4
        if (self.hands_played < 4):
            size += 1
        if (self.discard_limit > 0):
            size += 1
        for _ in range(size):
            rand_val = random.random()
            
            # Generate Jokers
            if rand_val < 0.1:  # 10% chance for a Joker
                joker_type = self.weighted_choice(['common', 'uncommon', 'rare'], [0.6, 0.3, 0.1])
                if joker_type == 'common':
                    card = CardSprite(Vector2(150, 80), random.randint(0, 1), 4, base_score_bonus=random.randint(10, 100), multiplier_bonus=0)
                elif joker_type == 'uncommon':
                    card = CardSprite(Vector2(150, 80), random.randint(0, 1), 4, base_score_bonus=0, multiplier_bonus=random.randint(2, 5))
                else:  # rare
                    card = CardSprite(Vector2(150, 80), random.randint(0, 1), 4, base_score_bonus=random.randint(10, 50), multiplier_bonus=random.randint(2, 5))

            # Generate Wildcard Suit Cards
            elif rand_val < 0.3:  # 10% chance for a Wildcard Suit Card
                card = CardSprite(Vector2(150, 80), random.randint(0, 12), random.randint(0, 3), is_wildcard_suit=True)
            
            # Generate Cards
            elif rand_val < 0.8:  # 30% chance for a bonus card
                card_type = self.weighted_choice(['common', 'uncommon', 'rare'], [0.7, 0.2, 0.1])
                frame_x = random.randint(0, 12)
                frame_y = random.randint(0, 3)
                if card_type == 'common':
                    base_score_bonus = random.randint(5, 20)
                    multiplier_bonus = 0
                elif card_type == 'uncommon':
                    base_score_bonus = 0
                    multiplier_bonus = random.randint(2, 4)
                else:  # rare
                    base_score_bonus = random.randint(10, 30)
                    multiplier_bonus = random.randint(2, 6)

                card = CardSprite(Vector2(150, 80), frame_x, frame_y, base_score_bonus, multiplier_bonus)
            
            else: #standard card
                frame_x = random.randint(0, 12)
                frame_y = random.randint(0, 3)
                card = CardSprite(Vector2(150, 80), frame_x, frame_y)

            new_cards.append(card)

        return new_cards


    def update_joker_display(self):
        start_x = 7
        y_position = 23
        for i, joker in enumerate(self.jokers):
            tween_card(joker, Vector2(start_x + i * 19, y_position))
            self.add_child(joker)

    def show_booster_pack(self):
        start_x = 7
        y_position = 103
        for i, card in enumerate(self.booster_cards):
            card.position = Vector2(start_x + i * 16, y_position)
            self.add_child(card)
        self.current_card_index = 0
        self.hand_indicator.position = Vector2(start_x, y_position + 20)
        self.add_child(self.hand_indicator)
        self.update_booster_indicator_position()

        # Add booster pack label
        booster_label = Text2DNode(Vector2(64, 60), font, "Opening Boosters", 0, Vector2(1, 1), 1.0, 0, 0)
        self.remaining_discard_text.text = f"-"
        self.remaining_hands_text.text = f"-"
        self.add_child(booster_label)

    def handle_booster_selection(self):
        if engine_io.check_just_pressed(engine_io.DPAD_LEFT):
            self.move_left_booster()
        elif engine_io.check_just_pressed(engine_io.DPAD_RIGHT):
            self.move_right_booster()
        if engine_io.check_just_pressed(engine_io.DPAD_UP):
            self.select_booster_card()
        elif engine_io.check_just_pressed(engine_io.DPAD_DOWN):
            self.deselect_booster_card()
        if engine_io.check_just_pressed(engine_io.A):
            self.confirm_booster_selection()

    def move_left_booster(self):
        self.current_card_index = (self.current_card_index - 1) % len(self.booster_cards)
        self.update_booster_indicator_position()

    def move_right_booster(self):
        self.current_card_index = (self.current_card_index + 1) % len(self.booster_cards)
        self.update_booster_indicator_position()

    def update_booster_indicator_position(self):
        if self.booster_cards:
            card_position = self.booster_cards[self.current_card_index].position
            self.hand_indicator.position = Vector2(card_position.x, card_position.y + 20)
            self.hand_indicator.opacity = 1
            self.hand_type_text.text = self.booster_cards[self.current_card_index].print_rules()

    def select_booster_card(self):
        card = self.booster_cards[self.current_card_index]
        if card not in self.selected_cards and len(self.selected_cards) < 2:
            card.select(True)
            self.selected_cards.append(card)

    def deselect_booster_card(self):
        card = self.booster_cards[self.current_card_index]
        if card in self.selected_cards:
            card.select(False)
            self.selected_cards.remove(card)

    def confirm_booster_selection(self):
        for card in self.selected_cards:
            card.select(False)
            if card.original_frame_y == 4:
                if len(self.jokers) < 2:
                    self.jokers.append(card)
                else:
                    # Remove the oldest joker and add the new one
                    self.jokers.pop(0)
                    self.jokers.append(card)
            else:
                self.player_collection.append(card)
        self.selected_cards = []
        for card in self.booster_cards:
            card.position=Vector2(150,70)
        self.current_card_index = 0
        self.booster_cards = []
        self.is_booster_selection = False
        self.deck = Deck(self.player_collection)
        self.deck.shuffle()
        self.start_new_game()
        self.hand_type_text.text = f"Round {self.current_game}"
        self.update_hand_indicator_position()
        self.update_joker_display()

    def start_new_game(self):
        self.score = 0
        self.target_score += 500  # Increase target score for next game
        self.total_score_text.text = f"{self.score}/{self.target_score}"
        self.hands_played = 0
        self.discard_limit = 4  # Reset discard limit
        self.current_game += 1
        self.draw_hand()
        self.hand_indicator.opacity = 1
        self.update_game_info()

# Make an instance of our game
game = PokerGame(Vector2(0, 0), 256, 256)

# Make a camera to render the scene
camera = CameraNode()
camera.position = Vector3(64, 64, 1)

# Start rendering the scene
engine.start()
