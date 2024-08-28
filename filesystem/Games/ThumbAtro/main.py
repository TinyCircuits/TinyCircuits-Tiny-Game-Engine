import engine_main
import engine
import engine_draw
import engine_io
import engine_audio

from engine_nodes import Sprite2DNode, CameraNode, Rectangle2DNode, Text2DNode
from engine_resources import TextureResource, FontResource, WaveSoundResource
from engine_math import Vector2, Vector3
from engine_draw import Color
from engine_animation import Tween, Delay, ONE_SHOT, EASE_SINE_IN
import random
import time
import math


font = FontResource("munro-narrow_10.bmp")
cards_texture = TextureResource("BiggerCards2read.bmp")
background = TextureResource("thumbatrobackground.bmp")
money_sound = WaveSoundResource("money.wav")
select_sound = WaveSoundResource("select.wav")
play_sound = WaveSoundResource("play.wav")
shuffle_sound = WaveSoundResource("shuffle.wav")
#overlay = TextureResource("thumbatrooverlay.bmp")

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
    return tw

def tween_scale(node, start_scale, end_scale, duration=1200):
    tw = Tween()
    tw.start(node, 'scale', start_scale, end_scale, duration, 1, ONE_SHOT, EASE_SINE_IN)
    tweens.append(tw)

def tween_color(node, start, end, duration=1200):
    tw = Tween()
    tw.start(node, 'color', start, end, duration, 1, ONE_SHOT, EASE_SINE_IN)
    tweens.append(tw)

class AnimatedText2DNode(Text2DNode):
    def __init__(self, position, font, initial_value, target_value, duration=2000):
        super().__init__(self, position, font, str(initial_value))
        self.initial_value = initial_value
        self.target_value = target_value
        self.duration = duration
        self.start_time = time.ticks_ms()
        self.finished = False

    def tick(self, dt):
        if self.finished:
            return
        elapsed_time = time.ticks_diff(time.ticks_ms(), self.start_time)
        if elapsed_time >= self.duration:
            self.text = str(self.target_value)
            self.finished = True
        else:
            current_value = self.initial_value + (self.target_value - self.initial_value) * (elapsed_time / self.duration)
            self.text = str(int(current_value))  # assuming we want to display whole numbers

class Modifier:
    def apply_bonus(self, card, selected_cards, hand):
        return 0, 0
    
    def get_rule_prefix(self):
        return ""

class ScoreBonusModifier(Modifier):
    def __init__(self, base_score_bonus=0, multiplier_bonus=0, apply_only_once=False):
        self.base_score_bonus = base_score_bonus
        self.multiplier_bonus = multiplier_bonus
        self.apply_only_once = apply_only_once

    def check_condition(self, card):
        return False

    def apply_bonus(self, card, selected_cards, hand):
        #normal cards left in hand do not trigger
        if not isinstance(card, JokerCard) and card not in selected_cards:
            return 0, 0
        #Jokers might only trigger a single global boost and normal cards just trigger themselves
        if self.apply_only_once or not isinstance(card, JokerCard): 
            return self.base_score_bonus, self.multiplier_bonus
        #Jokers may check each card selected to apply bonuses
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
    def __init__(self, base_score_bonus, apply_only_once=False):
        super().__init__(base_score_bonus, 0, apply_only_once=apply_only_once)

    def check_condition(self, card):
        return True

class MultiplierBonusModifier(ScoreBonusModifier):
    def __init__(self, bonus, apply_only_once=False):
        super().__init__(0, bonus, apply_only_once=apply_only_once)

    def check_condition(self, card):
        return True

class RareBonusModifier(ScoreBonusModifier):
    def __init__(self, base_score_bonus, multiplier_bonus, apply_only_once=False):
        super().__init__(base_score_bonus, multiplier_bonus, apply_only_once=apply_only_once)

    def check_condition(self, card):
        return True

class WildcardModifier(Modifier):
    def get_rule_prefix(self):
        return "Wild"

