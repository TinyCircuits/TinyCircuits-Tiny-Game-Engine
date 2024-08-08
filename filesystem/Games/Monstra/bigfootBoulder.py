import engine_draw as draw
from engine_nodes import (
	Sprite2DNode,
	Circle2DNode,
)
from engine_math import Vector2

from constants import SHOW_HITBOX

HITBOX_PADDING = 0
BOULDER_ACCELERATION = 40 # pixels / sec.^2


class BigfootBoulder(Sprite2DNode):
	def __init__(self, texture, shadowTexture, position, landTimer, onLand):
		super().__init__(self)
		# Note: the BigfootBoulder position is the location where it will land
		# We render the shadow in this position
		self.texture = shadowTexture
		self.frame_count_x = 3
		self.frame_current_x = 0
		self.playing = False
		self.position = position
		boulderOffsetY = texture.height / 2
		self.boulderDistance = landTimer ** 2 * BOULDER_ACCELERATION + \
			boulderOffsetY
		self.landTimer = landTimer
		self.landTimerStart = landTimer
		self.onLand = onLand
		self.transparent_color = draw.white
		# Boulder
		self.boulder = Sprite2DNode(
			position=Vector2(0, -self.boulderDistance),
			texture=texture,
			transparent_color=draw.white,
		)
		self.add_child(self.boulder)
		# Hitbox for collision debugging
		self.hitbox = Circle2DNode(
			color=draw.red,
			radius=texture.width / 2 - HITBOX_PADDING,
		)
		self.boulder.add_child(self.hitbox)
		if not SHOW_HITBOX:
			self.hitbox.scale = 0

	def stop(self):
		self.landTimer = 0

	def tick(self, dt):
		if self.landTimer <= 0:
			return

		# Update timer, boulder position, and shadow frame
		if self.landTimer > 0:
			self.landTimer -= dt
			distance = (self.landTimerStart - self.landTimer) ** 2 * \
				BOULDER_ACCELERATION
			self.boulder.position.y = -self.boulderDistance + distance
			if self.boulderDistance - distance > 100:
				self.frame_current_x = 0
			elif self.boulderDistance - distance > 50:
				self.frame_current_x = 1
			else:
				self.frame_current_x = 2

		# When timer expires, boulder has landed
		if self.landTimer <= 0:
			self.onLand(self)
