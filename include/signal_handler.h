#ifndef SIGNAL_HANDLER_H
#define SIGNAL_HANDLER_H

#include "beatmap_processor.h"

// Global processor reference for signal handling
extern beatmap_processor_t *g_processor;

// Function declarations
void setup_signal_handlers(beatmap_processor_t *processor);
void signal_handler(int sig);
void print_status(beatmap_processor_t *processor);

#endif // SIGNAL_HANDLER_H
