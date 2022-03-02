# everdrive-cfw

everdrive-cfw is a custom firmware which can replace `GBAOS.gba` on Everdrive GBA flashcarts. It is currently incomplete and mostly experimental. Uses a subset of ChaN's `fatfs` along with the [official GBA Everdrive IO library](https://krikzz.com/pub/support/everdrive-gba/development/).

## Components

 - loader_mb: For use with [STLink v2 multiboot](https://github.com/shinyquagsire23/stlink_gba_multiboot). Replace `GBAOS.gba` with `loader_mb_mb.gba` to chainload `main` via link port.
 - main: Replaces `GBAOS.gba`

## Building

Requires `devkitARM` and `libgba`. Read through `test.sh` and `upload.sh` to get an idea of how to build and run.