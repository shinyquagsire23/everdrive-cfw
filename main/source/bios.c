

#include "bios.h"
#include "sys.h"


u16 bi_reg_rd(u16 reg);
void bi_reg_wr(u16 reg, u16 data);
u8 bi_dma_busy();
u8 bi_sd_dat_val();
u8 bi_sd_dma_to_rom(void *dst, int slen);
void bi_set_ram_bank(u16 bank);
u8 bi_eep_read_dw(u8 *dst, u16 addr);
u8 bi_eep_write_dw(u8 *src, u16 addr);

#define BUS_CONFIG *((u32 *)0x4000204)


u16 cart_cfg;
u8 sd_cfg;
u16 eep_size;
//int has_init = 0;

u8 bi_init() {
    //if (has_init) return 0;

    bi_reg_wr(REG_KEY, 0xA5);//unlock everdrive registers (write only)
    cart_cfg = CFG_REGS_ON | CFG_NROM_RAM | CFG_ROM_WE_ON; //regs read on, switch from boot rom to psram, psram write on
    bi_reg_wr(REG_CFG, cart_cfg);
    sd_cfg = 0;
    bi_reg_wr(REG_SD_CFG, sd_cfg);
    bi_set_eep_size(EEP_SIZE_8K);

    //has_init = 1;

    return 0;
}

u16 bi_reg_rd(u16 reg) {

    return *((vu16 *) (ED_REG_BASE + reg * 2));
}

void bi_reg_wr(u16 reg, u16 data) {

    *((vu16 *) (ED_REG_BASE + reg * 2)) = data;
}

void bi_lock_regs() {
    cart_cfg &= ~(CFG_REGS_ON | CFG_ROM_WE_ON);
    bi_reg_wr(REG_CFG, cart_cfg);
}

void bi_unlock_regs() {

    bi_reg_wr(REG_KEY, 0xA5);
    cart_cfg |= (CFG_REGS_ON | CFG_ROM_WE_ON);
    bi_reg_wr(REG_CFG, cart_cfg);
}

void bi_dma_mem(void *src, void *dst, int len) {


    DMA_SRC = (u32) src;
    DMA_DST = (u32) dst;
    DMA_LEN = len / 2;
    DMA_CTR = 0x8000;

    while ((DMA_CTR & 0x8000) != 0);

}

u8 bi_sd_wait_f0() {

    u8 resp;
    u16 i;
    u8 mode = SD_MODE4 | SD_WAIT_F0 | SD_STRT_F0;


    for (i = 0; i < 65000; i++) {

        bi_sd_mode(mode);
        bi_reg_rd(REG_SD_DAT);

        for (;;) {
            resp = bi_reg_rd(REG_STATUS);
            if ((resp & STAT_SD_BUSY) == 0)break;
        }

        if ((resp & STAT_SDC_TOUT) == 0)return 0;

        mode = SD_MODE4 | SD_WAIT_F0;
    }



    return 1;
}

u8 bi_sd_dma_wr(void *src) {

    bi_reg_wr(REG_SD_RAM, 0);
    bi_sd_mode(SD_MODE4);
    DMA_SRC = (u32) src;
    DMA_DST = (u32) (ED_REG_BASE + REG_SD_DAT * 2);
    DMA_LEN = 256;
    DMA_CTR = 0x8040;

    while ((DMA_CTR & 0x8000) != 0);

    return 0;
}

void bi_sd_read_crc_ram(void *dst) {

    bi_reg_wr(REG_SD_RAM, 0);
    DMA_SRC = (u32) (ED_REG_BASE + REG_SD_RAM * 2);
    DMA_DST = (u32) dst;
    DMA_LEN = 256;
    DMA_CTR = 0x8100;

    while ((DMA_CTR & 0x8000) != 0);
}

