import engine_main
import engine
import engine_draw
import engine_io
from engine_resources import TextureResource, FontResource
from engine_nodes import Sprite2DNode, CameraNode, Rectangle2DNode, Text2DNode, EmptyNode
from engine_animation import Tween, Delay, EASE_SINE_IN, EASE_SINE_OUT, EASE_LINEAR, ONE_SHOT
from engine_math import Vector2, Vector3
import random
import json
import gc
from micropython import mem_info
from collections import deque
from os import listdir, remove
from time import time


fillcheat = False #if True, pressing RB will fill the grid with whatever is defined in fillpattern
#fillpattern = [None,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14] #tiles 2 to 32768
fillpattern = [9,9,0,1,2,0,1,0,0,1,0,1,1,0,1,0] #one move away from winning (left), or simultaneous win+lose (right).


def printmem(reason=None, heap=False):
    freemem = gc.mem_free()
    totalmem = gc.mem_alloc() + freemem
    print("\n" + "="*79)
    if reason: print(f"***{reason}***")
    print(f"{(totalmem-freemem)/totalmem*100}% RAM used")
    if heap: mem_info(1)
    else: mem_info()
    print("="*79)


#layer layout:
#0: tile glow (not currently implemented)
#1: tiles
#2: foreground tile glow (not currently implemented)
#3: foreground tiles (not currently implemented)
#4: game overlays (undo etc.)
#5: menu background
#6: menu box
#7: menu text

#import engine_debug
#engine_debug.enable_all()
engine_io.gui_toggle_button(None) #disable menu button - this game uses a custom menu implementation

boardtex = TextureResource("background-full.bmp", True) #assets
tiletex = TextureResource("tiles.bmp", True)
undotex = TextureResource("undo.bmp", True)
selectbox = TextureResource("selectbox.bmp", True)
font = FontResource("SperryPC8x16.bmp")

engine_draw.set_background(boardtex)
camera = CameraNode()

gridorigin = (-47,-47) #non-game-specific vars
gridscale = 31
dirs = [[1,0],[0,1],[-1,0],[0,-1]]
valid = {(i,j) for i in range(4) for j in range(4)}
darktext = engine_draw.Color(0x77/255.0, 0x6e/255.0, 0x65/255.0)
lighttext = engine_draw.Color(0xf9/255.0, 0xf6/255.0, 0xf2/255.0)
inputmode = 1 #What handles buttons: 0 = menu, 1 = ingame, anything else = disabled
vanishing = [] #tiles to be deleted when their animation ends
mainloop = True

grid = [[None for i in range(4)] for j in range(4)] #game state
win = False
gameover = False
history = deque([], 100) #maxlen=100 - could be increased, only uses a few percent of RAM
score = 0
moves = 0
undos = 0
starttime = time()


def loadgame():
    global win, score, moves, undos, starttime
    with open("2048save.json", "r") as savefile:
        savedata = json.loads(savefile.read())
    setgrid(savedata[0], fadein=True)
    win = savedata[1]
    score = savedata[2]
    moves = savedata[3]
    undos = savedata[4]
    starttime = time()-savedata[5]


def savegame():
    with open("2048save.json", "w") as savefile:
        savefile.write(json.dumps([getgrid(), win, score, moves, undos, time()-starttime]))


