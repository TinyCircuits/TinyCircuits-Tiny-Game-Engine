import engine_main

import engine
import engine_io
import engine_draw

import os
#os.chdir("/Games/BustAThumb/")
import TitlePage
import GamePage

page = 0

engine.fps_limit(30)

titlePage = TitlePage.TitlePage()
gamePage = GamePage.GamePage()
while(True):
    if(page == 0):
        titlePage.start()
        page = 1
    elif(page == 1):
        #gamePage.load()
        gamePage.start()
        page = 0
    else:
        break