u8 bi_sd_dma_to_rom(void *dst, int slen) {

    vu32 buff[256];
    //
    while (slen) {
        //iprintf("%x %x\n", dst, buff);

        if (bi_sd_wait_f0() != 0)return 1;

        //
#if 1
        bi_reg_wr(REG_CFG, cart_cfg | CFG_AUTO_WE);
        DMA_SRC = (u32) dst;
        DMA_DST = (u32) buff;
        DMA_LEN = 256;
        DMA_CTR = 0x8000;

        __asm__("nop");
        __asm__("nop");

        while ((DMA_CTR & 0x8000) != 0);
        bi_reg_wr(REG_CFG, cart_cfg);
#endif

#if 0
        buff[0] = 0x1234;
        DMA_SRC = (u32) (ED_REG_BASE + REG_SD_DAT * 2);
        DMA_DST = (u32) dst;
        DMA_LEN = 256;
        DMA_CTR = 0x8000 | (2<<7);

        __asm__("nop");
        __asm__("nop");

        while ((DMA_CTR & 0x8000) != 0);
#endif

#if 0
        for (int i = 0; i < 256; i++)
        {
            //buff[i] = *(u16*)((intptr_t)dst + i*2);
            *(u16*)(dst + i*2) = bi_reg_rd(REG_SD_DAT);
        }
#endif

#if 0
        bi_reg_wr(REG_CFG, cart_cfg | CFG_AUTO_WE);
        //buff[0] = *(vu16*)((intptr_t)dst);
        //buff[0] = bi_reg_rd(REG_SD_RAM);
        for (int i = 0; i < 256; i++)
        {
            //buff[i] = *(u16*)((intptr_t)dst + i*2);
            //*(u16*)(dst + i*2) = bi_reg_rd(REG_SD_DAT);

            buff[i] = *(vu16*)((intptr_t)dst + i*2);
            //buff[i] = *(vu16*)((intptr_t)dst + i*2 + 0x20000);
            //buff[i] = bi_reg_rd(REG_SD_DAT);

            //*(vu16*)((intptr_t)dst + i*2) = 0x1234;
            //buff[i] = *(vu8*)((intptr_t)0x0E000000 + i);

            //buff[i] = *(vu32*)((intptr_t)dst + i);

            //*(u16*) (ED_REG_BASE + REG_SD_DAT * 2) = 0;
            //*(vu16*)((intptr_t)dst + i*2) = bi_reg_rd(REG_SD_DAT);
        }
        bi_reg_wr(REG_CFG, cart_cfg);
#endif
        

        slen--;

        dst = (intptr_t)dst + 512;

    }



    return 0;
}

u8 bi_sd_dma_rd(void *dst, int slen) {
    if (((u32) dst & 0xE000000) == 0x8000000
        || ((u32) dst & 0xE000000) == 0xA000000
        || ((u32) dst & 0xE000000) == 0xC000000) return bi_sd_dma_to_rom(dst, slen);

    while (slen) {


        if (bi_sd_wait_f0() != 0)return 1;


        DMA_SRC = (u32) (ED_REG_BASE + REG_SD_DAT * 2);
        DMA_DST = (u32) dst;
        DMA_LEN = 256;
        DMA_CTR = 0x8000;// | (2<<5);

        __asm__("nop");
        __asm__("nop");

        while ((DMA_CTR & 0x8000) != 0);


        slen--;
        dst = (intptr_t)dst + 512;
    }

    return 0;
}

void bi_sd_cmd_wr(u8 data) {

    bi_reg_wr(REG_SD_CMD, data);
    //if ((sd_cfg & SD_SPD_HI))return;
    while ((bi_reg_rd(REG_STATUS) & STAT_SD_BUSY));
}

u8 bi_sd_cmd_rd() {

    u8 dat = bi_reg_rd(REG_SD_CMD);
    //if ((sd_cfg & SD_SPD_HI))return dat;
    while ((bi_reg_rd(REG_STATUS) & STAT_SD_BUSY));
    return dat;
}

u8 bi_sd_cmd_val() {

    u8 dat = bi_reg_rd(REG_SD_CMD + 2);
    return dat;
}

void bi_sd_dat_wr(u8 data) {

    bi_reg_wr(REG_SD_DAT, 0xff00 | data);
    //if ((sd_cfg & SD_SPD_HI))return;
    while ((bi_reg_rd(REG_STATUS) & STAT_SD_BUSY));
}

void bi_sd_dat_wr16(u16 data) {

    bi_reg_wr(REG_SD_DAT, data);
    //if ((sd_cfg & SD_SPD_HI))return;
    while ((bi_reg_rd(REG_STATUS) & STAT_SD_BUSY));
}

