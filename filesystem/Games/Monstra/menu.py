import engine
import engine_draw as draw
import engine_io as io
import engine_audio as audio
from engine_nodes import Sprite2DNode
from engine_math import Vector2

from random import uniform

from constants import (
	SCREEN_WIDTH,
	MIN_X,
	MENU_CHANNEL,
	MENU_FIREWORK_CHANNEL,
	MUSIC_CHANNEL,
)

# Padding around list of icons on x-axis
ICON_WIDTH = 24
ICON_OFFSET = 28 # distance in sprite between left edge and start of icon
ICON_PADDING = 10
ICON_POS_Y = 5
ICON_FRAMES = 5 # frame 0 is ghost; frames 1-4 are enemy "defeated" animation
# Margin above arrow
ARROW_MARGIN = 3
POS_X = 0
POS_Y = 0

CAMPFIRE_POS_X = 0
CAMPFIRE_POS_Y = 41

FIREWORK_COUNT = 3 # number of fireworks per round
FIREWORK_POS_X_MIN = -36
FIREWORK_POS_X_MAX = 42
FIREWORK_POS_Y_MIN = -40
FIREWORK_POS_Y_MAX = -25
FIREWORK_DELAY_MIN = 0.2 # delay between fireworks
FIREWORK_DELAY_MAX = 1.4 # delay between fireworks
FIREWORK_ROUND_DELAY = 1.5 # delay between rounds of fireworks
END_GAME_FIREWORK_ROUNDS = 3


