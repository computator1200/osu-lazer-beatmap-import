#include "beatmap_processor.h"
#include "zip_utils.h"
#include "signal_handler.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>

int count_beatmaps(const char *beatmaps_path) {
    DIR *dir = opendir(beatmaps_path);
    if (dir == NULL) {
        return -1;
    }
    
    int count = 0;
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        // Skip . and .. entries
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        
        char full_path[1024];
        snprintf(full_path, sizeof(full_path), "%s/%s", beatmaps_path, entry->d_name);
        
        if (is_directory(full_path)) {
            count++;
        }
    }
    
    closedir(dir);
    return count;
}

int init_processor(beatmap_processor_t *processor, const char *beatmaps_path) {
    if (processor == NULL || beatmaps_path == NULL) {
        return -1;
    }
    
    // Initialize processor state
    processor->beatmaps_path = strdup(beatmaps_path);
    processor->output_path = NULL;
    processor->should_stop = false;
    processor->is_paused = false;
    processor->processed_count = 0;
    
    if (processor->beatmaps_path == NULL) {
        return -1;
    }
    
    // Count total beatmaps
    processor->total_count = count_beatmaps(beatmaps_path);
    if (processor->total_count < 0) {
        free(processor->beatmaps_path);
        processor->beatmaps_path = NULL;
        return -1;
    }
    
    printf("Found %d beatmap directories to process\n", processor->total_count);
    return 0;
}

void cleanup_processor(beatmap_processor_t *processor) {
    if (processor == NULL) {
        return;
    }
    
    if (processor->beatmaps_path != NULL) {
        free(processor->beatmaps_path);
        processor->beatmaps_path = NULL;
    }
    
    if (processor->output_path != NULL) {
        free(processor->output_path);
        processor->output_path = NULL;
    }
}

void pause_processor(beatmap_processor_t *processor) {
    if (processor != NULL) {
        processor->is_paused = true;
    }
}

void resume_processor(beatmap_processor_t *processor) {
    if (processor != NULL) {
        processor->is_paused = false;
    }
}

void stop_processor(beatmap_processor_t *processor) {
    if (processor != NULL) {
        processor->should_stop = true;
        processor->is_paused = false;
    }
}

int process_beatmaps(beatmap_processor_t *processor) {
    if (processor == NULL) {
        return -1;
    }
    
    DIR *beatmaps_dir = opendir(processor->beatmaps_path);
    if (beatmaps_dir == NULL) {
        fprintf(stderr, "Could not open beatmap directory %s: %s\n", 
                processor->beatmaps_path, strerror(errno));
        return -1;
    }
    
    printf("Opened beatmap directory %s\n", processor->beatmaps_path);
    
    struct dirent *entry;
    while ((entry = readdir(beatmaps_dir)) != NULL && !processor->should_stop) {
        // Handle pause
        while (processor->is_paused && !processor->should_stop) {
            usleep(100000); // Sleep for 100ms
        }
        
        if (processor->should_stop) {
            break;
        }
        
        // Skip . and .. entries
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        
        char beatmap_path[1024];
        snprintf(beatmap_path, sizeof(beatmap_path), "%s/%s", processor->beatmaps_path, entry->d_name);
        
        if (!is_directory(beatmap_path)) {
            continue;
        }
        
        printf("Processing beatmap: %s (%d/%d)\n", entry->d_name, 
               processor->processed_count + 1, processor->total_count);
        
        // Create zip file path
        char zip_path[1024];
        snprintf(zip_path, sizeof(zip_path), "%s/%s.osz", processor->output_path, entry->d_name);
        
        // Create zip from directory using libzip
        if (create_zip_from_directory(beatmap_path, zip_path) == 0) {
            printf("Successfully zipped: %s\n", entry->d_name);
            
            // Open the .osz file (equivalent to the original 'open' command)
            char open_command[1024];
            snprintf(open_command, sizeof(open_command), "open '%s'", zip_path);
            if (system(open_command) != 0) {
                fprintf(stderr, "Warning: Could not open %s\n", zip_path);
            }
        } else {
            fprintf(stderr, "Error zipping beatmap: %s\n", entry->d_name);
        }
        
        processor->processed_count++;
        
        // Small delay to allow signal handling
        usleep(10000); // 10ms
    }
    
    closedir(beatmaps_dir);
    
    if (processor->should_stop) {
        printf("Processing stopped by user. Processed %d/%d beatmaps.\n", 
               processor->processed_count, processor->total_count);
    } else {
        printf("Processing completed. Processed %d/%d beatmaps.\n", 
               processor->processed_count, processor->total_count);
    }
    
    return 0;
}
