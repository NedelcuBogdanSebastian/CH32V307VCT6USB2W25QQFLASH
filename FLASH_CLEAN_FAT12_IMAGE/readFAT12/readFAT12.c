
/*
    TO DETERMINE FILE SIZE IN WINDOWS:
    >>> for %A in (WSCLI.htm) do @echo %~zA
    
    For testing you need to drag and drop the FLASH FAT12 partition
	on top of the exe and you will see the file list from the FAT12 partition.
	
	
    Here's a list of the functions in the code along with their respective functionality descriptions:
	
    ==============================================================================================================================
    uint16_t read16(const uint8_t *buf, int offset)
	---------------
        Functionality: Reads and returns a 16-bit (2-byte) little-endian value from the specified offset in the buffer.

    uint32_t read32(const uint8_t *buf, int offset)
	---------------
        Functionality: Reads and returns a 32-bit (4-byte) little-endian value from the specified offset in the buffer.

    void load_bpb(struct BPB *bpb, const char *buffer)
	---------------
        Functionality: Loads the BIOS Parameter Block (BPB) from the provided FAT12 buffer. The BPB contains metadata 
		about the FAT12 filesystem layout, such as the number of reserved sectors, sectors per FAT, and root directory size. 
		It calculates the root directory sector, root directory size, and data region start sector.

    uint32_t get_file_location(const struct BPB *bpb, uint16_t starting_cluster)
	---------------
        Functionality: Calculates the byte offset (location) in the buffer where the data for a file's starting cluster is 
		located. This is determined based on the starting cluster number and the layout of the FAT12 file system.

    uint32_t get_file_size(struct BPB *bpb, const char *buffer, const char *filename_to_find)
	---------------
        Functionality: Searches for a specific file in the FAT12 root directory and returns its size in bytes. It looks for 
		the file by its name and extension (in 8.3 format). If the file is not found, it returns 0.

    void list_files(struct BPB *bpb, const char *buffer)
	---------------
        Functionality: Lists the files in the root directory of the FAT12 partition by reading directory entries. It prints 
		the filename, extension, and the location (byte offset) of each file's starting cluster in the buffer.

    uint16_t get_next_cluster(const struct BPB *bpb, uint16_t current_cluster, const char *buffer)
	---------------
        Functionality: Reads the FAT12 table to determine the next cluster in the chain for a file. FAT12 uses a 12-bit 
		cluster entry, and this function decodes that to get the next cluster in the file's cluster chain.

    int loadDataspaceBuff(char *fname)
	---------------
        Functionality: Loads the entire content of a binary file (such as an SD card image or a FAT12 partition image) 
		into a buffer. This function opens the file, reads its size, allocates memory for the buffer, and reads the file content into the buffer. It returns 0 on success or 1 on error.

    int load_file_to_buffer(struct BPB *bpb, const char *buffer, const char *filename_to_find, char *fileBuffer, uint32_t buffer_size)
	---------------
        Functionality: Loads a specific file from the FAT12 filesystem into the provided buffer (fileBuffer). It reads the 
		file cluster by cluster, following the FAT12 cluster chain. If the file size exceeds the buffer, it returns an error. The function returns the number of bytes successfully loaded or -1 if the file is not found.

    int main(int argc, char *argv[])
	---------------
        Functionality: The program's entry point. It takes the filename of a FAT12 disk image as an argument, loads it into
		memory, and initializes the FAT12 filesystem. It loads the BPB, lists the files in the root directory, and performs 
		other operations (currently commented out). After processing, it frees the buffer and waits for a keypress before exiting.
    ==============================================================================================================================
	
    These functions collectively handle reading, interpreting, and displaying the contents of a FAT12 partition, such as those 
    used in older floppy disks or embedded systems.	
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>

#include "FAT12.h"


struct BPB bpb;

// Global file pointer to simulate SD card access
FILE *sdcard_file;

// Global buffer simulating SD card
char *sdcard_buffer;

// Size of the buffer representing the SD card
int32_t sdcard_size;

// Define a buffer to hold the file data (e.g., max size 2048 bytes)
char fileBuffer[FILEBUFFER_SIZE]; // Maximum 50kB file


int loadDataspaceBuff(char *fname)
{
    // Open the file in binary mode ("rb" means read binary)
    sdcard_file = fopen(fname, "rb");
    if (sdcard_file == NULL) {
        perror("Error opening file");
        return 1;
    }

    // Find the size of the file
    if (fseek(sdcard_file, 0, SEEK_END) != 0) {  // Move the file pointer to the end of the file
        perror("Error seeking to the end of the file");
        fclose(sdcard_file);
        return 1;
    }

    sdcard_size = ftell(sdcard_file);  // Get the current file pointer (this is the size of the file)
    if (sdcard_size == -1) {
        perror("Error getting the file size");
        fclose(sdcard_file);
        return 1;
    }

    rewind(sdcard_file);  // Move the file pointer back to the beginning

    // Allocate memory for the buffer based on file size
    sdcard_buffer = (char *)malloc(sdcard_size);
    if (sdcard_buffer == NULL) {
        perror("Memory allocation failed");
        fclose(sdcard_file);
        return 1;
    }

    // Read the entire file into the buffer
    size_t read_size = fread(sdcard_buffer, 1, sdcard_size, sdcard_file);
    if (read_size != sdcard_size) {
        perror("Error reading file");
        free(sdcard_buffer);
        fclose(sdcard_file);
        return 1;
    }

    // You can now process the buffer which contains the binary data
    // For demonstration, let's just print the file size
    printf("File read successfully, size: %ld bytes\n", sdcard_size);

    return 0;  // Success
}


int main(int argc, char *argv[]) 
{
    // Check if file name is provided
    if (argc < 2) {
        printf("Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    // Open the file provided as the first argument
    char *fn = argv[1];

    if (loadDataspaceBuff(fn) != 0) {
        printf("SD CARD INIT ERROR\n");
        return 1;     
    } else {
        printf("SD CARD INIT OK\n");
    } 
    /* ONE SECTOR HAS 512 BYTES
    0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
    0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
    0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
    0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
    0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
    000000000000
    */
    // Load BPB from the boot sector (first sector of the buffer)
    load_bpb(&bpb, sdcard_buffer);

    // List files in the root directory
    list_files(&bpb, sdcard_buffer);    
    
    printf("\n");
    // Example: Get file size of a specific file
    const char *filename_to_find = "WSCLI   .HTM";  // 8.3 format
    uint32_t file_size = get_file_size(&bpb, sdcard_buffer, filename_to_find);

    if (file_size > 0) {
        printf("Size of file %s: %u bytes\n", filename_to_find, file_size);
    } else {
        printf("File %s not found\n", filename_to_find);
    }    
    printf("\n");    
    // TO DETERMINE FILE SIZE IN WINDOWS:
    // >>> for %A in (WSCLI.htm) do @echo %~zA

    // sect 104 - 123 
    // FILE WSCLI.HTM HAS 10037 BYTES
    
    // Example: Load a specific file into the buffer
    const char *filename_to_load = "WSCLI   .HTM";  // 8.3 format
    int bytes_loaded = load_file_to_buffer(&bpb, sdcard_buffer, filename_to_load, fileBuffer, sizeof(fileBuffer));

    if (bytes_loaded > 0) {
        // File successfully loaded into fileBuffer
        printf("File %s loaded with %d bytes\n", filename_to_load, bytes_loaded);
    }
    
    if (bytes_loaded > 0) {        
        // Print file content
        fileBuffer[bytes_loaded] = '\0';  // Null-terminate the buffer to handle it as a string
        
        //printf("%s\n", fileBuffer);       // Print buffer content safely
    }
/*        
        for (size_t i = 0; i < bytes_loaded + 100; i++) 
            printf("%c", fileBuffer[i]);
                
    } else {
        printf("Failed to load file %s\n", filename_to_load);
    }
*/
    
    // Free the buffer when you're done
    free(sdcard_buffer);

    // Close the file
    fclose(sdcard_file);

    printf("\nPress any key to continue...\n");
    getchar();  // Waits for a keypress

    return 0;
}

/*        
        // Print each line
        char *lineStart = fileBuffer;
        
        for (size_t i = 0; i < bytes_loaded; i++) {
            if (fileBuffer[i] == '\n' || fileBuffer[i] == '\0') {
                // Replace newline with null-terminator for the current line
                fileBuffer[i] = '\0';
                
                // Print the current line
                printf("%s\n", lineStart);
                
                // Move to the start of the next line
                lineStart = &fileBuffer[i + 1];
            }
        }
    
        // Print the last line if it does not end with a newline
        if (lineStart < &fileBuffer[bytes_loaded]) {
            printf("%s\n", lineStart);
        }        
*/        
