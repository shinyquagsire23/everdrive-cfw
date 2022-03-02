#!/bin/zsh
cd loader_mb && make && cd ..
cd main && make && cd ..
python3 -u upload.py /dev/tty.usbmodem4E774E6C31301 /Users/maxamillion/workspace/everdrive-cfw/loader_mb/loader_mb_mb.gba /Users/maxamillion/workspace/everdrive-cfw/main/main_mb.gba 