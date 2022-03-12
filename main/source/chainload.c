#include "chainload.h"

#include <gba_console.h>
#include <gba_video.h>
#include <gba_interrupt.h>
#include <gba_systemcalls.h>
#include <gba_input.h>
#include <gba_sio.h>

int sio_waiting = 0;

void serial_init() {
    REG_RCNT = 0;
    REG_SIOCNT = SIO_32BIT | SIO_SO_HIGH | SIO_IRQ;
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

void irq_sio()
{
    sio_waiting = 1;
}

void sio_chainload()
{
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

void sio_handle()
{
    u32 cmd = xfer32(0);
    if (cmd == 0xF00F8765) {
        sio_chainload();
        return;
    }

    sio_waiting = 0;
    char v = xfer32(0);
    //if (v && v != 0xFF)
    //    iprintf("%c", v);
}