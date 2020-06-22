//
// Created by luoyuanzhen on 2020/6/17.
//

#include "ftp_client.h"

int control_fd;

int way_code=PORT;

int main(int argc,char* argv[]){

    if(argc!=2){
        error_exit("using command:./ftp_client PORT");
    }

    int port=atoi(argv[1]);//port use for the PORT way.

    if((control_fd=create_connect_socket(default_server_port,"127.0.0.1"))<0){
        error_exit("create control socket failed.");
    }

    int retcode=response_login();
    //printf("get retcode:%d\n",retcode);
    switch(retcode){
        case ADMIT:printf("welcome!\n");break;
        case REFUSE:error_exit("login failed.");break;
        default:error_exit("get retcode failed.");
    }


    while(1){

        char command[N];
        memset(command,0,N);

        printf("<FTP Client>:");
        //fflush(stdout);

        getstd(command, N);

        /**if change the connect way.
         * There is no need to send to server**/
        if(strcmp(command,PORT_WAY)==0){
            way_code=PORT;
            continue;
        }
        else if(strcmp(command,PASV_WAY)==0){
            way_code=PASV;
            continue;
        }
        else if(strcmp(command,CWAY)==0){
            see_connect_way(port);
            continue;
        }

        /**send command to server**/
        if(send_data_once(control_fd,command)<0){
            close(control_fd);
            error_exit("client send command failed.");
        }

        /**get command's code.**/
        int command_retcode=get_command_retcode();
        //printf("command:%s,get code:%d\n",command,command_retcode);
        if(command_retcode<0){
            printf("unknown command.try ?\n");
        }
        else if(command_retcode==QUIT_C){
            close(control_fd);
            error_exit("bye!\n");
        }
        else {
            int data_fd=client_choose_cway(port);
            //printf("data_fd:%d\n",data_fd);
            switch (command_retcode) {
                case HELP_C:
                case DIR_C:client_gets(data_fd);break;
                case FILE_C:
                    if(strncmp(command,GET,3)==0){
                        client_getf(data_fd,command);
                    }
                    else if(strncmp(command,PUT,3)==0){
                        client_putf(data_fd,command);
                    }
                    else if(strncmp(command,RM,2)==0){
                        client_gets(data_fd);
                    }
                    break;
            }
            close(data_fd);
        }
    }
}

int response_login(){
    char message[N];

    printf("welcome to tinyFTP.\n");
    printf("Username:");

    getstd(message,N);

    printf("get username from stdin:%s,len:%d\n",message,strlen(message));

    /**waiting for the READY code from server**/
    char code[N];
    recv(control_fd,code,N,0);

    /**send username**/
    if(send_data_once(control_fd,message)<0){
        return -1;
    }

    /**waitring for the READY code from server**/
    recv(control_fd,code,N,0);

    bzero(message,strlen(message));

    printf("Password:");
    /**get password**/
    getstd(message,N);
    printf("get password:%s,len:%d\n",message,strlen(message));
    /**send password**/
    if(send_data_once(control_fd,message)<0){
        return -1;
    }

    /**get reply code from server**/
    return get_command_retcode();
}

int get_command_retcode(){
    char s_retcode[N];
    memset(s_retcode,0,N);

    if(recv(control_fd,s_retcode,N,0)<0){
        error_exit("receive reply code failed.");
    }

    //printf("get retcode first time:%s\n",s_retcode);
    return atoi(s_retcode);
}

/**param port use for port**/
int client_choose_cway(int port){
    if(way_code==PASV){
        return client_data_connect();
    }
    else{
        return client_data_open(port);
    }
}
/**use for cilent PASV
 * server should send a port to connect**/
int client_data_connect(){

    /**send PASV ask for server connect port**/
    char pasv_code[N];
    sprintf(pasv_code,"%d",PASV);
    if(send_data_once(control_fd,pasv_code)<0){
        error_exit("send pasv failed.");
    }

    /**waiting for the port from server**/
    int s_port=get_command_retcode();

    sleep(1);
    /**connect the server by s_port**/
    struct sockaddr_in server_addr;
    socklen_t len= sizeof(server_addr);
    char host_ip[N];

    memset(host_ip,0,N);

    getpeername(control_fd,(struct sockaddr*)&server_addr,&len);//get server's addr;
    inet_ntop(AF_INET,&server_addr.sin_addr,host_ip, sizeof(host_ip));//get server's host ip;

    int data_fd;
    if((data_fd=create_connect_socket(s_port,host_ip))<0){
        return -1;
    }

    return data_fd;
}
/**use for client PORT
 * port:owner listening port**/
int client_data_open(int port){

    int listen_fd=create_listen_socket(port);

    /**send port PORT for server connect port**/
    char port_code[N];
    sprintf(port_code,"%d %d",PORT,port);
    if(send_data_once(control_fd,port_code)<0){
        error_exit("send pasv failed.");
    }

    int data_fd=accept_client_fd(listen_fd);
    close(listen_fd);
    return data_fd;
}

void client_gets(int data_fd){

    int status=0;
    /**waiting for START from server**/
    if(recv(control_fd,&status, sizeof(status),0)<0){
        printf("receive status START failed.\n");
        return;
    }

    char buffer[N];
    memset(buffer,0,N);
    int flag=0;
    /**receive data from server**/
    while((flag=recv(data_fd,buffer,N,0))>0){
        printf("%s",buffer);
        memset(buffer,0,N);
    }

    printf("\n");
    /**waiting for OK from server**/
    if(recv(control_fd,&status, sizeof(status),0)<0){
        printf("receive status START failed.\n");
        return;
    }

}

void client_getf(int data_fd,char* command){

    char filename[N];
    get_content(filename,command,3);

    int status=0;
    /**waiting for START from server**/
    if(recv(control_fd,&status, sizeof(status),0)<0){
        printf("receive status START failed.\n");
        return;
    }

    FILE* fd=NULL;
    if((fd=fopen(filename,"w"))==NULL){
        printf("can't open file:%s\n",filename);
    }

    char buffer[N];
    memset(buffer,0,N);
    int size=0;
    /**receive data from server**/
    while((size=recv(data_fd,buffer,N,0))>0){
        //printf("get file content:%s\n",buffer);
        fwrite(buffer,1,size,fd);
    }

    /**waiting for OK from server**/
    if(recv(control_fd,&status, sizeof(status),0)<0){
        printf("receive status START failed.\n");
        return;
    }
    fclose(fd);
}

void client_putf(int data_fd,char* command){

    char filename[N];
    get_content(filename,command,strlen(PUT));

    FILE* fd=NULL;
    if((fd=fopen(filename,"r"))==NULL){//invalid filename,send error message.
        printf("invalid filename:%s\n",filename);

        char buffer[N];
        sprintf(buffer,"%s","stop transfer.caused by client.\n");

        control_response(control_fd,STOP);

        if(send_data_once(data_fd,buffer)<0){
            error_exit("send error_filename failed.");
        }

    }else{//correct filename,send file.
        size_t len;

        control_response(control_fd,START);

        char buffer[N];
        do{
            if((len=fread(buffer,1,N,fd))<0){
                printf("read filename failed.\n");
            }

            printf("send file content:%s\n",buffer);
            if(send(data_fd,buffer,len,0)<0){
                error_exit("send file failed.\n");
            }
        }while(len==N);

        fclose(fd);
    }


}
void see_connect_way(int port){
    if(way_code==PORT){
        printf("connect way:port <%d>\n",port);
    }
    if(way_code==PASV){
        printf("connect way:pasv\n");
    }
}
