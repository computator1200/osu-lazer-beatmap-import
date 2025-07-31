#ifndef BEATMAP_PROCESSOR_H
#define BEATMAP_PROCESSOR_H

#include <stdbool.h>

// Structure to hold processor state
typedef struct {
    char *beatmaps_path;
    char *output_path;
    char *log_file_path;
    volatile bool should_stop;
    volatile bool is_paused;
    int processed_count;
    int failed_count;
    int total_count;
} beatmap_processor_t;

// Function declarations
int init_processor(beatmap_processor_t *processor, const char *beatmaps_path);
int process_beatmaps(beatmap_processor_t *processor);
void cleanup_processor(beatmap_processor_t *processor);
void pause_processor(beatmap_processor_t *processor);
void resume_processor(beatmap_processor_t *processor);
void stop_processor(beatmap_processor_t *processor);
int count_beatmaps(const char *beatmaps_path);
void log_failed_beatmap(beatmap_processor_t *processor, const char *beatmap_name, const char *error_message);
int init_error_log(beatmap_processor_t *processor);

#endif // BEATMAP_PROCESSOR_H
