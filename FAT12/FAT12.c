
#include <stdint.h>
#include "SPI_FLASH.h"
#include "FAT12.h"


//#define DEBUGFAT12

// Function to read 16-bit values (little endian)
uint16_t read16(const uint8_t *buf, uint16_t offset) {
    uint32_t result = 0;
    result |= (buf[offset + 1] << 8);
    result |=  buf[offset];
    return result;
}

// Function to read 32-bit values (little endian)
uint32_t read32(const uint8_t *buf, uint16_t offset) {
    uint32_t result = 0;
    result |= (buf[offset + 3] << 24);
    result |= (buf[offset + 2] << 16);
    result |= (buf[offset + 1] << 8);
    result |=  buf[offset];
    return result;
}

uint16_t read16_spi(uint32_t address) {
    uint8_t buf[2];
    FLASH_RD_Block_Start(address);
    FLASH_RD_Block(buf, sizeof(buf));
    FLASH_RD_Block_End();
    return read16(buf, 0);
}

uint32_t read32_spi(uint32_t address) {
    uint8_t buf[4];
    FLASH_RD_Block_Start(address);
    FLASH_RD_Block(buf, sizeof(buf));
    FLASH_RD_Block_End();
    return read32(buf, 0);
}


void load_bpb_spi(struct BPB *bpb) {
    uint32_t bpb_address = 0;  // Address where BPB is located in the flash
    uint8_t buffer[BPB_SIZE];  // Adjust BPB_SIZE to your BPB size

    FLASH_RD_Block_Start(bpb_address);
    FLASH_RD_Block(buffer, sizeof(buffer));
    FLASH_RD_Block_End();

    bpb->bytes_per_sector = read16_spi(bpb_address + 11);
    bpb->sectors_per_cluster = buffer[13];
    bpb->reserved_sectors = read16_spi(bpb_address + 14);
    bpb->num_fats = buffer[16];
    bpb->root_dir_entries = read16_spi(bpb_address + 17);
    bpb->total_sectors = read16_spi(bpb_address + 19);
    bpb->sectors_per_fat = read16_spi(bpb_address + 22);

    // Calculate root directory sector and size
    bpb->root_dir_sector = bpb->reserved_sectors + (bpb->num_fats * bpb->sectors_per_fat);
    bpb->root_dir_size = (bpb->root_dir_entries * FAT12_ENTRY_SIZE + bpb->bytes_per_sector - 1) / bpb->bytes_per_sector;

    // Calculate start of data region
    bpb->data_start_sector = bpb->root_dir_sector + bpb->root_dir_size;

#ifdef DEBUGFAT12
    printf("        FAT12 data\n");
    printf("=========================\n");
    printf("Bytes_per_sector: %d\n", bpb->bytes_per_sector);
    printf("Sectors_per_cluster: %d\n", bpb->sectors_per_cluster);
    printf("Reserved_sectors: %d\n", bpb->reserved_sectors);
    printf("Num_fats: %d\n", bpb->num_fats);
    printf("Root_dir_entries: %d\n", bpb->root_dir_entries);
    printf("Total_sectors: %d\n", bpb->total_sectors);
    printf("Sectors_per_fat: %d\n", bpb->sectors_per_fat);
    printf("Root_dir_sector: %d\n", bpb->root_dir_sector);
    printf("Root_dir_size: %d\n", bpb->root_dir_size);
    printf("Data_start_sector: %d\n", bpb->data_start_sector);
    printf("=========================\n");
#endif
}


// Function to get file data location from starting cluster
uint32_t get_file_location_spi(const struct BPB *bpb, uint16_t starting_cluster) {
    // In FAT12, cluster numbering starts from 2 (clusters 0 and 1 are reserved)
    uint32_t first_data_sector = bpb->data_start_sector;
#ifdef DEBUGFAT12
    printf("First data sector starts at: %d\n", first_data_sector);
#endif
    uint32_t sector = first_data_sector + (starting_cluster - 2) * bpb->sectors_per_cluster;
    return sector * bpb->bytes_per_sector;  // Return byte offset in the buffer
}