class SteelCardModifier(ScoreBonusModifier):
    #override as triggers only if card remains in hand
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
    def __init__(self, position, frame_x, frame_y, rarity='common', modifiers=None):
        super().__init__(self, position)
        self.rarity = rarity
        self.initialize_attributes(frame_x, frame_y, modifiers)
        self.background = self.create_background()
        self.bonus_overlay = self.create_bonus_overlay()
        self.add_child(self.background)
        if self.bonus_overlay:
            self.add_child(self.bonus_overlay)

    def initialize_attributes(self, frame_x, frame_y, modifiers):
        self.texture = cards_texture
        self.frame_count_x = 13
        self.frame_count_y = 6
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
        self.layer = 4
        self.modifiers = modifiers if modifiers else []

    def create_background(self):
        background = Sprite2DNode(Vector2(0, 0))
        self.set_sprite_attributes(background, 6, 4, 2, 1)
        return background
    
    def create_bonus_overlay(self):
        for modifier in self.modifiers:
            if isinstance(modifier, RareBonusModifier):
                return self.create_overlay(9, random.randint(9,10), opacity=0.4)
            if isinstance(modifier, BaseScoreBonusModifier):
                return self.create_overlay(7, 4)
            if isinstance(modifier, MultiplierBonusModifier):
                return self.create_overlay(8, 4)
            if isinstance(modifier, WildcardModifier):
                return self.create_overlay(6, 5)
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
        sprite.frame_count_y = 6
        sprite.frame_current_x = frame_x
        sprite.frame_current_y = frame_y
        sprite.texture = cards_texture
        sprite.playing = False
        sprite.layer = layer
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

    #def __str__(self):
    #    return (f"Card(rank={self.rank_value}, suit={self.original_frame_y}, position=({self.position.x}, {self.position.y}))")

    #def __repr__(self):
    #    return self.__str__()

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
    def __init__(self, position, rarity, modifiers=None):
        super().__init__(position, 0, 4, rarity, modifiers)
        frame_x, frame_y = self.get_frame_for_modifiers(modifiers)  # Determine frame_x and frame_y based on modifiers
        self.original_frame_x = frame_x
        self.original_frame_y = frame_y
        self.frame_current_x = frame_x
        self.frame_current_y = frame_y

    def create_bonus_overlay(self):
        has_base_score_bonus = any(isinstance(modifier, ScoreBonusModifier) and modifier.base_score_bonus > 0 for modifier in self.modifiers)
        has_multiplier_bonus = any(isinstance(modifier, ScoreBonusModifier) and modifier.multiplier_bonus > 0 for modifier in self.modifiers)

        if self.rarity == 'uncommon':
            self.background.opacity = 0
            self.background = self.create_overlay(12, 4)

        if self.rarity == 'rare':
            return self.create_overlay(random.randint(9,10), random.randint(4,5), opacity=0.4) 
        elif has_base_score_bonus and not has_multiplier_bonus:
            return self.create_overlay(7, 5)
        elif has_multiplier_bonus and not has_base_score_bonus:
            return self.create_overlay(8, 5)
        return None
    
    def get_frame_for_modifiers(self, modifiers):
        modifier_frames = {
            EvenCardModifier: (2, 5),
            OddCardModifier: (1, 5),
            FibonacciModifier: (5, 5),
            AceModifier: (0, 5),
            NonFaceCardModifier: (3, 5),
            FaceCardModifier: (4, 5),
            BaseScoreBonusModifier: (0, 4),
            MultiplierBonusModifier: (1, 4),
            RareBonusModifier: (2, 4)
        }
    
        for modifier in modifiers:
            modifier_type = type(modifier)
            if modifier_type in modifier_frames:
                return modifier_frames[modifier_type]
        return 0, 4  # Default value if no matching modifier is found

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
        self.create_new_bonus_overlay()

    def apply_upgrade(self, game):
        if self.hand_type in game.hand_type_levels:
            game.hand_type_levels[self.hand_type] += self.level_bonus
        else:
            game.hand_type_levels[self.hand_type] = self.level_bonus

    def print_rules(self):
        return f"{self.hand_type} +{self.level_bonus}"

    def create_new_bonus_overlay(self):
        if self.level_bonus > 1:  # This indicates it is a rare card
            self.bonus_overlay = self.create_overlay(9, 4, opacity=0.3, layer=5)
            self.add_child(self.bonus_overlay)
    
