# CH32V307VCT6USB2W25QQFLASH

This project is a modified version of the **USBFS UDISK** from EXAM. 
It adds **FAT12** file system functionality, allowing you to:
- Retrieve file names
- Retrieve file sizes
- Read files into a buffer

## Extras in `FLASH_CLEAN_FAT12_IMAGE` Directory

This project also includes additional utilities and example files located in the `FLASH_CLEAN_FAT12_IMAGE` directory:
- **readFAT12 C project** (built using **Embarcadero DEV-C++**)  
  This project implements the same FAT12 functions and reads from an image file made from the **25Q32 flash** on the development board.
    - You have two image files: `FLASH_CARE_MERGE` and `FLASH_CARE_NU_MERGE`.
    - Drag and drop the image file on top of `readFAT12.exe` to see all the files and their locations.

    *(Note: The two images show some differences that were noticed after formatting the UDISK in Windows to FAT/4096/QUICK FORMAT. Further analysis is planned to understand these differences.)*

- **Utilities included**:
  - `formatx.exe`: Renamed version of Windows 10's format tool.
  - `HxD64.exe`: Hex editor to save UDISK images to files (e.g., `TESTFLASHIMG`).
  - `readFAT12.exe`: Utility to display the contents of FAT12 images. Drag the image file onto it to view content.
  - `win32diskimager-1.0.0-install.exe`: Tool to write FAT12 images or save images from the UDISK.
  - `25Q32FLASHformatted.img`: A clean FAT12 image for formatting the UDISK.

---

## Testing Options in `main.c`

There are three code blocks in `main.c` that you can comment/uncomment to test different functionalities:
1. **Print RAW Data**: Print the addresses in range `0xA000 - 0xA7B0` to display the first file's bytes in RAW format.
2. **Enable UDISK**: Enable the UDISK, so that the **25Q32 FLASH** memory appears as a disk in Windows (use the USB near the ethernet connector).
3. **Show Files**: Display files from FAT12-formatted FLASH memory and get the file size for one of them.

---

## How To's

### 1. Initialize Flash Memory
To initialize the flash memory for the first time, use **Win32DiskImager** to write the clean FAT12 4096 formatted image:
```25Q32FLASHformatted.img```

**Note:** This gives you a clean FAT12 file system on the 25Q32 FLASH memory.

---

### 2. Format in Windows

To format the UDISK in Windows:
1. Go to `Create and Format Harddisk Partitions`.
2. Format as `FAT/4096` with volume name `FLASH`.

**Note:** Trash chunks from deleted files will still be present after formatting.

---

### 3. Alternate Formatting Method

Alternatively, you can use the renamed Windows 10 format utility (`formatx.exe`) to format the Winbond dataflash memory:
```formatx.exe F: /FS:FAT /V:FLASH /Q /X```

**Note:** This is a quick format, and like before, trash chunks from deleted files will still be present.

---

### 4. Saving an Image of the UDISK

To save the current UDISK contents to an image file, use HxD:

```Open Disk > FLASH (F:) > Save the image file```

### 5. Viewing Files from the FAT12 Image

To view the files stored in the FAT12 flash memory image, simply drag and drop the image file onto the readFAT12.exe utility.

