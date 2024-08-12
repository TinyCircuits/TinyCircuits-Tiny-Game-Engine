#Scrolling clouds effect created by transistortester
#Inspired by the opening of the C64 demo "Uncensored" created by Booze Design: https://www.youtube.com/watch?v=9LFD4SzW3e0
#Press A to quit and B to toggle and outline

import engine_main
import engine
import engine_io
import engine_draw
import math
from time import ticks_ms
from random import randint

showoutline = False

@micropython.viper
def fillscreen(fill:int):
    scr = ptr16(engine_draw.back_fb_data())
    i:int = 0
    for i in range(16384):
        scr[i] = fill


#cloudshape = bytearray([4,4,3,3,3,4,4,3,2,2,1,1,1,1,1,2,3,3,3,4,4,3,3,4,4,5,5,5,6,6,5,5,4,4,5,5,5,4,3,3,2,2,2,2,2,2,2,3,3,3,2,1,0,0,0,0,0,0,0,1,2,2,2,1,1,1,1,1,1,1,1,1,2,2,4,5,5,5,4,3,3,3,4,5,6,5,4,3,2,2,2,3,4,4,3,3,3,3,3,3,3,4,5,4,3,3,3,3,4,4,5,6,6,6,5,5,5,4,4,5,6,6,5,5,4,4,4,3,3,3,3,3,4,6,6,5,5,4,4,4,4,3,3,4])
cloudshape = bytearray([11, 11, 11, 12, 13, 14, 14, 14, 14, 14, 15, 15, 14, 12, 11, 10, 10, 9, 9, 9, 9, 10, 10, 11, 11, 11, 10, 10, 10, 9, 9, 9, 9, 9, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 7, 6, 6, 6, 5, 5, 5, 5, 5, 5, 5, 6, 6, 6, 7, 8, 9, 12, 12, 13, 13, 14, 15, 16, 17, 19, 21, 22, 23, 22, 22, 22, 21, 21, 21, 21, 21, 21, 21, 18, 16, 15, 15, 14, 14, 13, 13, 13, 13, 13, 14, 14, 15, 15, 14, 14, 14, 13, 13, 13, 13, 13, 11, 10, 9, 9, 8, 8, 7, 7, 7, 7, 7, 7, 7, 7, 8, 8, 9, 9, 10, 11, 11, 10, 10, 10, 9, 9, 9, 9, 9, 9, 9, 9, 10, 10, 10, 11, 11, 12, 13, 11, 10, 9, 8, 7, 6, 5, 5, 4, 4, 3, 3, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 2, 5, 6, 7, 7, 7, 7, 7, 7, 7, 8, 7, 5, 4, 4, 3, 3, 3, 3, 3, 4, 4, 5, 7, 9, 14, 15, 15, 15, 15, 16, 16, 16, 17, 17, 18, 18, 19, 20, 21, 21, 20, 19, 17, 16, 16, 15, 15, 15, 15, 15, 15, 15, 16, 16, 17, 15, 13, 12, 11, 11, 10, 10, 10, 10, 10, 11, 11])
clouds = [] #each is [xpos, ypos, shade]
cloudheight = max(cloudshape)
nextcloud = 0
fill = 0 #background colour

@micropython.viper
def drawcloud(xoff:int, yoff:int, outline:int, shade:int, bgshade:int):
    xpos:int = 0
    ypos:int = 0
    bufpos:int = 0
    fill:int = 0
    cloudptr = ptr8(cloudshape)
    cloudsize:int = int(len(cloudshape))
    scr = ptr16(engine_draw.back_fb_data())
    while xpos < 128:
        bufpos = (yoff + cloudptr[(xpos+xoff)%cloudsize]) * 128 + xpos
        fill = outline
        if bufpos < 0:
            bufpos = xpos
            fill = shade
        while bufpos < 16384 and scr[bufpos] == bgshade: #small bug: if the previous cloud is *exactly* bgshade it will be overwritten
            scr[bufpos] = fill
            fill = shade
            bufpos += 128
        xpos += 1

def RGB888to565(c):
    return ((c[0]>>3)<<11) | ((c[1]>>2)<<5) | c[2]>>3

engine.fps_limit(60)
mainloop = True
while mainloop:
    if engine.tick():
        if engine_io.A.is_just_pressed:
            mainloop = False
        if engine_io.B.is_just_pressed:
            showoutline = not showoutline
        nextcloud -= 1
        if nextcloud <= 0:
            t = ticks_ms()/7
            colour = RGB888to565([
                int(math.sin(math.radians(t*1.2+50)/5)*100+127),
                int(math.sin(math.radians(t+150)/5)*100+127),
                int(math.sin(math.radians(t*1.1)/5)*100+127)])
            clouds.append([randint(0, len(cloudshape)-1), 128.0, colour])
            nextcloud = 20
        if len(clouds) and clouds[0][1]+cloudheight < 0: #completely off screen
            fill = clouds.pop(0)[2] #remove layer, but set background colour just in case the next cloud isn't overlapping the top of the screen
        
        fillscreen(fill)
        
        t = ticks_ms()/2000
        for cloud in reversed(clouds):
            cloud[1] -= 0.5
            if showoutline:
                drawcloud(int(cloud[0] + math.sin(cloud[1]*0.07+t)*15), int(cloud[1]), 1, int(cloud[2]), fill)
            else:
                drawcloud(int(cloud[0] + math.sin(cloud[1]*0.07+t)*15), int(cloud[1]), int(cloud[2]), int(cloud[2]), fill)
        
        #print(len(clouds))
