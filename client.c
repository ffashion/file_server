#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include<arpa/inet.h>
#include <pthread.h>
#include <unistd.h>
#include <stdint.h>
#include "client.h"

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
    fp = fopen(new_filename,"wb+");

    receive_package.file_content = malloc(receive_package.file_content_len);

    //记录已读如内存的2048个数
    //int i = 0;
    
    int get_file_content_section_num = receive_package.file_content_len / 2048; //以2048分片
    int Last_bytes = receive_package.file_content_len % 2048; 
    
    for(int i=0;i<=get_file_content_section_num -1;i++){
        if(read(client_fd,(uint8_t *)receive_package.file_content + 2048*i,2048) == -1){
            printf("read error");
        }
    }

    if(read(client_fd,(uint8_t *)receive_package.file_content + 2048*get_file_content_section_num,Last_bytes) == -1){
            printf("read error");
    }
    
    // while(1){
    //     if(receive_package.file_content_len )

    //     if(read(client_fd,(uint8_t *)receive_package.file_content + 2048*i,2048) == -1){
            
    //         read(client_fd,(uint8_t *)receive_package.file_content + 2048*i,receive_package.file_content_len-2048*i);
    //         i++;
    //         //exit(-1);
    //     }else{
    //         i++;
    //     }

    //     if(2048*i >= receive_package.file_content_len){
    //         break;
    //     }
        
    // }
    
    if(fwrite(receive_package.file_content,1,receive_package.file_content_len,fp) == -1){
        printf("fwrite error");
    }
    

    close(client_fd);
    free(receive_package.filename);
    free(receive_package.file_content);
    return 0;
    
}