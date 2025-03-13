# Collects all the files that should be put on a Thumby Color for making UF2 production images.
# The file tree that should be uploaded will be placed in the `production_files/tree` folder

import shutil
import os
import time


output_folder = "production_files"
output_folder_tree = output_folder + "/tree"
output_folder_temp = output_folder + "/temp"


# Delete any existing folder and recreate it and subfolders
if os.path.isdir(output_folder):
    shutil.rmtree(output_folder, True)
    time.sleep(1)

os.mkdir(output_folder)
os.mkdir(output_folder_tree)
os.mkdir(output_folder_temp)
time.sleep(1)

# Legacy Thumby system files and games download and copy
print("\n##### Grabbing files from GitHub repos for Games and core files #####\n")
os.system("git clone https://github.com/TinyCircuits/TinyCircuits-Thumby-Color-Games.git " + output_folder_temp + "/TinyCircuits-Thumby-Color-Games")
os.system("git clone https://github.com/TinyCircuits/TinyCircuits-Thumby-Games.git " + output_folder_temp + "/TinyCircuits-Thumby-Games")
os.system("git clone https://github.com/TinyCircuits/TinyCircuits-Thumby-Code-Editor.git " + output_folder_temp + "/TinyCircuits-Thumby-Code-Editor")
time.sleep(1)


# Thumby Color system files and games copy
shutil.copyfile("filesystem/main.py", output_folder_tree + "/main.py")
shutil.copytree("filesystem/system", output_folder_tree + "/system")

shutil.copytree(output_folder_temp + "/TinyCircuits-Thumby-Color-Games" + "/4Connect",     output_folder_tree + "/Games/4Connect")
shutil.copytree(output_folder_temp + "/TinyCircuits-Thumby-Color-Games" + "/2048",         output_folder_tree + "/Games/2048")
shutil.copytree(output_folder_temp + "/TinyCircuits-Thumby-Color-Games" + "/BustAThumb",   output_folder_tree + "/Games/BustAThumb")
shutil.copytree(output_folder_temp + "/TinyCircuits-Thumby-Color-Games" + "/Chess",        output_folder_tree + "/Games/Chess")
shutil.copytree(output_folder_temp + "/TinyCircuits-Thumby-Color-Games" + "/Monstra",      output_folder_tree + "/Games/Monstra")
shutil.copytree(output_folder_temp + "/TinyCircuits-Thumby-Color-Games" + "/PuzzleAttack", output_folder_tree + "/Games/PuzzleAttack")
shutil.copytree(output_folder_temp + "/TinyCircuits-Thumby-Color-Games" + "/Sand",         output_folder_tree + "/Games/Sand")
shutil.copytree(output_folder_temp + "/TinyCircuits-Thumby-Color-Games" + "/Solitaire",    output_folder_tree + "/Games/Solitaire")
shutil.copytree(output_folder_temp + "/TinyCircuits-Thumby-Color-Games" + "/SongOfMorus",  output_folder_tree + "/Games/SongOfMorus")
shutil.copytree(output_folder_temp + "/TinyCircuits-Thumby-Color-Games" + "/Tagged",       output_folder_tree + "/Games/Tagged")
shutil.copytree(output_folder_temp + "/TinyCircuits-Thumby-Color-Games" + "/ThumbAtro",    output_folder_tree + "/Games/ThumbAtro")
shutil.copytree(output_folder_temp + "/TinyCircuits-Thumby-Color-Games" + "/Thumgeon_II",  output_folder_tree + "/Games/Thumgeon_II")


# Legacy lib copy
shutil.copytree(output_folder_temp + "/TinyCircuits-Thumby-Code-Editor" + "/CoreThumbyFiles/lib", output_folder_tree + "/lib")

# Legacy games copy
shutil.copytree(output_folder_temp + "/TinyCircuits-Thumby-Games" + "/Annelid",     output_folder_tree + "/Games/Legacy/Annelid")
shutil.copytree(output_folder_temp + "/TinyCircuits-Thumby-Games" + "/Brickd",      output_folder_tree + "/Games/Legacy/Brickd")
shutil.copytree(output_folder_temp + "/TinyCircuits-Thumby-Games" + "/SpaceDebris", output_folder_tree + "/Games/Legacy/SpaceDebris")
# shutil.copytree(output_folder_temp + "/TinyCircuits-Thumby-Games" + "/Thumgeon",    output_folder_tree + "/Games/Legacy/Thumgeon")
shutil.copytree(output_folder_temp + "/TinyCircuits-Thumby-Games" + "/TinyBlocks",  output_folder_tree + "/Games/Legacy/TinyBlocks")
# shutil.copytree(output_folder_temp + "/TinyCircuits-Thumby-Games" + "/TinyHeli",    output_folder_tree + "/Games/Legacy/TinyHeli")
shutil.copytree(output_folder_temp + "/TinyCircuits-Thumby-Games" + "/SaurRun",     output_folder_tree + "/Games/Legacy/SaurRun")
# shutil.copytree(output_folder_temp + "/TinyCircuits-Thumby-Games" + "/Tennis",      output_folder_tree + "/Games/Legacy/Tennis")