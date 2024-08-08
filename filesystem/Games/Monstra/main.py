import time
start = time.ticks_ms()
print("Main: Hello!")

import engine_main # for new firmware  # noqa: F401
import engine
import engine_draw as draw
import engine_io as io
import engine_audio as audio
from engine_nodes import CameraNode
from engine_resources import (
	TextureResource,
	# FontResource,
	WaveSoundResource,
)

import collision
from constants import (
	MIN_X,
	MAX_X,
	MIN_Y,
	MAX_Y,
	DISABLE_MUSIC,
	MENU_CHANNEL,
	PLAYER_SHOOT_CHANNEL,
	PLAYER_HIT_CHANNEL,
	ENEMY_ATTACK_CHANNEL,
	ENEMY_HIT_CHANNEL,
	MUSIC_CHANNEL,
)
from loadScreen import LoadScreen
from menu import Menu
from timer import Timer
from gameOver import GameOver
from player import Player
from projectiles import Projectiles
from playerShot import PlayerShot
from bigfoot import Bigfoot
from mothman import Mothman
from frogman import Frogman
from background import (
	Background,
	FrogmanReeds,
)

print("Main: Loaded dependencies; clock:", time.ticks_ms() - start)

# Keep track of which components are being shown
layers = []


def show(component, *args):
	layers.append(component)
	component.show(*args)


def hideAll():
	while len(layers) > 0:
		layers.pop().hide()


def loadTexture(path):
	engine.tick()
	return TextureResource(path)


def loadWaveSound(path):
	engine.tick()
	return WaveSoundResource(path)


logoTexture = TextureResource("bmp/glitchbitLogo.bmp")
introFilesTexture = TextureResource("bmp/introFiles.bmp")

# Make a camera to render the scene
draw.set_background_color(draw.white)
camera = CameraNode()
loadScreen = LoadScreen(
	logoTexture=logoTexture,
	introFilesTexture=introFilesTexture,
)

# Set the FPS limit so that game code doesn't run too fast
engine.fps_limit(60)
show(loadScreen)

# All BMPs need to be saved in RGB565 16-bit (no alpha)
# Backgrounds are required to be 128 x 128; zoom and scaling won't apply when
# `draw.set_background` is called
menuBackgroundTexture = loadTexture("bmp/menuBackground.bmp")
menuCampfireTexture = loadTexture("bmp/menuCampfire.bmp")
menuFireworkTexture = loadTexture("bmp/menuFirework.bmp")
gameOverTexture = loadTexture("bmp/gameOver.bmp")
youWinTexture = loadTexture("bmp/youWin.bmp")

clockTexture = loadTexture("bmp/clock.bmp")
heartTexture = loadTexture("bmp/heart.bmp")
menuArrowTexture = loadTexture("bmp/menuArrow.bmp")

floorBigfootTexture = loadTexture("bmp/floorBigfoot.bmp")
bigfootMenuTexture = loadTexture("bmp/bigfootMenu.bmp")
bigfootBlockTexture = loadTexture("bmp/bigfootBlock.bmp")
bigfootHitTexture = loadTexture("bmp/bigfootHit.bmp")
bigfootIdleTexture = loadTexture("bmp/bigfootIdle.bmp")
bigfootMoveTexture = loadTexture("bmp/bigfootMove.bmp")
bigfootSlamTexture = loadTexture("bmp/bigfootSlam.bmp")
bigfootThrowTexture = loadTexture("bmp/bigfootThrow.bmp")
bigfootBoulderTexture = loadTexture("bmp/bigfootBoulder.bmp")
bigfootBoulderShadowTexture = loadTexture("bmp/bigfootBoulderShadow.bmp")
bigfootTreeLargeTexture = loadTexture("bmp/bigfootTreeLarge.bmp")
bigfootTreeSmallTexture = loadTexture("bmp/bigfootTreeSmall.bmp")

