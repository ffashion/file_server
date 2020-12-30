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
    char *ptr = vptr;
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


int main(int argc,char *args[]){
    FILE *fp = NULL;
    
    struct sockaddr_in client = {0};
    client.sin_family =  AF_INET;
    client.sin_port = htons(8082);
    client.sin_addr.s_addr = inet_addr("192.168.123.144");
    int client_fd = socket(AF_INET,SOCK_STREAM,0);
    
    struct package receive_package = {0};
    receive_package.filename = malloc(1024);


    if(connect(client_fd,(struct sockaddr *)&client,sizeof(client)) == -1){
        printf("连接服务器失败");
    }
    

    read(client_fd,&receive_package.package_len,4);
    
    printf("package len:%d\n",receive_package.package_len);
    
    read(client_fd,&receive_package.filename_len,4);
    printf("filename len:%d\n",receive_package.filename_len);
    
    read(client_fd,&receive_package.file_content_len,4);
    printf("file content len:%d\n",receive_package.file_content_len);
    
    read(client_fd,receive_package.filename,receive_package.filename_len);
    printf("filename :%s\n",receive_package.filename);

    char *new_filename = malloc(100);
    strcat(new_filename,"new_");
    strcat(new_filename,receive_package.filename);
    fp = fopen(new_filename,"wb");

    receive_package.file_content = malloc(receive_package.file_content_len);

    
    char *buffer = malloc(2048);
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