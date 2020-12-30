#ifndef CLIENT_H
#define CLIENT_H
    #include <stdint.h>
    #define SECTION_SIZE 2048
    struct package{
        uint32_t package_len;
        uint32_t filename_len;
        uint32_t file_content_len;
        char *filename;
        char *file_content;
    };
#endif