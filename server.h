#ifndef SERVER_H
#define SERVER_H
    #define LISTEN_PORT 8082
    //#define LISTEN_ADDR INADDR_ANY; //绑定所有ip地址
    #define LISTEN_ADDR "192.168.123.144"
    #define SECTION_SIZE 2048
    struct package{
        uint32_t package_len;
        uint32_t filename_len;
        uint32_t file_content_len;
        char *filename;
        char *file_content;
    };
#endif