u8 bi_sd_dat_rd() {

    u16 dat = bi_reg_rd(REG_SD_DAT) >> 8;
    //if ((sd_cfg & SD_SPD_HI))return dat;
    while ((bi_reg_rd(REG_STATUS) & STAT_SD_BUSY));
    return dat;

}

void bi_sd_mode(u8 mode) {

    sd_cfg &= ~SD_MODE_BITS;
    sd_cfg |= mode & SD_MODE_BITS;

    bi_reg_wr(REG_SD_CFG, sd_cfg);
}

void bi_sd_speed(u8 speed) {

    sd_cfg &= ~SD_SPD_BITS;
    sd_cfg |= speed & SD_SPD_BITS;

    bi_reg_wr(REG_SD_CFG, sd_cfg);
}

u8 bi_eep_read(void *dst, u16 addr, u16 len) {


    len /= 8;
    addr /= 8;

    while (len--) {
        bi_eep_read_dw(dst, addr++);
        dst = (intptr_t)dst + 8;
    }

    return 0;
}

u8 bi_eep_write(void *src, u16 addr, u16 len) {


    len /= 8;
    addr /= 8;

    while (len--) {
        bi_eep_write_dw(src, addr++);
        src = (intptr_t)src + 8;
    }

    return 0;

}

u8 bi_eep_write_dw(u8 *src, u16 addr) {

    u16 i;
    u16 u;
    u16 stream[2 + 14 + 64 + 1];
    u16 *eep_ptr = (u16 *) ED_EEP_BASE;
    u8 val = 0;

    i = 0;
    stream[i++] = 1;
    stream[i++] = 0;
    for (u = 0; u < eep_size; u++) {
        stream[i++] = addr >> (eep_size - 1 - u);
    }
    //stream[i++] = 0;

    for (u = 0; u < 8 * 8; u++) {
        if (u % 8 == 0)val = *src++;
        stream[i++] = val >> 7;
        val <<= 1;
    }
    stream[i++] = 0;

    bi_dma_mem(stream, eep_ptr, i * 2);

    /*
    for (i = 0; i < 64; i++) {
        if ((*eep_ptr & 1) == 1)return 0;
        // gConsPrint("eep busy wr...");
    }*/

    return 0;

}

u8 bi_eep_read_dw(u8 *dst, u16 addr) {

    u16 i;
    u16 u;
    u16 stream[68];
    u16 val;

    u16 *eep_ptr = (u16 *) ED_EEP_BASE;


    for (i = 0; i < 32; i++)stream[i] = 0;

    i = 0;
    stream[i++] = 1;
    stream[i++] = 1;
    for (u = 0; u < eep_size; u++) {
        stream[i++] = addr >> (eep_size - 1 - u);
    }
    stream[i++] = 0;

    bi_dma_mem(stream, eep_ptr, i * 2);

    bi_dma_mem(eep_ptr, stream, 68 * 2);

    i = 4;
    val = 0;
    for (u = 0; u < 8 * 8; u++) {
        val <<= 1;
        val |= stream[i++] & 1;
        if (u % 8 == 7)*dst++ = val;
    }
    /*
        for (i = 0; i < 16; i++) {
            if ((*eep_ptr & 1) == 1)return 0;
            // gConsPrint("eep busy rd...");
        }*/

    return 0;


}

#define SRAM_ADDR 0xE000000

u16 bi_flash_id() {

    u16 id;

    vu8 *fla_ptr = (u8 *) SRAM_ADDR;
    fla_ptr[0x5555] = 0xaa;
    fla_ptr[0x2aaa] = 0x55;
    fla_ptr[0x5555] = 0x90;

    id = fla_ptr[0] | (fla_ptr[1] << 8);

    fla_ptr[0] = 0xf0;

    return id;
}

void bi_flash_erase_chip() {

    vu8 *fla_ptr = (u8 *) SRAM_ADDR;
    fla_ptr[0x5555] = 0xaa;
    fla_ptr[0x2aaa] = 0x55;
    fla_ptr[0x5555] = 0x80;

    fla_ptr[0x5555] = 0xaa;
    fla_ptr[0x2aaa] = 0x55;
    fla_ptr[0x5555] = 0x10;

    u16 ctr = 0;
    while (fla_ptr[0] != 0xff) {
        ctr++;
    }
    /*
        gConsPrint("cer ctr: ");
        gAppendHex16(ctr);*/
}

