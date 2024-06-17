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

# Load card sprite texture
cards_texture = TextureResource("BiggerCards2.bmp")
font = FontResource("../../assets/outrunner_outline.bmp")

tweens = []

def tween_card(card, target):
    tw = Tween()
    tw.start(card, 'position', card.position, target, 500, 2, ONE_SHOT, EASE_SINE_IN)
    tweens.append(tw)

class CardSprite(Sprite2DNode):
    def __init__(self, position, frame_x, frame_y, base_score_bonus=0, multiplier_bonus=1):
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
        self.base_score_bonus = base_score_bonus
        self.multiplier_bonus = multiplier_bonus

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
        self.in_hand = False
        self.discarded = True

class Deck:
    def __init__(self, collection):
        self.collection = collection
        self.cards = self.create_deck()
        self.shuffle()

    def create_deck(self):
        return [CardSprite(Vector2(150, 80), card.original_frame_x, card.original_frame_y, card.base_score_bonus, card.multiplier_bonus) for card in self.collection]

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
        self.played_hands = []
        self.current_card_index = 0
        self.draw_hand()
        self.hand_indicator = self.create_hand_indicator()
        self.discard_limit = 4
        self.hands_played = 0
        self.setup_board()
        self.score = 0
        self.target_score = 1000
        self.booster_cards = []
        self.is_booster_selection = False

        # Text nodes for displaying scores
        self.base_score_text = Text2DNode(Vector2(64, 10), font, "", 0, Vector2(1, 1), 1.0, 0, 0)
        self.multiplier_text = Text2DNode(Vector2(64, 20), font, "", 0, Vector2(1, 1), 1.0, 0, 0)
        self.round_score_text = Text2DNode(Vector2(64, 30), font, "", 0, Vector2(1, 1), 1.0, 0, 0)
        self.total_score_text = Text2DNode(Vector2(64, 40), font, "", 0, Vector2(1, 1), 1.0, 0, 0)

        self.add_child(self.base_score_text)
        self.add_child(self.multiplier_text)
        self.add_child(self.round_score_text)
        self.add_child(self.total_score_text)

        self.update_joker_display()

    def create_initial_collection(self):
        collection = []
        for suit in range(4):
            for rank in range(13):
                card = CardSprite(Vector2(150, 80), rank, suit)
                collection.append(card)
        return collection

    def draw_hand(self):
        self.hand = self.deck.draw_cards(8)
        self.update_hand_positions()

    def update_hand_positions(self):
        start_x = 7
        y_position = 98
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
            tween.tick(dt)
            if tween.finished:
                tweens.remove(tween)

        if self.is_booster_selection:
            self.handle_booster_selection()
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

        self.update_hand_indicator_position()

    def move_left(self):
        self.current_card_index = (self.current_card_index - 1) % len(self.hand)
        self.update_hand_indicator_position()

    def move_right(self):
        self.current_card_index = (self.current_card_index + 1) % len(self.hand)
        self.update_hand_indicator_position()

    def update_hand_indicator_position(self):
        if self.hand:
            card_position = self.hand[self.current_card_index].position
            self.hand_indicator.position = Vector2(card_position.x, card_position.y + 20)

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
            self.evaluate_hand()
            self.hands_played += 1
            self.display_played_hand()
            for card in self.selected_cards:
                card.mark_played()
                self.hand.remove(card)
            new_cards = self.deck.draw_cards(len(self.selected_cards))
            self.hand.extend(new_cards)
            self.update_hand_positions()
            self.selected_cards = []

        if self.hands_played >= 4:
            self.end_game()
            self.open_booster_packs()

    def discard_and_draw(self):
        if self.discard_limit > 0:
            for card in self.selected_cards:
                self.hand.remove(card)
                card.destroy()
                card.mark_discarded()
            new_cards = self.deck.draw_cards(len(self.selected_cards))
            self.hand.extend(new_cards)
            self.update_hand_positions()
            self.selected_cards = []
            self.discard_limit -= 1

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

        is_flush = len(suit_count) == 1
        is_straight = self.is_straight(hand_ranks)
        base_score = 0
        multiplier = 1

        if is_flush and is_straight and (12 in hand_ranks or 14 in hand_ranks):
            base_score = 100  # Royal Flush
            multiplier = 10
        elif is_flush and is_straight:
            base_score = 90  # Straight Flush
            multiplier = 9
        elif 4 in rank_count.values():
            base_score = 80  # Four of a Kind
            multiplier = 8
        elif 3 in rank_count.values() and 2 in rank_count.values():
            base_score = 70  # Full House
            multiplier = 7
        elif is_flush:
            base_score = 60  # Flush
            multiplier = 6
        elif is_straight:
            base_score = 50  # Straight
            multiplier = 5
        elif 3 in rank_count.values():
            base_score = 40  # Three of a Kind
            multiplier = 4
        elif list(rank_count.values()).count(2) == 2:
            base_score = 30  # Two Pair
            multiplier = 3
        elif 2 in rank_count.values():
            base_score = 20  # One Pair
            multiplier = 2
        else:
            base_score = 10  # High Card
            multiplier = 1

        # Add the rank of each card to the base score
        base_score += sum(hand_ranks)

        # Add card-specific bonuses
        base_score += sum(card.base_score_bonus for card in self.selected_cards)
        multiplier *= max(card.multiplier_bonus for card in self.selected_cards)

        # Add joker bonuses
        if self.jokers:
            joker_bonus = sum(joker.base_score_bonus for joker in self.jokers)
            base_score += joker_bonus
            multiplier *= max(joker.multiplier_bonus for joker in self.jokers)

        # Calculate the final score
        final_score = base_score * multiplier

        # Update text nodes
        self.base_score_text.text = f"{base_score} X {multiplier}"
        self.round_score_text.text = f"Score: {final_score}"
        self.score += final_score
        self.total_score_text.text = f"Total: {self.score}/{self.target_score}"

    def is_straight(self, ranks):
        ranks = sorted(set(ranks))
        if len(ranks) != 5:
            return False
        if ranks[-1] - ranks[0] == 4:
            return True
        if set(ranks) == {2, 3, 4, 5, 14}:
            return True
        return False

    def display_played_hand(self):
        start_x = 10
        y_position = 65
        for i, card in enumerate(self.selected_cards):
            tween_card(card, Vector2(start_x + i * 18 + (1 if i != 0 else 0), y_position))
            card.mark_played()
            self.add_child(card)

    def reset_hand(self):
        if self.hands_played < 4:
            self.hand = self.deck.draw_cards(8)
            self.update_hand_positions()
            self.selected_cards = []
        else:
            self.end_game()

    def end_game(self):
        for card in self.hand:
            card.visible = False
            card.opacity = 0
        self.hand_indicator.opacity = 0

    def open_booster_packs(self):
        print("Opening booster packs...")
        self.is_booster_selection = True
        self.booster_cards = self.draw_booster_pack()
        self.show_booster_pack()

    def draw_booster_pack(self):
        new_cards = []
        for _ in range(5):
            frame_x = random.randint(0, 12)
            frame_y = random.randint(0, 3)
            base_score_bonus = random.randint(1, 10)
            multiplier_bonus = random.randint(1, 3)
            card = CardSprite(Vector2(150, 80), frame_x, frame_y, base_score_bonus, multiplier_bonus)
            new_cards.append(card)

        if random.random() < 0.2:
            joker1 = CardSprite(Vector2(150, 80), 0, 4, base_score_bonus=5, multiplier_bonus=2)
            joker2 = CardSprite(Vector2(150, 80), 1, 4, base_score_bonus=5, multiplier_bonus=2)
            self.jokers.extend([joker1, joker2])
            self.jokers = self.jokers[:5]

        self.update_joker_display()
        return new_cards

    def update_joker_display(self):
        start_x = 200
        y_position = 50
        for i, joker in enumerate(self.jokers):
            tween_card(joker, Vector2(start_x, y_position + i * 18))
            self.add_child(joker)

    def show_booster_pack(self):
        start_x = 7
        y_position = 95
        for i, card in enumerate(self.booster_cards):
            card.position = Vector2(start_x + i * 16, y_position)
            self.add_child(card)
        self.current_card_index = 0
        self.hand_indicator.position = Vector2(start_x, y_position + 20)
        self.add_child(self.hand_indicator)

        # Add booster pack label
        booster_label = Text2DNode(Vector2(64, 60), font, "Opening Boosters", 0, Vector2(1, 1), 1.0, 0, 0)
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
        card_position = self.booster_cards[self.current_card_index].position
        self.hand_indicator.position = Vector2(card_position.x, card_position.y + 20)
        self.hand_indicator.opacity = 1

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
            self.player_collection.append(card)
        self.selected_cards = []
        for card in self.booster_cards:
            card.position=Vector2(150,70)
        self.booster_cards = []
        self.is_booster_selection = False
        self.deck = Deck(self.player_collection)
        self.deck.shuffle()
        self.start_new_game()

    def start_new_game(self):
        self.score = 0
        self.target_score += 500  # Increase target score for next game
        self.total_score_text.text = f"Total: {self.score}/{self.target_score}"
        self.hands_played = 0
        self.discard_limit = 4  # Reset discard limit
        self.draw_hand()
        self.hand_indicator.opacity = 1

# Make an instance of our game
game = PokerGame(Vector2(0, 0), 256, 256)

# Make a camera to render the scene
camera = CameraNode()
camera.position = Vector3(64, 64, 1)

# Start rendering the scene
engine.start()