floorMothmanTexture = loadTexture("bmp/floorMothman.bmp")
FLOOR_MOTHMAN_FRAME_COUNT = 3
FLOOR_MOTHMAN_FPS = 10
FLOOR_MOTHMAN_FRAME_SEQ = [0, 1, 0, 1, 1, 2, 1, 2]
mothmanMenuTexture = loadTexture("bmp/mothmanMenu.bmp")
mothmanAttackLaserTexture = loadTexture("bmp/mothmanAttackLaser.bmp")
mothmanAttackTornadoTexture = loadTexture("bmp/mothmanAttackTornado.bmp")
mothmanMoveTexture = loadTexture("bmp/mothmanFlying.bmp")
mothmanHitTexture = loadTexture("bmp/mothmanHit.bmp")
mothmanLaserTexture = loadTexture("bmp/mothmanLaser.bmp")
mothmanTornadoTexture = loadTexture("bmp/mothmanTornado.bmp")

floorFrogmanTexture = loadTexture("bmp/floorFrogman.bmp")
frogmanReedLeftTexture = loadTexture("bmp/frogmanReedLeft.bmp")
frogmanReedRightTexture = loadTexture("bmp/frogmanReedRight.bmp")
frogmanTexture = loadTexture("bmp/frogman.bmp")
frogmanMenuTexture = loadTexture("bmp/frogmanMenu.bmp")
frogmanLilypadTexture = loadTexture("bmp/frogmanLilypad.bmp")
frogmanBubbleLargeTexture = loadTexture("bmp/frogmanBubbleLarge.bmp")
frogmanBubbleSmallTexture = loadTexture("bmp/frogmanBubbleSmall.bmp")
frogmanOrbTexture = loadTexture("bmp/frogmanOrb.bmp")

playerTexture = loadTexture("bmp/player.bmp")
playerShotTexture = loadTexture("bmp/playerShot.bmp")
playerWTFTexture = loadTexture("bmp/playerWTF.bmp")

# Font holds information about a font that can be used in Text2DNode to
# display text. The file needs to be a 16-bit RGB565 .bmp file consisting of
# characters all of the same height. Widths of the characters are marked by
# any alternating colors in the bottom row of pixels of the bitmap.
# Characters should be in one large row.
# font = FontResource("font.bmp")

print("Main: Loaded BMPs; clock:", time.ticks_ms() - start)

# All .wav files can be 8 or 16-bit PCM but only at 11025Hz
menuSelectSound = loadWaveSound("audio/select.wav")
menuCampfireSound = loadWaveSound("audio/menuCampfire.wav")
menuFireworkSound = loadWaveSound("audio/menuFirework.wav")
playerShootSound = loadWaveSound("audio/playerShoot.wav")
playerHitSound = loadWaveSound("audio/playerHit.wav")

bigfootScreamSound = loadWaveSound("audio/bigfootScream.wav")
bigfootBoulderThrowSound = loadWaveSound("audio/bigfootBoulderThrow.wav")
bigfootBoulderLandSound = loadWaveSound("audio/bigfootBoulderLand.wav")

mothmanScreamSound = loadWaveSound("audio/mothmanScream.wav")
mothmanLaserShootSound = loadWaveSound("audio/mothmanLaserShoot.wav")
mothmanLaserHitSound = loadWaveSound("audio/mothmanLaserHit.wav")
mothmanLungeSound = loadWaveSound("audio/mothmanLunge.wav")
mothmanLungeHitSound = loadWaveSound("audio/mothmanLungeHit.wav")
mothmanTornadoShootSound = loadWaveSound("audio/mothmanTornadoShoot.wav")
mothmanTornadoHitSound = loadWaveSound("audio/mothmanTornadoHit.wav")

frogmanScreamSound = loadWaveSound("audio/frogmanScream.wav")
frogmanBubbleHitSound = loadWaveSound("audio/frogmanBubbleHit.wav")
frogmanBubbleShootSound = loadWaveSound("audio/frogmanBubbleShoot.wav")
frogmanWandHitSound = loadWaveSound("audio/frogmanWandHit.wav")
frogmanWandShootSound = loadWaveSound("audio/frogmanWandShoot.wav")
if not DISABLE_MUSIC:
	music = loadWaveSound("audio/battleMusic.wav")

print("Main: Loaded WAVs; clock:", time.ticks_ms() - start)

audio.set_volume(1)

# Durations and cooldowns (in seconds)
GAME_DURATION = 120
currentGameIndex = None


