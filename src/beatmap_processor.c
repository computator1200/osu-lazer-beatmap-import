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
#include <time.h>

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

int init_error_log(beatmap_processor_t *processor) {
    if (processor == NULL) {
        return -1;
    }
    
    // Create log file path
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        fprintf(stderr, "Error getting current working directory for log file\n");
        return -1;
    }
    
    // Generate log filename with timestamp
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    char timestamp[64];
    strftime(timestamp, sizeof(timestamp), "%Y%m%d_%H%M%S", tm_info);
    
    size_t log_path_len = strlen(cwd) + strlen("/beatmap_errors_") + strlen(timestamp) + strlen(".log") + 1;
    processor->log_file_path = malloc(log_path_len);
    if (processor->log_file_path == NULL) {
        fprintf(stderr, "Error allocating memory for log file path\n");
        return -1;
    }
    
    snprintf(processor->log_file_path, log_path_len, "%s/beatmap_errors_%s.log", cwd, timestamp);
    
    // Create/initialize the log file
    FILE *log_file = fopen(processor->log_file_path, "w");
    if (log_file == NULL) {
        fprintf(stderr, "Error creating log file %s: %s\n", processor->log_file_path, strerror(errno));
        free(processor->log_file_path);
        processor->log_file_path = NULL;
        return -1;
    }
    
    // Write header
    fprintf(log_file, "Beatmap Processing Error Log\n");
    fprintf(log_file, "Started: %s", ctime(&now));
    fprintf(log_file, "Beatmaps Directory: %s\n", processor->beatmaps_path);
    fprintf(log_file, "Output Directory: %s\n", processor->output_path);
    fprintf(log_file, "===================================\n\n");
    
    fclose(log_file);
    
    printf("Error log file: %s\n", processor->log_file_path);
    return 0;
}

void log_failed_beatmap(beatmap_processor_t *processor, const char *beatmap_name, const char *error_message) {
    if (processor == NULL || processor->log_file_path == NULL || beatmap_name == NULL || error_message == NULL) {
        return;
    }
    
    FILE *log_file = fopen(processor->log_file_path, "a");
    if (log_file == NULL) {
        fprintf(stderr, "Warning: Could not open log file for writing: %s\n", strerror(errno));
        return;
    }
    
    time_t now = time(NULL);
    char timestamp[64];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", localtime(&now));
    
    fprintf(log_file, "[%s] FAILED: %s\n", timestamp, beatmap_name);
    fprintf(log_file, "  Error: %s\n\n", error_message);
    
    fclose(log_file);
}

int init_processor(beatmap_processor_t *processor, const char *beatmaps_path) {
    if (processor == NULL || beatmaps_path == NULL) {
        return -1;
    }
    
    // Initialize processor state
    processor->beatmaps_path = strdup(beatmaps_path);
    processor->output_path = NULL;
    processor->log_file_path = NULL;
    processor->should_stop = false;
    processor->is_paused = false;
    processor->processed_count = 0;
    processor->failed_count = 0;
    
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
    
    if (processor->log_file_path != NULL) {
        free(processor->log_file_path);
        processor->log_file_path = NULL;
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
        
        // Check if directory is accessible
        DIR *test_dir = opendir(beatmap_path);
        if (test_dir == NULL) {
            char error_msg[256];
            snprintf(error_msg, sizeof(error_msg), "Cannot access beatmap directory: %s", strerror(errno));
            log_failed_beatmap(processor, entry->d_name, error_msg);
            fprintf(stderr, "Error accessing beatmap directory %s: %s\n", entry->d_name, strerror(errno));
            processor->failed_count++;
            processor->processed_count++;
            continue;
        }
        closedir(test_dir);
        
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
                char error_msg[256];
                snprintf(error_msg, sizeof(error_msg), "Failed to open .osz file with system open command");
                log_failed_beatmap(processor, entry->d_name, error_msg);
                fprintf(stderr, "Warning: Could not open %s\n", zip_path);
                processor->failed_count++;
            }
        } else {
            char error_msg[256];
            snprintf(error_msg, sizeof(error_msg), "Failed to create zip file from directory: %s", beatmap_path);
            log_failed_beatmap(processor, entry->d_name, error_msg);
            fprintf(stderr, "Error zipping beatmap: %s\n", entry->d_name);
            processor->failed_count++;
        }
        
        processor->processed_count++;
        
        // Small delay to allow signal handling
        usleep(10000); // 10ms
    }
    
    closedir(beatmaps_dir);
    
    // Write final summary to log
    if (processor->log_file_path != NULL && processor->failed_count > 0) {
        FILE *log_file = fopen(processor->log_file_path, "a");
        if (log_file != NULL) {
            time_t now = time(NULL);
            fprintf(log_file, "===================================\n");
            fprintf(log_file, "Processing completed: %s", ctime(&now));
            fprintf(log_file, "Total processed: %d\n", processor->processed_count);
            fprintf(log_file, "Total failed: %d\n", processor->failed_count);
            fprintf(log_file, "Success rate: %.2f%%\n", 
                    processor->total_count > 0 ? 
                    ((double)(processor->processed_count - processor->failed_count) / processor->total_count) * 100.0 : 0.0);
            fclose(log_file);
        }
    }
    
    if (processor->should_stop) {
        printf("Processing stopped by user. Processed %d/%d beatmaps", 
               processor->processed_count, processor->total_count);
        if (processor->failed_count > 0) {
            printf(" (%d failed)", processor->failed_count);
        }
        printf(".\n");
    } else {
        printf("Processing completed. Processed %d/%d beatmaps", 
               processor->processed_count, processor->total_count);
        if (processor->failed_count > 0) {
            printf(" (%d failed)", processor->failed_count);
        }
        printf(".\n");
    }
    
    if (processor->failed_count > 0) {
        printf("Failed beatmaps logged to: %s\n", processor->log_file_path);
    }
    
    return 0;
}