class Tile(Sprite2DNode):
    def __init__(self, value=0, gridpos=(0,0), fadein=False):
        super().__init__(self)
        self.texture = tiletex
        self.transparent_color = engine_draw.black
        self.playing = False
        self.frame_count_y = 17
        self.frame_current_y = value
        self.position = Vector2(gridorigin[0]+gridpos[0]*gridscale, gridorigin[1]+gridpos[1]*gridscale)
        
        self.value = value #current tile - 0 through 16 map to 2 through 131072. 10 is 2048
        self.gridpos = gridpos #current position
        self.newpos = gridpos #new position on next update
        self.merge = False #These flags trigger the given action after the next/ongoing slide ends
        self.vanish = False
        
        self.tween_opacity = Tween() #animation controls
        self.tween_scale = Tween()
        self.tween_slide = Tween()
        self.tween_slide.after = self._dummy #for compatibility with comparisons - is <bound_method <Tween>.None> by default, which python sees as different from None
        self.tween_merge = Tween()
        self.tween_merge.after = self._unpop
        self.appear_delay = Delay()
        
        self.readytodelete = False
        
        if fadein:
            self.opacity = 0.0
            self.appear_delay.start(100, self.appear)
    
    def appear(self, dummy=None):
        self.tween_opacity.start(self, "opacity", 0.0, 1.0, 200, 1.0, ONE_SHOT, EASE_LINEAR)
        self.tween_scale.start(self, "scale", Vector2(0,0), Vector2(1,1), 200, 1.0, ONE_SHOT, EASE_LINEAR)
    
    def update(self):
        self.stopanimation()
        if self.merge:
            self.tween_slide.after = self._merge
        elif self.vanish:
            self.tween_slide.after = self._vanish
        else:
            self.tween_slide.after = self._dummy
            
        if (self.newpos != self.gridpos) or self.merge or self.vanish: #animation needed
            self.tween_slide.start(self, "position", self.position, Vector2(gridorigin[0]+self.newpos[0]*gridscale, gridorigin[1]+self.newpos[1]*gridscale), 100, 1.0, ONE_SHOT, EASE_SINE_IN)
            self.gridpos = self.newpos
        
        self.merge = False
        self.vanish = False
    
    def stopanimation(self): #in case of interruption, set all parameters to their final state
        self.appear_delay.after = None
        self.tween_slide.stop()
        self.tween_slide.after() #manually run this specific callback since it updates state
        self.tween_merge.stop()
        self.tween_scale.stop()
        self.tween_opacity.stop()
        self.scale = Vector2(1,1) #in case appear_delay never finished 
        self.opacity = 1.0
    
    def _merge(self, dummy=None):
        self.tween_slide.after = self._dummy
        self.frame_current_y += 1
        if not self.tween_scale.finished: self.tween_scale.stop()
        self.tween_merge.start(self, "scale", self.scale, Vector2(1.3,1.3), 50, 1.0, ONE_SHOT, EASE_SINE_OUT)
    
    def _unpop(self, dummy=None):
        self.tween_scale.start(self, "scale", self.scale, Vector2(1,1), 50, 1.0, ONE_SHOT, EASE_SINE_IN)
    
    def _vanish(self, dummy=None):
        self.tween_slide.after = self._dummy
        self.stopanimation()
        self.opacity = 0.5
        self.readytodelete = True
    
    def _dummy(self, dummy=None):
        pass


class UndoSprite(Sprite2DNode):
    def __init__(self):
        super().__init__(self)
        self.texture = undotex
        self.transparent_color = engine_draw.black
        self.opacity = 0.0
        self.position = Vector2(-40, -56)
        self.layer = 4
        self.fade = Tween()
        self.delay = Delay()
    
    def show(self):
        self.fade.stop()
        self.opacity = 1.0
        self.delay.start(500, self._startfade)
    
    def _startfade(self, dummy=None):
        self.fade.start(self, "opacity", 1.0, 0.0, 1000, 1.0, ONE_SHOT, EASE_SINE_OUT)

undosprite = UndoSprite()


def playtime():
    t = int(time() - starttime)
    return f"{t//60//60:>02}:{t//60%60:>02}:{t%60:>02}"


class DefaultText(Text2DNode):
    def __init__(self, text="", color=darktext):
        super().__init__(self)
        self.font = font
        self.color = color
        self.text = text
        self.layer = 7


class TimeText(Text2DNode):
    def __init__(self, text="", color=darktext):
        super().__init__(self)
        self.font = font
        self.color = color
        self.text = text
        self.layer = 7
    
    def tick(self, dt):
        if self.opacity > 0:
            self.text = f"Time: {playtime()}"


