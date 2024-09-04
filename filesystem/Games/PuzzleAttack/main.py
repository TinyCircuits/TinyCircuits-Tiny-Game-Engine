import engine_main

import engine
from engine_draw import Color
import engine_draw
from engine_nodes import Rectangle2DNode, CameraNode, EmptyNode, Sprite2DNode, Text2DNode
from engine_animation import Tween, ONE_SHOT, EASE_ELAST_IN_OUT
from engine_math import Vector2, Vector3
from engine_resources import TextureResource, FontResource, WaveSoundResource
import engine_io
import engine_audio
import engine_save

import json
import os
import math

CHEAT_MODE = const(True)

engine_save.set_location("save.data")

engine.fps_limit(30)

font = FontResource("outrunner_outline.bmp")

sfxWin = WaveSoundResource("sfx/win.wav")
sfxLose = WaveSoundResource("sfx/lose.wav")
sfxFall = WaveSoundResource("sfx/fall.wav")
sfxSwap = WaveSoundResource("sfx/swap.wav")
sfxNav = WaveSoundResource("sfx/nav.wav")
sfxCursor = WaveSoundResource("sfx/cursor.wav")
sfxPops = [WaveSoundResource("sfx/pop"+str(i+1)+".wav") for i in range(8)]

texLogo = TextureResource("logo.bmp")
texBG = TextureResource("bg.bmp")
texBlocks = TextureResource("blocks.bmp")
texCursor = TextureResource("cursor.bmp")
texCheckmark = TextureResource("checkmark.bmp")
texLock = TextureResource("lock.bmp")
TRANSPARENT_COLOR = const(0xf81f)

MODE_NORMAL = const(0)
MODE_EXTRA = const(1)
MODE_CUSTOM = const(2)

BLOCK_GREEN = const(0)
BLOCK_PURPLE = const(1)
BLOCK_RED = const(2)
BLOCK_YELLOW = const(3)
BLOCK_CYAN = const(4)
BLOCK_BLUE = const(5)

BLOCK_STATE_NORMAL = const(0)
BLOCK_STATE_FALLING = const(1)
BLOCK_STATE_SQUISH = const(2)
BLOCK_STATE_DARK = const(3)
BLOCK_STATE_POP = const(4)
BLOCK_STATE_LIGHT = const(5)
BLOCK_STATE_POP2 = const(6)

SM_INTRO = const(0)
SM_PRESS_START = const(1)
SM_MODE_SELECT = const(2)
SM_STAGE_SELECT = const(3)
SM_LEVEL_LOADING = const(4)
SM_LEVEL_UNLOADING = const(5)
SM_CURSOR_SELECT = const(6)
SM_LEVEL_MENU  = const(7)
SM_SWAP_ANIM = const(8)
SM_FALL_ANIM = const(9)
SM_MATCH_ANIM = const(10)
SM_MOVE_OVER = const(11)
SM_LEVEL_LOST = const(12)
SM_LEVEL_WIN = const(13)

class Block(Sprite2DNode):
    def __init__(self, id):
        super().__init__(self)
        self.texture = texBlocks
        self.transparent_color = Color(TRANSPARENT_COLOR)
        self.frame_count_x = 7
        self.frame_count_y = 6
        self.frame_current_x = BLOCK_STATE_NORMAL
        self.frame_current_y = id
        self.loop = False
        self.playing = False
        self.layer = 1
        self.tween = Tween()
        self.normal = True
        self.falling = False
        self.fallingY = None
    def tweenMove(self, position, duration):
        self.tween.start(self, "position", self.position, position, duration, ONE_SHOT, EASE_ELAST_IN_OUT)
    def updateFalling(self):
        prevFalling = self.falling
        if self.fallingY is not None and self.normal:
            if not self.tween.finished and self.position.y > self.fallingY:
                self.falling = True
            elif self.tween.finished:
                self.falling = False
        else:
            self.falling = False
        self.fallingY = self.position.y
        if self.normal:
            if self.falling:
                self.frame_current_x = BLOCK_STATE_FALLING
            elif prevFalling:
                self.frame_current_x = BLOCK_STATE_SQUISH
            elif self.frame_current_x == BLOCK_STATE_SQUISH:
                self.frame_current_x = BLOCK_STATE_NORMAL

