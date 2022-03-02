#include <gba_console.h>
#include <gba_video.h>
#include <gba_interrupt.h>
#include <gba_systemcalls.h>
#include <gba_input.h>
#include <gba_sio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bios.h"

void serial_init() {
    REG_RCNT = 0;
    REG_SIOCNT = SIO_32BIT | SIO_SO_HIGH;
}

u32 xfer32(u32 tosend) 
{
    //Wait for exchange to start
    REG_SIODATA32 = tosend;
    REG_SIOCNT &= ~(SIO_START | SIO_32BIT);
    REG_SIOCNT |= SIO_START | SIO_32BIT;
    while(REG_SIOCNT & SIO_START){} // wait for send

    //Master sends data, we read
    //while((REG_RCNT & 0x1) == 0){}
    u32 ret = REG_SIODATA32; //Get data

    u32 timeout = 1000;
    while (!(REG_RCNT & 1))
    {
        if (!--timeout) break;
    }
    
    return ret;
}

u8 disk_read[0x200];

int main(void) {
    irqInit();
    //irqEnable(IRQ_VBLANK);

    bi_init();

    serial_init();

    while (xfer32(0xF00F1234) != 0xF00F5678)
    {
        VBlankIntrWait();
    }

    u32 len = xfer32(0xF00F0000) & 0xFFFFFFF0;
    u32* out = (u32*)0x08000000;

    while (len)
    {
        *out = xfer32((u32)out);
        out++;
        len -= 4;
    }

    void (*entry)(void) __attribute__((noreturn)) = (void*)0x08000000;
    entry();
}