// Function to list files from the root directory and their locations
void list_files_spi(struct BPB *bpb) {
    uint32_t root_dir_offset = bpb->root_dir_sector * bpb->bytes_per_sector;
    uint8_t entry[FAT12_ENTRY_SIZE];
#ifdef DEBUGFAT12
    printf("Start listing files from the FLASH\n");
    printf("Number of root dir entry %d\n", bpb->root_dir_entries);
#endif
    // The entry address refers to the location of the file's metadata (e.g., name, size, starting cluster) in the root directory

    for (uint16_t i = 0; i < bpb->root_dir_entries; i++) {
        uint32_t entry_address = root_dir_offset + i * FAT12_ENTRY_SIZE; // entry adr + file number * entry size

#ifdef DEBUGFAT12
        printf("File %d starts at entry address 0x%X\n", i, entry_address);
#endif
        FLASH_RD_Block_Start(entry_address);
        FLASH_RD_Block(entry, sizeof(entry));
        FLASH_RD_Block_End();

        // First byte 0x00 indicates no more entries
        if (entry[0] == 0x00) break;

        // Check if it's a valid file (skip deleted/unused entries)
        if ((uint8_t)entry[0] == 0xE5 || (entry[11] & 0x08)) continue;

        // Extract filename (8 chars) and extension (3 chars)
        char filename[9] = {0};  // 8 characters + null terminator
        char ext[4] = {0};       // 3 characters + null terminator
        strncpy(filename, (char*)entry, 8);
        strncpy(ext, (char*)entry + 8, 3);

        // ================================================================

        // Extract the file size (little endian, 4 bytes at offset 28)
        uint32_t file_size = read32((uint8_t*)entry, 28);

        // ================================================================

        // Extract the starting cluster (little endian)
        uint16_t starting_cluster = read16(entry, 26);

        // The file_location refers to the actual starting point of the file's data in the data region (based on its starting cluster).

        // Calculate the file's location in the buffer
        uint32_t file_location = get_file_location_spi(bpb, starting_cluster);

        printf("%s.%s   -   starts at location 0x%X and has the size: %u bytes\n", filename, ext, file_location, file_size);

#ifdef DEBUGFAT12
        // Print file name, extension, and starting location
        printf("File: %.8s.%.3s, Location: 0x%X (Starting Cluster: %u)\n", filename, ext, file_location, starting_cluster);
#endif
    }
}


// Function to get the size of a specific file
uint32_t get_file_size_spi(struct BPB *bpb, const char *filename_to_find) {
    uint32_t root_dir_offset = bpb->root_dir_sector * bpb->bytes_per_sector;
    uint8_t entry[FAT12_ENTRY_SIZE];

    // Iterate through the root directory entries
    for (uint16_t i = 0; i < bpb->root_dir_entries; i++) {
        uint32_t entry_address = root_dir_offset + i * FAT12_ENTRY_SIZE;

        FLASH_RD_Block_Start(entry_address);
        FLASH_RD_Block(entry, sizeof(entry));
        FLASH_RD_Block_End();

        // First byte 0x00 indicates no more entries
        if (entry[0] == 0x00) break;

        // Check if it's a valid file (skip deleted/unused entries)
        if ((uint8_t)entry[0] == 0xE5 || (entry[11] & 0x08)) continue;

        // Extract filename (8 chars) and extension (3 chars)
        char filename[9] = {0};
        char ext[4] = {0};
        strncpy(filename, entry, 8);
        strncpy(ext, entry + 8, 3);

        // Make the file name to be normal, not 8.3 !!!

        // Combine the filename and extension to a normal file mane,
        // not the 8.3 of the FAT, to compare with the target
        char full_filename[FAT12_FILENAME_LENGTH + 2] = {0};  // 8.3 format + dot

        // Remove trailing spaces from filename
        for (int j = 7; j >= 0 && filename[j] == ' '; j--) {
            filename[j] = '\0';
        }
        for (int j = 2; j >= 0 && ext[j] == ' '; j--) {
            ext[j] = '\0';
        }

        snprintf(full_filename, sizeof(full_filename), "%.8s.%.3s", filename, ext);

        // printf("File name = %s\n", full_filename);

        // Compare with the target file name (case-sensitive)
        if (strcmp(full_filename, filename_to_find) == 0) {
            // Extract the file size (little endian, 4 bytes at offset 28)
            uint32_t file_size = read32((uint8_t*)entry, 28);
            return file_size;
        }
    }
    return 0;  // File not found
}


/*
// Function to get the next cluster from the FAT12 table using SPI
uint16_t get_next_cluster_spi(const struct BPB *bpb, uint16_t current_cluster) {
    // FAT12 uses 1.5 bytes per cluster entry (12 bits)
    uint32_t fat_offset = bpb->reserved_sectors * bpb->bytes_per_sector;
    uint32_t entry_offset = current_cluster + (current_cluster / 2);  // 1.5-byte entries

    // Compute the starting byte for the FAT entry
    uint32_t fat_start_byte = fat_offset + entry_offset;

    uint16_t next_cluster = 0;
    
    if (current_cluster & 1) {
        // Odd cluster, read 12-bit value spanning 1.5 bytes
        uint8_t entry_value1 = spi_read_byte(fat_start_byte);
        uint8_t entry_value2 = spi_read_byte(fat_start_byte + 1);
        next_cluster = (entry_value1 >> 4) | (entry_value2 << 4);
    } else {
        // Even cluster, read 12-bit value spanning 2 bytes
        uint8_t entry_value1 = spi_read_byte(fat_start_byte);
        uint8_t entry_value2 = spi_read_byte(fat_start_byte + 1);
        next_cluster = (entry_value1 | (entry_value2 << 8)) & 0x0FFF;
    }

    return next_cluster;
}
*/





