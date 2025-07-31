/* zip error strings */

const char * const _zip_err_str[] = {
    "No error",
    "Multi-disk zip archives not supported",
    "Renaming temporary file failed",
    "Closing zip archive failed",
    "Seek error",
    "Read error",
    "Write error",
    "CRC error",
    "Containing zip archive was closed",
    "No such file",
    "File already exists",
    "Can't open file",
    "Failure to create temporary file",
    "Zlib error",
    "Malloc failure",
    "Entry has been changed",
    "Compression method not supported",
    "Premature end of file",
    "Invalid argument",
    "Not a zip archive",
    "Internal error",
    "Zip archive inconsistent",
    "Can't remove file",
    "Entry has been deleted",
    "Encryption method not supported",
    "Read-only archive",
    "No password provided",
    "Wrong password provided",
    "Operation not supported",
    "Resource still in use",
    "Tell error",
    "Compressed data invalid",
    "Cancelled",
    "Operation cancelled",
    NULL
};

const int _zip_err_str_count = sizeof(_zip_err_str) / sizeof(_zip_err_str[0]) - 1;

const char * const _zip_err_details[] = {
    "No additional details",
    NULL
};

const int _zip_err_details_count = sizeof(_zip_err_details) / sizeof(_zip_err_details[0]) - 1;
