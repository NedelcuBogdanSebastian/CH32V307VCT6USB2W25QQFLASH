# CH32V307VCT6USB2W25QQFLASH

This example is a modified of the USBFS UDISK from EXAM.

It adds FAT12 file system function to make:
    - get file names 
    - get file size
    - read file to buffer






To initialize for the first time the flash memory
we use the win32diskimaker to write the:
>>> 25Q32FLASHformatted.img
which is a clean FAT12 4096 formatted image of 4Mb
THIS WILL LET US HAVE A CLEAN FLASH MEMORY WITH FAT12 FILE SYSTEM

================================================

To format in Windows use the:
>>> CREATE AND FORMAT HARDDISK PARTITIONS
Format in FAT/4096, Volume name FLASH
ALL THE TRASH CHUNKS FROM FILES ARE STILL KEEPT!!!
================================================

Other method to format the Winbond dataflash 
memory in Windows to FAT12 you can use the:
>>> formatx.exe
This is the format from Windows 10, just renamed :)
>>> formatx.exe F: /FS:FAT /V:FLASH /Q /X
The format is a quick format.
ALL THE TRASH CHUNKS FROM FILES ARE STILL KEEPT!!!
================================================

To make an image file of the content use XxD, 
Open Disk > FLASH (F) > Save

================================================

To see the files from the FAT12 flash memory image 
just drag and drop the image file on top of the:
>>> readFAT12.exe
