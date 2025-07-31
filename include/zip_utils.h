#ifndef ZIP_UTILS_H
#define ZIP_UTILS_H

#include <stdbool.h>

// Function declarations for zip utilities
int create_zip_from_directory(const char *source_dir, const char *zip_path);
int add_directory_to_zip(const char *dir_path, const char *zip_path, const char *base_name);
bool is_directory(const char *path);

#endif // ZIP_UTILS_H
