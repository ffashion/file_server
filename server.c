#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include<arpa/inet.h>
#include <pthread.h>
#include <unistd.h>
#include <netdb.h>
#include "server.h"
int server_listen(){
    struct sockaddr_in server = {0};
    server.sin_family =  AF_INET;
    server.sin_port = htons(LISTEN_PORT);
    server.sin_addr.s_addr = inet_addr(LISTEN_ADDR);
    int server_fd = -1;
    if( (server_fd = socket(AF_INET,SOCK_STREAM,0)) == -1){
        printf("socket create error\n");
        exit(-1);
    }
    printf("socket create ok\n");
    if(bind(server_fd,(struct sockaddr *)&server,sizeof(server)) == -1){
        printf("socket bind error\n");
        exit(-1);
    }
    printf("socket bind ok\n");
    
    if(listen(server_fd,4) == -1){
        printf("listen error\n");
        exit(-1);
    }
    printf("listen ok\n");
    return server_fd;
}
int accept_request(int server_fd){
    struct sockaddr_in tmp_sock = {0};
    tmp_sock.sin_family = AF_INET;
    tmp_sock.sin_port = 0; //accept设定随机端口
    tmp_sock.sin_addr.s_addr = inet_addr(LISTEN_ADDR);
    socklen_t tmp_sock_len = sizeof(tmp_sock);
    int client_fd = accept(server_fd,(struct sockaddr*)&tmp_sock,&tmp_sock_len);
    if(client_fd == -1){
        printf("accept error\n");
        exit(-1);
    }
    printf("accept ok\n");
    return client_fd;
}
long get_file_size(char *filename){
    FILE *fp = NULL;
    
    if((fp = fopen(filename,"rb")) == NULL){
        printf("获取文件失败");
        return -1;
    }
    fseek(fp,0,SEEK_END);
    //获取文件大小
    long file_size = ftell(fp);
    fclose(fp);
    return file_size;
}
char  *set_file2buf(char *filename){
    FILE *fp = NULL;
    long file_size;
    char *file_buf;
    if((fp = fopen(filename,"rb")) == NULL){
        printf("文件打开失败\n");
        exit(-1);
    }
    file_size = get_file_size(filename);
    file_buf = malloc(file_size);
    
    memset(file_buf,0,file_size);
    rewind(fp);//设置当前的读取偏移位置为文件开头
    fread(file_buf,1,file_size,fp);
    
    fclose(fp);
    return file_buf;

}

int main(int argc,char *args[]){
     char *filename = NULL;
    if(argc >= 2){
        filename = args[1];
    }else{
        printf("请指定要传输的文件\n");
        exit(-1);
    }
    
    int server_fd = server_listen();
    int client_fd = -1;
    struct package package = {0};
    long file_size = get_file_size(filename);
    int def_sock_buf_size = -1;
    socklen_t opt_len;
    
    //pthread_create();
    while(1){
        client_fd= accept_request(server_fd);
        getsockopt(client_fd,SOL_SOCKET, SO_SNDBUF,&def_sock_buf_size,&opt_len);
        
        
        package.package_len = 4 + 4 + 4 + strlen(filename)+1 + file_size;
        package.filename_len = strlen(filename) +1;
        package.file_content_len = file_size;
        package.filename = filename;
        package.file_content = set_file2buf(filename);
        

        write(client_fd,&package.package_len,4);
        write(client_fd,&package.filename_len,4);
        write(client_fd,&package.file_content_len,4);
        write(client_fd,package.filename,strlen(filename)+1);
        
        int get_file_content_section_num = package.file_content_len / 2048; //以2048分片
        int Last_bytes = package.file_content_len % 2048; 
        
        for(int i=0;i<=get_file_content_section_num -1;i++){
            if(write(client_fd,(uint8_t *)package.file_content + 2048*i,2048) == -1){
                printf("write error");
            }
        }

        if(write(client_fd,(uint8_t *)package.file_content + 2048*get_file_content_section_num,Last_bytes) == -1){
                printf("write error");
        }
        //for test 说明内存内容每次,socket 套接字传输有问题
        // FILE * fp_test = NULL;
        // fp_test = fopen("what.mp4","wb+");;
        // fwrite(package.file_content,1,package.file_content_len,fp_test);
        
        close(client_fd);
    }
    
}
