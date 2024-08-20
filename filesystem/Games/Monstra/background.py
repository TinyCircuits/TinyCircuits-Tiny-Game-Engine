from engine_nodes import (
	Sprite2DNode,
	EmptyNode,
)
import engine_draw as draw
import engine_io as io
from engine_math import Vector2
from engine_animation import (
	Tween,
	PING_PONG,
	EASE_LINEAR,
)

from constants import (
	MIN_X,
	MAX_X,
)

SHAKE_SPEED = 0.05
SHAKE_MAX_X = 2
BACKGROUND_POS_X = 0
BACKGROUND_POS_Y = 0

FROGMAN_REED_LEFT_POS_X = MIN_X + 10
FROGMAN_REED_RIGHT_POS_X = MAX_X - 8
FROGMAN_REED_POS_Y = 30


class Background(Sprite2DNode):
	def __init__(self):
		super().__init__(self)
		# Adjust for difference in positioning for sprites
		self.position = Vector2(BACKGROUND_POS_X, BACKGROUND_POS_Y)
		self.playing = False
		self.shakeTimer = 0
		self.initialFrameTimer = 0
		self.frameTimer = 0
		self.frameSeq = [0]
		self.frameIndex = 0
		self.rumbleIntensity = 0
		self.tween = Tween()
		self.hide()

	def tick(self, dt):
		if not self.shown():
			return

		# Handle shake timer
		if self.shakeTimer > 0:
			self.shakeTimer -= dt
			io.rumble(self.rumbleIntensity)
		elif self.position.x != BACKGROUND_POS_X:
			self.stop()

		# Handle frame timer
		if len(self.frameSeq) > 1:
			if self.frameTimer > 0:
				self.frameTimer -= dt
			else:
				self.frameTimer = self.initialFrameTimer
				# Update current frame
				if self.frameIndex < len(self.frameSeq) - 1:
					self.frameIndex += 1
				else:
					self.frameIndex = 0
				self.frame_current_x = self.frameSeq[self.frameIndex]

	def shake(self, duration, rumbleIntensity):
		print("Background: Shake screen for", duration, "seconds")
		self.shakeTimer = duration
		self.rumbleIntensity = rumbleIntensity
		self.tween.start(
			self, # object
			"position", # attribute
			Vector2(BACKGROUND_POS_X - SHAKE_MAX_X, BACKGROUND_POS_Y), # start
			Vector2(BACKGROUND_POS_X + SHAKE_MAX_X, BACKGROUND_POS_Y), # end
			duration * 1000, # duration
			10, # speed
			PING_PONG, # loop_type
			EASE_LINEAR, # ease_type
		)

	def stop(self):
		# Restore position and stop vibration
		io.rumble(0)
		self.tween.stop()
		self.position.x = BACKGROUND_POS_X

	def show(self, texture, frameCountX=1, fps=0, frameSeq=[0]):
		self.texture = texture
		self.frame_count_x = frameCountX
		self.initialFrameTimer = 1 / fps if fps > 0 else 0
		self.frameTimer = self.initialFrameTimer
		self.frameSeq = frameSeq
		self.frameIndex = 0
		self.frame_current_x = self.frameSeq[self.frameIndex]
		self.scale = Vector2(1, 1)

	def hide(self):
		self.scale = Vector2(0, 0)
		self.stop()

	def shown(self):
		return self.scale.x > 0


class FrogmanReeds(EmptyNode):
	def __init__(self, leftTexture, rightTexture):
		super().__init__(self)
		self.reedLeft = Sprite2DNode(
			position=Vector2(
				FROGMAN_REED_LEFT_POS_X,
				FROGMAN_REED_POS_Y,
			),
			texture=leftTexture,
			transparent_color=draw.white,
		)
		self.reedRight = Sprite2DNode(
			position=Vector2(
				FROGMAN_REED_RIGHT_POS_X,
				FROGMAN_REED_POS_Y,
			),
			texture=rightTexture,
			transparent_color=draw.white,
		)
		self.hide()

	def show(self):
		self.reedLeft.scale = Vector2(1, 1)
		self.reedRight.scale = Vector2(1, 1)

	def hide(self):
		self.reedLeft.scale = Vector2(0, 0)
		self.reedRight.scale = Vector2(0, 0)

	def shown(self):
		return (
			self.reedLeft.scale.x > 0 or
			self.reedRight.scale.x
		)