class TarotCard(CardSprite):
    def __init__(self, position, frame_x, frame_y, level_bonus=2):
        super().__init__(position, frame_x, frame_y)
        self.level_bonus = level_bonus
        self.create_tarot_bonus_overlay()

    def create_tarot_bonus_overlay(self):
        if self.level_bonus > 2:  # This indicates it is a rare card
            self.bonus_overlay = self.create_overlay(9, 4, opacity=0.3, layer=5)
            self.add_child(self.bonus_overlay)

class RemoveRankTarotCard(TarotCard):
    def __init__(self, position, frame_x, frame_y, rank_to_remove, level_bonus=2):
        super().__init__(position, frame_x, frame_y, level_bonus)
        self.rank_to_remove = rank_to_remove

    def apply_upgrade(self, game):
        num_cards_to_remove = 4 if self.level_bonus > 2 else 2
        cards_to_remove = [card for card in game.player_collection if card.original_frame_x == self.rank_to_remove][:num_cards_to_remove]
        game.player_collection = [card for card in game.player_collection if card not in cards_to_remove]

    def print_rules(self):
        rank_name = RANK_NAMES[self.rank_to_remove]
        return f"Remove {4 if self.level_bonus > 2 else 2} {rank_name}s"

class RemoveSuitTarotCard(TarotCard):
    def __init__(self, position, frame_x, frame_y, suit_to_remove, level_bonus=2):
        super().__init__(position, frame_x, frame_y, level_bonus)
        self.suit_to_remove = suit_to_remove

    def apply_upgrade(self, game):
        num_cards_to_remove = 4 if self.level_bonus > 2 else 2
        suit_cards = [card for card in game.player_collection if card.original_frame_y == self.suit_to_remove]
        cards_to_remove = random_sample(suit_cards, num_cards_to_remove)
        game.player_collection = [card for card in game.player_collection if card not in cards_to_remove]

    def print_rules(self):
        suit_name = SUITS[self.suit_to_remove]
        return f"Remove {4 if self.level_bonus > 2 else 2} {suit_name}"

class UpgradeRankTarotCard(TarotCard):
    def __init__(self, position, frame_x, frame_y, rank_to_upgrade, level_bonus=2):
        super().__init__(position, frame_x, frame_y, level_bonus)
        self.rank_to_upgrade = rank_to_upgrade

    def apply_upgrade(self, game):
        eligible_cards = [card for card in game.player_collection if card.original_frame_x == self.rank_to_upgrade]
        cards_to_upgrade = random_sample(eligible_cards, self.level_bonus)
        for card in cards_to_upgrade:
            card.original_frame_x = (card.original_frame_x + 1) % 13

    def print_rules(self):
        rank_name = RANK_NAMES[self.rank_to_upgrade]
        return f"Up rank {self.level_bonus} {rank_name}s"
    
def random_sample(lst, n):
    result = []
    indices = list(range(len(lst)))
    for _ in range(min(n, len(lst))):
        index = random.choice(indices)
        result.append(lst[index])
        indices.remove(index)
    return result

