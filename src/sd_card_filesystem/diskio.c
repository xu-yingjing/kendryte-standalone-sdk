/*-----------------------------------------------------------------------*/
/* Low level disk I/O module SKELETON for FatFs     (C)ChaN, 2019        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "ff.h"			/* Obtains integer types */
#include "diskio.h"		/* Declarations of disk functions */
#include "sdcard.h"

/* Definitions of physical drive number for each drive */
#define DEV_SD 0


/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
	DSTATUS stat;
	int result;

	switch (pdrv) {
	case DEV_SD :
		result = 0;

		// translate the reslut code here
		if (result == 0)
		{
			stat = 0;
		}
		else
		{
			stat = STA_NOINIT;
		}

		return stat;
	}
	return STA_NOINIT;
}



/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive nmuber to identify the drive */
)
{
	DSTATUS stat;
	int result;

	switch (pdrv) {
	case DEV_SD :
		result = sd_init();

		// translate the reslut code here
		if (result == 0)
		{
			stat = 0;
		}
		else
		{
			stat = STA_NOINIT;
		}

		return stat;
	}
	return STA_NOINIT;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	LBA_t sector,	/* Start sector in LBA */
	UINT count		/* Number of sectors to read */
)
{
	DRESULT res;
	int result;

	switch (pdrv) {
	case DEV_SD :
		// translate the arguments here

		result = sd_read_sector_dma(buff, sector, count);

		// translate the reslut code here
		if (result == 0)
		{
			res = RES_OK;
		}
		else
		{
			res = RES_ERROR;
		}

		return res;
	}

	return RES_PARERR;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if FF_FS_READONLY == 0

DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber to identify the drive */
	const BYTE *buff,	/* Data to be written */
	LBA_t sector,		/* Start sector in LBA */
	UINT count			/* Number of sectors to write */
)
{
	DRESULT res;
	int result;

	switch (pdrv) {
	case DEV_SD :
		// translate the arguments here

		result = sd_write_sector_dma((uint8_t *)buff, sector, count);

		// translate the reslut code here
		if (result == 0)
		{
			res = RES_OK;
		}
		else
		{
			res = RES_ERROR;
		}

		return res;
	}

	return RES_PARERR;
}

#endif


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
	DRESULT res;
	int result;

	switch (pdrv) {
	case DEV_SD :

		// Process of the command for the RAM drive
		switch (cmd)
		{
			case CTRL_SYNC:
				result = 0;
				break;
			case GET_SECTOR_COUNT:
				*(DWORD *)buff = (cardinfo.SD_csd.DeviceSize + 1) << 10;
				result = 0;
				break;
			case GET_SECTOR_SIZE:
				*(WORD *)buff = cardinfo.CardBlockSize;
				result = 0;
				break;
			case GET_BLOCK_SIZE:
				*(DWORD *)buff = cardinfo.CardBlockSize;
				result = 0;
				break;
			default:
				result = 0xFF;
				break;
		}
		
		if (result == 0)
		{
			res = RES_OK;
		}
		else
		{
			res = RES_ERROR;
		}

		return res;
	}

	return RES_PARERR;
}