class Cursor(Sprite2DNode):
    def __init__(self):
        super().__init__(self)
        self.texture = texCursor
        self.transparent_color = Color(TRANSPARENT_COLOR)
        self.frame_count_y = 2
        self.loop = True
        self.playing = True
        self.fps = 4
        self.layer = 2

BLOCK_COLS = const(6)
BLOCK_ROWS = const(12)
BLOCKS_START_X = const(10)
BLOCKS_START_Y = const(4)

stageName = ""
stagePrefix = ""
blocks = [None for _ in range(BLOCK_COLS*BLOCK_ROWS)]
moves = 0

cam = CameraNode(Vector3(64,64,0))
sprBG = Sprite2DNode(Vector2(64,64),texBG,Color(TRANSPARENT_COLOR),0,1,1,0,Vector2(2,2))
sprLogo = Sprite2DNode(Vector2(64,-64),texLogo,Color(TRANSPARENT_COLOR),0,1,1,0)

menuBlocks = [Block(id) for id in range(6)]

rectMenuGameMode = Rectangle2DNode(Vector2(64,64),120,80,engine_draw.black,1,False,0,Vector2(1,1),100)
rectMenuGameModeBorder = Rectangle2DNode(Vector2(0,0),118,78,engine_draw.white,1,True,0,Vector2(1,1),100)
txtMenuGameModeOptions = [
    Text2DNode(Vector2(0,-25),font,"NORMAL",0,Vector2(2,2),1,1,1,engine_draw.white,100),
    Text2DNode(Vector2(0,0),font,"EXTRA",0,Vector2(2,2),1,1,1,engine_draw.white,100),
    Text2DNode(Vector2(0,25),font,"CUSTOM",0,Vector2(2,2),1,1,1,engine_draw.white,100),
]
rectMenuGameMode.add_child(rectMenuGameModeBorder)
for txt in txtMenuGameModeOptions:
    rectMenuGameMode.add_child(txt)
    
rectToast = Rectangle2DNode(Vector2(64,64),100,60,engine_draw.lightgrey,1,False,0,Vector2(1,1),110)
txtToast = Text2DNode(Vector2(0,0),font,"",0,Vector2(1,1),1,1,1,engine_draw.black,110)
rectToast.add_child(txtToast)

rectMenuStage = Rectangle2DNode(Vector2(64,72),100,110,engine_draw.black,1,False,0,Vector2(1,1),100)
rectMenuStageBorder = Rectangle2DNode(Vector2(0,0),98,108,engine_draw.white,1,True,0,Vector2(1,1),100)
txtMenuStageTitle = Text2DNode(Vector2(64,9),font,"Stage 1",0,Vector2(1,1),1,1,1,engine_draw.white,100)
txtMenuStageLevels = []
sprMenuStageClears = []
i = 0
for col in range(2):
    for row in range(5):
        txt = Text2DNode(Vector2(col*48-34,row*20-40),font,str(i+1),0,Vector2(1,1),1,1,1,engine_draw.white,100)
        txtMenuStageLevels.append(txt)
        rectMenuStage.add_child(txt)
        spr = Sprite2DNode(Vector2(col*48-14,row*20-40),texCheckmark,Color(TRANSPARENT_COLOR),0,2,1,0,Vector2(1,1),1,False,False,100)
        spr.loop = False
        spr.frame_current_x = 0
        sprMenuStageClears.append(spr)
        rectMenuStage.add_child(spr)
        i += 1
rectMenuStageLock = Rectangle2DNode(Vector2(0,0),96,106,engine_draw.darkgrey,1,False,0,Vector2(1,1),100)
sprMenuStageLock = Sprite2DNode(Vector2(0,0),texLock,Color(TRANSPARENT_COLOR),0,1,1,0,Vector2(1,1),1,False,False,100)
rectMenuStage.add_child(rectMenuStageBorder)
rectMenuStage.add_child(rectMenuStageLock)
rectMenuStage.add_child(sprMenuStageLock)

