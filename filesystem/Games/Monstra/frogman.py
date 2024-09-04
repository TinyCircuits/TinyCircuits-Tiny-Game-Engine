import engine_draw as draw
import engine_io as io
from engine_nodes import (
	Sprite2DNode,
	Rectangle2DNode,
	EmptyNode,
)
from engine_math import Vector2
from random import uniform
from math import sin, cos, pi

from constants import (
	MIN_X,
	MAX_X,
	MAX_Y,
	SHOW_HITBOX,
)
from hearts import Hearts
from frogmanWandShot import FrogmanWandShot
from frogmanBubbleShot import FrogmanBubbleShot

# Frogman attributes
MAX_HP = 12
SHIELD_SUMMON_HP = 5 # summon shield when frogman health hits this level

MOVE_SPEED = 30 # x position move speed in pixels / sec.
ATTACK_DURATION = 0.5 # sec.
STUN_DURATION = 0.5 # sec.
WAND_SHOT_SPEED = 28 # pixels / sec.
BUBBLE_SHOT_CHANCE_FACTOR = 0.60 # affects bubble shot attack probability
# Note: bubble shot probability decreases based on total bubbles on screen
BUBBLE_SHOT_SPEED = 20 # pixels / sec.
BUBBLE_LARGE_CHANCE = 0.1 # proportion of bubbles that are large
BUBBLE_SMALL_RUMBLE = 0.25
BUBBLE_LARGE_RUMBLE = 0.4
SHIELD_ROTATE_SPEED = 1.1 # radians / sec.
MIN_MOVE_TIME = 2 # min. duration spent moving (sec.)
MAX_MOVE_TIME = 4

POS_Y = -30
POS_Y_RANGE = 10 # pixels (half of sine wave amplitude)
MOUTH_POS_X = -3
MOUTH_POS_Y = 6
LILYPAD_POS_Y = 5 # pixel offset
HITBOX_PADDING = 4

# shield sprites are positioned in a circle around Frogman with these
# attributes
SHIELD_RADIUS = 22 # pixels
SHIELD_ANGLE = 2 * pi / 8
SHIELD_BLINK_PATTERN = [2, 4, 0, 3, 1, 5] # length of array is orb count
SHIELD_BLINK_DELAY = 1 # sec.
SHIELD_BLINK_DURATION = 0.1 # sec.
SHIELD_HITBOX_PADDING = 0
SHIELD_MIN_ROTATE_TIME = 7 # min. duration rotating same direction (sec.)
SHIELD_MAX_ROTATE_TIME = 9

FRAME_STUNNED = 0
FRAME_ATTACK_BUBBLE = 5
FRAMES_IDLE = [1, 2, 3, 4]
FRAMES_ATTACK_WAND_SHOT = [6, 7, 8, 9]
IDLE_FPS = 2

STATE_MOVING = 1
STATE_ATTACK_BUBBLE = 2
STATE_ATTACK_WAND_SHOT = 3
STATE_STUNNED = 4


