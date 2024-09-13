#ifndef __FAT12_H__
#define __FAT12_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>

#define BYTES_PER_SECTOR 512
#define FAT12_ENTRY_SIZE 32
#define FAT12_FILENAME_LENGTH 11

#define FILEBUFFER_SIZE 1024*50 // 50kB buffer

// BIOS Parameter Block (BPB) for FAT12 structure to store disk layout
struct BPB {
    uint16_t bytes_per_sector;
    uint8_t sectors_per_cluster;
    uint16_t reserved_sectors;
    uint8_t num_fats;
    uint16_t root_dir_entries;
    uint16_t total_sectors;
    uint16_t sectors_per_fat;
    uint32_t root_dir_sector;
    uint32_t root_dir_size;
    uint32_t data_start_sector; // New field to store the start of data region
};


void load_bpb(struct BPB *bpb, const char *buffer);
uint32_t get_file_location(const struct BPB *bpb, uint16_t starting_cluster);
//uint32_t get_file_location_in_sectors(const struct BPB *bpb, uint16_t starting_cluster);
uint32_t get_file_size(struct BPB *bpb, const char *buffer, const char *filename_to_find);
void list_files(struct BPB *bpb, const char *buffer);
uint16_t get_next_cluster(const struct BPB *bpb, uint16_t current_cluster, const char *buffer);
int load_file_to_buffer(struct BPB *bpb, const char *buffer, const char *filename_to_find, char *fileBuffer, uint32_t buffer_size);

#endif // FAT12_H