rectBorder = Rectangle2DNode(Vector2(BLOCKS_START_X-2+BLOCK_COLS*5+2,BLOCKS_START_Y-2+BLOCK_ROWS*5+2), BLOCK_COLS*10+3, BLOCK_ROWS*10+3, engine_draw.white, 1.0, True)
cursor = Cursor()
lblLevelTitle = Text2DNode(Vector2(100,10),font,"LEVEL",0,Vector2(1,1),1,1,1,engine_draw.white)
lblLevel = Text2DNode(Vector2(100,25),font,"---",0,Vector2(1,1),1,1,1,engine_draw.white)
lblMovesTitle = Text2DNode(Vector2(100,80),font,"MOVES",0,Vector2(1,1),1,1,1,engine_draw.white)
lblMoves = Text2DNode(Vector2(100,105),font,"0",0,Vector2(3,3),1,1,1,engine_draw.white)
lblOverlay = Text2DNode(Vector2(BLOCKS_START_X+BLOCK_COLS*5,BLOCKS_START_Y+BLOCK_ROWS*5),font,"",0,Vector2(1,1),1,1,1,engine_draw.white,3)
sprCheckmark = Sprite2DNode(Vector2(100,41),texCheckmark,Color(TRANSPARENT_COLOR),0,2,1,0,Vector2(1,1),1,False,False)

for block in menuBlocks:
    block.opacity = 0

rectMenuGameMode.opacity = 0
rectMenuGameModeBorder.opacity = 0
txtMenuGameModeOptions[0].opacity = 0
txtMenuGameModeOptions[1].opacity = 0
txtMenuGameModeOptions[2].opacity = 0

rectMenuStage.opacity = 0
rectMenuStageBorder.opacity = 0
txtMenuStageTitle.opacity = 0
rectMenuStageLock.opacity = 0
sprMenuStageLock.opacity = 0
for txt in txtMenuStageLevels:
    txt.opacity = 0
for spr in sprMenuStageClears:
    spr.opacity = 0

rectToast.opacity = 0
txtToast.opacity = 0

rectBorder.opacity = 0
cursor.opacity = 0
lblLevelTitle.opacity = 0
lblLevel.opacity = 0
lblMovesTitle.opacity = 0
lblMoves.opacity = 0
sprCheckmark.opacity = 0

