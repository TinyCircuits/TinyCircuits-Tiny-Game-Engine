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

# Constants
SUITS = ["Hearts", "Spades", "Diamonds", "Clubs"]
SHORT_SUITS = ["Hrt", "Spd", "Dia", "Clb"]
RANK_NAMES = ["Ace", "2", "3", "4", "5", "6", "7", "8", "9", "10", "Jack", "Queen", "King"]
SHORT_RANK_NAMES = ["A", "2", "3", "4", "5", "6", "7", "8", "9", "10", "J", "Q", "K"]
HAND_TYPES = [
    "Flush five", "Flush house", "5 of a Kind", "Royal Flush", "Straight Flush",
    "4 of a Kind", "Full House", "Flush", "Straight", "3 of a Kind", "Two Pair", 
    "One Pair", "High Card"
]

tweens = []

def tween_card(card, target, duration=500, speed=2):
    tw = Tween()
    tw.start(card, 'position', card.position, target, duration, speed, ONE_SHOT, EASE_SINE_IN)
    tweens.append(tw)

def tween_opacity(node, start_opacity, end_opacity, duration=1000):
    tw = Tween()
    tw.start(node, 'opacity', start_opacity, end_opacity, duration, 1, ONE_SHOT, EASE_SINE_IN)
    tweens.append(tw)

class Modifier:
    def apply_bonus(self, card, selected_cards, hand):
        return 0, 0
    
    def get_rule_prefix(self):
        return ""

class ScoreBonusModifier(Modifier):
    def __init__(self, base_score_bonus=0, multiplier_bonus=0):
        self.base_score_bonus = base_score_bonus
        self.multiplier_bonus = multiplier_bonus

    def check_condition(self, card):
        return False

    def apply_bonus(self, card, selected_cards, hand):
        total_base_bonus = 0
        total_mult_bonus = 0
        for selected_card in selected_cards:
            if self.check_condition(selected_card):
                total_base_bonus += self.base_score_bonus
                total_mult_bonus += self.multiplier_bonus
        return total_base_bonus, total_mult_bonus

    def get_rule_prefix(self):
        parts = []
        if self.base_score_bonus > 0:
            parts.append(f"+{self.base_score_bonus}")
        if self.multiplier_bonus > 0:
            parts.append(f"x{self.multiplier_bonus}")
        return " ".join(parts)
    
class BaseScoreBonusModifier(ScoreBonusModifier):
    def __init__(self, base_score_bonus):
        super().__init__(base_score_bonus, 0)

    def check_condition(self, card):
        return True

class MultiplierBonusModifier(ScoreBonusModifier):
    def __init__(self, bonus):
        super().__init__(0, bonus)

    def check_condition(self, card):
        return True

class RareBonusModifier(ScoreBonusModifier):
    def __init__(self, base_score_bonus, multiplier_bonus):
        super().__init__(base_score_bonus, multiplier_bonus)

    def check_condition(self, card):
        return True

class WildcardModifier(Modifier):
    def get_rule_prefix(self):
        return "Wild"

class SteelCardModifier(ScoreBonusModifier):
    def apply_bonus(self, card, selected_cards, hand):
        if card in hand:
            return self.base_score_bonus, self.multiplier_bonus
        return 0, 0

    def get_rule_prefix(self):
        return f"Steel +{self.base_score_bonus}"

class CoinCardModifier(Modifier):
    def get_rule_prefix(self):
        return "Coin"

class FaceCardModifier(ScoreBonusModifier):
    def check_condition(self, card):
        return 10 < card.rank_value < 14

    def get_rule_prefix(self):
        return "Faces " + super().get_rule_prefix()

class AceModifier(ScoreBonusModifier):
    def check_condition(self, card):
        return card.rank_value == 14

    def get_rule_prefix(self):
        return "Aces " + super().get_rule_prefix()

class FibonacciModifier(ScoreBonusModifier):
    def check_condition(self, card):
        return card.rank_value in {2, 3, 5, 8, 13}

    def get_rule_prefix(self):
        return "Fibonacci " + super().get_rule_prefix()

