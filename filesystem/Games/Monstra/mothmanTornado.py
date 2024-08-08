import engine_draw as draw
from engine_nodes import (
	Sprite2DNode,
	Rectangle2DNode,
)
from engine_math import Vector2
from random import uniform

from constants import SHOW_HITBOX

HITBOX_PADDING = 3
MOVE_SPEED = 40 # pixels / sec.
RANDOM_WALK_FACTOR = 5 # move randomly left/right by this factor of MOVE_SPEED
TORNADO_STUN_DURATION = 3 # sec.
FRAMES = 4


class MothmanTornado(Sprite2DNode):
	def __init__(self, texture, position, collisionCheck):
		super().__init__(self)
		self.texture = texture
		self.position = position
		self.collisionCheck = collisionCheck
		self.transparent_color = draw.white
		self.frame_count_x = FRAMES
		self.fps = FRAMES * 2
		self.moving = True
		# Hitbox for collision debugging
		self.hitbox = Rectangle2DNode(
			color=draw.red,
			width=self.texture.width / FRAMES - HITBOX_PADDING * 2,
			height=self.texture.height - HITBOX_PADDING * 2,
		)
		if not SHOW_HITBOX:
			self.hitbox.scale = Vector2(0, 0)
		self.add_child(self.hitbox)

	def stop(self):
		self.moving = False

	def tick(self, dt):
		if not self.moving:
			return

		# Move projectile downward and randomly left/right
		self.position.x += uniform(-RANDOM_WALK_FACTOR, RANDOM_WALK_FACTOR) * \
			MOVE_SPEED * dt
		self.position.y += MOVE_SPEED * dt

		# Run collision check
		self.collisionCheck(self, TORNADO_STUN_DURATION)
