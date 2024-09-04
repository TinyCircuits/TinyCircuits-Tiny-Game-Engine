import engine_draw as draw
from engine_nodes import (
	Sprite2DNode,
	EmptyNode,
)
from math import ceil
from engine_math import Vector2

from constants import (
	MIN_X,
	MIN_Y,
)

POS_X = MIN_X + 3
POS_Y = MIN_Y + 3
SPACE_BETWEEN = 1 # pixels between each heart
FRAME_FULL = 0
FRAME_HALF = 1
FRAME_EMPTY = 2


class Hearts(EmptyNode):
	def __init__(self, texture, maxHP: int):
		super().__init__(self)
		self.position = Vector2(
			POS_X + texture.width / 3 / 2,
			POS_Y + texture.height / 2,
		)
		self.maxHP = maxHP
		self.hp = maxHP
		self.hearts = []
		for i in range(ceil(maxHP / 2)):
			heart = Sprite2DNode(
				texture=texture,
				transparent_color=draw.white,
				frame_count_x=3,
				playing=False,
				position=Vector2(i * texture.width / 3 + SPACE_BETWEEN, 0),
			)
			self.add_child(heart)
			self.hearts.append(heart)
		self.setHP(maxHP)

	def setHP(self, hp: int):
		# clamp and set HP
		if hp < 0:
			hp = 0
		elif hp > self.maxHP:
			hp = self.maxHP
		self.hp = hp

		# update heart sprites
		for i, heart in enumerate(self.hearts):
			if hp >= 2:
				heart.frame_current_x = FRAME_FULL
				hp -= 2
			elif hp >= 1:
				heart.frame_current_x = FRAME_HALF
				hp -= 1
			else:
				heart.frame_current_x = FRAME_EMPTY

		return self.hp

	def show(self):
		for i, heart in enumerate(self.hearts):
			heart.scale = Vector2(1, 1)

	def hide(self):
		for i, heart in enumerate(self.hearts):
			heart.scale = Vector2(0, 0)

	def shown(self):
		if len(self.hearts) == 0:
			return False
		return self.hearts[0].scale.x > 0