# Create boss selection menu
def onMenuSelect(index):
	global currentGameIndex
	currentGameIndex = index
	if index == 0:
		audio.play(bigfootScreamSound, MENU_CHANNEL, False)
		playBigfoot()
		onGameStart()
	elif index == 1:
		audio.play(mothmanScreamSound, MENU_CHANNEL, False)
		playMothman()
		onGameStart()
	elif index == 2:
		audio.play(frogmanScreamSound, MENU_CHANNEL, False)
		playFrogman()
		onGameStart()
	else:
		print("Main: Invalid game index:", index)


def shakeScreen(duration, rumbleIntensity):
	background.shake(duration, rumbleIntensity)


# Called when game timer expires
def onTimerExpire():
	hideAll()
	show(gameOver, False)


# Called when user exits game over screen to return to main menu
def onRestart():
	hideAll()
	show(menu)


# Called when game starts
def onGameStart():
	if not DISABLE_MUSIC:
		musicChan = audio.play(music, MUSIC_CHANNEL, True)
		musicChan.gain = 0.15


# Called when game over is shown
def onGameOver(isWinner):
	io.rumble(0)
	audio.stop(MUSIC_CHANNEL)
	if isWinner:
		menu.markDefeated(currentGameIndex)
	else:
		menu.resetProgress(currentGameIndex)


# Helper function to return player collision rectangle
def getPlayerCollision():
	return collision.Rectangle(
		player.position.x + player.hitbox.position.x,
		player.position.y + player.hitbox.position.y,
		player.hitbox.width,
		player.hitbox.height,
	)


def playBigfoot():
	print("Main: Playing bigfoot...")
	hideAll()

	def playerShotCollision(shot):
		if (
			shot.position.y > MAX_Y or
			shot.position.y < MIN_Y
		):
			# Fell off the screen
			projectiles.remove(shot)
			return

		shotRect = collision.Rectangle(
			shot.position.x + shot.hitbox.position.x,
			shot.position.y + shot.hitbox.position.y,
			shot.hitbox.width,
			shot.hitbox.height
		)
		if collision.rectRect(
			shotRect,
			collision.Rectangle(
				bigfoot.position.x + bigfoot.hitbox.position.x,
				bigfoot.position.y + bigfoot.hitbox.position.y,
				bigfoot.hitbox.width,
				bigfoot.hitbox.height,
			),
		):
			print("Main: Player hit Bigfoot!")
			projectiles.remove(shot)
			successful = bigfoot.hit()
			if successful:
				audio.play(playerHitSound, PLAYER_HIT_CHANNEL, False)

			# Test win condition
			if bigfoot.dead():
				hideAll()
				show(gameOver, True)

	def onPlayerAttack(position):
		audio.play(playerShootSound, PLAYER_SHOOT_CHANNEL, False)
		# Create new projectile
		proj = PlayerShot(
			texture=playerShotTexture,
			position=position,
			collisionCheck=playerShotCollision,
		)
		projectiles.add(proj)
	show(background, floorBigfootTexture)
	show(player, onPlayerAttack)
	show(timer, GAME_DURATION)
	show(bigfoot)
	show(projectiles)


def playMothman():
	print("Main: Playing mothman...")
	hideAll()

	# Function to detect player shot collisions
	def playerShotCollision(shot):
		if (
			shot.position.y > MAX_Y or
			shot.position.y < MIN_Y
		):
			# Fell off the screen
			projectiles.remove(shot)
			return

		shotRect = collision.Rectangle(
			shot.position.x + shot.hitbox.position.x,
			shot.position.y + shot.hitbox.position.y,
			shot.hitbox.width,
			shot.hitbox.height
		)
		if collision.rectRect(
			shotRect,
			collision.Rectangle(
				mothman.position.x + mothman.hitbox.position.x,
				mothman.position.y + mothman.hitbox.position.y,
				mothman.hitbox.width,
				mothman.hitbox.height,
			),
		):
			print("Main: Player hit Mothman!")
			audio.play(playerHitSound, PLAYER_HIT_CHANNEL, False)
			projectiles.remove(shot)
			mothman.hit()

			# Test win condition
			if mothman.dead():
				hideAll()
				show(gameOver, True)

	def onPlayerAttack(position):
		audio.play(playerShootSound, PLAYER_SHOOT_CHANNEL, False)
		# Create new projectile
		proj = PlayerShot(
			texture=playerShotTexture,
			position=position,
			collisionCheck=playerShotCollision,
		)
		projectiles.add(proj)
	show(
		background, floorMothmanTexture,
		FLOOR_MOTHMAN_FRAME_COUNT, FLOOR_MOTHMAN_FPS, FLOOR_MOTHMAN_FRAME_SEQ,
	)
	show(player, onPlayerAttack)
	show(timer, GAME_DURATION)
	show(mothman)
	show(projectiles)