def loadLevel(stage, level):
    global stageName, stagePrefix, blocks, moves
    
    with open(stage) as f:
        stageData = json.load(f)
    
    stageName = stageData["name"]
    stagePrefix = stageData["prefix"]
    levelData = stageData["levels"][level]
    
    for i in range(len(blocks)):
        if blocks[i] is not None:
            block = blocks[i]
            block.mark_destroy()
            blocks[i] = None
            
    moves = levelData[len(levelData)-1]
            
    rowStart = BLOCK_ROWS - (len(levelData)-1)
    colStart = BLOCK_COLS//2 - len(levelData[0])//2
    for dr in range(len(levelData)-1):
        for dc in range(len(levelData[0])):
            row = rowStart + dr
            col = colStart + dc
            symbol = levelData[dr][dc]
            if symbol == "G":
                id = BLOCK_GREEN
            elif symbol == "P":
                id = BLOCK_PURPLE
            elif symbol == "R":
                id = BLOCK_RED
            elif symbol == "Y":
                id = BLOCK_YELLOW
            elif symbol == "C":
                id = BLOCK_CYAN
            elif symbol == "B":
                id = BLOCK_BLUE
            elif symbol == " ":
                id = None
            else:
                raise Exception("Unknown symbol! "+symbol)
            if id is not None:
                block = Block(id)
                height = 130 + (BLOCK_ROWS-row)*10
                block.position = Vector2(BLOCKS_START_X+col*10+5,BLOCKS_START_Y+row*10+5-height)
                block.tweenMove(Vector2(BLOCKS_START_X+col*10+5,BLOCKS_START_Y+row*10+5),height//10*50)
                blocks[row*BLOCK_COLS+col] = block
                
    lblLevel.text = stagePrefix+"-"+str(level+1)
    lblMoves.text = str(moves)
    sprBG.texture = TextureResource(stage[0:-5]+".bmp")
    sprCheckmark.frame_current_x = engine_save.load(stage,bytearray(10))[level]

def checkFalling(blocks):
    ret = []
    for col in range(BLOCK_COLS):
        ground = True
        for row in range(BLOCK_ROWS-1,-1,-1):
            block = blocks[row*BLOCK_COLS+col]
            if block is not None:
                if not ground:
                    ret.append((col, row, block))
            else:
                ground = False
    return ret

def startFallAnim(blocksFalling):
    for col, row, block in blocksFalling:
        block.tweenMove(Vector2(block.position.x,block.position.y+10),200)
        i = row*BLOCK_COLS+col
        blocks[i], blocks[i+BLOCK_COLS] = blocks[i+BLOCK_COLS], blocks[i]

def checkMatching(blocks):
    ret = []
    for row in range(BLOCK_ROWS):
        for col in range(BLOCK_COLS):
            block = blocks[row*BLOCK_COLS+col]
            if block is not None:
                id = block.frame_current_y
                matchStart, matchEnd = col, col
                while matchStart > 0:
                    checkBlock = blocks[row*BLOCK_COLS+(matchStart-1)]
                    if checkBlock is not None and checkBlock.frame_current_y == id:
                        matchStart -= 1
                    else:
                        break
                while matchEnd < BLOCK_COLS-1:
                    checkBlock = blocks[row*BLOCK_COLS+(matchEnd+1)]
                    if checkBlock is not None and checkBlock.frame_current_y == id:
                        matchEnd += 1
                    else:
                        break
                if (matchEnd - matchStart) >= 2:
                    ret.append((col, row, block))
                    continue
                matchStart, matchEnd = row, row
                while matchStart > 0:
                    checkBlock = blocks[(matchStart-1)*BLOCK_COLS+col]
                    if checkBlock is not None and checkBlock.frame_current_y == id:
                        matchStart -= 1
                    else:
                        break
                while matchEnd < BLOCK_ROWS-1:
                    checkBlock = blocks[(matchEnd+1)*BLOCK_COLS+col]
                    if checkBlock is not None and checkBlock.frame_current_y == id:
                        matchEnd += 1
                    else:
                        break
                if (matchEnd - matchStart) >= 2:
                    ret.append((col, row, block))
                    continue
    return ret

standardStages = [("stages/standard/stage"+str(i+1)+".json") for i in range(6)]
extraStages = [("stages/extra/extra"+str(i+1)+".json") for i in range(6)]
customStages = [("stages/custom/"+f) for f in os.listdir("stages/custom") if f.endswith(".json")]

extraUnlockChecked = False
extraUnlock = None
def checkExtraUnlock():
    global extraUnlockChecked, extraUnlock
    unlocked = True
    for stage in standardStages:
        clears = engine_save.load(stage,bytearray(10))
        for i in range(10):
            if clears[i] == 0:
                unlocked = False
                break
        if not unlocked:
            break
    extraUnlockChecked = True
    extraUnlock = unlocked
    return unlocked

def loadTitles(stages):
    ret = []
    for stage in stages:
        with open(stage) as f:
            stageData = json.load(f)
            ret.append(stageData["name"])
    return ret

def loadLevelCounts(stages):
    ret = []
    for stage in stages:
        with open(stage) as f:
            stageData = json.load(f)
            ret.append(len(stageData["levels"]))
    return ret

stages = standardStages
levelCounts = None
titles = None
clears = None
locks = None
stage = 0
level = 0

cursorCol, cursorRow = BLOCK_COLS//2-1, BLOCK_ROWS//2
block1, block2 = None, None
blocksFalling = []
blocksMatching = []
popLevel = 0
modeSelect = 0

rumbleFrames = 0
def rumble(frames, intensity):
    global rumbleFrames
    engine_io.rumble(intensity)
    rumbleFrames = frames

def toast(texts):
    width = 0
    for text in texts:
        width = max(width,len(text)*9+10)
    height = len(texts)*9+20
        
    text = texts[0]
    for i in range(1,len(texts)):
        text += "\n"+texts[i]
    
    rectToast.opacity = 1
    rectToast.width = width
    rectToast.height = height
    txtToast.opacity = 1
    txtToast.text = text
    frame = 0
    while True:
        if not engine.tick():
            continue
        frame += 1
        if frame > 10 and engine_io.A.is_just_pressed:
            break
    rectToast.opacity = 0
    txtToast.opacity = 0

state = SM_INTRO
frame = 0
stateLoad = True
stateUnload = False
def setState(next):
    global state, frame, stateUnload
    state = next
    frame = 0
    stateUnload = True

while True:
    if not engine.tick():
        continue
    frame += 1
    stateLoad = (frame == 1)
    stateUnload = False
    
    if rumbleFrames > 0:
        rumbleFrames -= 1
        if rumbleFrames == 0:
            engine_io.rumble(0)
        
    if state == SM_INTRO:
        if stateLoad:
            sprLogo.opacity = 1
        
        if frame <= 16:
            sprLogo.position = Vector2(64,60-(1<<(16-frame)))
        else:
            setState(SM_PRESS_START)
        
    elif state == SM_PRESS_START:
        if stateLoad:
            sprLogo.opacity = 1
            for block in menuBlocks:
                block.opacity = 1
                
        for i in range(len(menuBlocks)):
            r = math.pi * 2 * i/len(menuBlocks) + frame * 0.08
            menuBlocks[i].position = Vector2(64+55*math.cos(r),64+55*math.sin(r))
        if engine_io.A.is_just_pressed:
            engine_audio.play(sfxNav,0,False)
            rumble(3,0.4)
            modeSelect = MODE_NORMAL
            setState(SM_MODE_SELECT)
            
        if stateUnload:
            sprLogo.opacity = 0
            for block in menuBlocks:
                block.opacity = 0
        
    elif state == SM_MODE_SELECT:
        if stateLoad:
            rectMenuGameMode.opacity = 1
            rectMenuGameModeBorder.opacity = 1
            txtMenuGameModeOptions[0].opacity = 1
            txtMenuGameModeOptions[0].color = engine_draw.white
            txtMenuGameModeOptions[1].opacity = 1
            txtMenuGameModeOptions[1].color = engine_draw.darkgrey
            txtMenuGameModeOptions[2].opacity = 1
            txtMenuGameModeOptions[2].color = engine_draw.darkgrey
            checkExtraUnlock()
        
        for i in range(len(menuBlocks)):
            r = math.pi * 2 * i/len(menuBlocks) + frame * 0.08
            menuBlocks[i].position = Vector2(64+55*math.cos(r),64+55*math.sin(r))
        if engine_io.DOWN.is_just_pressed:
            modeSelect = (modeSelect + 1) % 3
        if engine_io.UP.is_just_pressed:
            modeSelect = (modeSelect + 2) % 3
        if engine_io.A.is_just_pressed:
            if modeSelect == MODE_EXTRA and not extraUnlock:
                toast(["Beat","Normal Mode","to unlock","Extra Mode!"])
            elif modeSelect == MODE_CUSTOM and len(customStages) == 0:
                toast(["No Custom","Mode stages","are found!"])
            else:
                engine_audio.play(sfxNav,0,False)
                rumble(3,0.4)
                setState(SM_STAGE_SELECT)
        if engine_io.B.is_just_pressed:
            setState(SM_PRESS_START)
        for i in range(3):
            txtMenuGameModeOptions[i].color = engine_draw.white if (i == modeSelect) else engine_draw.darkgrey
        
        if stateUnload:
            rectMenuGameMode.opacity = 0
            rectMenuGameModeBorder.opacity = 0
            txtMenuGameModeOptions[0].opacity = 0
            txtMenuGameModeOptions[1].opacity = 0
            txtMenuGameModeOptions[2].opacity = 0
        
    elif state == SM_STAGE_SELECT:
        if stateLoad:
            if modeSelect == MODE_NORMAL:
                stages = standardStages
            elif modeSelect == MODE_EXTRA:
                stages = extraStages
            elif modeSelect == MODE_CUSTOM:
                stages = customStages
            levelCounts = loadLevelCounts(stages)              
            clears = [engine_save.load(stages[i],bytearray(levelCounts[i])) for i in range(len(stages))]
            locks = [False]
            for i in range(1,len(clears)):
                clear = clears[i-1]
                locked = locks[i-1]
                if modeSelect != MODE_CUSTOM:
                    for check in clear:
                        if not check:
                            locked = True
                            break
                locks.append(locked)
            if modeSelect == MODE_CUSTOM:
                stage = 0
            else:
                stage = len(stages)-1
                for i in range(1,len(stages)):
                    if locks[i]:
                        stage = i-1
                        break
            titles = loadTitles(stages)
            rectMenuStage.opacity = 1
            rectMenuStageBorder.opacity = 1
            txtMenuStageTitle.opacity = 1
            rectMenuStageLock.opacity = 0.75 if locks[stage] else 0
            sprMenuStageLock.opacity = 1 if locks[stage] else 0
            txtMenuStageTitle.text = titles[stage]
            for i in range(10):
                txt = txtMenuStageLevels[i]
                spr = sprMenuStageClears[i]
                if i < levelCounts[stage]:
                    txt.opacity = 1
                    spr.frame_current_x = clears[stage][i]
                    spr.opacity = 1
                else:
                    txt.opacity = 0
                    spr.opacity = 0
                
        
        updateStage = False
        if engine_io.LEFT.is_just_pressed and stage > 0:
            stage -= 1
            updateStage = True
        if engine_io.RIGHT.is_just_pressed and stage < len(stages)-1:
            stage += 1
            updateStage = True
            
        if engine_io.A.is_just_pressed and not locks[stage]:
            level = 0
            for i in range(len(clears[stage])):
                if not clears[stage][i]:
                    level = i
                    break
            engine_audio.play(sfxNav,0,False)
            rumble(3,0.4)
            setState(SM_LEVEL_LOADING)
            
        if engine_io.B.is_just_pressed:
            setState(SM_MODE_SELECT)
            
        if updateStage:
            updateStage = False
            rectMenuStageLock.opacity = 0.75 if locks[stage] else 0
            sprMenuStageLock.opacity = 1 if locks[stage] else 0
            txtMenuStageTitle.text = titles[stage]
            for i in range(10):
                txt = txtMenuStageLevels[i]
                spr = sprMenuStageClears[i]
                if i < levelCounts[stage]:
                    txt.opacity = 1
                    spr.frame_current_x = clears[stage][i]
                    spr.opacity = 1
                else:
                    txt.opacity = 0
                    spr.opacity = 0
                
        if stateUnload:
            rectMenuStage.opacity = 0
            rectMenuStageBorder.opacity = 0
            txtMenuStageTitle.opacity = 0
            rectMenuStageLock.opacity = 0
            sprMenuStageLock.opacity = 0
            for txt in txtMenuStageLevels:
                txt.opacity = 0
            for spr in sprMenuStageClears:
                spr.opacity = 0
    
    elif state == SM_LEVEL_LOADING:
        if stateLoad:
            loadLevel(stages[stage],level)
            rectBorder.opacity = 1
            lblLevelTitle.opacity = 1
            lblLevel.opacity = 1
            lblMovesTitle.opacity = 1
            lblMoves.opacity = 1
            sprCheckmark.opacity = 1
        
        falling = False
        for block in blocks:
            if block is not None and not block.tween.finished:
                falling = True
                break
        if not falling:
            cursorCol, cursorRow = BLOCK_COLS//2-1, BLOCK_ROWS//2
            cursor.position = Vector2(BLOCKS_START_X+cursorCol*10+10,BLOCKS_START_Y+cursorRow*10+5)
            cursor.opacity = 1
            setState(SM_CURSOR_SELECT)
            
    elif state == SM_LEVEL_UNLOADING:
        setState(SM_STAGE_SELECT)
        if stateUnload:
            for i in range(len(blocks)):
                block = blocks[i]
                if block is not None:
                    block.mark_destroy()
                    blocks[i] = None
            rectBorder.opacity = 0
            lblLevelTitle.opacity = 0
            lblLevel.opacity = 0
            lblMovesTitle.opacity = 0
            lblMoves.opacity = 0
            sprCheckmark.opacity = 0
            cursor.opacity = 0
            lblOverlay.opacity = 0
            sprBG.texture = texBG
    
    elif state == SM_CURSOR_SELECT:
        if engine_io.LEFT.is_just_pressed and cursorCol > 0:
            cursorCol -= 1
        if engine_io.RIGHT.is_just_pressed and cursorCol < BLOCK_COLS-2:
            cursorCol += 1
        if engine_io.UP.is_just_pressed and cursorRow > 0:
            cursorRow -= 1
        if engine_io.DOWN.is_just_pressed and cursorRow < BLOCK_ROWS-1:
            cursorRow += 1
            
        if engine_io.RB.is_just_pressed:
            level = (level+1) % len(clears[stage])
            loadLevel(stages[stage],level)
        if engine_io.LB.is_just_pressed:
            level = (level+len(clears[stage])-1) % len(clears[stage])
            loadLevel(stages[stage],level)
            
        if engine_io.A.is_just_pressed:
            i = cursorRow*BLOCK_COLS+cursorCol
            block1 = blocks[i]
            block2 = blocks[i+1]
            if block1 is not None or block2 is not None:
                blocks[i], blocks[i+1] = blocks[i+1], blocks[i]
                if block1 is not None:
                    block1.tweenMove(Vector2(block1.position.x+10,block1.position.y),200)
                if block2 is not None:
                    block2.tweenMove(Vector2(block2.position.x-10,block2.position.y),200)
                moves -= 1
                lblMoves.text = str(moves)
                popLevel = 0
                setState(SM_SWAP_ANIM)
                
        if engine_io.B.is_just_pressed:
            setState(SM_LEVEL_UNLOADING)

        cursor.position = Vector2(BLOCKS_START_X+cursorCol*10+10,BLOCKS_START_Y+cursorRow*10+5)

    elif state == SM_SWAP_ANIM:
        animDone = True
        if block1 is not None and not block1.tween.finished:
            animDone = False
        if block2 is not None and not block2.tween.finished:
            animDone = False
        if animDone:
            engine_audio.play(sfxSwap,0,False)
            rumble(3,0.4)
            blocksFalling = checkFalling(blocks)
            if len(blocksFalling) > 0:
                startFallAnim(blocksFalling)
                setState(SM_FALL_ANIM)
            else:
                blocksMatching = checkMatching(blocks)
                if len(blocksMatching) > 0:
                    setState(SM_MATCH_ANIM)
                else:
                    setState(SM_MOVE_OVER)
                    
    # TODO figure out how to do FALLING and SQUISH block states
    elif state == SM_FALL_ANIM:
        animDone = True
        for _, _, block in blocksFalling:
            if not block.tween.finished:
                animDone = False
                break
        if animDone:
            blocksFalling = checkFalling(blocks)
            if len(blocksFalling) > 0:
                startFallAnim(blocksFalling)
                setState(SM_FALL_ANIM)
            else:
                blocksMatching = checkMatching(blocks)
                if len(blocksMatching) > 0:
                    setState(SM_MATCH_ANIM)
                else:
                    setState(SM_MOVE_OVER)
                
    # TODO popping animation
    elif state == SM_MATCH_ANIM:
        if stateLoad:
            for _, _, block in blocksMatching:
                block.normal = False
                
        if frame < 30:
            for _, _, block in blocksMatching:
                block.frame_current_x = BLOCK_STATE_LIGHT if ((frame%2)==0) else BLOCK_STATE_NORMAL
        elif frame >= 30:
            if frame == 30:
                for _, _, block in blocksMatching:
                    block.frame_current_x = BLOCK_STATE_POP
            blockPopIndex = (frame-30)//4
            if ((frame-30)%4) == 0:
                if 0 < blockPopIndex <= len(blocksMatching):
                    blocksMatching[blockPopIndex-1][2].opacity = 0
                if blockPopIndex < len(blocksMatching):
                    blocksMatching[blockPopIndex][2].frame_current_x = BLOCK_STATE_POP2
                    engine_audio.play(sfxPops[popLevel],0,False)
                    rumble(2,0.25)
                    popLevel = min(len(sfxPops)-1,popLevel+1)
                else:
                    for col, row, block in blocksMatching:
                        blocks[row*BLOCK_COLS+col] = None
                        block.mark_destroy()
                    blocksFalling = checkFalling(blocks)
                    if len(blocksFalling) > 0:
                        startFallAnim(blocksFalling)
                        setState(SM_FALL_ANIM)
                    else:
                        setState(SM_MOVE_OVER)
    
    elif state == SM_MOVE_OVER:
        cleanBoard = True
        for block in blocks:
            if block is not None:
                cleanBoard = False
                break
        if cleanBoard:
            lblOverlay.text = "YOU\nWIN"
            lblOverlay.scale = Vector2(2,2)
            lblOverlay.opacity = 1
            cursor.opacity = 0
            sprCheckmark.frame_current_x = 1
            setState(SM_LEVEL_WIN)
        elif moves > 0:
            setState(SM_CURSOR_SELECT)
        elif moves == 0:
            for block in blocks:
                if block is not None:
                    block.frame_current_x = BLOCK_STATE_DARK
            lblOverlay.text = " TRY \nAGAIN"
            lblOverlay.scale = Vector2(1,1)
            lblOverlay.opacity = 1
            cursor.opacity = 0
            setState(SM_LEVEL_LOST)
            
    elif state == SM_LEVEL_WIN:
        lblOverlay.opacity = 1 if ((frame % 30) < 15) else 0
        if frame == 5:
            engine_audio.play(sfxWin,0,False)
        if frame > 30 and engine_io.A.is_just_pressed:
            newClear = clears[stage][level] == 0
            clears[stage][level] = 1
            engine_save.save(stages[stage],clears[stage])
            level = (level+1) % len(clears[stage])
            complete = True
            for clear in clears[stage]:
                if not clear:
                    complete = False
                    break
            
            if newClear and complete:
                level = 0
                if modeSelect == MODE_NORMAL and stage == 5:
                    toast(["NORMAL","COMPLETE","","Extra Stages","Unlocked!"])
                    extraUnlock = True
                else:
                    stage = (stage+1) % len(stages)            
                setState(SM_LEVEL_UNLOADING)
            else:
                loadLevel(stages[stage],level)
                lblOverlay.text = ""
                lblOverlay.opacity = 0
                cursor.opacity = 1
                cursorCol, cursorRow = BLOCK_COLS//2-1, BLOCK_ROWS//2
                setState(SM_CURSOR_SELECT)
        
    elif state == SM_LEVEL_LOST:
        lblOverlay.opacity = 1 if ((frame % 30) < 15) else 0
        if frame == 5:
            engine_audio.play(sfxLose,0,False)
        if frame > 30 and engine_io.A.is_just_pressed:
            loadLevel(stages[stage],level)
            lblOverlay.text = ""
            lblOverlay.opacity = 0
            cursor.opacity = 1
            cursorCol, cursorRow = BLOCK_COLS//2-1, BLOCK_ROWS//2
            setState(SM_CURSOR_SELECT)
            
    for block in blocks:
        if block is not None:
            block.updateFalling()