class Frogman(Sprite2DNode):
	def __init__(
		self, texture, heartTexture, orbTexture,
		bubbleLargeTexture, bubbleSmallTexture, lilypadTexture,
		onWandShot, wandShotCollision,
		onBubbleShot, bubbleShotCollision,
	):
		super().__init__(self)
		self.texture = texture
		self.orbTexture = orbTexture
		self.bubbleLargeTexture = bubbleLargeTexture
		self.bubbleSmallTexture = bubbleSmallTexture
		self.onWandShot = onWandShot
		self.wandShotCollision = wandShotCollision
		self.onBubbleShot = onBubbleShot
		self.bubbleShotCollision = bubbleShotCollision
		self.transparent_color = draw.white
		self.frame_count_x = 10
		self.playing = False

		# Frogman properties
		halfWidth = texture.width / 2 / self.frame_count_x
		self.minX = MIN_X + halfWidth
		self.maxX = MAX_X - halfWidth
		self.hp = MAX_HP
		self.shields = None
		self.shieldRotateDirection = 1
		self.wandShot = None
		self.bubbleShot = None
		self.activeBubbles = []
		self.moveDuration = 0

		# Create heart sprites
		self.hearts = Hearts(
			texture=heartTexture,
			maxHP=MAX_HP,
		)

		# Create lilypad
		self.lilypad = Sprite2DNode(
			texture=lilypadTexture,
			transparent_color=draw.white,
			position=Vector2(0, LILYPAD_POS_Y),
		)
		self.add_child(self.lilypad)
		self.layer = 1

		# Hitbox for collision debugging
		self.hitbox = Rectangle2DNode(
			color=draw.blue,
			width=self.texture.width / self.frame_count_x - HITBOX_PADDING * 2,
			height=self.texture.height - HITBOX_PADDING * 2,
		)
		self.add_child(self.hitbox)
		self.hide()

	def hit(self):
		print("Frogman: hit!")
		self.setHP(self.hp - 1)
		self.setState(STATE_STUNNED)

	def dead(self):
		return self.hp == 0

	def setHP(self, hp):
		# return if game over
		if not self.shown():
			return

		self.hp = self.hearts.setHP(hp)
		# summon small frogmans if needed
		if hp <= SHIELD_SUMMON_HP and self.shields is None:
			self.shields = FrogmanShields(self, self.orbTexture)

	def attack(self):
		bubbleShotChance = BUBBLE_SHOT_CHANCE_FACTOR / (len(self.activeBubbles) + 1)
		if uniform(0, 1) >= bubbleShotChance:
			self.setState(STATE_ATTACK_WAND_SHOT)
			print("Frogman: Wand Shot Attack")
			# Wand shot is fired at a randomly selected point at bottom of screen
			destX = uniform(MIN_X, MAX_X)
			destY = MAX_Y
			# Pythagorus, you've done it again, old boy
			# We essentially compute the amount of time to travel along the
			# hypotenuse at WAND_SHOT_SPEED (pixels per second per pixel)
			velocityScaler = WAND_SHOT_SPEED * ((
				(destX - self.position.x) ** 2 +
				(destY - self.position.y) ** 2
			) ** -0.5)

			# Create new projectile
			self.wandShot = FrogmanWandShot(
				texture=self.orbTexture,
				# Set position to frogman mouth
				position=Vector2(
					self.position.x + MOUTH_POS_X,
					self.position.y + MOUTH_POS_Y,
				),
				velocityX=(destX - self.position.x) * velocityScaler,
				velocityY=(destY - self.position.y) * velocityScaler,
				collisionCheck=self.wandShotCollision
			)
		else:
			print("Frogman: Bubble Shot Attack")
			self.setState(STATE_ATTACK_BUBBLE)
			# Wand shot is fired at a randomly selected point at bottom of screen
			destX = uniform(MIN_X, MAX_X)
			destY = MAX_Y
			velocityScaler = BUBBLE_SHOT_SPEED * ((
				(destX - self.position.x) ** 2 +
				(destY - self.position.y) ** 2
			) ** -0.5)

			bubbleTexture = self.bubbleSmallTexture
			rumble = BUBBLE_SMALL_RUMBLE
			if uniform(0, 1) < BUBBLE_LARGE_CHANCE:
				bubbleTexture = self.bubbleLargeTexture
				rumble = BUBBLE_LARGE_RUMBLE
			self.bubbleShot = FrogmanBubbleShot(
				texture=bubbleTexture,
				# Set position to frogman mouth
				position=Vector2(
					self.position.x + MOUTH_POS_X,
					self.position.y + MOUTH_POS_Y,
				),
				velocityX=(destX - self.position.x) * velocityScaler,
				velocityY=(destY - self.position.y) * velocityScaler,
				rumbleIntensity=rumble,
				collisionCheck=self.bubbleShotCollision
			)

	def tick(self, dt):
		if not self.shown():
			return

		# Perform state transitions when timer expires
		if self.stateTimer > 0:
			self.stateTimer -= dt
		if self.stateTimer <= 0:
			if (
				self.state == STATE_ATTACK_BUBBLE or
				self.state == STATE_ATTACK_WAND_SHOT or
				self.wandShot is not None or
				self.bubbleShot is not None
			):
				# Fire projectiles
				if self.wandShot is not None:
					print("Frogman: Wand shot: fire")
					self.wandShot.start()
					self.onWandShot(self.wandShot)
					self.wandShot = None
				if self.bubbleShot is not None:
					print("Frogman: Bubble shot: fire")
					self.bubbleShot.start()
					self.onBubbleShot(self.bubbleShot)
					self.bubbleShot = None
				print("Frogman: Move start")
				self.setState(STATE_MOVING)
			elif self.state == STATE_MOVING or self.state == STATE_STUNNED:
				self.attack()

		if (self.state == STATE_MOVING or (
				self.state == STATE_STUNNED and
				self.wandShot is None and
				self.bubbleShot is None
			)
		):
			# Move frogman
			self.position.x += self.moveDirection * dt * MOVE_SPEED
			if self.position.x >= self.maxX:
				self.position.x = self.maxX # comment this out?
				self.moveDirection = -self.moveDirection
			elif self.position.x <= self.minX:
				self.position.x = self.minX # comment this out?
				self.moveDirection = -self.moveDirection
			self.position.y = POS_Y + self.moveDirection * sin(
				2 * pi *
				(self.position.x - self.minX) /
				(self.maxX - self.minX)
			) * POS_Y_RANGE

		if self.state == STATE_MOVING:
			# Wiggle
			frameIndex = int(
				(self.moveDuration - self.stateTimer) * IDLE_FPS
			) % len(FRAMES_IDLE)
			self.frame_current_x = FRAMES_IDLE[frameIndex]
		elif self.state == STATE_ATTACK_WAND_SHOT:
			frameIndex = min(
				len(FRAMES_ATTACK_WAND_SHOT) - 1,
				int(
					(1 - self.stateTimer / ATTACK_DURATION) *
					len(FRAMES_ATTACK_WAND_SHOT)
				)
			)
			self.frame_current_x = FRAMES_ATTACK_WAND_SHOT[frameIndex]

		# Rotate small frogmans
		if self.shields is not None:
			# Change rotate direction
			if self.rotateChangeTimer > 0:
				self.rotateChangeTimer -= dt
			if self.rotateChangeTimer <= 0:
				self.rotateChangeTimer = uniform(
					SHIELD_MIN_ROTATE_TIME,
					SHIELD_MAX_ROTATE_TIME,
				)
				self.shieldRotateDirection *= -1
			self.shields.rotate(
				dt *
				SHIELD_ROTATE_SPEED *
				self.shieldRotateDirection,
			)

	def setState(self, state):
		self.state = state
		if state == STATE_MOVING:
			self.stateTimer = uniform(MIN_MOVE_TIME, MAX_MOVE_TIME)
			self.moveDuration = self.stateTimer
		elif state == STATE_ATTACK_BUBBLE:
			self.frame_current_x = FRAME_ATTACK_BUBBLE
			self.stateTimer = ATTACK_DURATION
		elif state == STATE_ATTACK_WAND_SHOT:
			self.stateTimer = ATTACK_DURATION
		elif state == STATE_STUNNED:
			self.frame_current_x = FRAME_STUNNED
			self.stateTimer = STUN_DURATION

	def show(self):
		self.position.x = uniform(self.minX, self.maxX)
		self.position.y = POS_Y
		self.moveDirection = 1 if uniform(0, 1) >= 0.5 else -1
		self.rotateChangeTimer = 0
		self.shieldRotateDirection = 1 if uniform(0, 1) >= 0.5 else -1
		self.scale = Vector2(1, 1)
		self.lilypad.scale = self.scale
		if SHOW_HITBOX:
			self.hitbox.scale = self.scale
		self.hearts.show()
		self.setHP(MAX_HP)
		self.setState(STATE_MOVING)

	def hide(self):
		self.wandShot = None
		self.bubbleShot = None
		self.shields = None
		self.activeBubbles = []
		self.scale = Vector2(0, 0)
		self.lilypad.scale = self.scale
		self.hitbox.scale = self.scale
		self.hearts.hide()
		io.rumble(0)

	def shown(self):
		return self.scale.x > 0


