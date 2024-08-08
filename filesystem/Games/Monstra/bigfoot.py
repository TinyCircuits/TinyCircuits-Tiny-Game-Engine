import engine_draw as draw
import engine_io as io
from engine_nodes import (
	Sprite2DNode,
	Rectangle2DNode,
)
from engine_math import Vector2
from math import atan2, sin, cos
from random import (
	uniform,
	randrange
)

from constants import (
	MIN_X,
	MAX_X,
	MIN_Y,
	MAX_Y,
	SHOW_HITBOX,
)
from hearts import Hearts
from bigfootBoulder import BigfootBoulder
from bigfootTree import BigfootTree
from player import (
	POS_Y as PLAYER_POS_Y,
	POS_X_LEFT as PLAYER_POS_X_LEFT,
	POS_X_RIGHT as PLAYER_POS_X_RIGHT,
)

# Bigfoot attributes
MAX_HP = 12
HITBOX_PADDING = 3

MOVE_SPEED = 20 # pixels / sec.
MOVE_PATH = [
	Vector2(MIN_X + 25,	-10),
	Vector2(MIN_X + 25,	MIN_Y + 20),
	Vector2(0,			MIN_Y + 50),
	Vector2(MAX_X - 25,	MIN_Y + 20),
	Vector2(MAX_X - 25,	-10),
]
MIN_MOVE_TIME = 2 # min. duration spent moving (sec.)
MAX_MOVE_TIME = 4

# If hit, show hit frame for HIT_DURATION then show block frame for
# BLOCK_DURATION. During these durations, Bigfoot is invulnerable.
HIT_DURATION = 0.5 # seconds to display hit frame
BLOCK_DURATION = 1.8 # seconds to display block frame

RUMBLE_DURATION = 0.3 # seconds to rumble after boulder / tree land

BOULDER_ATTACK_COUNT = 3 # number of boulders to drop
BOULDER_THROW_DELAY = 0.8 # stop moving for this delay before first throw
BOULDER_THROW_DELAY_BETWEEN = 0.7 # delay between each throw
BOULDER_LAND_DELAY = 2.5 # duration between throwing and landing
BOULDER_LAND_RUMBLE = 0.25

SLAM_MOVE_SPEED = 160
SLAM_RUMBLE = 0.75
SLAM_SCREEN_SHAKE_DURATION = 1
SLAM_TREE_LAND_DELAY = 1.5 # should probably be longer than shake duration
SLAM_TREE_LAND_RUMBLE = 0.5
SLAM_TREE_CLEANUP_DELAY = 0.5
SLAM_TREE_LARGE_CHANCE = 0.5 # probability tree is large [0, 1]
SLAM_TREE_POS_Y = PLAYER_POS_Y - 6

BLOCK_FRAMES = 1
HIT_FRAMES = 1
IDLE_FRAMES = 2
MOVE_FRAMES = 4
SLAM_FRAMES = 2
THROW_FRAMES = 1

STATE_MOVING = 1
STATE_BOULDER_ATTACK = 2
STATE_SLAM_ATTACK = 3


