#include <sys/socket.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <unistd.h>
#include <netdb.h>
#include <errno.h>
#include <openssl/md5.h>
#include <fcntl.h> 
#include <sys/stat.h>
#include <sys/mman.h>
#include "server.h"
char *get_file_open(char *filename,size_t *file_size_p,int *fd_p){
    struct stat file_stat;
    char *result;
    if((*fd_p = open(filename,O_RDWR)) == -1){
        perror("open");
        exit(-1);
    }
    if(stat(filename,&file_stat) == -1){
        perror("stat");
        exit(-1);
    }
    *file_size_p= file_stat.st_size;
    if( (result = mmap(NULL,*file_size_p,PROT_READ,MAP_PRIVATE,*fd_p,0)) == NULL){
        perror("mmap");
        exit(-1);
    }
    return result;
}

int get_file_close(char *addr,size_t length,int fd){
    //参数1传get_file_open的返回值 参数2传文件的长度
    if(munmap(addr,length) == -1){
        perror("munmap");
        return -1;
    }
    close(fd);
    return 0;
}
int server_listen(int port){
    struct sockaddr_in server = {0};
    server.sin_family =  AF_INET;
    server.sin_addr.s_addr = inet_addr(LISTEN_ADDR);
    if(port == 0){
        server.sin_port = htons(LISTEN_PORT);
    }else{
        server.sin_port = htons(port);
    }
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
//未使用 
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
//未使用
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
    //fread(file_buf,1,file_size,fp);
    fread(file_buf,file_size,1,fp);
    fclose(fp);
    return file_buf;

}
//自动write 第一次write不够的字节
//保证完整写入n个字节
int write_n(int fd,void *vptr,size_t n){
    //
    size_t nleft = n;
    //
    ssize_t nwritten = 0;
    char *ptr = vptr;
    while(nleft > 0 ){
        if( (nwritten = write(fd,ptr,nleft)) <= 0 ){
            if(nwritten < 0 && errno == EINTR){
                nwritten = 0; //写入0字节 ，call again
            }else{
                return -1;//error
            }
        }
        nleft -= nwritten;
        ptr += nwritten;
    }

    return n;

}

//TODO
int md5_compute(){
    
    return 0;
}
int process_once_request(int server_fd,size_t file_size,char *filename,char *file_content_addr){
        
        int client_fd = -1;
        struct package package = {0};
        package.package_len = 4 + 4 + 4 + strlen(filename)+1 + file_size;
        package.filename_len = strlen(filename)+1;
        package.file_content_len = file_size;
        package.filename = filename;
        package.file_content = file_content_addr;
        
        int file_content_section_num = package.file_content_len / SECTION_SIZE; //以2048分片
        int last_bytes = package.file_content_len % SECTION_SIZE;
        
        client_fd= accept_request(server_fd);


        write(client_fd,&package.package_len,4);
        write(client_fd,&package.filename_len,4);
        write(client_fd,&package.file_content_len,4);
        write(client_fd,package.filename,package.filename_len);
        
        for(int i=0;i<=file_content_section_num-1;i++){
            if(write_n(client_fd,(uint8_t *)package.file_content+SECTION_SIZE*i,SECTION_SIZE) == -1){
                printf("write error\n");
            }
        }
        if(write_n(client_fd,(uint8_t *)package.file_content+SECTION_SIZE*file_content_section_num,last_bytes) == -1){
                printf("write error\n");
        }

        shutdown(client_fd,SHUT_RDWR);//直到客户端拿到数据再关闭socket
        close(client_fd);
        printf("文件传输完成\n");
}
int main(int argc,char *argv[]){
    char *filename = NULL;
    int port;
    if(argc == 1){
        printf("Usage: ./server filename [port]\n");
        exit(-1);
    }else if(argc == 2){
        filename = argv[1];
        port = 0;
    }else if(argc == 3){
        filename = argv[1];
        port = atoi(argv[2]);
    }
    int server_fd = server_listen(port);
    size_t file_size;
    int fd;
    char *file_buffer;
    file_buffer = get_file_open(filename,&file_size,&fd);

    while(1){
        process_once_request(server_fd,file_size,filename,file_buffer);
    }
    //这里没啥用 2333 
    get_file_close(file_buffer,file_size,fd);
    
    
}