class NonFaceCardModifier(ScoreBonusModifier):
    def check_condition(self, card):
        return card.rank_value <= 10

    def get_rule_prefix(self):
        return "Numbers " + super().get_rule_prefix()

class EvenCardModifier(ScoreBonusModifier):
    def check_condition(self, card):
        return card.rank_value % 2 == 0

    def get_rule_prefix(self):
        return "Even " + super().get_rule_prefix()

class OddCardModifier(ScoreBonusModifier):
    def check_condition(self, card):
        return card.rank_value % 2 != 0

    def get_rule_prefix(self):
        return "Odd " + super().get_rule_prefix()

class CardSprite(Sprite2DNode):
    def __init__(self, position, frame_x, frame_y, modifiers=None):
        super().__init__(self, position)
        self.initialize_attributes(frame_x, frame_y, modifiers)
        self.background = self.create_background()
        self.bonus_overlay = self.create_bonus_overlay()
        self.add_child(self.background)
        if self.bonus_overlay:
            self.add_child(self.bonus_overlay)

    def initialize_attributes(self, frame_x, frame_y, modifiers):
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
        self.modifiers = modifiers if modifiers else []

    def create_background(self):
        background = Sprite2DNode(Vector2(0, 0))
        self.set_sprite_attributes(background, 6, 4, 2, 1)
        return background
    
    def create_bonus_overlay(self):
        for modifier in self.modifiers:
            if isinstance(modifier, RareBonusModifier):
                return self.create_overlay(9, 4, opacity=0.4)
            if isinstance(modifier, BaseScoreBonusModifier):
                return self.create_overlay(7, 4)
            if isinstance(modifier, MultiplierBonusModifier):
                return self.create_overlay(8, 4)
            if isinstance(modifier, WildcardModifier):
                return self.create_overlay(10, 4)
            if isinstance(modifier, SteelCardModifier):
                return self.create_overlay(12, 4)
            if isinstance(modifier, CoinCardModifier):
                return self.create_overlay(11, 4, layer=5)
        return None

    def create_overlay(self, frame_x, frame_y, layer=3, opacity=1.0):
        overlay = Sprite2DNode(Vector2(0, 0))
        self.set_sprite_attributes(overlay, frame_x, frame_y, layer, opacity)
        return overlay

    def set_sprite_attributes(self, sprite, frame_x, frame_y, layer, opacity):
        sprite.frame_count_x = 13
        sprite.frame_count_y = 5
        sprite.frame_current_x = frame_x
        sprite.frame_current_y = frame_y
        sprite.texture = cards_texture
        sprite.playing = False
        sprite.set_layer(layer)
        sprite.transparent_color = Color(0x0400)
        sprite.opacity = opacity

    @property
    def rank_value(self):
        return 14 if self.original_frame_x == 0 else self.original_frame_x + 1

    def select(self, is_selected):
        self.selected = is_selected
        self.position.y += -5 if is_selected else 5

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
        if self.bonus_overlay:
            self.bonus_overlay.opacity = 0

    def __str__(self):
        return (f"Card(rank={self.rank_value}, suit={self.original_frame_y}, position=({self.position.x}, {self.position.y}))")

    def __repr__(self):
        return self.__str__()

    def apply_modifiers(self, selected_cards, hand):
        total_base_score_bonus = 0
        total_multiplier_bonus = 0
        for modifier in self.modifiers:
            base_score_bonus, multiplier_bonus = modifier.apply_bonus(self, selected_cards, hand)
            total_base_score_bonus += base_score_bonus
            total_multiplier_bonus += multiplier_bonus
        return total_base_score_bonus, total_multiplier_bonus

    def print_rules(self):
        def get_rule(rank, suit):
            rule = f"{rank} of {suit}"
            for modifier in self.modifiers:
                prefix = modifier.get_rule_prefix()
                if prefix:
                    rule += f" {prefix}"
            return rule

        rank = RANK_NAMES[self.original_frame_x]
        short_rank = SHORT_RANK_NAMES[self.original_frame_x]
        suit = SUITS[self.original_frame_y]
        short_suit = SHORT_SUITS[self.original_frame_y]
        rule = get_rule(rank, suit)
        if len(rule) > 20:
            rule = get_rule(short_rank, short_suit)

        return rule
    