class FrogmanShields(EmptyNode):
	def __init__(self, frogman, texture):
		super().__init__(self)
		self.sprites = []
		self.hitboxes = []
		self.angle = 0
		self.blinkIndex = 0
		self.blinkTimer = 0
		self.blinking = False
		for i in SHIELD_BLINK_PATTERN:
			sg = Sprite2DNode(
				texture=texture,
				frame_count_x=7,
				transparent_color=draw.white,
				playing=False,
			)
			sg.frame_current_x = i
			frogman.add_child(sg)
			self.sprites.append(sg)
			self.rotate(0)

			hitbox = Rectangle2DNode(
				color=draw.orange,
				width=sg.texture.width / sg.frame_count_x - SHIELD_HITBOX_PADDING * 2,
				height=sg.texture.height - SHIELD_HITBOX_PADDING * 2,
			)
			if not SHOW_HITBOX:
				hitbox.scale = Vector2(0, 0)
			sg.add_child(hitbox)
			self.hitboxes.append(hitbox)

	def rotate(self, angleOffset):
		self.angle = (self.angle + angleOffset) % (2 * pi)
		for i, sprite in enumerate(self.sprites):
			angle = self.angle + i * SHIELD_ANGLE
			sprite.position.x = SHIELD_RADIUS * cos(angle)
			sprite.position.y = SHIELD_RADIUS * sin(angle)

	def tick(self, dt):
		if self.blinkTimer > 0:
			self.blinkTimer -= dt
		if self.blinkTimer <= 0:
			sg = self.sprites[SHIELD_BLINK_PATTERN[self.blinkIndex]]
			if self.blinking:
				# stop blinking
				sg.frame_current_x = self.blinkIndex
				self.blinking = False
				self.blinkTimer = SHIELD_BLINK_DELAY
				if self.blinkIndex >= len(SHIELD_BLINK_PATTERN) - 1:
					self.blinkIndex = 0
				else:
					self.blinkIndex += 1
			else:
				# start blinking
				sg.frame_current_x = len(SHIELD_BLINK_PATTERN)
				self.blinking = True
				self.blinkTimer = SHIELD_BLINK_DURATION
