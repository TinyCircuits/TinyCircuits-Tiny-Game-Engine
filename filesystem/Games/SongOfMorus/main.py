import engine_main

import engine
import machine
import os
import CutscenePage
import TitlePage
import GamePage
import GradientBackground
from engine_math import Vector2,Vector3


engine.fps_limit(30)
machine.freq(225 * 1000 * 1000) #only for real device

cutscenePage = CutscenePage.CutscenePage()
titlePage = TitlePage.TitlePage()
gamePage = GamePage.GamePage()
GradientBackground.GradientBackground.prepareColor()

page = 0
while(True):
    if(page == 0):
        machine.freq(225 * 1000 * 1000) #only for real device
        cutscenePage.firstFrame()
        cutscenePage.preload()
        titlePage.preload()
        machine.freq(150 * 1000 * 1000) #only for real device
        cutscenePage.start()
        page = 1
    elif(page == 1):
        titlePage.start()
        page = 2
    elif(page == 2):
        machine.freq(225 * 1000 * 1000) #only for real device
        gamePage.preload()
        gamePage.start()
        page = 0
    else:
        break
