import engine_io as io
import engine_draw as draw
from engine_nodes import (
	Sprite2DNode,
	Rectangle2DNode,
)
from engine_math import Vector2
from math import copysign

from constants import (
	MIN_X,
	MAX_X,
	MAX_Y,
	SHOW_HITBOX,
)

MOVE_SPEED = 200 # pixels / sec.
HITBOX_PADDING = 10
ATTACK_DURATION = 0.2 # attack frame duration
STUN_BLINK_RATE = 0.6 # Hz.

POS_X_LEFT = MIN_X + 25
POS_X_CENTER = 0
POS_X_RIGHT = MAX_X - 25
POS_Y = MAX_Y - 25
POS_Y_WTF = 0 # distance between top of player and bottom of WTF

# Cooldowns after specific action is taken
COOLDOWN_ATTACK = 1
COOLDOWN_MOVE = 0.2
COOLDOWN_BUTTON = 0.05 # button de-bounce time (release duration)

# Sprite frames
FRAME_IDLE = 0
FRAME_ATTACK = 1
FRAME_HIT = 2
FRAMES_ROLL_LEFT = [6, 5, 4, 3]
FRAMES_ROLL_RIGHT = [3, 4, 5, 6]


class Player(Sprite2DNode):
	def __init__(self, texture, wtfTexture):
		super().__init__(self)
		self.texture = texture
		self.transparent_color = draw.white
		self.frame_count_x = 7
		self.frame_current_x = FRAME_IDLE
		self.playing = False
		self.position.y = POS_Y
		self.rollFrames = None
		self.rollMove = 0

		# Exclamation sprite
		self.wtf = Sprite2DNode(
			texture=wtfTexture,
			transparent_color=draw.white,
			position=Vector2(
				0,
				-(texture.height / 2 + wtfTexture.height / 2 + POS_Y_WTF),
			),
		)
		self.add_child(self.wtf)
		# Hitbox for collision debugging
		self.hitbox = Rectangle2DNode(
			color=draw.blue,
			width=self.texture.width / self.frame_count_x - HITBOX_PADDING * 2,
			height=self.texture.height - HITBOX_PADDING * 2,
		)
		self.add_child(self.hitbox)
		self.hide()

	# Define a callback that the engine will invoke every game tick
	def tick(self, dt):
		if not self.shown():
			return

		# Move player to destination position
		maxMove = self.destPositionX - self.position.x
		self.position.x += copysign(
			min(abs(maxMove), MOVE_SPEED * dt),
			maxMove,
		)
		moving = abs(maxMove) > 0

		# Update cooldowns
		if self.cooldownBumperLeft > 0:
			self.cooldownBumperLeft -= dt
		if self.cooldownBumperRight > 0:
			self.cooldownBumperRight -= dt
		if self.cooldownA > 0:
			self.cooldownA -= dt
		if self.cooldownB > 0:
			self.cooldownB -= dt
		if self.cooldownMove > 0:
			self.cooldownMove -= dt
		if self.cooldownAttack > 0:
			self.cooldownAttack -= dt
		if self.cooldownBlock > 0:
			self.cooldownBlock -= dt

		# Update roll animation
		if moving:
			frameIndex = int(
				len(self.rollFrames) *
				(self.destPositionX - self.position.x) / self.rollMove
			)
			self.frame_current_x = self.rollFrames[
				min(frameIndex, len(self.rollFrames) - 1)
			]
		# Clear attack frame
		elif self.cooldownAttack <= COOLDOWN_ATTACK - ATTACK_DURATION:
			self.frame_current_x = FRAME_IDLE

		# Blink player during stun
		if self.stunned:
			if self.cooldownMove <= 0:
				self.stunned = False
				self.wtf.scale = Vector2(0, 0)
				self.scale = Vector2(1, 1)
			# Blink (roughly every half second)
			elif self.cooldownMove % STUN_BLINK_RATE > STUN_BLINK_RATE / 2:
				# Don't use 0 because `shown()` should still return True
				self.scale = Vector2(0.01, 0.01)
			else:
				self.scale = Vector2(1, 1)

		# Check player move inputs
		if io.LB.is_just_pressed:
			if (
				not moving and
				not self.stunned and
				self.cooldownMove <= 0 and
				self.cooldownBumperLeft <= 0
			):
				if self.destPositionX == POS_X_RIGHT:
					self.setPosition(POS_X_CENTER)
					self.rollFrames = FRAMES_ROLL_LEFT
				elif self.destPositionX == POS_X_CENTER:
					self.setPosition(POS_X_LEFT)
					self.rollFrames = FRAMES_ROLL_LEFT
			self.cooldownBumperLeft = COOLDOWN_BUTTON

		elif io.RB.is_just_pressed:
			if (
				not moving and
				not self.stunned and
				self.cooldownMove <= 0 and
				self.cooldownBumperRight <= 0
			):
				if self.destPositionX == POS_X_LEFT:
					self.setPosition(POS_X_CENTER)
					self.rollFrames = FRAMES_ROLL_RIGHT
				elif self.destPositionX == POS_X_CENTER:
					self.setPosition(POS_X_RIGHT)
					self.rollFrames = FRAMES_ROLL_RIGHT
			self.cooldownBumperRight = COOLDOWN_BUTTON

		# Check player attack input
		if io.A.is_just_pressed:
			if (
				not moving and
				not self.stunned and
				self.cooldownAttack <= 0 and
				self.cooldownA <= 0
			):
				self.cooldownAttack = COOLDOWN_ATTACK
				# TODO: Unable to move a bit following attack??
				self.cooldownMove = max(COOLDOWN_MOVE, ATTACK_DURATION)
				print("Player: attack")
				self.frame_current_x = FRAME_ATTACK
				self.onAttack(Vector2(
					# Set position to top of player
					self.position.x,
					self.position.y - self.texture.height / 2,
				))
			self.cooldownA = COOLDOWN_BUTTON

	def setPosition(self, pos: int):
		self.cooldownMove = COOLDOWN_MOVE
		if pos <= POS_X_LEFT:
			print("Player: move left")
			self.destPositionX = POS_X_LEFT
		elif pos == POS_X_CENTER:
			print("Player: move center")
			self.destPositionX = POS_X_CENTER
		else:
			print("Player: move right")
			self.destPositionX = POS_X_RIGHT
		self.rollMove = self.destPositionX - self.position.x

	def stun(self, duration):
		self.stunned = True
		self.cooldownMove = duration
		self.wtf.scale = self.scale

	def show(self, onAttack):
		self.cooldownBumperLeft = COOLDOWN_BUTTON
		self.cooldownBumperRight = COOLDOWN_BUTTON
		self.cooldownA = COOLDOWN_BUTTON
		self.cooldownB = COOLDOWN_BUTTON
		self.cooldownMove = COOLDOWN_MOVE
		self.cooldownAttack = 0
		self.cooldownBlock = 0
		self.stunned = False
		self.setPosition(POS_X_CENTER)
		self.onAttack = onAttack
		self.scale = Vector2(1, 1)
		if SHOW_HITBOX:
			self.hitbox.scale = Vector2(1, 1)

	def hide(self):
		self.scale = Vector2(0, 0)
		self.wtf.scale = self.scale
		self.hitbox.scale = self.scale

	def shown(self):
		return self.scale.x > 0
