#include "signal_handler.h"
#include <signal.h>
#include <stdio.h>
#include <unistd.h>

beatmap_processor_t *g_processor = NULL;

void setup_signal_handlers(beatmap_processor_t *processor) {
    g_processor = processor;
    signal(SIGINT, signal_handler);  // Ctrl+C
    signal(SIGTERM, signal_handler); // Termination signal
    signal(SIGUSR1, signal_handler); // Pause signal
    signal(SIGUSR2, signal_handler); // Resume signal
}

void signal_handler(int sig) {
    if (g_processor == NULL) {
        return;
    }
    
    switch (sig) {
        case SIGINT:
        case SIGTERM:
            printf("\nReceived termination signal. Stopping gracefully...\n");
            stop_processor(g_processor);
            break;
        case SIGUSR1:
            printf("\nPausing processor...\n");
            pause_processor(g_processor);
            print_status(g_processor);
            break;
        case SIGUSR2:
            printf("\nResuming processor...\n");
            resume_processor(g_processor);
            break;
        default:
            break;
    }
}

void print_status(beatmap_processor_t *processor) {
    if (processor == NULL) {
        return;
    }
    
    printf("Status: %s | Processed: %d/%d beatmaps\n", 
           processor->is_paused ? "PAUSED" : (processor->should_stop ? "STOPPING" : "RUNNING"),
           processor->processed_count, 
           processor->total_count);
    
    if (processor->is_paused) {
        printf("Send SIGUSR2 to resume or SIGINT/SIGTERM to stop\n");
        printf("Process ID: %d\n", getpid());
    }
}