def playFrogman():
	print("Main: Playing frogman...")
	hideAll()

	# Function to detect player shot collisions
	def playerShotCollision(shot):
		if (
			shot.position.y > MAX_Y or
			shot.position.y < MIN_Y
		):
			# Fell off the screen
			projectiles.remove(shot)
			return

		shotRect = collision.Rectangle(
			shot.position.x + shot.hitbox.position.x,
			shot.position.y + shot.hitbox.position.y,
			shot.hitbox.width,
			shot.hitbox.height
		)
		if collision.rectRect(
			shotRect,
			collision.Rectangle(
				frogman.position.x + frogman.hitbox.position.x,
				frogman.position.y + frogman.hitbox.position.y,
				frogman.hitbox.width,
				frogman.hitbox.height,
			),
		):
			print("Main: Player hit Frogman!")
			audio.play(playerHitSound, PLAYER_HIT_CHANNEL, False)
			projectiles.remove(shot)
			frogman.hit()

			# Test win condition
			if frogman.dead():
				hideAll()
				show(gameOver, True)
		elif frogman.shields is not None:
			for i, sprite in enumerate(frogman.shields.sprites):
				hitbox = frogman.shields.hitboxes[i]
				if collision.rectRect(
					shotRect,
					collision.Rectangle(
						frogman.position.x + sprite.position.x + hitbox.position.x,
						frogman.position.y + sprite.position.y + hitbox.position.y,
						hitbox.width,
						hitbox.height,
					)
				):
					print("Main: Frogman shield hit!")
					projectiles.remove(shot)
					break

	def onPlayerAttack(position):
		audio.play(playerShootSound, PLAYER_SHOOT_CHANNEL, False)
		# Create new projectile
		proj = PlayerShot(
			texture=playerShotTexture,
			position=position,
			collisionCheck=playerShotCollision,
		)
		projectiles.add(proj)

	show(background, floorFrogmanTexture)
	show(player, onPlayerAttack)
	show(timer, GAME_DURATION)
	show(frogman)
	show(frogmanReeds)
	show(projectiles)


# Create various game components
menu = Menu(
	backgroundTexture=menuBackgroundTexture,
	campfireTexture=menuCampfireTexture,
	campfireSound=menuCampfireSound,
	arrowTexture=menuArrowTexture,
	iconTextures=[
		bigfootMenuTexture,
		mothmanMenuTexture,
		frogmanMenuTexture,
	],
	iconFPS=[
		0.6,
		5,
		2,
	],
	fireworkTexture=menuFireworkTexture,
	fireworkSound=menuFireworkSound,
	selectSound=menuSelectSound,
	onSelect=onMenuSelect,
)
background = Background()
timer = Timer(
	clockTexture=clockTexture,
	onExpire=onTimerExpire,
)
gameOver = GameOver(
	winTexture=youWinTexture,
	lossTexture=gameOverTexture,
	onShow=onGameOver,
	onRestart=onRestart,
)
player = Player(
	texture=playerTexture,
	wtfTexture=playerWTFTexture,
)
projectiles = Projectiles()


# Bigfoot
def onBoulderThrow(boulder):
	audio.play(bigfootBoulderThrowSound, ENEMY_ATTACK_CHANNEL, False)
	projectiles.add(boulder)


def onBoulderLand(boulder):
	projectiles.remove(boulder)
	audio.play(bigfootBoulderLandSound, ENEMY_HIT_CHANNEL, False)

	if collision.circleRect(
		collision.Circle(
			boulder.position.x + boulder.hitbox.position.x,
			boulder.position.y + boulder.hitbox.position.y,
			boulder.hitbox.radius,
		),
		getPlayerCollision(),
	):
		print("Main: Player hit by Bigfoot boulder!")
		hideAll()
		show(gameOver, False)


