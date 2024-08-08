import engine_draw as draw
from engine_nodes import (
	Sprite2DNode,
	Rectangle2DNode,
)
from engine_math import Vector2

from constants import SHOW_HITBOX

PLAYER_SHOT_SPEED = 100 # pixels / sec.
PLAYER_SHOT_HITBOX_PADDING = 1


class PlayerShot(Sprite2DNode):
	def __init__(self, texture, position, collisionCheck):
		super().__init__(self)
		self.position = position
		self.collisionCheck = collisionCheck
		self.texture = texture
		self.transparent_color = draw.white
		self.moving = True
		# Hitbox for collision debugging
		self.hitbox = Rectangle2DNode(
			color=draw.orange,
			width=self.texture.width - PLAYER_SHOT_HITBOX_PADDING * 2,
			height=self.texture.height - PLAYER_SHOT_HITBOX_PADDING * 2,
		)
		self.add_child(self.hitbox)
		if not SHOW_HITBOX:
			self.hitbox.scale = Vector2(0, 0)

	def stop(self):
		self.moving = False

	def tick(self, dt):
		if not self.moving:
			return

		# Move projectile upward
		self.position.y -= PLAYER_SHOT_SPEED * dt

		# Run collision check
		self.collisionCheck(self)