class Menu(EmptyNode):
    def __init__(self):
        super().__init__(self)
        self.position = Vector3(0,0,0)
        
        self.bg = Rectangle2DNode(width=130, height=130, color=engine_draw.Color(238/255.0, 228/255.0, 218/255.0), opacity=0.0, layer=5)
        #self.add_child(self.bg)
        
        self.entries = []
        self.buttons = [] #each is [position, callback]
        self.selected = 0
        
        self.selectbox = Sprite2DNode(texture=selectbox, transparent_color=engine_draw.black, opacity=0.0, layer=6)
        self.movetween = Tween()
        
        self.fadetweens = []
        self.appeardelay = Delay()
        #self.add_child(self.selectbox)
    
    def configure(self, screen):
        while len(self.entries): #purge everything
            entry = self.entries.pop()
            entry.opacity = 0.0
            #self.remove_child(entry)
            entry.mark_destroy()
        self.buttons = []
        
        if screen == "ingame":
            self.bg.color = engine_draw.Color(238/255.0, 228/255.0, 218/255.0)
            contents = [
                [DefaultText(text="Continue"), self.hide],
                [DefaultText(text="New Game"), self._button_newgame],
                [DefaultText(text="Save and Quit"), self._button_quit],
                None,
                [DefaultText(text=f"Score: {score}"), None],
                [DefaultText(text=f"Moves: {moves}"), None],
                [DefaultText(text=f"Undos: {undos}"), None],
                [TimeText(), None]
            ]
        
        elif screen == "win":
            self.bg.color = engine_draw.Color(237/255.0, 194/255.0, 46/255.0)
            contents = [
                [DefaultText(text="You Win!", color=lighttext), None],
                None,
                [DefaultText(text="Keep Going", color=lighttext), self.hide],
                [DefaultText(text="New Game", color=lighttext), self._button_newgame],
                [DefaultText(text="Save and Quit", color=lighttext), self._button_quit],
                [DefaultText(text=f"Score: {score}", color=lighttext), None],
                #[DefaultText(text=f"Moves: {moves}", color=lighttext), None],
                [DefaultText(text=f"Undos: {undos}", color=lighttext), None],
                [DefaultText(text=f"Time: {playtime()}", color=lighttext), None]
            ]
            if gameover: #I commend anyone who actually sees this in the wild
                contents[1] = [DefaultText(text="Also, Game Over!", color=lighttext), None]
        
        elif screen == "gameover":
            self.bg.color = engine_draw.Color(238/255.0, 228/255.0, 218/255.0)
            contents = [
                [DefaultText(text="Game Over!"), None],
                None,
                [DefaultText(text="Close"), self.hide],
                [DefaultText(text="New Game"), self._button_newgame],
                [DefaultText(text="Quit"), self._button_quit],
                [DefaultText(text=f"Score: {score}"), None],
                [DefaultText(text=f"Moves: {moves}"), None],
                [DefaultText(text=f"Undos: {undos}"), None]
            ]
        
        for num, entry in enumerate(contents):
            if entry == None: continue
            position = Vector2(0, 16*num - 16*len(contents)/2 + 8)
            entry[0].position = position
            entry[0].opacity = 0.0
            #self.add_child(entry[0])
            self.entries.append(entry[0])
            if entry[1] != None:
                self.buttons.append([position, entry[1]])
        
        if len(self.buttons):
            self.selected = 0
            self.selectbox.position = Vector2(self.buttons[0][0].x, self.buttons[0][0].y) #use a copy or the text position will be changed too
    
    def down(self):
        if len(self.buttons):
            self.selected += 1
            self.selected %= len(self.buttons)
            self.movetween.start(self.selectbox, "position", None, self.buttons[self.selected][0], 100, 1.0, ONE_SHOT, EASE_SINE_OUT)
    
    def up(self):
        if len(self.buttons):
            self.selected -= 1
            self.selected %= len(self.buttons)
            self.movetween.start(self.selectbox, "position", None, self.buttons[self.selected][0], 100, 1.0, ONE_SHOT, EASE_SINE_OUT)
    
    def select(self):
        if len(self.buttons):
            self.buttons[self.selected][1]()
    
    def show(self, screen=None, delay=False):
        global inputmode
        inputmode = None #disallow input during transitions
        if screen == None:
            if gameover:
                screen = "gameover"
            else:
                screen = "ingame"
        self.configure(screen)
        if not delay:
            self._fade(1)
            self.fadetweens[0].after = self._aftershow
        else:
            self.appeardelay.start(1000, self._delayshow)
    
    def _delayshow(self, dummy=None):
        self._fade(1)
        self.fadetweens[0].after = self._aftershow
    
    def _aftershow(self, dummy=None):
        global inputmode
        inputmode = 0
    
    def hide(self):
        global inputmode
        inputmode = None
        self._fade(0)
        self.fadetweens[0].after = self._afterhide
    
    def _afterhide(self, dummy=None):
        global inputmode
        inputmode = 1
    
    def _fade(self, value):
        self.fadetweens = []
        
        tween = Tween()
        tween.start(self.bg, "opacity", None, 0.7*value, 300, 1.0, ONE_SHOT, EASE_SINE_OUT)
        self.fadetweens.append(tween)
        
        for i in self.entries:
            tween = Tween()
            tween.start(i, "opacity", None, 1.0*value, 300, 1.0, ONE_SHOT, EASE_SINE_OUT)
            self.fadetweens.append(tween)
        
        if len(self.buttons):
            tween = Tween()
            tween.start(self.selectbox, "opacity", None, 1.0*value, 300, 1.0, ONE_SHOT, EASE_SINE_OUT)
            self.fadetweens.append(tween)
    
    def _button_newgame(self):
        global win, gameover, history, score, moves, undos, starttime
        setgrid()
        win = False
        gameover = False
        history = deque([], 100)
        score = 0
        moves = 0
        undos = 0
        starttime = time()
        addtile()
        addtile()
        self.hide()
    
    def _button_quit(self):
        global mainloop
        mainloop = False
        if gameover:
            if "2048save.json" in listdir("./"):
                os.remove("2048save.json")
        else:
            savegame()

menu = Menu()


def addtile():
    empty = []
    for x, y in valid:
        if grid[y][x] == None:
            empty.append((x,y))
    if len(empty):
        pos = random.choice(empty)
        value = random.choice([0,0,0,0,0,0,0,0,0,1])
        grid[pos[1]][pos[0]] = Tile(value=value, gridpos=pos, fadein=True)