def onTreeDrop(tree):
	projectiles.add(tree)


def onTreeLand(tree):
	audio.play(bigfootBoulderLandSound, ENEMY_HIT_CHANNEL, False)

	if collision.rectRect(
		collision.Rectangle(
			tree.position.x + tree.hitbox.position.x,
			tree.position.y + tree.hitbox.position.y,
			tree.hitbox.width,
			tree.hitbox.height,
		),
		getPlayerCollision(),
	):
		print("Main: Player hit by Bigfoot tree!")
		hideAll()
		show(gameOver, False)


def onTreeCleanup(tree):
	if bigfoot.shown():
		projectiles.remove(tree)


bigfoot = Bigfoot(
	blockTexture=bigfootBlockTexture,
	hitTexture=bigfootHitTexture,
	idleTexture=bigfootIdleTexture,
	moveTexture=bigfootMoveTexture,
	slamTexture=bigfootSlamTexture,
	throwTexture=bigfootThrowTexture,
	heartTexture=heartTexture,
	boulderTexture=bigfootBoulderTexture,
	boulderShadowTexture=bigfootBoulderShadowTexture,
	treeLargeTexture=bigfootTreeLargeTexture,
	treeSmallTexture=bigfootTreeSmallTexture,
	onBoulderThrow=onBoulderThrow,
	onBoulderLand=onBoulderLand,
	getPlayerCollision=getPlayerCollision,
	shakeScreen=shakeScreen,
	onTreeDrop=onTreeDrop,
	onTreeLand=onTreeLand,
	onTreeCleanup=onTreeCleanup,
)


# Mothman
def onLaserShot(projectile):
	audio.play(mothmanLaserShootSound, ENEMY_ATTACK_CHANNEL, False)
	projectiles.add(projectile)


def onTornado(projectile):
	audio.play(mothmanTornadoShootSound, ENEMY_ATTACK_CHANNEL, False)
	projectiles.add(projectile)


def onLunge():
	audio.play(mothmanLungeSound, ENEMY_ATTACK_CHANNEL, False)


def laserCollision(shot):
	if collision.rectRect(
		collision.Rectangle(
			shot.position.x + shot.hitbox.position.x,
			shot.position.y + shot.hitbox.position.y,
			shot.hitbox.width,
			shot.hitbox.height,
		),
		getPlayerCollision(),
	):
		projectiles.remove(shot)
		print("Main: Player hit by Mothman laser!")
		audio.play(mothmanLaserHitSound, ENEMY_HIT_CHANNEL, False)
		hideAll()
		show(gameOver, False)


def tornadoCollision(torn, stunDuration):
	if (
		torn.position.x - torn.hitbox.width / 2 > MAX_X or
		torn.position.x + torn.hitbox.width / 2 < MIN_X or
		torn.position.y > MAX_Y or
		torn.position.y < MIN_Y
	):
		projectiles.remove(torn)
		return

	if collision.rectRect(
		collision.Rectangle(
			torn.position.x + torn.hitbox.position.x,
			torn.position.y + torn.hitbox.position.y,
			torn.hitbox.width,
			torn.hitbox.height,
		),
		getPlayerCollision(),
	):
		projectiles.remove(torn)
		print("Main: Player hit by Mothman tornado!")
		audio.play(mothmanTornadoHitSound, ENEMY_HIT_CHANNEL, False)
		player.stun(stunDuration)


def lungeCollision():
	if collision.rectRect(
		collision.Rectangle(
			mothman.position.x + mothman.hitbox.position.x,
			mothman.position.y + mothman.hitbox.position.y,
			mothman.hitbox.width,
			mothman.hitbox.height,
		),
		getPlayerCollision(),
	):
		print("Main: Player hit by Mothman lunge!")
		audio.play(mothmanLungeHitSound, ENEMY_HIT_CHANNEL, False)
		hideAll()
		show(gameOver, False)


