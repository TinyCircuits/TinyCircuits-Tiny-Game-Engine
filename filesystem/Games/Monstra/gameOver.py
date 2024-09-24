import engine_io as io
from engine_nodes import Sprite2DNode
from engine_math import Vector2

COOLDOWN_RESTART = 1.5 # after game ends, ignore inputs for duration
POS_X = 0
POS_Y = 0


class GameOver(Sprite2DNode):
	def __init__(self, winTexture, lossTexture, onShow, onRestart):
		super().__init__(self)
		self.position = Vector2(POS_X, POS_Y)
		self.winTexture = winTexture
		self.lossTexture = lossTexture
		self.onShow = onShow
		self.onRestart = onRestart
		self.hide()

	def tick(self, dt):
		if not self.shown():
			return

		if self.cooldownRestart > 0:
			self.cooldownRestart -= dt

		# Game is over; attempt restart
		if io.A.is_just_pressed and self.cooldownRestart <= 0:
			self.onRestart()

	def setScale(self, val):
		self.scale = Vector2(val, val)

	def show(self, isWinner):
		self.onShow(isWinner)
		if isWinner:
			print("Game Over: Winner! :-)")
			self.texture = self.winTexture
		else:
			print("Game Over: You Lost :(")
			self.texture = self.lossTexture
		self.cooldownRestart = COOLDOWN_RESTART
		self.setScale(1)

	def hide(self):
		self.setScale(0)

	def shown(self):
		return self.scale.x > 0
