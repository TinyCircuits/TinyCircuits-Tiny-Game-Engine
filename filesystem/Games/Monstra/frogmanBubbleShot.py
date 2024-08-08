import engine_draw as draw
import engine_io as io
from engine_nodes import (
	Sprite2DNode,
	Circle2DNode,
)

from constants import SHOW_HITBOX

HITBOX_PADDING = 3
BUBBLE_RUMBLE_DURATION = 0.25


class FrogmanBubbleShot(Sprite2DNode):
	def __init__(
		self, texture, position, velocityX, velocityY, rumbleIntensity,
		collisionCheck,
	):
		super().__init__(self)
		self.texture = texture
		self.position = position
		self.velocityX = velocityX
		self.velocityY = velocityY
		self.rumbleIntensity = rumbleIntensity
		self.collisionCheck = collisionCheck
		self.transparent_color = draw.white
		self.moving = False
		self.bounces = 0
		self.frame_count_x = 4
		self.playing = True
		self.rumbleTimer = 0
		self.layer = 1
		# Hitbox for collision debugging
		self.hitbox = Circle2DNode(
			color=draw.red,
			radius=self.texture.height / 2 - HITBOX_PADDING,
		)
		if not SHOW_HITBOX:
			self.hitbox.scale = 0
		self.add_child(self.hitbox)

	def start(self):
		self.moving = True

	def stop(self):
		self.moving = False
		# Stop rumbling abruptly because we are getting garbage collected
		if self.rumbleTimer > 0:
			io.rumble(0)

	def bounce(self, velocityX, velocityY):
		self.bounces += 1
		# TODO: Fix this
		self.velocityX = velocityX
		self.velocityY = velocityY
		self.rumbleTimer = BUBBLE_RUMBLE_DURATION

	def tick(self, dt):
		if self.rumbleTimer > 0:
			self.rumbleTimer -= dt
			io.rumble(0 if self.rumbleTimer <= 0 else self.rumbleIntensity)

		if not self.moving:
			return

		# Move projectile toward destination
		self.position.x += self.velocityX * dt
		self.position.y += self.velocityY * dt

		# Run collision check
		self.collisionCheck(self)