class JokerCard(CardSprite):
    def __init__(self, position, frame_x, frame_y, modifiers=None):
        super().__init__(position, frame_x, frame_y, modifiers)

    def create_bonus_overlay(self):
        has_base_score_bonus = any(isinstance(modifier, ScoreBonusModifier) and modifier.base_score_bonus > 0 for modifier in self.modifiers)
        has_multiplier_bonus = any(isinstance(modifier, ScoreBonusModifier) and modifier.multiplier_bonus > 0 for modifier in self.modifiers)

        if has_base_score_bonus and has_multiplier_bonus:
            return self.create_overlay(9, 4, opacity=0.4)  # Rare equivalent
        elif has_base_score_bonus:
            return self.create_overlay(7, 4)
        elif has_multiplier_bonus:
            return self.create_overlay(8, 4)
        return None

    def print_rules(self):
        rule = "Joker"
        for modifier in self.modifiers:
            prefix = modifier.get_rule_prefix()
            if prefix:
                rule += f" {prefix}"
        return rule
    
class HandTypeUpgradeCard(CardSprite):
    def __init__(self, position, frame_x, frame_y, hand_type, level_bonus):
        super().__init__(position, frame_x, frame_y)
        self.hand_type = hand_type
        self.level_bonus = level_bonus

    def apply_upgrade(self, game):
        if self.hand_type in game.hand_type_levels:
            game.hand_type_levels[self.hand_type] += self.level_bonus
        else:
            game.hand_type_levels[self.hand_type] = self.level_bonus

    def print_rules(self):
        return f"{self.hand_type} +{self.level_bonus}"
    
class TarotCard(CardSprite):
    def __init__(self, position, frame_x, frame_y, rank_or_suit, is_rank):
        super().__init__(position, frame_x, frame_y)
        self.rank_or_suit = rank_or_suit
        self.is_rank = is_rank

    def apply_upgrade(self, game):
        if self.is_rank:
            # Remove all cards of the specified rank from the collection
            game.player_collection = [card for card in game.player_collection if card.original_frame_x != self.rank_or_suit]
        else:
            # Remove all cards of the specified suit from the collection
            game.player_collection = [card for card in game.player_collection if card.original_frame_y != self.rank_or_suit]

    def print_rules(self):
        if self.is_rank:
            rank_name = RANK_NAMES[self.rank_or_suit]
            return f"Remove all {rank_name}s"
        else:
            suit_name = SUITS[self.rank_or_suit]
            return f"Remove all {suit_name}"