/*
// Function to get the next cluster from the FAT12 table
uint16_t get_next_cluster(const struct BPB *bpb, uint16_t current_cluster, const char *buffer) {
    // FAT12 uses 1.5 bytes per cluster entry (12 bits)
    uint32_t fat_offset = bpb->reserved_sectors * bpb->bytes_per_sector;
    uint32_t entry_offset = current_cluster + (current_cluster / 2);  // 1.5-byte entries

    // Extract FAT12 12-bit value
    uint16_t next_cluster;
    if (current_cluster & 1) {
		uint16_t entry_value = (uint16_t)buffer[fat_offset + entry_offset];
		uint16_t next_value = (uint16_t)buffer[fat_offset + entry_offset + 1] << 4;
		next_cluster = (entry_value >> 4) | next_value;
    } else {
        next_cluster = read16((uint8_t*)buffer, fat_offset + entry_offset) & 0x0FFF;
    }

    return next_cluster;
}




// Function to load a file into the buffer
int load_file_to_buffer(struct BPB *bpb, const char *buffer, const char *filename_to_find, char *fileBuffer, uint32_t buffer_size) {
    uint32_t root_dir_offset = bpb->root_dir_sector * bpb->bytes_per_sector;

    // Iterate through the root directory entries
    for (uint16_t i = 0; i < bpb->root_dir_entries; i++) {
        const char *entry = buffer + root_dir_offset + i * FAT12_ENTRY_SIZE;

        // First byte 0x00 indicates no more entries
        if (entry[0] == 0x00) break;

        // Check if it's a valid file (skip deleted/unused entries)
        if ((uint8_t)entry[0] == 0xE5 || (entry[11] & 0x08)) continue;

        // Extract filename (8 chars) and extension (3 chars)
        char filename[9] = {0};
        char ext[4] = {0};
        strncpy(filename, entry, 8);
        strncpy(ext, entry + 8, 3);

        // Combine the filename and extension to compare with the target
        char full_filename[FAT12_FILENAME_LENGTH + 2] = {0};  // 8.3 format + dot
        snprintf(full_filename, sizeof(full_filename), "%.8s.%.3s", filename, ext);

        // Remove trailing spaces from filename and extension
        for (int j = 7; j >= 0 && filename[j] == ' '; j--) {
            filename[j] = '\0';
        }
        for (int j = 2; j >= 0 && ext[j] == ' '; j--) {
            ext[j] = '\0';
        }

        // Compare with the target file name (case-sensitive)
        if (strcmp(full_filename, filename_to_find) == 0) {
            
            
            // Extract the file size (little endian, 4 bytes at offset 28)
            uint32_t file_size = read32((uint8_t*)entry, 28);
            
            printf("File size %u\n", file_size); // =================================================================================================
            
            if (file_size > buffer_size) {
                printf("Error: Buffer too small for file %s (size: %u bytes)\n", filename_to_find, file_size);
                return -1;  // File size exceeds buffer
            }

            // Extract the starting cluster (little endian)
            uint16_t starting_cluster = read16((uint8_t*)entry, 26);
            uint16_t current_cluster = starting_cluster;

            // Read the file data cluster by cluster
            uint32_t bytes_read = 0;
            
            while (bytes_read < file_size) {
                uint32_t cluster_location = get_file_location(bpb, current_cluster);
                
                //#define INVALID_CLUSTER_LOCATION 0xFFFF
                //if (cluster_location == INVALID_CLUSTER_LOCATION) {
                //    fprintf(stderr, "Error: Invalid cluster location for cluster %u\n", current_cluster);
                //    break;
                //}
            
                uint32_t cluster_size = bpb->sectors_per_cluster * bpb->bytes_per_sector;
                uint32_t remaining_bytes = file_size - bytes_read;
                uint32_t bytes_to_copy = (remaining_bytes < cluster_size) ? remaining_bytes : cluster_size;
            
                // Ensure buffer has enough space
                if (bytes_read + bytes_to_copy > FILEBUFFER_SIZE) {
                    fprintf(stderr, "Error: Buffer overflow. fileBuffer size: %u, bytes to copy: %u\n", FILEBUFFER_SIZE, bytes_to_copy);
                    break;
                }
            
                // Copy data from the cluster to the file buffer
                memcpy(fileBuffer + bytes_read, buffer + cluster_location, bytes_to_copy);
                bytes_read += bytes_to_copy;
            
                // If we've read all the bytes needed, we are done
                if (bytes_read >= file_size) {
                    break;
                }
            
                // Get the next cluster from the FAT
                current_cluster = get_next_cluster(bpb, current_cluster, buffer);
            
                // Check if we have reached the end of the file
                if (current_cluster >= 0xFF8) {  // End-of-file marker
                    if (bytes_read < file_size) {
                        // Calculate how many bytes are still needed
                        uint32_t bytes_needed = file_size - bytes_read;
                        // Copy the remaining bytes from the current cluster
                        memcpy(fileBuffer + bytes_read, buffer + cluster_location, bytes_needed);
                        bytes_read += bytes_needed;
                        fprintf(stderr, "Warning: Reached end-of-file marker, but copied remaining %u bytes from the last cluster\n", bytes_needed);
                    }
                    break;
                }
            }

            printf("==== File %s loaded into buffer (size: %u bytes)\n", filename_to_find, bytes_read);
            return bytes_read;  // Return the actual number of bytes read
        }
    }

    printf("File %s not found\n", filename_to_find);
    return -1;  // File not found
}

*/