class Bigfoot(Sprite2DNode):
	def __init__(
		self,
		blockTexture,
		hitTexture,
		idleTexture,
		moveTexture,
		slamTexture,
		throwTexture,
		heartTexture,
		boulderTexture, boulderShadowTexture,
		treeLargeTexture, treeSmallTexture,
		onBoulderThrow, onBoulderLand,
		getPlayerCollision, shakeScreen,
		onTreeDrop, onTreeLand, onTreeCleanup,
	):
		super().__init__(self)
		self.blockTexture = blockTexture
		self.hitTexture = hitTexture
		self.idleTexture = idleTexture
		self.moveTexture = moveTexture
		self.slamTexture = slamTexture
		self.throwTexture = throwTexture
		self.boulderTexture = boulderTexture
		self.boulderShadowTexture = boulderShadowTexture
		self.treeLargeTexture = treeLargeTexture
		self.treeSmallTexture = treeSmallTexture
		self.onBoulderThrow = onBoulderThrow
		self.onBoulderLand = onBoulderLand
		self.getPlayerCollision = getPlayerCollision
		self.shakeScreen = shakeScreen
		self.onTreeDrop = onTreeDrop
		self.onTreeLand = onTreeLand
		self.onTreeCleanup = onTreeCleanup
		self.transparent_color = draw.white
		self.fps = 3
		self.playing = False

		# Bigfoot properties
		self.movePathIndex = 0
		self.hp = MAX_HP
		self.boulders = []
		self.slamPath = []
		self.tree = None
		self.hitTimer = 0
		self.blockTimer = 0
		self.rumbleTimer = 0

		# Create heart sprites
		self.hearts = Hearts(
			texture=heartTexture,
			maxHP=MAX_HP,
		)

		# Hitbox for collision debugging
		self.hitbox = Rectangle2DNode(
			color=draw.blue,
			width=self.idleTexture.width / IDLE_FRAMES - HITBOX_PADDING * 2,
			height=self.idleTexture.height - HITBOX_PADDING * 2,
		)
		self.add_child(self.hitbox)
		self.hide()

	def hit(self):
		if self.blockTimer > 0:
			print("Bigfoot: blocked hit.")
			return False
		print("Bigfoot: hit!")
		self.setHP(self.hp - 1)
		self.hitTimer = HIT_DURATION
		self.blockTimer = HIT_DURATION + BLOCK_DURATION
		return True

	def dead(self):
		return self.hp == 0

	def setHP(self, hp):
		# return if game over
		if not self.shown():
			return

		self.hp = self.hearts.setHP(hp)

	def throwBoulder(self):
		print("Bigfoot: Tossing boulder")
		playerRect = self.getPlayerCollision()

		# Try several random positions so that boulders do not overlap
		i = 0
		while i < 10:
			pos = Vector2(
				uniform(PLAYER_POS_X_LEFT, PLAYER_POS_X_RIGHT),
				uniform(
					max(MIN_Y, playerRect.y - 15),
					min(MAX_Y, playerRect.y + 15),
				),
			)
			# Check position validity
			isValid = True
			for boulder in self.boulders:
				if abs(boulder.position.x - pos.x) < self.boulderTexture.width:
					isValid = False
					break
			if isValid:
				# Good to go!
				break
			# Else, pick another random position; give up after 10 tries
			i += 1

		def onLand(boulder):
			print("Bigfoot: Boulder landed")
			self.boulders.remove(boulder)
			self.rumble(BOULDER_LAND_RUMBLE)
			self.onBoulderLand(boulder)

		boulder = BigfootBoulder(
			texture=self.boulderTexture,
			shadowTexture=self.boulderShadowTexture,
			position=pos,
			landTimer=BOULDER_LAND_DELAY,
			onLand=onLand,
		)
		self.boulders.append(boulder)
		self.onBoulderThrow(boulder)

	def dropTree(self):
		if uniform(0, 1) < SLAM_TREE_LARGE_CHANCE:
			texture = self.treeLargeTexture
		else:
			texture = self.treeSmallTexture

		if self.position.x < 0:
			print("Bigfoot: Drop tree left")
			pos = Vector2(MIN_X + texture.width / 7 / 2, SLAM_TREE_POS_Y)
			scaleX = 1
		else:
			print("Bigfoot: Drop tree right")
			pos = Vector2(MAX_X - texture.width / 7 / 2, SLAM_TREE_POS_Y)
			scaleX = -1

		def onLand(tree):
			print("Bigfoot: Tree landed")
			self.tree = None
			self.rumble(SLAM_TREE_LAND_RUMBLE)
			self.onTreeLand(tree)

		self.tree = BigfootTree(
			texture=texture,
			scaleX=scaleX,
			position=pos,
			landTimer=SLAM_TREE_LAND_DELAY,
			cleanupTimer=SLAM_TREE_CLEANUP_DELAY,
			onLand=onLand,
			onCleanup=self.onTreeCleanup,
		)
		self.onTreeDrop(self.tree)
		self.shakeScreen(SLAM_SCREEN_SHAKE_DURATION, SLAM_RUMBLE)

	def rumble(self, intensity):
		self.rumbleTimer = RUMBLE_DURATION
		io.rumble(intensity)

	def tick(self, dt):
		if not self.shown():
			return

		# Perform state transitions when timer expires
		if self.stateTimer > 0:
			self.stateTimer -= dt
		if self.hitTimer > 0:
			self.hitTimer -= dt
		if self.blockTimer > 0:
			self.blockTimer -= dt
		if self.rumbleTimer > 0:
			self.rumbleTimer -= dt

		# Update current texture
		if self.hitTimer > 0:
			self.texture = self.hitTexture
			self.frame_count_x = HIT_FRAMES
			self.playing = False
		elif self.state == STATE_SLAM_ATTACK:
			self.texture = self.slamTexture
			self.frame_count_x = SLAM_FRAMES
			self.playing = False
		elif self.state == STATE_BOULDER_ATTACK:
			if (
				len(self.boulders) == 0 or
				self.stateTimer < BOULDER_THROW_DELAY_BETWEEN / 2
			):
				self.texture = self.idleTexture
				self.frame_count_x = IDLE_FRAMES
			else:
				self.texture = self.throwTexture
				self.frame_count_x = THROW_FRAMES
			self.playing = True
		elif self.blockTimer > 0:
			self.texture = self.blockTexture
			self.frame_count_x = BLOCK_FRAMES
			self.playing = False
		elif self.state == STATE_MOVING:
			self.texture = self.moveTexture
			self.frame_count_x = MOVE_FRAMES
			self.playing = True

		# Update state
		if self.stateTimer <= 0:
			if self.state == STATE_MOVING:
				self.setState(STATE_BOULDER_ATTACK)
			elif self.state == STATE_BOULDER_ATTACK:
				if len(self.boulders) < BOULDER_ATTACK_COUNT:
					self.throwBoulder()
					self.stateTimer = BOULDER_THROW_DELAY_BETWEEN
				else:
					self.setState(STATE_MOVING)
		if self.rumbleTimer <= 0:
			io.rumble(0)

		# Move Bigfoot
		if self.state == STATE_MOVING:
			# Determine next destination in path
			dest = MOVE_PATH[self.movePathIndex]
			# Check to see if destination was reached
			if (
				abs(self.position.x - dest.x) < MOVE_SPEED * dt and
				abs(self.position.y - dest.y) < MOVE_SPEED * dt
			):
				# dest reached
				# if we reached the center, do a slam attack
				if dest.x == 0 and self.hp < MAX_HP:
					self.setState(STATE_SLAM_ATTACK)
				# update index and dest
				if self.movePathIndex == len(MOVE_PATH) - 1:
					self.moveDirection = -1
				elif self.movePathIndex == 0:
					self.moveDirection = 1
				self.movePathIndex += self.moveDirection
				dest = MOVE_PATH[self.movePathIndex]

			# Move Bigfoot to dest
			dx = dest.x - self.position.x
			dy = dest.y - self.position.y
			angle = atan2(dy, dx) # move along this angle
			self.position.x += MOVE_SPEED * dt * cos(angle)
			self.position.y += MOVE_SPEED * dt * sin(angle)

			# Update sprite orientation based on movement direction
			self.scale.x = self.moveDirection
		elif self.state == STATE_SLAM_ATTACK:
			dest = self.slamPath[0]
			if (
				abs(self.position.x - dest.x) < SLAM_MOVE_SPEED * dt and
				abs(self.position.y - dest.y) < SLAM_MOVE_SPEED * dt
			):
				# dest reached
				self.frame_current_x = 1
				self.slamPath.pop(0)
				if len(self.slamPath) == 0:
					self.setState(STATE_MOVING)
				else:
					self.dropTree()
			else:
				# Update sprite orientation based on movement direction
				if (
					(dest.x < self.position.x and self.scale.x > 0) or
					(dest.x > self.position.x and self.scale.x < 0)
				):
					self.scale.x *= -1

				# Move Bigfoot to dest
				dx = dest.x - self.position.x
				dy = dest.y - self.position.y
				angle = atan2(dy, dx) # move along this angle
				self.position.x += SLAM_MOVE_SPEED * dt * cos(angle)
				self.position.y += SLAM_MOVE_SPEED * dt * sin(angle)

	def setState(self, state):
		self.state = state
		if state == STATE_MOVING:
			print("Bigfoot: Move start")
			self.stateTimer = uniform(MIN_MOVE_TIME, MAX_MOVE_TIME)
		elif state == STATE_BOULDER_ATTACK:
			print("Bigfoot: Boulder attack start")
			self.stateTimer = BOULDER_THROW_DELAY
		elif state == STATE_SLAM_ATTACK:
			print("Bigfoot: Slam attack start")
			# Set slam path
			if uniform(0, 1) >= 0.5:
				slamX = MIN_X + self.texture.width / 2
			else:
				slamX = MAX_X - self.texture.width / 2
			self.slamPath = [
				Vector2(slamX, self.position.y),
				Vector2(self.position.x, self.position.y),
			]

	def show(self):
		self.setState(STATE_MOVING)
		self.movePathIndex = randrange(len(MOVE_PATH))
		self.moveDirection = 1 if uniform(0, 1) >= 0.5 else -1
		dest = MOVE_PATH[self.movePathIndex]
		self.position.x = dest.x
		self.position.y = dest.y
		self.scale = Vector2(1, 1)
		self.hitTimer = 0
		self.blockTimer = 0
		self.rumbleTimer = 0
		if SHOW_HITBOX:
			self.hitbox.scale = self.scale
		self.hearts.show()
		self.setHP(MAX_HP)

	def hide(self):
		self.laserShot = None
		self.tornado = None
		self.scale = Vector2(0, 0)
		self.hitbox.scale = self.scale
		self.hearts.hide()
		self.boulders = []
		self.tree = None
		io.rumble(0)

	def shown(self):
		return self.scale.x != 0