class Deck:
    def __init__(self, collection):
        self.collection = collection
        self.cards = self.create_deck()
        self.shuffle()

    def create_deck(self):
        return [CardSprite(Vector2(150, 80), card.original_frame_x, card.original_frame_y, card.modifiers) for card in self.collection]

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
        self.joker_selection = False
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
        self.hand_type_levels = {}
        self.is_booster_selection = False
        self.is_booster_wait = False
        self.extra_booster_count = 0
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
            if self.joker_selection:
                if engine_io.check_just_pressed(engine_io.BUMPER_LEFT) or engine_io.check_just_pressed(engine_io.BUMPER_RIGHT):
                    self.joker_selection = False
                    self.update_hand_indicator_position()
                elif engine_io.check_just_pressed(engine_io.DPAD_LEFT):
                    self.move_left_joker()
                elif engine_io.check_just_pressed(engine_io.DPAD_RIGHT):
                    self.move_right_joker()
            else:
                if (engine_io.check_just_pressed(engine_io.BUMPER_LEFT) or engine_io.check_just_pressed(engine_io.BUMPER_RIGHT)) and self.jokers:
                    self.joker_selection = True
                    self.current_card_index = 0 if engine_io.check_just_pressed(engine_io.BUMPER_LEFT) else len(self.jokers) - 1
                    self.update_joker_indicator_position()
                    self.update_selected_joker_rules()
                elif engine_io.check_just_pressed(engine_io.DPAD_LEFT):
                    self.move_left()
                elif engine_io.check_just_pressed(engine_io.DPAD_RIGHT):
                    self.move_right()
                elif engine_io.check_just_pressed(engine_io.DPAD_UP):
                    self.select_card()
                elif engine_io.check_just_pressed(engine_io.DPAD_DOWN):
                    self.deselect_card()
                elif engine_io.check_just_pressed(engine_io.A):
                    self.play_hand()
                elif engine_io.check_just_pressed(engine_io.B):
                    self.discard_and_draw()


    def move_left_joker(self):
        self.current_card_index = (self.current_card_index - 1) % len(self.jokers)
        self.update_joker_indicator_position()
        self.update_selected_joker_rules()

    def move_right_joker(self):
        self.current_card_index = (self.current_card_index + 1) % len(self.jokers)
        self.update_joker_indicator_position()
        self.update_selected_joker_rules()

    def update_joker_indicator_position(self):
        if self.jokers and 0 <= self.current_card_index < len(self.jokers):
            joker_position = self.jokers[self.current_card_index].position
            self.hand_indicator.position = Vector2(joker_position.x, joker_position.y + 20)
            self.hand_indicator.set_layer(7)

    def update_selected_joker_rules(self):
        self.hand_type_text.text = self.jokers[self.current_card_index].print_rules()

    def move_left(self):
        self.current_card_index = (self.current_card_index - 1) % len(self.hand)
        self.update_hand_indicator_position()
        self.update_selected_card_rules()

    def move_right(self):
        self.current_card_index = (self.current_card_index + 1) % len(self.hand)
        self.update_hand_indicator_position()
        self.update_selected_card_rules()

    def update_hand_indicator_position(self):
        if not self.joker_selection and self.hand and 0 <= self.current_card_index < len(self.hand):
            card_position = self.hand[self.current_card_index].position
            self.hand_indicator.position = Vector2(card_position.x, card_position.y + 20)
            self.hand_indicator.set_layer(7)

    def update_selected_card_rules(self):
        if not self.joker_selection:
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

        hand_conditions = [
            (HAND_TYPES[0], 160, 16, lambda: is_flush and 5 in rank_count.values()),  # "Flush five"
            (HAND_TYPES[1], 140, 14, lambda: is_flush and 3 in rank_count.values() and 2 in rank_count.values()),  # "Flush house"
            (HAND_TYPES[2], 120, 12, lambda: 5 in rank_count.values()),  # "5 of a Kind"
            (HAND_TYPES[3], 100, 10, lambda: is_flush and is_straight and (12 in hand_ranks or 14 in hand_ranks)),  # "Royal Flush"
            (HAND_TYPES[4], 100, 8, lambda: is_flush and is_straight),  # "Straight Flush"
            (HAND_TYPES[5], 60, 7, lambda: 4 in rank_count.values()),  # "4 of a Kind"
            (HAND_TYPES[6], 40, 4, lambda: 3 in rank_count.values() and 2 in rank_count.values()),  # "Full House"
            (HAND_TYPES[7], 35, 4, lambda: is_flush),  # "Flush"
            (HAND_TYPES[8], 30, 4, lambda: is_straight),  # "Straight"
            (HAND_TYPES[9], 30, 3, lambda: 3 in rank_count.values()),  # "3 of a Kind"
            (HAND_TYPES[10], 20, 2, lambda: list(rank_count.values()).count(2) == 2),  # "Two Pair"
            (HAND_TYPES[11], 10, 2, lambda: 2 in rank_count.values())  # "One Pair"
        ]

        base_score = 5  # Default score for High Card
        multiplier = 1  # Default multiplier for High Card
        hand_name = "High Card"  # Default hand name

        for name, score, mult, condition in hand_conditions:
            if condition():
                hand_name = name
                base_score = score
                multiplier = mult
                break

        # Apply hand type upgrades
        if hand_name in self.hand_type_levels:
            level = self.hand_type_levels[hand_name]
            base_score += level * 10
            multiplier += level
            hand_name += f" +{level}"

        base_score_text = f"+{base_score}"
        multiplier_text = f"x{multiplier}"
        

        self.display_score_animation(base_score_text, Vector2(54,0), self.base_score_text.position, Color(0x001F))
        self.display_score_animation(multiplier_text, Vector2(74,0), self.mult_score_text.position, Color(0xF800))

        for card in self.selected_cards:
            # Check for CoinCardModifier and increase extra booster count
            if any(isinstance(modifier, CoinCardModifier) for modifier in card.modifiers):
                self.extra_booster_count += 1


        # Add card-specific bonuses and show score animations
        for card in self.selected_cards + self.hand:
            base_bonus, mult_bonus = card.apply_modifiers(self.selected_cards, self.hand)

            if card in self.selected_cards:
                base_bonus += card.rank_value
            
            if base_bonus > 0:
                base_score += base_bonus
                base_score_text = f"+{base_bonus}"
                # Display base score animation in blue
                base_start_position = Vector2(card.position.x, card.position.y - 10)  # Shift up by 20 pixels
                self.display_score_animation(base_score_text, base_start_position, self.base_score_text.position, Color(0x001F))

            if mult_bonus > 0:
                multiplier += mult_bonus
                multiplier_text = f"x{mult_bonus}"
                # Display multiplier animation in red
                multiplier_start_position = Vector2(card.position.x, card.position.y + 10)  # Shift down by 20 pixels
                self.display_score_animation(multiplier_text, multiplier_start_position, self.mult_score_text.position, Color(0xF800))

        # Add joker bonuses and show score animations
        for joker in self.jokers:
            base_bonus, mult_bonus = joker.apply_modifiers(self.selected_cards, self.hand)
            if base_bonus > 0:
                base_score += base_bonus
                base_score_text = f"+{base_bonus}"
                # Display base score animation in blue
                base_start_position = Vector2(joker.position.x, joker.position.y - 10)  # Shift up by 20 pixels
                self.display_score_animation(base_score_text, base_start_position, self.base_score_text.position, Color(0x001F))

            if mult_bonus > 0:
                multiplier += mult_bonus
                multiplier_text = f"x{mult_bonus}"
                # Display multiplier animation in red
                multiplier_start_position = Vector2(joker.position.x, joker.position.y + 10)  # Shift down by 20 pixels
                self.display_score_animation(multiplier_text, multiplier_start_position, self.mult_score_text.position, Color(0xF800))

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
        wildcard_count = sum(1 for card in self.selected_cards if any(isinstance(modifier, WildcardModifier) for modifier in card.modifiers))
        
        # Get the suits of the non-wildcard cards
        non_wildcard_suits = [card.original_frame_y for card in self.selected_cards if not any(isinstance(modifier, WildcardModifier) for modifier in card.modifiers)]
        
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
        if self.hands_played < 4:
            size += 1
        if self.discard_limit > 0:
            size += 1
        size += self.extra_booster_count
        size = min(size, 7)
        for _ in range(size):
            rand_val = random.random()

            # Generate Jokers
            if rand_val < 0.1:  # 10% chance for a Joker
                joker_type = self.weighted_choice(['common', 'uncommon', 'rare'], [0.6, 0.3, 0.1])
                if joker_type == 'common':
                    joker_sub_type = self.weighted_choice([EvenCardModifier(random.randint(10, 20),0),
                                                           OddCardModifier(random.randint(10, 20),0),
                                                           FibonacciModifier(random.randint(10, 20),0),
                                                           AceModifier(random.randint(10, 20),0),
                                                           NonFaceCardModifier(random.randint(10, 20),0),
                                                           FaceCardModifier(random.randint(10, 20),0)],
                                                           [0.3, 0.3, 0.3, 0.2, 0.2, 0.2])
                    card = JokerCard(Vector2(150, 80), 0, 4, modifiers=[joker_sub_type])
                elif joker_type == 'uncommon':
                    joker_sub_type = self.weighted_choice([EvenCardModifier(0,random.randint(1, 3)),
                                                           OddCardModifier(0,random.randint(1, 3)),
                                                           FibonacciModifier(0,random.randint(1, 3)),
                                                           AceModifier(0,random.randint(1, 3)),
                                                           NonFaceCardModifier(0,random.randint(1, 3)),
                                                           FaceCardModifier(0,random.randint(1, 3)),
                                                           BaseScoreBonusModifier(random.randint(10, 50)),
                                                           MultiplierBonusModifier(random.randint(2, 5)),
                                                           RareBonusModifier(random.randint(10, 50), random.randint(2, 5))],
                                                           [0.15, 0.15, 0.15, 0.15, 0.15, 0.15, 0.15, 0.25, 0.15])
                    card = JokerCard(Vector2(150, 80), 1, 4, modifiers=[joker_sub_type])
                else:  # rare
                    card = JokerCard(Vector2(150, 80), 2, 4, modifiers=[RareBonusModifier(random.randint(20, 70), random.randint(4, 7))])


            # Generate Wildcard Suit Cards
            elif rand_val < 0.25:  # 15% chance for a Wildcard Suit Card
                card = CardSprite(Vector2(150, 80), random.randint(0, 12), random.randint(0, 3), modifiers=[WildcardModifier()])

            # Generate Tarot Cards
            elif rand_val < 0.3:  # 5% chance for a Tarot Card
                rank_or_suit = random.randint(0, 3)
                is_rank = self.weighted_choice(['rank', 'suite'], [0.7, 0.3]) == 'rank'
                if is_rank:
                    rank_or_suit = random.randint(0, 12) 
                card = TarotCard(Vector2(150, 80), 4, 4, rank_or_suit, is_rank)

            # Generate bonus cards (common/uncommon/rare)
            elif rand_val < 0.6:  # 30% chance for a bonus card
                card_type = self.weighted_choice(['common', 'uncommon', 'rare'], [0.7, 0.2, 0.1])
                frame_x = random.randint(0, 12)
                frame_y = random.randint(0, 3)
                if card_type == 'common':
                    modifiers = [BaseScoreBonusModifier(random.randint(5, 20))]
                elif card_type == 'uncommon':
                    modifiers = [MultiplierBonusModifier(random.randint(2, 4))]
                else:  # rare
                    modifiers = [RareBonusModifier(random.randint(10, 30), random.randint(2, 6))]

                card = CardSprite(Vector2(150, 80), frame_x, frame_y, modifiers=modifiers)
            # Generate hand upgrades
            elif rand_val < 0.7: # 10% chance for an upgrade card
                frame_x = 3
                frame_y = 4
                weights = [1, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 5, 4]  # Adjust weights as necessary based on rarity
                hand_type = self.weighted_choice(HAND_TYPES, weights)
                level_bonus = 1
                card = HandTypeUpgradeCard(Vector2(150, 80), frame_x, frame_y, hand_type, level_bonus)
            # Generate Steel and Coin Cards
            elif rand_val < 0.8:  # 10% chance for a Steel card
                frame_x = random.randint(0, 12)
                frame_y = random.randint(0, 3)
                card = CardSprite(Vector2(150, 80), frame_x, frame_y, modifiers=[SteelCardModifier(random.randint(10, 100))])
            elif rand_val < 0.9:  # 10% chance for a Coin card
                frame_x = random.randint(0, 12)
                frame_y = random.randint(0, 3)
                card = CardSprite(Vector2(150, 80), frame_x, frame_y, modifiers=[CoinCardModifier()])
            else:
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
            if isinstance(card, HandTypeUpgradeCard):
                card.apply_upgrade(self)
            elif isinstance(card, TarotCard):
                card.apply_upgrade(self)
            elif isinstance(card, JokerCard):
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
        self.extra_booster_count = 0
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
