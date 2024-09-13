
#include <stdint.h>
#include "FAT12.h"

/*
#define FAT12_BPB_SIZE 512

struct FAT12_BPB {
    uint8_t  jmp_instruction[3];      // Jump instruction
    char     oem_name[8];             // OEM name
    uint16_t bytes_per_sector;         // Bytes per sector
    uint8_t  sectors_per_cluster;      // Sectors per cluster
    uint16_t reserved_sectors;         // Reserved sectors
    uint8_t  num_fats;                 // Number of FATs
    uint16_t root_dir_entries;         // Root directory entries
    uint16_t total_sectors;            // Total sectors
    uint8_t  media_descriptor;         // Media descriptor
    uint16_t sectors_per_fat;          // Sectors per FAT
    uint16_t sectors_per_track;        // Sectors per track
    uint16_t num_heads;                // Number of heads
    uint32_t hidden_sectors;           // Hidden sectors
    uint32_t total_sectors_large;      // Total sectors (if larger than 65536)
    uint8_t  drive_number;             // Drive number
    uint8_t  reserved;                 // Reserved byte
    uint8_t  extended_boot_record_signature; // Extended boot record signature
    uint32_t volume_serial_number;     // Volume serial number
    char     volume_label[11];         // Volume label
    char     file_system_type[8];      // File system type
    uint8_t  boot_code[448];           // Boot code
    uint16_t boot_sector_signature;    // Boot sector signature (0x55AA)
};
*/

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


// Function to load BIOS Parameter Block from FAT12
void load_bpb(struct BPB *bpb, const char *buffer) {
    bpb->bytes_per_sector = read16((uint8_t*)buffer, 11);
    bpb->sectors_per_cluster = buffer[13];
    bpb->reserved_sectors = read16((uint8_t*)buffer, 14);
    bpb->num_fats = buffer[16];
    bpb->root_dir_entries = read16((uint8_t*)buffer, 17);
    bpb->total_sectors = read16((uint8_t*)buffer, 19);
    bpb->sectors_per_fat = read16((uint8_t*)buffer, 22);

    // Calculate root directory sector and size
    bpb->root_dir_sector = bpb->reserved_sectors + (bpb->num_fats * bpb->sectors_per_fat);
    bpb->root_dir_size = (bpb->root_dir_entries * FAT12_ENTRY_SIZE + bpb->bytes_per_sector - 1) / bpb->bytes_per_sector;

    // Calculate start of data region
    bpb->data_start_sector = bpb->root_dir_sector + bpb->root_dir_size;    
}


// Function to get file data location from starting cluster
uint32_t get_file_location(const struct BPB *bpb, uint16_t starting_cluster) {
    // In FAT12, cluster numbering starts from 2 (clusters 0 and 1 are reserved)
    uint32_t first_data_sector = bpb->data_start_sector;
    uint32_t sector = first_data_sector + (starting_cluster - 2) * bpb->sectors_per_cluster;
    return sector * bpb->bytes_per_sector;  // Return byte offset in the buffer
}

/*
// Function to calculate file location in sectors
uint32_t get_file_location_in_sectors(const struct BPB *bpb, uint16_t starting_cluster) {
    // Calculate the sector number for the given starting cluster
    uint32_t sector_number = (starting_cluster - 2) * bpb->sectors_per_cluster + bpb->reserved_sectors;
    return sector_number;
}
*/

// Function to list files from the root directory and their locations
void list_files(struct BPB *bpb, const char *buffer) {
    // Root directory starts after reserved sectors + FAT areas
    uint32_t root_dir_offset = bpb->root_dir_sector * bpb->bytes_per_sector;

    for (uint16_t i = 0; i < bpb->root_dir_entries; i++) {
        const char *entry = buffer + root_dir_offset + i * FAT12_ENTRY_SIZE;

        // First byte 0x00 indicates no more entries
        if (entry[0] == 0x00) break;

        // Check if it's a valid file (skip deleted/unused entries)
        if ((uint8_t)entry[0] == 0xE5 || (entry[11] & 0x08)) continue;

        // Extract filename (8 chars) and extension (3 chars)
        char filename[9] = {0};  // 8 characters + null terminator
        char ext[4] = {0};       // 3 characters + null terminator
        strncpy(filename, entry, 8);
        strncpy(ext, entry + 8, 3);

        // Extract the starting cluster (little endian)
        uint16_t starting_cluster = read16((uint8_t*)entry, 26);

        // Calculate the file's location in the buffer
        uint32_t file_location = get_file_location(bpb, starting_cluster);
        //uint32_t file_location_sector = get_file_location_in_sectors(bpb, starting_cluster);

        // Print file name, extension, and starting location
        //printf("File: %.8s.%.3s, Location: 0x%X at Sector %d (Starting Cluster: %u)\n", filename, ext, file_location, file_location_sector, starting_cluster);
        printf("File: %.8s.%.3s, Location: 0x%X (Starting Cluster: %u)\n", filename, ext, file_location, starting_cluster);

    }
}


// Function to get the size of a specific file
uint32_t get_file_size(struct BPB *bpb, const char *buffer, const char *filename_to_find) {
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

        // Remove trailing spaces from filename
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
            return file_size;
        }
    }
    return 0;  // File not found
}


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
            
            //printf("File size %u\n", file_size); // =================================================================================================
            
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

            //printf("==== File %s loaded into buffer (size: %u bytes)\n", filename_to_find, bytes_read);
            return bytes_read;  // Return the actual number of bytes read
        }
    }

    printf("File %s not found\n", filename_to_find);
    return -1;  // File not found
}
