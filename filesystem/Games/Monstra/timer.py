import engine_draw as draw
from engine_nodes import Sprite2DNode
from engine_math import Vector2

POS_X = 55
POS_Y = -56


class Timer(Sprite2DNode):
	def __init__(self, clockTexture, onExpire):
		super().__init__(self)
		# Node properties
		self.texture = clockTexture
		self.transparent_color = draw.white
		self.frame_count_x = 8
		self.playing = False
		self.position = Vector2(POS_X, POS_Y)
		self.onExpire = onExpire
		self.hide()

	def tick(self, dt):
		if not self.shown():
			return

		# Update timer
		self.gameTimer -= dt
		if self.gameTimer < 0:
			self.gameTimer = 0 # clamp game timer since it's displayed

		# Print game timer every second
		if int(self.gameTimer + dt) > int(self.gameTimer):
			print("Timer:", self.gameTimer, "sec.")

		# Update timer frame
		self.frame_current_x = 7 - int(self.gameTimer / self.duration * 8)
		if self.gameTimer <= 0:
			print("Timer: Expired")
			self.onExpire()

	def setScale(self, val):
		self.scale = Vector2(val, val)

	def shown(self):
		return self.gameTimer > 0

	def show(self, duration):
		# Timer properties
		self.duration = duration
		self.gameTimer = duration
		self.setScale(1)

	def hide(self):
		self.gameTimer = 0
		self.setScale(0)
