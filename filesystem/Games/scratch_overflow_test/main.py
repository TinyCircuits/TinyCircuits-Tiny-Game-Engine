import engine_main

from engine_resources import WaveSoundResource

# If the firmware is provided 1MiB and the filesystem 13MiB
# then scratch will be 16 - (1+13) = 2MiB. This file is
# 2.6MB and should cause an exceptions and not erase
# the filesystem, cause file upload errors, or change
# the content of any files if the size overflow is caught
wave = WaveSoundResource("large.wav")