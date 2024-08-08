import engine_draw as draw
from engine_nodes import (
	Sprite2DNode,
	Circle2DNode,
)
from random import randrange

from constants import SHOW_HITBOX

HITBOX_PADDING = 0


class FrogmanWandShot(Sprite2DNode):
	def __init__(self, texture, position, velocityX, velocityY, collisionCheck):
		super().__init__(self)
		self.texture = texture
		self.position = position
		self.velocityX = velocityX
		self.velocityY = velocityY
		self.collisionCheck = collisionCheck
		self.transparent_color = draw.white
		self.frame_count_x = 7
		self.playing = False
		self.moving = False
		self.frame_current_x = randrange(6)
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

	def tick(self, dt):
		if not self.moving:
			return

		# Move projectile toward destination
		self.position.x += self.velocityX * dt
		self.position.y += self.velocityY * dt

		# Run collision check
		self.collisionCheck(self)
