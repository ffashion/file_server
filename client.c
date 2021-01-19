#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include<arpa/inet.h>
#include <pthread.h>
#include <unistd.h>
#include <stdint.h>
#include <errno.h>
#include "client.h"
//保证读取数据的完整性
int read_n(int fd,void *vptr,size_t n){
    //记录还差多少字节读完
    size_t nleft = n;
    //已经每次read的数量
    ssize_t nread;
    char *ptr = (char *)vptr;
    //判断nleft的大小
    while (nleft > 0) {
        if ( (nread = read(fd, ptr, nleft)) < 0) {
            if (errno == EINTR)
                nread = 0;        /* and call read() again */
            else
                return -1;
        } else if (nread == 0){
            break;         /* EOF */
        }
                   

        nleft -= nread;
        ptr   += nread;
    }
    return n - nleft;     /* return >= 0 */
 
}

int connect2server(char *addr,int port){
    if(addr == NULL || port <= 0){
        return -1;
    }
    struct sockaddr_in client = {0};
    int client_fd;
    client.sin_family = AF_INET;
    client.sin_addr.s_addr = inet_addr(addr);
    client.sin_port = htons(port);
    client_fd = socket(AF_INET,SOCK_STREAM,0);
    if(connect(client_fd,(struct sockaddr *)&client,sizeof(client)) == -1){
        perror("连接服务器失败");
        return -1;
    }
    return client_fd;
}
int main(int argc,char *argv[]){
    char *addr  = NULL;
    int port = 0;
    FILE *fp = NULL;
    int client_fd;
    if(argc <= 1){
        printf("Usage: ./client [ip] [port]\n");
        printf("连接到127.0.0.1:8082\n");
        addr = DEFAULT_ADDR;
        port = DEFAULT_PORT;
    }else if(argc == 2){
        printf("连接到默认端口8082\n");
        addr = argv[1];
        port = DEFAULT_PORT;
    }else if(argc == 3){
        //printf("连接到指定ip和端口\n");
        addr = argv[1];
        port = atoi(argv[2]);
    }
    client_fd = connect2server(addr,port);


    struct package receive_package = {0};
    receive_package.filename = (char *)malloc(1024);
    
    read(client_fd,&receive_package.package_len,4);
    
    printf("package len:%d\n",receive_package.package_len);
    
    read(client_fd,&receive_package.filename_len,4);
    printf("filename len:%d\n",receive_package.filename_len);
    
    read(client_fd,&receive_package.file_content_len,4);
    printf("file content len:%d\n",receive_package.file_content_len);
    
    read(client_fd,receive_package.filename,receive_package.filename_len);
    printf("filename :%s\n",receive_package.filename);

   
    
    char *filename = (char *)malloc(100);
    #ifdef DEBUG_CLIENT
        strcat(filename,"new_");
    #endif
    strcat(filename,receive_package.filename);
    
    fp = fopen(filename,"wb");
    
   

    receive_package.file_content = (char *)malloc(receive_package.file_content_len);

    
    char *buffer = (char *)malloc(2048);
    memset(buffer,0,2048);
    int file_content_section_num = receive_package.file_content_len / SECTION_SIZE; //以2048分片
    int last_bytes = receive_package.file_content_len % SECTION_SIZE;
    
    
    for(int i=0;i<=file_content_section_num-1;i++){
        if(read_n(client_fd,(uint8_t *)receive_package.file_content+SECTION_SIZE*(i),SECTION_SIZE) == -1){
            printf("read error\n");
        }
    }
    if(read_n(client_fd,(uint8_t *)receive_package.file_content+SECTION_SIZE*file_content_section_num,last_bytes) == -1){
            printf("read error\n");
    }


    
    if(fwrite(receive_package.file_content,1,receive_package.file_content_len,fp) == -1){
        printf("fwrite error\n");
    }
    

    close(client_fd);
    free(receive_package.filename);
    free(receive_package.file_content);
    return 0;
    
}