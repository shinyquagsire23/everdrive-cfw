/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2016        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "diskio.h"     /* FatFs lower layer API */

#include "types.h"

#include "disk.h"

#include <string.h>
#include <stdlib.h>
#include <malloc.h>

/* Definitions of physical drive number for each drive */
#define DEV_SDCARD      0
#define SDMMC_DEFAULT_BLOCKLEN (512)
#define SDHC_BLOCK_COUNT_MAX (512)

/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
    BYTE pdrv       /* Physical drive nmuber to identify the drive */
)
{
    switch (pdrv)
    {
        case DEV_SDCARD:
        {
            /*switch(sdcard_check_card())
            {
                case SDMMC_INSERTED:
                    return 0;
                case SDMMC_NEW_CARD:
                    return STA_NOINIT;
                default:
                    return STA_NODISK;
            }*/
            return 0;
        }
    }

    return STA_NODISK;
}



/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
    BYTE pdrv               /* Physical drive nmuber to identify the drive */
)
{
    switch (pdrv)
    {
        case DEV_SDCARD:
        {
            if (diskInit())
                return STA_NODISK;

            return disk_status(pdrv);
        }
    }

    return STA_NODISK;
}

void memcpy32(u16* dst, u16* src, u32 len)
{
    for (u32 i = 0; i < len; i += 2)
    {
        *(dst++) = *(src++);
    }
}

/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
    BYTE pdrv,      /* Physical drive nmuber to identify the drive */
    BYTE *buff,     /* Data buffer to store read data */
    DWORD sector,   /* Start sector in LBA */
    UINT count      /* Number of sectors to read */
)
{
    switch(pdrv)
    {
        case DEV_SDCARD:
            break;

        default: return RES_PARERR;
    }
    u8* buffer = (u8*)buff;
    bool moved_buf = false;
    if((uintptr_t) buff & 7) 
    {
        buffer = memalign(8, SDMMC_DEFAULT_BLOCKLEN * SDHC_BLOCK_COUNT_MAX);
        if(!buffer)
            return RES_ERROR;
        moved_buf = true;
    }

    while(count)
    {
        u32 work = min(count, SDHC_BLOCK_COUNT_MAX);

        if(!moved_buf)
            buffer = (void*)buff;

        switch(pdrv)
        {
            case DEV_SDCARD:
            {
                if(diskRead(sector, buffer, work) != 0)
                {
                    if(moved_buf)
                        free(buffer);
                    return RES_ERROR;
                }
                break;
            }
        }
        //iprintf("%x\n", buff);
        if(moved_buf) 
            memcpy32(buff, buffer, work * SDMMC_DEFAULT_BLOCKLEN);

        sector += work;
        count -= work;
        buff += work * SDMMC_DEFAULT_BLOCKLEN;
    }

    if(moved_buf)
        free(buffer);

    return RES_OK;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

DRESULT disk_write (
    BYTE pdrv,          /* Physical drive nmuber to identify the drive */
    const BYTE *buff,   /* Data to be written */
    DWORD sector,       /* Start sector in LBA */
    UINT count          /* Number of sectors to write */
)
{
    switch(pdrv)
    {
        case DEV_SDCARD:
            break;

        default: return RES_PARERR;
    }

    u8* buffer = (u8*)buff;
    bool moved_buf = false;
    //if((uintptr_t) buff & 7) 
    {
        buffer = memalign(8, SDMMC_DEFAULT_BLOCKLEN * SDHC_BLOCK_COUNT_MAX);
        if(!buffer)
            return RES_ERROR;
        moved_buf = true;
    }

    while(count)
    {
        u32 work = min(count, SDHC_BLOCK_COUNT_MAX);

        if(moved_buf) 
            memcpy(buffer, buff, work * SDMMC_DEFAULT_BLOCKLEN);
        else        
            buffer = (void*)buff;

        switch(pdrv)
        {
            case DEV_SDCARD:
            {
                /*if(diskWrite(sector, buffer, work) != 0)
                {
                    if(moved_buf)
                        free(buffer);
                    return RES_ERROR;
                }*/
                return RES_ERROR;
                break;
            }
        }

        sector += work;
        count -= work;
        buff += work * SDMMC_DEFAULT_BLOCKLEN;
    }
    if(moved_buf) 
        free(buffer);

    return RES_OK;
}



/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl (
    BYTE pdrv,      /* Physical drive nmuber (0..) */
    BYTE cmd,       /* Control code */
    void *buff      /* Buffer to send/receive control data */
)
{
    if (cmd == CTRL_SYNC)
        return RES_OK;

    if (cmd == GET_SECTOR_SIZE)
    {
        *(u32*)buff = SDMMC_DEFAULT_BLOCKLEN;
        return RES_OK;
    }

    if (cmd == GET_BLOCK_SIZE)
    {
        *(u32*)buff = 1;
        return RES_OK;
    }

    if (cmd == GET_SECTOR_COUNT)
    {
        int sectors = -1;
        switch(pdrv)
        {
            case DEV_SDCARD:
                sectors = 0x7FFFFFFF;//sdcard_get_sectors();
                break;

            default: return RES_PARERR;
        }

        if(sectors < 0) return RES_ERROR;
        *(u32*)buff = sectors;
        return RES_OK;
    }

    return RES_PARERR;
}