def slide(direction):
    if direction < 2: #it is necessary to reverse the order of checks for two directions
        order = [3,2,1,0]
    else:
        order = [0,1,2,3]
    
    moved = False
    score = 0
    d = dirs[direction]
    for row in order:
        for col in order:
            tile = grid[row][col]
            if tile == None: #empty cell - skip
                continue
            y, x = row, col
            while True:
                x += d[0]
                y += d[1]
                if not (x,y) in valid: #hit boundry
                    x -= d[0] #undo move
                    y -= d[1]
                    break
                dest = grid[y][x]
                if dest != None: #hit tile
                    if dest.value != tile.value or dest.merge: #different type, or already merged - do nothing
                        x -= d[0] #undo move
                        y -= d[1]
                        break
                    else: #same type - merge needed
                        dest.vanish = True #mark one tile for deletion
                        vanishing.append(dest)
                        dest.update() #since this cannot interact anymore, and will be removed from the normal grid, trigger the update now
                        tile.merge = True #mark the other for merge
                        tile.value += 1
                        score += 1 << (tile.value + 1)
                        break
            if (y,x) != (row,col):
                grid[y][x] = tile #move the tile to its new place
                grid[row][col] = None
                tile.newpos = (x,y)
                moved = True
    
    if moved:
        for x, y in valid:
            if grid[y][x] != None:
                grid[y][x].update()
    
    return moved, score


def canmove():
    for x, y in valid:
        if grid[y][x] == None: return True
        for d in dirs:
            nx, ny = x+d[0], y+d[1]
            if (nx, ny) in valid:
                if grid[ny][nx] != None and grid[ny][nx].value == grid[y][x].value:
                    return True
    return False


def has2048():
    for x, y in valid:
        if grid[y][x] != None:
            if grid[y][x].value == 10:
                return True
    return False


def setgrid(new=None, fadein=False): #set grid from a serialized list - None for blank, 0 through 16 for tiles of 2 through 131072
    if new == None: new = [None for i in range(16)]
    for y in range(4):
        for x in range(4):
            if grid[y][x] != None:
                grid[y][x].stopanimation()
                grid[y][x].mark_destroy_all()
            if new[y*4+x] == None:
                grid[y][x] = None
            else:
                grid[y][x] = Tile(value=new[y*4+x], gridpos=(x,y), fadein=fadein)


def getgrid(): #return the grid as a serialized list
    serialized = []
    for y in range(4):
        for x in range(4):
            if grid[y][x] != None:
                serialized.append(grid[y][x].value)
            else:
                serialized.append(None)
    return serialized


if "2048save.json" in listdir("./"):
    loadgame()
else:
    addtile()
    addtile()


while mainloop:
    if engine.tick():
        if inputmode == 0: #menu
            if engine_io.MENU.is_just_pressed or engine_io.B.is_just_pressed:
                menu.hide()
            if engine_io.UP.is_just_pressed:
                menu.up()
            if engine_io.DOWN.is_just_pressed:
                menu.down()
            if engine_io.A.is_just_pressed:
                menu.select()
        
        elif inputmode == 1: #ingame
            if engine_io.B.is_just_pressed:
                if len(history):
                    gamestate = history.pop()
                    setgrid(gamestate[0])
                    score = gamestate[1]
                    moves = gamestate[2]
                    undosprite.show()
                    gameover = False
                    undos += 1
            
            move = None
            if engine_io.RIGHT.is_just_pressed: move = 0
            if engine_io.DOWN.is_just_pressed: move = 1
            if engine_io.LEFT.is_just_pressed: move = 2
            if engine_io.UP.is_just_pressed: move = 3
            if move != None:
                gamestate = [getgrid(), score, moves]
                moved, movescore = slide(move)
                if moved:
                    addtile()
                    score += movescore
                    moves += 1
                    history.append(gamestate)
                    if not canmove():
                        gameover = True
                    if not win and has2048():
                        win = True
                        menu.show(screen="win", delay=True)
                    elif gameover: #it's possible win and lose on the same move - The win screen will be prioritized, though the gameover screen will show on next opening the menu
                        menu.show(screen="gameover", delay=True)
            
            if engine_io.RB.is_just_pressed:
                print(len(history))
                gc.collect()
                printmem()
                if fillcheat:
                    setgrid(fillpattern)
            
            if engine_io.LB.is_just_pressed:
                addtile()
            
            if engine_io.MENU.is_just_pressed or engine_io.A.is_just_pressed:
                menu.show()
        
        temp = []
        for i in vanishing:
            if i.readytodelete:
                i.mark_destroy_all()
            else:
                temp.append(i)
        vanishing = temp