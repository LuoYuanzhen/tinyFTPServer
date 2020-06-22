//
// Created by luoyuanzhen on 2020/6/17.
//

#include "util_socket.h"

void error_exit(char* error){
    printf(error);
    printf("\n");
    exit(-1);
}

int create_listen_socket(int port){
    int fd,code=1;
    struct sockaddr_in addr;

    if((fd=socket(AF_INET,SOCK_STREAM,0))<0){
        printf("socket failed.\n");
        return -1;
    }

    addr.sin_port=htons(port);
    addr.sin_addr.s_addr=htonl(INADDR_ANY);
    addr.sin_family=AF_INET;

    setsockopt(fd,SOL_SOCKET,SO_REUSEADDR,&code, sizeof(int));

    if(bind(fd,(struct sockaddr *)&addr, sizeof(addr))<0){
        close(fd);
        printf("bind failed.\n");
        return -1;
    }

    //printf("listen\n");
    if(listen(fd,5)<0){
        close(fd);
        printf("listen failed.\n");
        return -1;
    }
    //printf("listen successfully.\n");
    return fd;
}

int accept_client_fd(int fd_listen){
    int fd;
    struct sockaddr_in addr;
    socklen_t len= sizeof(addr);
    if((fd=accept(fd_listen,(struct sockaddr *)&addr, &len))<0){
           printf("accept failed.\n");
           return -1;
    }

    return fd;
}

/**create connect socket:
 * connect host:port **/
int create_connect_socket(int port,char* host){
    struct sockaddr_in addr;
    int fd;

    addr.sin_family=AF_INET;
    addr.sin_addr.s_addr=inet_addr(host);
    addr.sin_port=htons(port);

    if((fd=socket(AF_INET,SOCK_STREAM,0))<0){
        printf("socket failed.\n");
        return -1;
    }
    if(connect(fd,(struct sockaddr *)&addr, sizeof(addr))<0){
        printf("connect failed.\n");
        return -1;
    }
    return fd;
}

/**control link response.
 * parameter code:see "util_socket.h".**/
int control_response(int socket_fd,int code){

    char s_code[N];
    sprintf(s_code,"%d",code);

    printf("s_code:%s\n",s_code);
    if(send(socket_fd,s_code,strlen(s_code),0)<0){
        printf("send failed.\n");
        return -1;
    }

    return 1;
}

/**receive data just once time.
 * receive fd could be control link or data link.**/
int receive_data_once(int fd,char* buffer,int size){

    memset(buffer,0,size);
    int len;
    if((len=recv(fd,buffer,size,0))<0){
        return -1;
    }
    //printf("receive_data buffer:%s,len:%d\n",buffer,strlen(buffer));
    return len;
}

int send_data_once(int fd,char* buffer){

    int maxlen=N;
    while(maxlen<strlen(buffer)){
        maxlen+=N;
    }
    char buf[maxlen];
    memset(buf,0,maxlen);

    sprintf(buf,"%s",buffer);

    //printf("send_data:%s,len%d\n",buf,strlen(buf));
    if(send(fd,buf,strlen(buf),0)<0){
        return -1;
    }
    return 1;
}

void getstd(char* buffer,int size){

    bzero(buffer,size);

    if(fgets(buffer,size,stdin)==NULL){
        printf("fgets failed.\n");
    }

    for(int i=0;i<size;i++){
        if(buffer[i]=='\n'){
            *(buffer+i)='\0';
            break;
        }
    }

}

void copy_string(char* a,char* b){
    for(int i=0;b[i]!=0;i++){
        a[i]=b[i];
    }
}

void get_content(char* content,char* command,int c_len){

    memset(content,0,N);
    int offset=c_len+1;
    int i=0;
    while(command[i]!='\0'){
        *(content+i)=command[i+offset];
        i++;
    }
}