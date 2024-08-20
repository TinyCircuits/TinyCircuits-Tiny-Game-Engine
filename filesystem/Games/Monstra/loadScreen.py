from engine_nodes import (
	Sprite2DNode,
	Rectangle2DNode,
)
from engine_math import Vector2
import engine_draw as draw

from constants import (
	SCREEN_WIDTH,
	SCREEN_HEIGHT,
	DISABLE_MUSIC,
)

POS_X = 0
POS_Y = 0
LOGO_FRAMES = 8
LOGO_DURATION = 4 if DISABLE_MUSIC else 5
INTRO_FRAMES = 4
# duration of first 3 intro frames (last frame shown until load complete)
INTRO_DURATION = 13 if DISABLE_MUSIC else 15
INTRO_SUBFRAMES_PER_FRAME = 3


class LoadScreen(Sprite2DNode):
	def __init__(self, logoTexture, introFilesTexture):
		super().__init__(self)
		self.timer = 0
		self.logoTexture = logoTexture
		self.introFilesTexture = introFilesTexture
		self.playing = False
		self.position = Vector2(POS_X, POS_Y)
		color = draw.Color(0xe6/0xff, 0xe6/0xff, 0xe6/0xff)

		self.rect = Rectangle2DNode(
			width=SCREEN_WIDTH,
			height=SCREEN_HEIGHT,
			color=color,
		)

		# TODO: Create rectangles for intro files
		color = draw.Color(0xc5/0xff, 0xce/0xff, 0xa5/0xff)

		self.fileTitleRect = Rectangle2DNode(
			position=Vector2(-46 + 56/2, -56 + 10/2),
			width=56,
			height=10,
			color=color,
			scale=Vector2(0, 0),
		)
		self.fileTitleRect.layer = 2
		self.fileLocationRect = Rectangle2DNode(
			position=Vector2(-38 + 100/2, 32 + 30/2),
			width=100,
			height=30,
			color=color,
			scale=Vector2(0, 0),
		)
		self.fileLocationRect.layer = 2

		self.add_child(self.rect)
		self.add_child(self.fileTitleRect)
		self.add_child(self.fileLocationRect)
		self.layer = 1
		self.hide()

	def tick(self, dt):
		self.timer += dt
		if self.timer < LOGO_DURATION:
			# Show logo
			self.texture = self.logoTexture
			self.frame_count_x = LOGO_FRAMES
			self.frame_current_x = int(self.timer / LOGO_DURATION * LOGO_FRAMES)
		else:
			# Show intro files
			introSubframe = int(
				(self.timer - LOGO_DURATION) / INTRO_DURATION *
				(INTRO_FRAMES - 1) * INTRO_SUBFRAMES_PER_FRAME
			)
			self.texture = self.introFilesTexture
			self.frame_count_x = INTRO_FRAMES
			if introSubframe >= (INTRO_FRAMES - 1) * INTRO_SUBFRAMES_PER_FRAME:
				self.frame_current_x = 3
			else:
				# TODO: Show subframes
				self.frame_current_x = int(introSubframe / 3)
				subframeIndex = introSubframe % INTRO_SUBFRAMES_PER_FRAME
				show = Vector2(1, 1)
				hide = Vector2(0, 0)
				if subframeIndex >= 2:
					self.fileTitleRect.scale = hide
					self.fileLocationRect.scale = hide
				elif subframeIndex >= 1:
					self.fileTitleRect.scale = hide
					self.fileLocationRect.scale = show
				else:
					self.fileTitleRect.scale = show
					self.fileLocationRect.scale = show

	def show(self):
		print("Load Screen: Show")
		scale = Vector2(1, 1)
		self.scale = scale
		self.rect.scale = scale

	def hide(self):
		scale = Vector2(0, 0)
		self.scale = scale
		self.rect.scale = scale
		self.fileTitleRect.scale = scale
		self.fileLocationRect.scale = scale

	def shown(self):
		return self.scale.x > 0
