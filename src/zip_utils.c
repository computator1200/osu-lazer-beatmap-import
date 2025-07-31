#include "zip_utils.h"
#include <zip.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

bool is_directory(const char *path) {
    struct stat statbuf;
    if (stat(path, &statbuf) != 0) {
        return false;
    }
    return S_ISDIR(statbuf.st_mode);
}

static int add_file_to_zip(zip_t *archive, const char *file_path, const char *archive_path) {
    zip_source_t *source = zip_source_file(archive, file_path, 0, 0);
    if (source == NULL) {
        fprintf(stderr, "Error creating source for file %s: %s\n", file_path, zip_strerror(archive));
        return -1;
    }
    
    zip_int64_t index = zip_file_add(archive, archive_path, source, ZIP_FL_ENC_UTF_8);
    if (index < 0) {
        fprintf(stderr, "Error adding file %s to archive: %s\n", file_path, zip_strerror(archive));
        zip_source_free(source);
        return -1;
    }
    
    return 0;
}

static int add_directory_recursive(zip_t *archive, const char *dir_path, const char *base_archive_path) {
    DIR *dir = opendir(dir_path);
    if (dir == NULL) {
        perror("opendir");
        return -1;
    }
    
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        // Skip . and .. entries
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        
        char full_path[1024];
        char archive_path[1024];
        
        snprintf(full_path, sizeof(full_path), "%s/%s", dir_path, entry->d_name);
        snprintf(archive_path, sizeof(archive_path), "%s/%s", base_archive_path, entry->d_name);
        
        if (is_directory(full_path)) {
            // Add directory to archive
            zip_int64_t dir_index = zip_dir_add(archive, archive_path, ZIP_FL_ENC_UTF_8);
            if (dir_index < 0) {
                fprintf(stderr, "Error adding directory %s to archive: %s\n", archive_path, zip_strerror(archive));
                closedir(dir);
                return -1;
            }
            
            // Recursively add contents
            if (add_directory_recursive(archive, full_path, archive_path) != 0) {
                closedir(dir);
                return -1;
            }
        } else {
            // Add file to archive
            if (add_file_to_zip(archive, full_path, archive_path) != 0) {
                closedir(dir);
                return -1;
            }
        }
    }
    
    closedir(dir);
    return 0;
}

int create_zip_from_directory(const char *source_dir, const char *zip_path) {
    int error;
    zip_t *archive = zip_open(zip_path, ZIP_CREATE | ZIP_TRUNCATE, &error);
    
    if (archive == NULL) {
        zip_error_t zip_error;
        zip_error_init_with_code(&zip_error, error);
        fprintf(stderr, "Error opening zip file %s: %s\n", zip_path, zip_error_strerror(&zip_error));
        zip_error_fini(&zip_error);
        return -1;
    }
    
    // Get the base name of the source directory for the archive root
    char *base_name = strrchr(source_dir, '/');
    if (base_name == NULL) {
        base_name = (char *)source_dir;
    } else {
        base_name++; // Skip the '/'
    }
    
    // Add the directory and its contents to the archive
    if (add_directory_recursive(archive, source_dir, base_name) != 0) {
        zip_close(archive);
        return -1;
    }
    
    if (zip_close(archive) != 0) {
        fprintf(stderr, "Error closing zip file %s: %s\n", zip_path, zip_strerror(archive));
        return -1;
    }
    
    return 0;
}

int add_directory_to_zip(const char *dir_path, const char *zip_path, const char *base_name __attribute__((unused))) {
    return create_zip_from_directory(dir_path, zip_path);
}
