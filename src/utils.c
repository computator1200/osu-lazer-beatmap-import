#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#include <sys/syslimits.h>

void print_usage(const char *program_name) {
    fprintf(stderr, "Usage: %s [beatmap directory]\n", program_name);
    printf("Directory path must NOT contain trailing slash\n");
    printf("\nControls:\n");
    printf("  Ctrl+C or SIGTERM: Stop processing gracefully\n");
    printf("  SIGUSR1: Pause processing\n");
    printf("  SIGUSR2: Resume processing\n");
    printf("\nExample signals:\n");
    printf("  kill -USR1 <pid>  # Pause\n");
    printf("  kill -USR2 <pid>  # Resume\n");
    printf("  kill -TERM <pid>  # Stop gracefully\n");
}

int create_output_directory(const char *base_path __attribute__((unused)), char **output_path) {
    char cwd[PATH_MAX];
    
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        fprintf(stderr, "Error getting current working directory: %s\n", strerror(errno));
        return -1;
    }
    
    // Create output directory path
    size_t path_len = strlen(cwd) + strlen("/beatmap_output") + 1;
    *output_path = malloc(path_len);
    if (*output_path == NULL) {
        fprintf(stderr, "Error allocating memory for output path\n");
        return -1;
    }
    
    snprintf(*output_path, path_len, "%s/beatmap_output", cwd);
    
    // Create the directory
    if (mkdir(*output_path, S_IRWXU | S_IRWXG | S_IRWXO) != 0) {
        if (errno != EEXIST) {
            fprintf(stderr, "Error creating output directory %s: %s\n", *output_path, strerror(errno));
            free(*output_path);
            *output_path = NULL;
            return -1;
        }
    }
    
    printf("Output directory: %s\n", *output_path);
    return 0;
}
