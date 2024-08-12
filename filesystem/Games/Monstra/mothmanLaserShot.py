import engine_draw as draw
from engine_math import Vector2
from engine_nodes import (
	Sprite2DNode,
	Rectangle2DNode,
)

from constants import SHOW_HITBOX

MOVE_SPEED = 120 # pixels / sec.
FRAMES = 7
FRAME_HIT_CHECK = 3
HITBOX_WIDTH = 24


class MothmanLaserShot(Sprite2DNode):
	def __init__(self, texture, position, fireDelay, duration, collisionCheck):
		super().__init__(self)
		self.texture = texture
		self.frame_count_x = FRAMES
		self.position = position
		self.fireDelay = fireDelay
		self.duration = duration
		self.fps = FRAMES / duration
		self.playing = False
		self.collisionCheck = collisionCheck
		self.transparent_color = draw.white
		self.active = True
		# Hitbox for collision debugging
		self.hitbox = Rectangle2DNode(
			color=draw.red,
			width=HITBOX_WIDTH,
			height=self.texture.height,
		)
		self.add_child(self.hitbox)
		self.hide()

	def stop(self):
		self.active = False

	def show(self):
		self.scale = Vector2(1, 1)
		if SHOW_HITBOX:
			self.hitbox.scale = Vector2(1, 1)

	def hide(self):
		self.scale = Vector2(0, 0)
		self.hitbox.scale = Vector2(0, 0)

	def tick(self, dt):
		if not self.active:
			return

		# Do nothing until fireDelay reaches 0
		if self.fireDelay > 0:
			self.fireDelay -= dt
			return

		# While active, show laser and check for collisions
		if self.duration > 0:
			self.duration -= dt
			self.playing = True
			self.show()
			# Run collision check
			if self.frame_current_x >= FRAME_HIT_CHECK:
				self.collisionCheck(self)
		else:
			self.hide()
