/********************************** (C) COPYRIGHT *******************************
* File Name          : main.c
* Author             : WCH
* Version            : V1.0.0
* Date               : 2022/08/20
* Description        : Main program body.
*********************************************************************************
* Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
* Attention: This software (modified or not) and binary are used for 
* microcontroller manufactured by Nanjing Qinheng Microelectronics.
*******************************************************************************/

/* @Note
 * UDisk Example:
 * This program provides examples of UDisk.Supports external SPI Flash and internal
 * Flash, selected by STORAGE_MEDIUM at SW_UDISK.h.
 *  */

#include "ch32v30x_usbfs_device.h"
#include "debug.h"
#include "SPI_FLASH.h"
#include "SW_UDISK.h"
#include "FAT12.h"

/*********************************************************************
 * @fn      main
 *
 * @brief   Main program.
 *
 * @return  none
 */
int main(void)
{
	struct BPB bpb;

	SystemCoreClockUpdate( );
	Delay_Init( );
	USART_Printf_Init( 115200 );
		
	printf( "SystemClk:%d\r\n",SystemCoreClock );

    // SPI flash init
    FLASH_Port_Init( );
    // FLASH ID check
    FLASH_IC_Check( );

    printf("Flash unique chip ID: %08X\n",(uint32_t)FLASH_ReadUNIQUEID( ));
    printf( "FAT12 W25Q32 4M-byte SPI NOR Flash Storage file list:\n" );
	printf("==============================================\n");

/*  // Print the addresses in range 0xA000 - 0xA7B0 to see first file bytes
    // Use this to debug the flash memory comunication after format to FAT12
    // and store the 4 files provided
    uint8_t byteRead;
    uint32_t address;
    // Loop over the address range
    for (address = 0xA000; address <= 0xA7B0; address++)
    {
        FLASH_RD_Block_Start(address);  // Start reading at the current address
        byteRead = SPI_FLASH_ReadByte();  // Read one byte
        FLASH_RD_Block_End();  // End the block read

        // Print the address and the byte read
        printf("Address: %08x, Data: %02x\n", address, byteRead);
    }
*/

/*
    // Enable Udisk
    Udisk_Capability = Flash_Sector_Count;
    Udisk_Status |= DEF_UDISK_EN_FLAG;
	// USBFSD device init
	USBFS_RCC_Init( );
    USBFS_Device_Init( ENABLE );
*/

	// Load the BIOS Parameter Block
	load_bpb_spi(&bpb);
	// List files in the root directory
	list_files_spi(&bpb);

	printf("==================== END =====================\n\n");

    // Example: Get file size of a specific file
    const char *filename_to_find = "WSCLI.HTM";
    uint32_t file_size = get_file_size_spi(&bpb, filename_to_find);

    if (file_size > 0) {
        printf("Size of file %s: %u bytes\n", filename_to_find, file_size);
    } else {
        printf("File %s not found\n", filename_to_find);
    }

	while(1) {

	}

	return 0;
}