class Deck:
    def __init__(self, collection):
        self.collection = collection
        self.cards = self.create_deck()
        self.shuffle()

    def create_deck(self):
        return [CardSprite(Vector2(150, 80), card.original_frame_x, card.original_frame_y, card.rarity, card.modifiers) for card in self.collection]

    def draw_cards(self, num):
        drawn_cards = []
        for _ in range(num):
            if self.cards:
                drawn_cards.append(self.cards.pop())
            else:
                return None
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
        self.game_over_time = None
        self.background = Sprite2DNode(Vector2(63, 63))
        self.background.frame_count_x = 1
        self.background.frame_count_y = 1
        self.background.texture = background
        self.background.playing = False
        self.background.layer = 0
        self.add_child(self.background)
        self.volume = .25
        engine_audio.set_volume(self.volume)
        self.sound_interval = 400
        self.last_sound_time = None
        self.playing_sounds = []
        self.current_channel = 0
        self.sound_play_duration = 0
        self.sound_start_time = None

        #self.overlay = Sprite2DNode(Vector2(63, 63))
        #self.overlay.frame_count_x = 1
        #self.overlay.frame_count_y = 1
        #self.overlay.texture = overlay
        #self.overlay.playing = False
        #self.overlay.layer = 7
        #self.overlay.opacity = 0.05
        #self.overlay.transparent_color = engine_draw.white
        #self.add_child(self.overlay)

        # Text nodes for displaying scores
        self.base_score_text = Text2DNode(Vector2(51, 24), font, "", 0, Vector2(1, 1), 1.0, 0, 0)
        self.base_score_text.layer = 1
        self.base_score_text.color = Color(0x04FF)
        self.mult_score_text = Text2DNode(Vector2(85, 24), font, "", 0, Vector2(1, 1), 1.0, 0, 0)
        self.mult_score_text.layer = 1
        self.mult_score_text.color = Color(0xFAEA)
        self.round_text = Text2DNode(Vector2(110, 75), font, "", 0, Vector2(1, 1), 1.0, 0, 0)
        self.round_text.layer = 1
        self.hand_type_text = Text2DNode(Vector2(82, 37), font, "", 0, Vector2(1, 1), 1.0, 0, 0)
        self.hand_type_text.layer = 1
        self.total_score_text = Text2DNode(Vector2(69, 10), font, "", 0, Vector2(1, 1), 1.0, 0, 0)
        self.total_score_text.layer = 1
        self.best_hand_text = Text2DNode(Vector2(44, 57), font, "", 0, Vector2(1, 1), 1.0, 0, 0)
        self.best_hand_text.layer = 1
        self.remaining_hands_text = Text2DNode(Vector2(104, 59), font, "", 0, Vector2(1, 1), 1.0, 0, 0)
        self.remaining_hands_text.layer = 1
        self.remaining_discard_text = Text2DNode(Vector2(118, 59), font, "", 0, Vector2(1, 1), 1.0, 0, 0)
        self.remaining_discard_text.layer = 1

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
        if self.hand is None:
            self.end_game()
            return
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
        indicator.frame_count_y = 6
        indicator.frame_current_x = 5
        indicator.frame_current_y = 4
        indicator.playing = False
        indicator.transparent_color = Color(0x0400)
        self.add_child(indicator)
        return indicator

    def setup_board(self):
        self.color = Color(0x0400)

    def toggle_volume(self):
        if self.volume == .25:
            self.volume = 0
        else:
            self.volume = .25
        engine_audio.set_volume(self.volume)
        self.hand_type_text.text = f"Volume: {engine_audio.get_volume()}"

    def play_sound(self):
        # Play the sound and keep track of the playing sound
        if self.volume == 0:
            return
        audio_channel = engine_audio.play(money_sound, self.current_channel, False)
        self.playing_sounds.append(audio_channel)
        engine_io.rumble(0.5)
        self.last_sound_time = time.ticks_ms()
        self.current_channel = (self.current_channel + 1) % 4  # Iterate over the 4 channels


    def stop_all_sounds(self):
        self.playing_sounds = []
        self.sound_interval = 400
        self.last_sound_time = None
        self.sound_play_duration = 0
        self.sound_start_time = None

    def tick(self, dt):
        if engine_io.MENU.is_just_pressed:
            self.toggle_volume()

        for tween in tweens[:]:
            if tween.finished:
                tweens.remove(tween)
                self.update_hand_indicator_position()

        if self.playing_sounds:
            # Play sounds with decreasing intervals
            if self.last_sound_time:
                if time.ticks_diff(time.ticks_ms(), self.last_sound_time) >= self.sound_interval:
                    self.play_sound()
                    self.sound_interval = max(20, self.sound_interval - 20)  # Decrease interval, but not below 20ms
                    self.last_sound_time = time.ticks_ms()

            # Stop sounds after the play duration has elapsed
            if self.sound_start_time and time.ticks_diff(time.ticks_ms(), self.sound_start_time) >= self.sound_play_duration:
                self.stop_all_sounds()

        if self.game_over:
            if self.game_over_time is None:
                self.game_over_time = time.ticks_ms()  
            elif time.ticks_ms() - self.game_over_time >= 5000:
                engine.end()
            return

        if self.hand_display_time and time.ticks_diff(time.ticks_ms(), self.hand_display_time) >= 2000:
            # Tween played cards off to the left
            for card in self.played_cards:
                target_position = Vector2(card.position.x - 100, card.position.y)
                tween_card(card, target_position)
            self.played_cards.clear()  # Clear played cards after tweening
            self.hand_display_time = None  # Reset the display time after tweening

        if self.is_booster_wait:
            if not self.hand_display_time and not tweens:  # Only proceed if all tweens are finished
                self.open_booster_packs()
        elif self.joker_selection:
            if engine_io.LB.is_just_pressed or engine_io.RB.is_just_pressed:
                self.joker_selection = False
                self.update_hand_indicator_position()
            elif engine_io.LEFT.is_just_pressed:
                self.move_left_joker()
            elif engine_io.RIGHT.is_just_pressed:
                self.move_right_joker()
            elif engine_io.B.is_just_pressed and self.is_booster_selection:
                self.discard_joker()
        elif self.is_booster_selection:
            if (engine_io.LB.is_just_pressed or engine_io.RB.is_just_pressed) and self.jokers:
                self.joker_selection = True
                self.current_card_index = 0 if engine_io.LB.is_just_pressed else len(self.jokers) - 1
                self.update_joker_indicator_position()
                self.update_selected_joker_rules()
            else:
                self.handle_booster_selection()
                self.update_booster_indicator_position()
        else:
            if (engine_io.LB.is_just_pressed or engine_io.RB.is_just_pressed) and self.jokers:
                self.joker_selection = True
                self.current_card_index = 0 if engine_io.LB.is_just_pressed else len(self.jokers) - 1
                self.update_joker_indicator_position()
                self.update_selected_joker_rules()
            elif engine_io.LEFT.is_just_pressed:
                self.move_left()
            elif engine_io.RIGHT.is_just_pressed:
                self.move_right()
            elif engine_io.UP.is_just_pressed:
                self.select_card()
            elif engine_io.DOWN.is_just_pressed:
                self.deselect_card()
            elif engine_io.A.is_just_pressed:
                self.play_hand()
            elif engine_io.B.is_just_pressed:
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
            self.hand_indicator.layer = 6

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
            self.hand_indicator.layer = 6

    def update_selected_card_rules(self):
        if not self.joker_selection:
            self.hand_type_text.text = self.hand[self.current_card_index].print_rules()

    def select_card(self):
        card = self.hand[self.current_card_index]
        if card not in self.selected_cards and len(self.selected_cards) < 5:
            card.select(True)
            self.selected_cards.append(card)
            self.update_hand_score_display()
            engine_audio.play(select_sound, 0, False)

    def deselect_card(self):
        card = self.hand[self.current_card_index]
        if card in self.selected_cards:
            card.select(False)
            self.selected_cards.remove(card)
            self.update_hand_score_display()
            engine_audio.play(select_sound, 0, False)

    def update_hand_score_display(self):
        if not self.selected_cards:
            self.hand_type_text.text = "Play/Discard 5 cards"
            return

        hand_name, base_score, multiplier = self.calculate_hand_score(self.selected_cards)

        self.hand_type_text.text = f"{hand_name}"
        self.base_score_text.text = f"{base_score}"
        self.mult_score_text.text = f"{multiplier}"

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
                if new_cards is None:
                    self.end_game()
                    return
                self.hand.extend(new_cards)
                self.update_hand_positions()
                self.selected_cards = []
            engine_audio.play(play_sound, 0, False)
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
            if new_cards is None:
                self.end_game()
                return
            self.hand.extend(new_cards)
            self.update_hand_positions()
            self.selected_cards = []
            self.discard_limit -= 1
            engine_audio.play(play_sound, 0, False)

        self.update_game_info()

    def evaluate_hand(self):
        if not self.selected_cards:
            return
        
        hand_name, base_score, multiplier = self.calculate_hand_score(self.selected_cards)

        base_score_text = f"+{base_score}"
        multiplier_text = f"x{multiplier}"

        self.display_score_animation(base_score_text, Vector2(54, 0), self.base_score_text.position, Color(0x04FF))
        self.display_score_animation(multiplier_text, Vector2(74, 0), self.mult_score_text.position, Color(0xFAEA))

        for card in self.selected_cards:
            if any(isinstance(modifier, CoinCardModifier) for modifier in card.modifiers):
                self.extra_booster_count += 1

        def apply_card_bonuses(card_list):
            nonlocal base_score, multiplier
            for card in card_list:
                base_bonus, mult_bonus = card.apply_modifiers(self.selected_cards, self.hand)

                if card in self.selected_cards:
                    base_bonus += card.rank_value

                if base_bonus > 0:
                    base_score += base_bonus
                    base_score_text = f"+{base_bonus}"
                    base_start_position = Vector2(card.position.x, card.position.y - 10)
                    self.display_score_animation(base_score_text, base_start_position, self.base_score_text.position, Color(0x04FF))

                if mult_bonus > 0:
                    multiplier += mult_bonus
                    multiplier_text = f"x{mult_bonus}"
                    multiplier_start_position = Vector2(card.position.x, card.position.y + 10)
                    self.display_score_animation(multiplier_text, multiplier_start_position, self.mult_score_text.position, Color(0xFAEA))

        apply_card_bonuses(self.selected_cards)
        apply_card_bonuses([card for card in self.hand if card not in self.selected_cards])

        for joker in self.jokers:
            base_bonus, mult_bonus = joker.apply_modifiers(self.selected_cards, self.hand)

            if base_bonus > 0:
                base_score += base_bonus
                base_score_text = f"+{base_bonus}"
                base_start_position = Vector2(joker.position.x, joker.position.y - 10)
                self.display_score_animation(base_score_text, base_start_position, self.base_score_text.position, Color(0x04FF))

            if mult_bonus > 0:
                multiplier += mult_bonus
                multiplier_text = f"x{mult_bonus}"
                multiplier_start_position = Vector2(joker.position.x, joker.position.y + 10)
                self.display_score_animation(multiplier_text, multiplier_start_position, self.mult_score_text.position, Color(0xFAEA))

        final_score = base_score * multiplier

        if final_score > self.highest_individual_hand_score:
            self.highest_individual_hand_score = final_score

        self.display_main_score_animation(final_score, Vector2(64,82), math.log10(final_score), engine_draw.white )

        self.base_score_text.text = f"{base_score}"
        self.mult_score_text.text = f"{multiplier}"
        self.hand_type_text.text = f"{hand_name}"
        self.score += final_score
        self.total_score_text.text = f"{self.score}/{self.target_score}"



    def calculate_hand_score(self, selected_cards):
        hand_ranks = [card.rank_value for card in selected_cards]
        rank_count = {}
        for rank in hand_ranks:
            if rank in rank_count:
                rank_count[rank] += 1
            else:
                rank_count[rank] = 1

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

        return hand_name, base_score, multiplier


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
        score_node.layer = 6
        self.add_child(score_node)
        tween_card(score_node, end_position, duration=1200, speed=1)
        tween_opacity(score_node, 1.0, 0.0, duration=1200)

    def display_main_score_animation(self, score, start_position, end_scale, color):
        score_node = AnimatedText2DNode(start_position, font, 0, score, 1500)
        score_node.color = color
        score_node.layer = 6
        self.add_child(score_node)
        duration= 1000 + end_scale * 500
        tween_scale(score_node, Vector2(1, 1), Vector2(end_scale, end_scale), duration)
        tween_opacity(score_node, 1.0, 0.0, duration)
        self.sound_play_duration = 500 + end_scale * 500
        self.sound_start_time = time.ticks_ms()
        self.play_sound()

    def display_played_hand(self):
        start_x = 10
        y_position = 65
        for i, card in enumerate(self.selected_cards):
            tween_card(card, Vector2(start_x + i * 18 + (1 if i != 0 else 0), y_position))
            card.mark_played()
            self.add_child(card)
        self.hand_display_time = time.ticks_ms()

    def end_game(self):
        self.game_over = True
        for card in self.hand:
            card.hide()
        for card in self.played_cards:
            card.hide()
        self.hand_indicator.opacity = 0
        self.hand_type_text.text = f"Game Over!"
        self.best_hand_text.text = f"Best Hand: {self.highest_individual_hand_score}"
        self.game_over_time = time.ticks_ms()


    def open_booster_packs(self):
        self.is_booster_selection = True
        self.is_booster_wait = False
        self.booster_cards = self.draw_booster_pack()
        self.hand_type_text.text = "Select 1 or 2"
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
        def generate_joker_card(joker_type):
            if joker_type == 'common':
                modifiers = [
                    self.weighted_choice(
                        [
                            EvenCardModifier(random.randint(10, 20), 0),
                            OddCardModifier(random.randint(10, 20), 0),
                            FibonacciModifier(random.randint(10, 20), 0),
                            AceModifier(random.randint(10, 20), 0),
                            NonFaceCardModifier(random.randint(10, 20), 0),
                            FaceCardModifier(random.randint(10, 20), 0)
                        ], [0.3, 0.3, 0.3, 0.2, 0.2, 0.2]
                    )
                ]
                return JokerCard(Vector2(150, 80), joker_type, modifiers=modifiers)
            elif joker_type == 'uncommon':
                modifiers = [
                    self.weighted_choice(
                        [
                            EvenCardModifier(0, random.randint(1, 3)),
                            OddCardModifier(0, random.randint(1, 3)),
                            FibonacciModifier(0, random.randint(1, 3)),
                            AceModifier(0, random.randint(1, 3)),
                            NonFaceCardModifier(0, random.randint(1, 3)),
                            FaceCardModifier(0, random.randint(1, 3)),
                            BaseScoreBonusModifier(random.randint(10, 20), apply_only_once=True),
                            MultiplierBonusModifier(random.randint(1, 4), apply_only_once=True),
                            RareBonusModifier(random.randint(10, 20), random.randint(1, 3), apply_only_once=True)
                        ], [0.15, 0.15, 0.15, 0.15, 0.15, 0.15, 0.15, 0.25, 0.15]
                    )
                ]
                return JokerCard(Vector2(150, 80), joker_type, modifiers=modifiers)
            else:  # rare
                modifiers = [
                    self.weighted_choice(
                        [
                            EvenCardModifier(random.randint(5, 10), random.randint(1, 3)),
                            OddCardModifier(random.randint(5, 10), random.randint(1, 3)),
                            FibonacciModifier(random.randint(5, 10), random.randint(1, 3)),
                            AceModifier(random.randint(5, 10), random.randint(1, 3)),
                            NonFaceCardModifier(random.randint(5, 10), random.randint(1, 3)),
                            FaceCardModifier(random.randint(5, 10), random.randint(1, 3)),
                            RareBonusModifier(random.randint(20, 50), random.randint(3, 5), apply_only_once=True)
                        ], [0.15, 0.15, 0.15, 0.15, 0.15, 0.15, 0.15, 0.15]
                    )
                ]
                return JokerCard(Vector2(150, 80), joker_type, modifiers=modifiers)

        def generate_card(card_type):
            if card_type == 'joker':
                joker_type = self.weighted_choice(['common', 'uncommon', 'rare'], [0.6, 0.3, 0.1])
                return generate_joker_card(joker_type)
            elif card_type == 'wildcard':
                return CardSprite(Vector2(150, 80), random.randint(0, 12), random.randint(0, 3), modifiers=[WildcardModifier()])
            elif card_type == 'tarot':
                tarot_type = self.weighted_choice(['rank', 'suite', 'upgrade'], [0.5, 0.3, 0.2])
                level_bonus = 4 if self.weighted_choice(['common', 'rare'], [0.7, 0.3]) == 'rare' else 2
                if tarot_type == 'rank':
                    rank_to_remove = random.randint(0, 12)
                    return RemoveRankTarotCard(Vector2(150, 80), 4, 4, rank_to_remove, level_bonus)
                elif tarot_type == 'suite':
                    suit_to_remove = random.randint(0, 3)
                    return RemoveSuitTarotCard(Vector2(150, 80), 11, 5, suit_to_remove, level_bonus)
                else:
                    rank_to_upgrade = random.randint(1, 12)  # Exclude Ace (index 0)
                    return UpgradeRankTarotCard(Vector2(150, 80), 12, 5, rank_to_upgrade, level_bonus)
            elif card_type == 'bonus':
                card_type = self.weighted_choice(['common', 'uncommon', 'rare'], [0.7, 0.2, 0.1])
                frame_x, frame_y = random.randint(0, 12), random.randint(0, 3)
                if card_type == 'common':
                    modifiers = [BaseScoreBonusModifier(random.randint(5, 20))]
                elif card_type == 'uncommon':
                    modifiers = [MultiplierBonusModifier(random.randint(2, 4))]
                else:  # rare
                    modifiers = [RareBonusModifier(random.randint(10, 20), random.randint(2, 5))]
                return CardSprite(Vector2(150, 80), frame_x, frame_y, modifiers=modifiers)
            elif card_type == 'upgrade':
                frame_x, frame_y = 3, 4
                weights = [1, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 5, 4]
                hand_type = self.weighted_choice(HAND_TYPES, weights)
                level_bonus = 2 if self.weighted_choice(['common', 'rare'], [0.7, 0.3]) == 'rare' else 1
                return HandTypeUpgradeCard(Vector2(150, 80), frame_x, frame_y, hand_type, level_bonus)
            elif card_type == 'steel':
                frame_x, frame_y = random.randint(0, 12), random.randint(0, 3)
                return CardSprite(Vector2(150, 80), frame_x, frame_y, modifiers=[SteelCardModifier(random.randint(10, 100))])
            elif card_type == 'coin':
                frame_x, frame_y = random.randint(0, 12), random.randint(0, 3)
                return CardSprite(Vector2(150, 80), frame_x, frame_y, modifiers=[CoinCardModifier()])
            else:
                frame_x, frame_y = random.randint(0, 12), random.randint(0, 3)
                return CardSprite(Vector2(150, 80), frame_x, frame_y)

        size = 4 + (1 if self.hands_played < 4 else 0) + (1 if self.discard_limit > 0 else 0) + self.extra_booster_count
        size = min(size, 7)

        card_types = ['joker', 'wildcard', 'tarot', 'bonus', 'upgrade', 'steel', 'coin', 'normal']
        weights = [0.15, 0.1, 0.1, 0.25, 0.1, 0.1, 0.1, 0.1]  # Weights summing to 1

        new_cards = [generate_card(self.weighted_choice(card_types, weights)) for _ in range(size)]

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
            card.position = Vector2(start_x + i * 18 + (1 if i != 0 else 0), y_position)
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
        if engine_io.LEFT.is_just_pressed:
            self.move_left_booster()
        elif engine_io.RIGHT.is_just_pressed:
            self.move_right_booster()
        if engine_io.UP.is_just_pressed:
            self.select_booster_card()
        elif engine_io.DOWN.is_just_pressed:
            self.deselect_booster_card()
        if engine_io.A.is_just_pressed:
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
        current_selected_jokers = len([c for c in self.selected_cards if isinstance(c, JokerCard)])
        if isinstance(card, JokerCard) and (len(self.jokers) + current_selected_jokers) >= 2:
            return  # Prevent selecting more jokers if the joker set is full
        if card not in self.selected_cards and len(self.selected_cards) < 2:
            card.select(True)
            self.selected_cards.append(card)
            engine_audio.play(select_sound, 0, False)

    def deselect_booster_card(self):
        card = self.booster_cards[self.current_card_index]
        if card in self.selected_cards:
            card.select(False)
            self.selected_cards.remove(card)
            engine_audio.play(select_sound, 0, False)

    def confirm_booster_selection(self):
        if not self.selected_cards:
            return
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
                #tween_card(card, Vector2(150, 80))
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

    def discard_joker(self):
        if self.jokers:
            joker_to_discard = self.jokers.pop(self.current_card_index)
            joker_to_discard.mark_discarded()
            self.update_joker_display()
            if self.jokers:
                self.current_card_index = 0
                self.update_joker_indicator_position()
            else:
                self.joker_selection = False
                self.update_hand_indicator_position()

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
        engine_audio.play(shuffle_sound, 0, False)


# Make an instance of our game
game = PokerGame(Vector2(0, 0), 256, 256)

class IntroSprite(Sprite2DNode):
    def __init__(self, position):
        super().__init__(self, position)
        self.triggered = False
        self.start_time = None

    def tick(self, dt):
        if not self.triggered:
            if self.start_time is None:
                self.start_time = time.ticks_ms()

            if time.ticks_diff(time.ticks_ms(), self.start_time) >= 1000:
                tw = tween_opacity(self, 1.0, 0.0, 1000)
                self.triggered = True

# Load card sprite texture
intro = TextureResource("title.bmp")
introScr = IntroSprite(Vector2(63, 63))
introScr.frame_count_x = 1
introScr.frame_count_y = 1
introScr.texture = intro
introScr.playing = False
introScr.layer = 7

# Make a camera to render the scene
camera = CameraNode()
camera.position = Vector3(64, 64, 1)




# Start rendering the scene
engine.start()
