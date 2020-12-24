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
    receive_package.filename = malloc(100);
    receive_package.file_content = malloc(10000);
    if(connect(client_fd,(struct sockaddr *)&client,sizeof(client)) == -1){
        printf("连接服务器失败");
    }
    

    read(client_fd,&receive_package.package_len,4);
    printf("%d\n",receive_package.package_len);
    
    read(client_fd,&receive_package.filename_len,4);
    printf("%d\n",receive_package.filename_len);
    
    read(client_fd,&receive_package.file_content_len,4);
    printf("%d\n",receive_package.file_content_len);
    
    read(client_fd,receive_package.filename,receive_package.filename_len);
    printf("%s\n",receive_package.filename);

    read(client_fd,receive_package.file_content,receive_package.file_content_len);
    fp = fopen(receive_package.filename,"w+");
    fwrite(receive_package.file_content,1,receive_package.file_content_len,fp);
    

    close(client_fd);
    free(receive_package.filename);
    free(receive_package.file_content);
    return 0;
    
}