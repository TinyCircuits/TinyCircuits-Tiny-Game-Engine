import engine_draw as draw
from engine_nodes import (
	Sprite2DNode,
	Rectangle2DNode,
)
from engine_math import Vector2
# from math import pi

from constants import SHOW_HITBOX

HITBOX_PADDING = 0
LAND_FRAME = 4 # 0 indexed


class BigfootTree(Sprite2DNode):
	def __init__(
		self, texture, scaleX, position,
		landTimer, cleanupTimer,
		onLand, onCleanup,
	):
		super().__init__(self)
		self.texture = texture
		self.position = position
		self.scale.x = scaleX
		self.landTimer = landTimer
		self.landTimerStart = landTimer
		self.cleanupTimer = cleanupTimer
		self.onLand = onLand
		self.onCleanup = onCleanup
		self.transparent_color = draw.white
		self.frame_count_x = 7
		self.playing = False
		# Hitbox for collision debugging
		self.hitbox = Rectangle2DNode(
			color=draw.red,
			width=self.texture.width / self.frame_count_x - HITBOX_PADDING * 2,
			height=self.texture.height - HITBOX_PADDING * 2,
		)
		if not SHOW_HITBOX:
			self.hitbox.scale = Vector2(0, 0)
		self.add_child(self.hitbox)

	def stop(self):
		self.landTimer = 0

	def tick(self, dt):
		if self.landTimer <= 0:
			return

		# Update timer
		if self.landTimer > 0:
			self.landTimer -= dt
		# Update frame
		self.frame_current_x = min(
			6,
			int(self.frame_count_x * (1 - self.landTimer / self.landTimerStart))
		)
		if self.frame_current_x == LAND_FRAME:
			self.onLand(self)
		if self.landTimer <= 0:
			self.onCleanup(self)
