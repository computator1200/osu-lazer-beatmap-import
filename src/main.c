#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "beatmap_processor.h"
#include "signal_handler.h"
#include "utils.h"

int main(int argc, char **argv) {
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }
    
    // Initialize processor
    beatmap_processor_t processor;
    if (init_processor(&processor, argv[1]) != 0) {
        fprintf(stderr, "Error initializing processor\n");
        return 1;
    }
    
    // Create output directory
    if (create_output_directory(NULL, &processor.output_path) != 0) {
        cleanup_processor(&processor);
        return 1;
    }
    
    // Setup signal handlers for pause/stop functionality
    setup_signal_handlers(&processor);
    
    printf("Starting beatmap processing...\n");
    printf("Process ID: %d\n", getpid());
    printf("Use Ctrl+C to stop gracefully\n");
    printf("Use 'kill -USR1 %d' to pause\n", getpid());
    printf("Use 'kill -USR2 %d' to resume\n", getpid());
    printf("\n");
    
    // Process beatmaps
    int result = process_beatmaps(&processor);
    
    // Cleanup
    cleanup_processor(&processor);
    
    return result == 0 ? 0 : 1;
}
