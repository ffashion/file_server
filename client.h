#ifndef CLIENT_H
#define CLIENT_H
    #include <stdint.h>
    struct package{
        uint32_t package_len;
        uint32_t filename_len;
        uint32_t file_content_len;
        char *filename;
        char *file_content;
    };
#endif