mothman = Mothman(
	attackLaserTexture=mothmanAttackLaserTexture,
	attackTornadoTexture=mothmanAttackTornadoTexture,
	moveTexture=mothmanMoveTexture,
	hitTexture=mothmanHitTexture,
	heartTexture=heartTexture,
	laserTexture=mothmanLaserTexture,
	tornadoTexture=mothmanTornadoTexture,
	onLaserShot=onLaserShot,
	laserCollision=laserCollision,
	onTornado=onTornado,
	tornadoCollision=tornadoCollision,
	onLunge=onLunge,
	lungeCollision=lungeCollision,
	getPlayerCollision=getPlayerCollision,
)


# Frogman
def onWandShot(projectile):
	audio.play(frogmanWandShootSound, ENEMY_ATTACK_CHANNEL, False)
	projectiles.add(projectile)


def onBubbleShot(projectile):
	audio.play(frogmanBubbleShootSound, ENEMY_ATTACK_CHANNEL, False)
	frogman.activeBubbles.append(projectile)
	projectiles.add(projectile)


def wandShotCollision(shot):
	if (
		shot.position.x > MAX_X or
		shot.position.x < MIN_X or
		shot.position.y > MAX_Y or
		shot.position.y < MIN_Y
	):
		projectiles.remove(shot)
		return

	if collision.circleRect(
		collision.Circle(
			shot.position.x + shot.hitbox.position.x,
			shot.position.y + shot.hitbox.position.y,
			shot.hitbox.radius,
		),
		getPlayerCollision(),
	):
		projectiles.remove(shot)
		print("Main: Player hit by Frogman wand shot!")
		audio.play(frogmanWandHitSound, ENEMY_HIT_CHANNEL, False)
		hideAll()
		show(gameOver, False)


def bubbleShotCollision(shot):
	SCREEN_PADDING = 5
	if (
		shot.position.x + shot.hitbox.radius > MAX_X + SCREEN_PADDING or
		shot.position.x - shot.hitbox.radius < MIN_X - SCREEN_PADDING or
		shot.position.y + shot.hitbox.radius > MAX_Y + SCREEN_PADDING or
		shot.position.y - shot.hitbox.radius < MIN_Y - SCREEN_PADDING
	):
		audio.play(frogmanBubbleHitSound, ENEMY_HIT_CHANNEL, False)
		if shot.bounces >= 4:
			frogman.activeBubbles.remove(shot)
			projectiles.remove(shot)
			return
		else:
			dx = shot.velocityX
			dy = shot.velocityY
			if(
				shot.position.x + shot.hitbox.radius > MAX_X or
				shot.position.x - shot.hitbox.radius < MIN_X
			):
				dx *= -1
			if (
				shot.position.y + shot.hitbox.radius > MAX_X or
				shot.position.y - shot.hitbox.radius < MIN_X
			):
				dy *= -1
			shot.bounce(dx, dy)
	if collision.circleRect(
		collision.Circle(
			shot.position.x + shot.hitbox.position.x,
			shot.position.y + shot.hitbox.position.y,
			shot.hitbox.radius,
		),
		getPlayerCollision(),
	):
		print("Main: Player hit by Frogman bubble shot!")
		audio.play(frogmanBubbleHitSound, ENEMY_HIT_CHANNEL, False)
		frogman.activeBubbles.remove(shot)
		projectiles.remove(shot)
		hideAll()
		show(gameOver, False)


frogman = Frogman(
	texture=frogmanTexture,
	heartTexture=heartTexture,
	orbTexture=frogmanOrbTexture,
	bubbleLargeTexture=frogmanBubbleLargeTexture,
	bubbleSmallTexture=frogmanBubbleSmallTexture,
	lilypadTexture=frogmanLilypadTexture,
	onWandShot=onWandShot,
	wandShotCollision=wandShotCollision,
	onBubbleShot=onBubbleShot,
	bubbleShotCollision=bubbleShotCollision,
)
frogmanReeds = FrogmanReeds(
	leftTexture=frogmanReedLeftTexture,
	rightTexture=frogmanReedRightTexture,
)

# Show main menu
hideAll()
loadScreen = None
show(menu)
audio.play(menuSelectSound, MENU_CHANNEL, False)

# Start rendering the scene
print("Main: Starting game engine; clock:", time.ticks_ms() - start)
engine.start()
print("Main: Game complete.")
