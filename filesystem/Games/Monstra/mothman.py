import engine_draw as draw
import engine_io as io
from engine_nodes import (
	Sprite2DNode,
	Rectangle2DNode,
)
from engine_math import Vector2
from random import uniform
from math import sin, cos, pi

from constants import (
	MAX_X,
	MAX_Y,
	SHOW_HITBOX,
)
from hearts import Hearts
from mothmanLaserShot import MothmanLaserShot
from mothmanTornado import MothmanTornado
from player import (
	POS_X_LEFT as PLAYER_POS_X_LEFT,
	POS_X_CENTER as PLAYER_POS_X_CENTER,
	POS_X_RIGHT as PLAYER_POS_X_RIGHT
)

# Mothman attributes
MAX_HP = 12

MOVE_SPEED = 60 # x position move speed in pixels / sec.
MOVE_SPEED_TORNADO_ATTACK = MOVE_SPEED * 3
MOVE_SPEED_LUNGE_ATTACK = MOVE_SPEED * 1.3
LASER_ATTACK_DURATION = 0.4 # attack duration time before laser appears (sec.)
LASER_SHOT_DURATION = 0.6 # duration laser beam is displayed (sec.)
LASER_SHOT_CHANCE = 0.3
TORNADO_ATTACK_DURATION = 0.7 # sec.
TORNADO_CHANCE = 0.4
# LUNGE_CHANCE = 1 - LASER_SHOT_CHANCE - TORNADO_CHANCE
LUNGE_ACCELERATION = 150 # pixels / sec.^2
MIN_MOVE_TIME = 1.5 # min. duration spent moving (sec.)
MAX_MOVE_TIME = 2.7
HIT_DURATION = 0.5 # seconds to display hit frame
RUMBLE_FACTOR = 2000000 # inversely proportional distance to player ^ 4
RUMBLE_MAX = 0.6

POS_Y = -30
POS_Y_RANGE = 15 # pixels (half of ellipse minor axis)
HITBOX_PADDING = 9

LASER_ATTACK_FRAMES = 7
TORNADO_ATTACK_FRAMES = 3
MOVE_FRAMES = 4
MOVE_FPS = 1.2 * MOVE_FRAMES
HIT_FRAMES = 1

STATE_MOVING = 1
STATE_LASER_ATTACK = 2
STATE_LASER_SHOOT = 3
STATE_TORNADO_ATTACK = 4
STATE_LUNGE_ATTACK = 5