void bi_flash_erase_sector(u8 sector) {

    vu8 *fla_ptr = (u8 *) SRAM_ADDR;
    fla_ptr[0x5555] = 0xaa;
    fla_ptr[0x2aaa] = 0x55;
    fla_ptr[0x5555] = 0x80;

    fla_ptr[0x5555] = 0xaa;
    fla_ptr[0x2aaa] = 0x55;
    fla_ptr[sector * 4096] = 0x30;

    u16 ctr = 0;
    while (fla_ptr[sector * 4096] != 0xff) {
        ctr++;
    }

}

void bi_flash_write(void *src, u32 addr, u32 len) {

    u8 *dat8 = (u8 *) src;
    vu8 *fla_ptr = (u8 *) SRAM_ADDR;

    while (len--) {
        fla_ptr[0x5555] = 0xaa;
        fla_ptr[0x2aaa] = 0x55;
        fla_ptr[0x5555] = 0xA0;
        fla_ptr[addr++] = *dat8++;
    }

}

void bi_flash_set_bank(u8 bank) {

    vu8 *fla_ptr = (u8 *) SRAM_ADDR;
    fla_ptr[0x5555] = 0xaa;
    fla_ptr[0x2aaa] = 0x55;
    fla_ptr[0x5555] = 0xB0;
    fla_ptr[0] = bank & 1;
}

void bi_set_save_type(u8 save_type) {

    if (save_type != 0)bi_set_save_type(0);
    cart_cfg &= ~BI_SAV_BITS;
    save_type &= BI_SAV_BITS;
    cart_cfg |= save_type;
    bi_reg_wr(REG_CFG, cart_cfg);
}

void bi_set_eep_size(u8 size) {
    eep_size = size;
}

void bi_sram_read(void *dst, u32 offset, u32 len) {

    u8 *src8 = (u8 *) (SRAM_ADDR + offset % 65536);
    u8 *dst8 = dst;
    bi_set_ram_bank(offset / 65536);
    while (len--)*dst8++ = *src8++;

    bi_set_ram_bank(0);
}

void bi_sram_write(void *src, u32 offset, u32 len) {

    u8 *src8 = src;
    u8 *dst8 = (u8 *) (SRAM_ADDR + offset % 65536);
    bi_set_ram_bank(offset / 65536);
    while (len--)*dst8++ = *src8++;

    bi_set_ram_bank(0);
}

void bi_set_ram_bank(u16 bank) {


    bank = bank == 3 ? BI_RAM_BNK_3 : bank == 2 ? BI_RAM_BNK_2 : bank == 1 ? BI_RAM_BNK_1 : BI_RAM_BNK_0;
    cart_cfg &= ~BI_RAM_BNK_3;
    cart_cfg |= bank;
    bi_reg_wr(REG_CFG, cart_cfg);
}

u16 bi_get_fpga_ver() {
    return bi_reg_rd(REG_FPGA_VER);
}


void bi_rtc_on() {
    cart_cfg |= CFG_RTC_ON;
    bi_reg_wr(REG_CFG, cart_cfg);
}

void bi_rtc_off() {
    cart_cfg &= ~CFG_RTC_ON;
    bi_reg_wr(REG_CFG, cart_cfg);
}



void bi_set_rom_bank(u8 bank) {

    if (bank == 0) {
        cart_cfg &= ~CFG_ROM_BANK;
    } else {
        cart_cfg |= CFG_ROM_BANK;
    }
    bi_reg_wr(REG_CFG, cart_cfg);
}

void bi_set_rom_mask(u32 rom_size) {

    if (rom_size > 0x1000000) {
        cart_cfg |= CFG_BIG_ROM;
    } else {
        cart_cfg &= ~CFG_BIG_ROM;
    }
    bi_reg_wr(REG_CFG, cart_cfg);
}

void bi_reboot(u8 quick_boot) {



    IRQ_ACK_REG = IRQ_GAME_PAK;

    GBA_TIMER0_CFG = 0;
    GBA_TIMER0_VAL = 0;
    GBA_TIMER1_CFG = 0;
    GBA_TIMER1_VAL = 0;

    //bi_reg_wr(REG_CFG, 0);
    bi_lock_regs();
    GBA_WAITCNT = 0;
    if (quick_boot)asm("swi 0x00");
    asm("swi 0x26");
}