class Menu(Sprite2DNode):
	def __init__(
		self, backgroundTexture, campfireTexture, campfireSound,
		arrowTexture, iconTextures, iconFPS, fireworkTexture, fireworkSound,
		selectSound, onSelect,
	):
		super().__init__(self)
		self.texture = backgroundTexture
		self.frame_count_x = 4
		self.fps = 8
		self.campfire = Sprite2DNode(
			texture=campfireTexture,
			transparent_color=draw.white,
			position=Vector2(CAMPFIRE_POS_X, CAMPFIRE_POS_Y),
			frame_count_x=8,
			fps=8,
			playing=True,
		)
		self.campfireSound = campfireSound
		self.campfireChan = None
		self.position = Vector2(POS_X, POS_Y)
		self.color = draw.lightgrey
		self.selectSound = selectSound
		self.onSelect = onSelect
		self.selected = 0

		# Create fireworks
		self.allDefeated = False
		self.fireworks = []
		for i in range(FIREWORK_COUNT):
			self.fireworks.append(Firework(
				texture=fireworkTexture,
				sound=fireworkSound,
			))
		self.fireworkTimer = 0
		self.fireworkRounds = 0

		# Keep track of enemy "defeated" animations in frames/sec.
		self.iconInitialFrameTimers = list(map(lambda fps: 1 / fps, iconFPS))
		self.iconFrameTimers = self.iconInitialFrameTimers.copy()

		# Create menu icons
		self.icons = []

		# Icons are equally spaced apart
		iconSpacing = (
			SCREEN_WIDTH - ICON_PADDING * 2 - len(iconTextures) * ICON_WIDTH
		) / (len(iconTextures) - 1)
		iconHeightMax = 0 # used to position arrow
		iconOffset = MIN_X + ICON_PADDING - ICON_OFFSET
		for i, texture in enumerate(iconTextures):
			icon = Sprite2DNode(
				texture=texture,
				transparent_color=draw.white,
				frame_count_x=ICON_FRAMES,
				playing=False,
				position=Vector2(
					# Center icon at `iconOffset`
					iconOffset + texture.width / 2 / ICON_FRAMES,
					ICON_POS_Y,
				),
			)
			self.icons.append(icon)
			self.add_child(icon)
			iconHeightMax = max(iconHeightMax, texture.height)
			iconOffset += ICON_WIDTH + iconSpacing

		# Create and position arrow
		self.selectedIndex = 0
		self.arrow = Sprite2DNode(
			texture=arrowTexture,
			transparent_color=draw.white,
			position=Vector2(
				0, # defer computing x value until `moveArrow` is called
				ICON_POS_Y -
				iconHeightMax / 2 - arrowTexture.height / 2 - ARROW_MARGIN
			),
		)
		self.add_child(self.arrow)
		self.moveArrow(0)

		self.hide()

	def tick(self, dt):
		if not self.shown():
			return

		# Check inputs
		if not self.allDefeated:
			if io.LB.is_just_pressed:
				self.moveArrow(-1)
			elif io.RB.is_just_pressed:
				self.moveArrow(1)
			elif (
				io.A.is_just_pressed or
				io.B.is_just_pressed
			):
				print("Menu: Select ", self.selectedIndex)
				self.onSelect(self.selectedIndex)

		# Animate defeated enemies
		for i, icon in enumerate(self.icons):
			if icon.frame_current_x == 0:
				continue # not defeated

			if self.iconFrameTimers[i] > 0:
				self.iconFrameTimers[i] -= dt
			if self.iconFrameTimers[i] <= 0:
				self.iconFrameTimers[i] = self.iconInitialFrameTimers[i]
				# Note: `icon.frame_current_x == 0` means that enemy was *not*
				# defeated. We do not animate those icons.
				if icon.frame_current_x >= ICON_FRAMES - 1:
					# Go back to first frame of animation
					icon.frame_current_x = 1
				elif icon.frame_current_x > 0:
					icon.frame_current_x += 1

		# Launch fireworks
		if self.allDefeated:
			if self.fireworkTimer > 0:
				self.fireworkTimer -= dt
			if self.fireworkTimer <= 0:
				if self.fireworkRounds >= END_GAME_FIREWORK_ROUNDS:
					print("Menu: Ending game engine")
					engine.end()
					return
				delay = 0
				for fw in self.fireworks:
					position = Vector2(
						uniform(FIREWORK_POS_X_MIN, FIREWORK_POS_X_MAX),
						uniform(FIREWORK_POS_Y_MIN, FIREWORK_POS_Y_MAX),
					)
					fw.show(delay, position)
					delay += uniform(FIREWORK_DELAY_MIN, FIREWORK_DELAY_MAX)
				self.fireworkRounds += 1
				self.fireworkTimer = delay + FIREWORK_ROUND_DELAY

	def moveArrow(self, amount):
		currIndex = self.selectedIndex
		self.selectedIndex = max(0, min(
			len(self.icons) - 1,
			self.selectedIndex + amount,
		))
		self.arrow.position.x = self.icons[self.selectedIndex].position.x
		if currIndex != self.selectedIndex:
			audio.play(self.selectSound, MENU_CHANNEL, False)

	def resetProgress(self, index):
		# If this enemy was already defeated, we don't reset their progress
		if self.icons[index].frame_current_x > 0:
			return
		# Reset player progress ;-(
		for icon in self.icons:
			icon.frame_current_x = 0

	def markDefeated(self, index):
		self.icons[index].frame_current_x = 1
		# Move arrow away from current enemy
		if index == len(self.icons) - 1:
			self.moveArrow(-1)
		else:
			self.moveArrow(1)

	def show(self):
		print("Menu: Show")
		self.setScale(1)
		self.campfireChan = audio.play(self.campfireSound, MUSIC_CHANNEL, True)
		self.campfireChan.gain = 0.20
		self.allDefeated = True
		for icon in self.icons:
			if icon.frame_current_x == 0:
				self.allDefeated = False
				break
		self.fireworkTimer = 0
		self.iconFrameTimers = self.iconInitialFrameTimers.copy()
		# Hide arrow if all defeated
		if self.allDefeated:
			self.fireworkRounds = 0
			self.arrow.scale = Vector2(0, 0)

	def hide(self):
		self.setScale(0)
		for fw in self.fireworks:
			fw.hide()
		if self.campfireChan is not None:
			self.campfireChan.stop()

	def setScale(self, val):
		scale = Vector2(val, val)
		self.scale = scale
		for i in self.icons:
			i.scale = scale
		self.arrow.scale = scale
		self.campfire.scale = scale

	def shown(self):
		return self.scale.x > 0


STATE_COUNTDOWN = 0
STATE_EXPLODING = 1
STATE_DONE = 2


class Firework(Sprite2DNode):
	def __init__(self, texture, sound):
		super().__init__(self)
		self.texture = texture
		self.transparent_color = draw.white
		self.sound = sound
		self.frame_count_x = 8
		self.fps = 8
		self.playing = False
		self.state = False
		self.stateTimer = 0
		self.hide()

	def tick(self, dt):
		if self.state == STATE_DONE:
			return

		if self.stateTimer > 0:
			self.stateTimer -= dt
		elif self.state == STATE_COUNTDOWN:
			self.state = STATE_EXPLODING
			self.stateTimer = self.frame_count_x / self.fps
			self.scale = Vector2(1, 1)
			self.frame_current_x = 0
			self.playing = True
			audio.play(self.sound, MENU_FIREWORK_CHANNEL, False)
		elif self.state == STATE_EXPLODING:
			self.hide()

	def show(self, delay, position):
		self.position = position
		self.state = STATE_COUNTDOWN
		self.stateTimer = delay

	def hide(self):
		self.playing = False
		self.state = STATE_DONE
		self.scale = Vector2(0, 0)
