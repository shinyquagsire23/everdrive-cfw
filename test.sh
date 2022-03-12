#!/bin/zsh
#cd buildtools/generate_fonts && python3 generate_fonts.py NineteenNinetyThree-L1Ay.ttf character_lists/en.txt 1 font True True 255 0 255 9 && cd ../..
#cp buildtools/generate_fonts/myVerdana.c main/source
#cp buildtools/generate_fonts/myVerdana.h main/source

yes | hdiutil attach ~/sdcard.img
cp buildtools/generate_fonts/font.img.bin /Volumes/RECOVERY/GBASYS/font.img.bin
cp buildtools/generate_fonts/font.vwflen.bin /Volumes/RECOVERY/GBASYS/font.vwflen.bin
hdiutil detach $(hdiutil info | grep RECOVERY | head -n1 | sed -e 's/Windows.*$//')

cd libtonc && make && cd ..
cd loader_mb && make && cd ..
cd main && make clean && make && cd ..
/Users/maxamillion/workspace/mgba/build/sdl/mgba main/main_mb.gba
#open /Applications/mGBA.app main/main_mb.gba
#python3 -u upload.py /dev/tty.usbmodem4E774E6C31301 /Users/maxamillion/workspace/everdrive-cfw/loader_mb/loader_mb_mb.gba /Users/maxamillion/workspace/everdrive-cfw/main/main_mb.gba 