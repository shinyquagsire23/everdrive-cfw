/* 
 * File:   bios.h
 * Author: krik
 *
 * Created on December 15, 2015, 1:50 PM
 */

#ifndef BIOS_H
#define	BIOS_H

#include "sys.h"

// Everdrive register base
#define ED_REG_BASE (0x9FC0000)

// Everdrive SRAM direct access
#define ED_EEP_BASE (0x9FE0000)

// Everdrive Registers
#define REG_CFG         (0x00)
#define REG_STATUS      (0x01)
#define REG_2           (0x02) // unk
#define REG_3           (0x03) // unk
#define REG_4           (0x04) // unk
#define REG_FPGA_VER    (0x05)
#define REG_GPIO        (0x06) // used to update bootloader
#define REG_7           (0x07) // unk
#define REG_SD_CMD      (0x08)
#define REG_SD_DAT      (0x09)
#define REG_SD_CFG      (0x0A)
#define REG_SD_RAM      (0x0B)
#define REG_KEY         (0x5A)

// ED N64 leftovers?
#define BI_USB_BUFF (0x7FFF)

// REG_SD_CFG stuff
#define SD_SPD_LO 0
#define SD_SPD_HI 1

#define SD_MODE1 0
#define SD_MODE2 2
#define SD_MODE4 4
#define SD_MODE8 6

// REG_CFG
#define CFG_REGS_ON     (0x0001) // ED regs are enabled (turn off when booting ROMs)
#define CFG_NROM_RAM    (0x0002) // ED RAM is mapped to 0x08000000
#define CFG_ROM_WE_ON   (0x0004) // ED RAM is writable at 0x08000000
#define CFG_AUTO_WE     (0x0008) // REG_SD_DAT automatically transfers to PSRAM on reads

// Save type == CFG mask 0x0070
#define BI_SAV_EEP     (0x0010)
#define BI_SAV_SRM     (0x0020)
#define BI_SAV_FLA64   (0x0040)
#define BI_SAV_FLA128  (0x0050)
#define BI_SAV_BITS (BI_SAV_EEP | BI_SAV_FLA128 | BI_SAV_FLA64 | BI_SAV_SRM)

#define BI_RAM_BNK_0 (0x0000)
#define BI_RAM_BNK_1 (0x0080)
#define BI_RAM_BNK_2 (0x0100)
#define BI_RAM_BNK_3 (0x0180)

#define CFG_RTC_ON      (0x0200) // enable RTC GPIO
#define CFG_ROM_BANK    (0x0400) // Selects hi 16MiB or lo 16MiB at 0x08000000
#define CFG_BIG_ROM     (0x0800) //speciall eeprom mapping for 32MB games
#define CFG_BOOTLOADER  (0x1000) // maps the bootloader to 0x08000000

// File-stuff only
#define BI_CART_FEA_RTC         0x0001
#define BI_CART_FEA_SPD         0x0002
#define BI_CART_FEA_BAT         0x0004
#define BI_CART_FEA_CRC_RAM     0x0008
#define BI_CART_TYPE            0xff00

// EEPROM size
#define EEP_SIZE_512 0x6
#define EEP_SIZE_8K  0xE

// REG_SD_??
#define STAT_SD_BUSY    1
#define STAT_SDC_TOUT   2

// REG_SD_??
#define SD_WAIT_F0   (0x0008)
#define SD_STRT_F0   (0x0010)
#define SD_MODE_BITS (0x001E)
#define SD_SPD_BITS  (0x0001)

#define GBA_WAITCNT *((vu32 *)0x4000204)
#define GBA_TIMER1_VAL *((vu16 *)0x4000104)
#define GBA_TIMER1_CFG *((vu16 *)0x4000106)
#define IRQ_ACK_REG *((vu16 *) 0x4000202)
#define IRQ_GAME_PAK (0x2000)

u8 bi_init();
void bi_lock_regs();
void bi_unlock_regs();
void bi_dma_mem(void *src, void *dst, int len);

void bi_sd_cmd_wr(u8 data);
u8 bi_sd_cmd_rd();
void bi_sd_dat_wr(u8 data);
u8 bi_sd_dat_rd();
u8 bi_sd_dma_rd(void *dst, int slen);

void bi_sd_mode(u8 mode);
void bi_sd_speed(u8 speed);

void bi_shutdown();


#endif	/* BIOS_H */