class Mothman(Sprite2DNode):
	def __init__(
		self,
		attackLaserTexture, attackTornadoTexture, moveTexture, hitTexture,
		heartTexture, laserTexture, tornadoTexture,
		onLaserShot, laserCollision,
		onTornado, tornadoCollision,
		onLunge, lungeCollision,
		getPlayerCollision,
	):
		super().__init__(self)
		self.attackLaserTexture = attackLaserTexture
		self.attackTornadoTexture = attackTornadoTexture
		self.moveTexture = moveTexture
		self.hitTexture = hitTexture
		self.laserTexture = laserTexture
		self.tornadoTexture = tornadoTexture
		self.onLaserShot = onLaserShot
		self.laserCollision = laserCollision
		self.onTornado = onTornado
		self.tornadoCollision = tornadoCollision
		self.onLunge = onLunge
		self.lungeCollision = lungeCollision
		self.getPlayerCollision = getPlayerCollision
		self.transparent_color = draw.white
		self.playing = False

		# Mothman properties
		self.movePosition = 0
		self.moveSpeed = MOVE_SPEED
		self.hp = MAX_HP
		self.laserShot = None
		self.tornado = None
		self.hitTimer = 0

		# Create heart sprites
		self.hearts = Hearts(
			texture=heartTexture,
			maxHP=MAX_HP,
		)

		# Hitbox for collision debugging
		self.hitbox = Rectangle2DNode(
			color=draw.blue,
			width=moveTexture.width / MOVE_FRAMES - HITBOX_PADDING * 2,
			height=moveTexture.height - HITBOX_PADDING * 2,
		)
		self.add_child(self.hitbox)
		self.hide()

	def setEllipsePath(self, center, radius):
		self.ellipseCenter = center
		self.ellipseRadius = radius
		# See https://stackoverflow.com/a/71655244
		self.ellipsePerimeter = pi * (2 * (radius.x ** 2 + radius.y ** 2)) ** 0.5
		self.ellipseAngleFactor = (2 / (radius.x ** 2 + radius.y ** 2)) ** 0.5

	def hit(self):
		print("Mothman: hit!")
		self.setHP(self.hp - 1)
		self.hitTimer = HIT_DURATION

	def dead(self):
		return self.hp == 0

	def setHP(self, hp):
		# return if game over
		if not self.shown():
			return

		self.hp = self.hearts.setHP(hp)

	def attack(self):
		# self.hitTimer = 0
		prob = uniform(0, 1)
		if prob < LASER_SHOT_CHANCE:
			print("Mothman: Laser Attack")
			self.setState(STATE_LASER_ATTACK)
			self.laserShot = MothmanLaserShot(
				texture=self.laserTexture,
				# Set position to mothman mouth
				position=Vector2(
					self.position.x,
					self.position.y - 4 + self.laserTexture.height / 2,
				),
				fireDelay=LASER_ATTACK_DURATION,
				duration=LASER_SHOT_DURATION,
				collisionCheck=self.laserCollision
			)
		elif prob < LASER_SHOT_CHANCE + TORNADO_CHANCE:
			print("Mothman: Tornado Attack")
			self.setState(STATE_TORNADO_ATTACK)
			proj = MothmanTornado(
				texture=self.tornadoTexture,
				# Set position to below mothman
				position=Vector2(self.position.x + 1, self.position.y + 12),
				collisionCheck=self.tornadoCollision,
			)
			self.onTornado(proj)
		else:
			print("Mothman: Lunge Attack")
			prob = uniform(0, 1)
			maxXRadius = MAX_X / 2
			if prob >= 2/3 and abs(PLAYER_POS_X_RIGHT - self.position.x) < maxXRadius:
				playerPosition = Vector2(PLAYER_POS_X_RIGHT, MAX_Y - 25)
			elif prob >= 1/3 and abs(PLAYER_POS_X_LEFT - self.position.x) < maxXRadius:
				playerPosition = Vector2(PLAYER_POS_X_LEFT, MAX_Y - 25)
			else:
				playerPosition = Vector2(PLAYER_POS_X_CENTER, MAX_Y - 25)

			self.setEllipsePath(
				# Center of ellipse is aligned with X of mothman and Y of player
				center=Vector2(playerPosition.x, self.position.y),
				radius=Vector2(
					# Ellipse axes then become the distances between mothman and
					# player, but leave some padding at the bottom of the
					# ellipse
					self.position.x - playerPosition.x,
					playerPosition.y - self.position.y - 20,
				),
			)
			self.moveSpeed = MOVE_SPEED_LUNGE_ATTACK
			self.lungePosition = 0
			self.setState(STATE_LUNGE_ATTACK)
			self.onLunge()

	def tick(self, dt):
		if not self.shown():
			return

		# Perform state transitions when timer expires
		if self.stateTimer > 0:
			self.stateTimer -= dt
		if self.hitTimer > 0:
			self.hitTimer -= dt
		if self.stateTimer <= 0:
			if self.state == STATE_LASER_ATTACK:
				self.setState(STATE_LASER_SHOOT)
				self.onLaserShot(self.laserShot)
				self.laserShot = None
			elif (
				self.state == STATE_LASER_SHOOT or
				self.state == STATE_TORNADO_ATTACK
			):
				print("Mothman: Move start")
				self.setState(STATE_MOVING)
			elif self.state == STATE_MOVING:
				self.attack()

		# Update current texture
		if self.state == STATE_LASER_ATTACK or self.state == STATE_LASER_SHOOT:
			self.texture = self.attackLaserTexture
			self.frame_count_x = LASER_ATTACK_FRAMES
			self.fps = LASER_ATTACK_FRAMES / LASER_ATTACK_DURATION
			if self.state == STATE_LASER_ATTACK:
				self.playing = True
			else:
				self.playing = False
				self.frame_current_x = LASER_ATTACK_FRAMES - 1
		elif self.state == STATE_TORNADO_ATTACK:
			self.texture = self.attackTornadoTexture
			self.frame_count_x = TORNADO_ATTACK_FRAMES
			self.fps = TORNADO_ATTACK_FRAMES / TORNADO_ATTACK_DURATION
			self.playing = True
		elif self.hitTimer > 0:
			self.texture = self.hitTexture
			self.frame_count_x = HIT_FRAMES
			self.playing = False
		else:
			self.texture = self.moveTexture
			self.frame_count_x = MOVE_FRAMES
			self.fps = MOVE_FPS
			self.playing = True

		# Move mothman
		# Move / lunge position refers to the distance traveled on the ellipse
		# perimeter starting from
		# (self.ellipseCenter.x + self.ellipseRadius.x, self.ellipseCenter.y)
		if self.state == STATE_LUNGE_ATTACK:
			percent = self.lungePosition / self.ellipsePerimeter
			if percent < 0.5:
				self.moveSpeed += dt * LUNGE_ACCELERATION
			else:
				self.moveSpeed -= dt * LUNGE_ACCELERATION
			self.lungePosition += dt * self.moveSpeed
			if self.lungePosition >= self.ellipsePerimeter:
				# End of lunge attack, return to moving normally
				self.setState(STATE_MOVING)
			else:
				angle = self.lungePosition * self.ellipseAngleFactor
				self.position.x = self.ellipseCenter.x + cos(angle) * self.ellipseRadius.x
				self.position.y = self.ellipseCenter.y - sin(angle) * self.ellipseRadius.y
				# Face direction in which we are moving if ellipse is wide enough
				if self.ellipseRadius.x > 15:
					self.scale.x = 1 if angle > pi else -1
				elif self.ellipseRadius.x < -15:
					self.scale.x = -1 if angle > pi else 1
		elif self.state == STATE_MOVING or self.state == STATE_TORNADO_ATTACK:
			self.movePosition += dt * self.moveSpeed
			if self.movePosition >= self.ellipsePerimeter:
				self.movePosition -= self.ellipsePerimeter
			angle = self.movePosition * self.ellipseAngleFactor
			self.position.x = self.ellipseCenter.x + cos(angle) * self.ellipseRadius.x
			self.position.y = self.ellipseCenter.y - sin(angle) * self.ellipseRadius.y
			# Face direction in which we are moving
			self.scale.x = 1 if angle > pi else -1

		# Rumble based on distance to mothman
		playerRect = self.getPlayerCollision()
		distanceSquared = (self.position.x - playerRect.x) ** 2 + \
			(self.position.y - playerRect.y) ** 2
		rumble = min(RUMBLE_MAX, RUMBLE_FACTOR / (distanceSquared ** 2))
		# Adjust based on hardware limits
		if rumble < 0.05:
			rumble = 0
		elif rumble < 0.2:
			rumble = 0.2
		io.rumble(rumble)

		# Detect lunge collision
		if self.state == STATE_LUNGE_ATTACK:
			self.lungeCollision()

	def setState(self, state):
		self.state = state
		if state == STATE_MOVING:
			self.stateTimer = uniform(MIN_MOVE_TIME, MAX_MOVE_TIME)
			self.moveSpeed = MOVE_SPEED
			halfWidth = self.moveTexture.width / 2 / MOVE_FRAMES
			self.setEllipsePath(
				center=Vector2(0, POS_Y),
				radius=Vector2(MAX_X - halfWidth, POS_Y_RANGE),
			)
		elif state == STATE_LASER_ATTACK:
			self.stateTimer = LASER_ATTACK_DURATION
		elif state == STATE_LASER_SHOOT:
			self.stateTimer = LASER_SHOT_DURATION
		elif state == STATE_TORNADO_ATTACK:
			self.stateTimer = TORNADO_ATTACK_DURATION
			self.moveSpeed = MOVE_SPEED_TORNADO_ATTACK
		# do nothing for lunge -- no timer needed

	def show(self):
		self.setState(STATE_MOVING)
		self.movePosition = uniform(0, self.ellipsePerimeter)
		self.scale = Vector2(1, 1)
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
		io.rumble(0)

	def shown(self):
		return self.scale.x != 0
