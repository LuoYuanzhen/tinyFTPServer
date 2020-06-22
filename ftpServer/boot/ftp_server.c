//
// Created by luoyuanzhen on 2020/6/17.
//

#include "ftp_server.h"

int main(int argc,char* argv[]){

    int pid;
    int hit=1;
    int control_fd,control_listen;

    if((control_listen=create_listen_socket(default_server_port))==-1){
        error_exit("create_listen_socket failed.");
    }

    /**now server is listenning for client by port default **/
    while(1){
        if((control_fd=accept_client_fd(control_listen))==-1){
            close(control_listen);
            error_exit("accept failed.");
        }

        /**create more pid to connect.
         * connect port:
         * consider now:first time a client request control link.**/
        if((pid=fork())<0){
            error_exit("fork failed.");
        }
        else if(pid==0) {
            close(control_listen);
            ftp_server_start(control_fd, hit++);
            close(control_fd);
            exit(0);
        }
        close(control_fd);
    }
    close(control_listen);
    return 0;
}

void ftp_server_start(int control_fd,int hit){

    char direc_abs[N];//absolute directory
    memset(direc_abs,0,N);

    if(request_login(control_fd)){
        control_response(control_fd,ADMIT);
    }
    else{
        control_response(control_fd,REFUSE);
        error_exit("can't find user.");
    }

    /**now start receiving the command from client.**/

    while(1){
        printf("**conected!**\n");
        char command[N];
        memset(command,0,N);
        int c_code=get_command(control_fd,command);

        printf("command:%s,c_code:%d\n",command,c_code);
        /**Tell client that server accepted.**/
        control_response(control_fd,c_code);

        if(c_code<0){
            continue;
        }
        else if(c_code==QUIT_C){
            break;
        }
        else{
            //int data_fd=server_data_connect(control_fd,default_client_port)
            int data_fd=server_choose_cway(control_fd,hit);
            printf("data_fd:%d\n",data_fd);
            if((data_fd)<0){
                close(control_fd);
                error_exit("server connect client port failed.");
            }
            switch(c_code){
                case HELP_C:server_help(data_fd,control_fd);break;
                case DIR_C:if(strcmp(command,PWD)==0) {
                                server_pwd(data_fd,control_fd,direc_abs);
                            }
                            else if(strcmp(command,MDIR)==0){
                                server_dir(data_fd,control_fd,direc_abs);
                            }
                            else if(strncmp(command,CD,2)==0){
                                server_cd(data_fd,control_fd,command,direc_abs);
                            }
                    break;

                case FILE_C:
                        if(strncmp(command,PUT,3)==0){//client put means server get.
                            server_get(data_fd,control_fd,command);
                        }
                        else if(strncmp(command,GET,3)==0){//client get means server put
                            server_put(data_fd,control_fd,command);
                        }
                        else if(strncmp(command,RM,2)==0){
                            server_rm(data_fd,control_fd,command);
                        }
                    break;
            }
            close(data_fd);
        }
        fflush(stdout);
    }
}

int request_login(int control_fd){

    /**Tell client server is ready to get username.**/
    if(control_response(control_fd,READY)==-1){
        error_exit("server response failed.");
    }

    char buffer[N];
    memset(buffer,0,N);

    struct user u;
    memset(u.username,0,N);
    memset(u.password,0,N);

    /**get the username from client.**/
    if(receive_data_once(control_fd,buffer, sizeof(buffer))<0){
        error_exit("receive username failed.");
    }

    copy_string(u.username,buffer);

    printf("receive username:%s,len:%d\n",u.username,strlen(u.username));

    /**Tell client ready for password**/
    if(control_response(control_fd,READY)==-1){
        error_exit("response READY failed.");
    }

    memset(buffer,0,N);
    /**get password**/
    if(receive_data_once(control_fd,buffer, sizeof(buffer))<0){
        error_exit("receive password failed.");
    }

    copy_string(u.password,buffer);

    printf("receive password:%s,len:%d\n",u.password,strlen(u.password));

    return check_user(u);
}

int check_user(struct user u){
    int len=db_num;
    for(int i=0;i<len;i++){
        if(strcmp(u.username,db_username[i])==0&&strcmp(u.password,db_password[i])==0){
            return 1;
        }
    }
    return 0;
}

int get_command(int control_fd,char* command){

    memset(command,0, N);

    /**get command from client**/
    if(receive_data_once(control_fd,command, N)<0){
        error_exit("receive command failed.");
    }

    return check_command(command);
}

int check_command(char* command){

    if(strcmp(command,QUIT)==0){
        return QUIT_C;
    }
    else if(strcmp(command,HELP)==0){
        return HELP_C;
    }
    else if(strncmp(command,CD,2)==0||strcmp(command,MDIR)==0
        ||strcmp(command,PWD)==0){
        return DIR_C;
    }
    else if(strncmp(command,PUT,3)==0||strncmp(command,GET,3)==0||strncmp(command,RM,2)==0){
        return FILE_C;
    }else{
        return -1;
    }
}

int server_choose_cway(int control_fd,int hit){
    /**receive connect way and port from client**/
    char command[N];
    memset(command,0,N);
    if(recv(control_fd,command,N,0)<0){
        error_exit("receive port PORT failed.");
    }
    char str_way[N];
    char str_port[N];

    memset(str_way,0,N);
    memset(str_port,0,N);

    strncpy(str_way,command,3);

    printf("str_way:%s\n",str_way);

    int request_code=atoi(str_way);
    int client_port;

    printf("should be way:%d\n",request_code);

    if(request_code==PORT){
        int i;
        for(i=4;i<strlen(command);i++){
            str_port[i-4]=command[i];
        }
        str_port[i]='\0';
        client_port=atoi(str_port);

        printf("port:%d\n",client_port);
        return server_data_connect(control_fd,client_port);
    }
    else{
        return server_data_open(control_fd,hit);
    }
}

/**client port way:
 * client use port <PORT>**/
int server_data_connect(int control_fd,int client_port){

    struct sockaddr_in client_addr;

    socklen_t len= sizeof(client_addr);
    char host_ip[N];

    bzero(host_ip,N);

    getpeername(control_fd,(struct sockaddr*)&client_addr,&len);//get client's addr;

    inet_ntop(AF_INET,&client_addr.sin_addr,host_ip, sizeof(host_ip));//get client's host ip;

    int data_fd;
    if((data_fd=create_connect_socket(client_port,host_ip))<0){
        return -1;
    }


    return data_fd;
}

/**client pasv way
 * send the port and listen.**/
int server_data_open(int control_fd,int hit){

    /**create a port and send to client**/
    int port=hit+default_server_port;
    control_response(control_fd,port);
    /****/

    /**listenning**/
    int listen_fd=create_listen_socket(port);
    int data_fd=accept_client_fd(listen_fd);

    close(listen_fd);
    return data_fd;
}

void server_help(int data_fd,int control_fd){

    char* help="*********************basic command**************************\n"
               "?                  ----get support command.\n"
               "quit               ----quit ftp.\n"
               "cd <directory>     ----change directory\n"
               "dir                ----list of the files in current directory.\n"
               "pwd                ----current directory.\n"
               "get <filename>     ----download file from server.\n"
               "put <filename>     ----upload file to server.\n"
               "port               ----change to positive connection way.\n"
               "pasv               ----change to passive connection way.\n"
               "**********************extension command*********************\n"
               "cway               ----see the connection way by now.\n"
               "rm  <filename>     ----remove the file on the directory.\n"
               "**********************hope you enjoy!***********************\n";

    control_response(control_fd,START);

    printf("len of help:%d\n",strlen(help));
    fflush(stdout);
    if(send_data_once(data_fd,help)<0){
        error_exit("send help failed.");
    }

    control_response(control_fd,OK);
    fflush(stdout);
}

void server_pwd(int data_fd,int control_fd,char* direc_abs){

    char pwd[N];
    memset(pwd,0,N);
    getcwd(pwd,N);
    printf("server get pwd:%s\n",pwd);
    control_response(control_fd,START);

    if(send_data_once(data_fd,pwd)<0){
        error_exit("send pwd failed.");
    }

    direc_abs=pwd;

    control_response(control_fd,OK);

}

void server_dir(int data_fd,int control_fd,char* direc_abs){
    DIR *dir;
    struct dirent *ptr;

    getcwd(direc_abs,N);
    dir=opendir(direc_abs);

    printf("open directory:%s\n",direc_abs);
    if(dir==NULL){
        printf("dir incorrect.\n");
    }

    control_response(control_fd,START);

    while((ptr=readdir(dir))!=NULL) {
        if(strcmp(ptr->d_name,".")==0||strcmp(ptr->d_name,"..")==0)
            continue;
        char buf[N]={0};
        strcat(buf,ptr->d_name);
        strcat(buf,"\n");
        if (send_data_once(data_fd, buf) < 0) {
            error_exit("send pwd failed.");
        }
    }
    closedir(dir);

    control_response(control_fd,OK);
}

void server_cd(int data_fd,int control_fd,char* command,char* direc_abs){

    char dirto[N];
    get_content(dirto,command,2);

    printf("get cd to %s\n",dirto);

    char message[N];
    memset(message,0,N);
    if(chdir(dirto)==-1){//change failed.
        char* buf;
        sprintf(buf,"can't find directory:%s",dirto);
        strcat(message,buf);
        free(buf);
    }
    else {//change successfully.
        strcat(message, "change directory to ");
        strcat(message, dirto);
        strcat(message, " successfully.");
    }
    printf("message:%s\n",message);

    control_response(control_fd,START);

    if(send_data_once(data_fd,message)<0){
        error_exit("send pwd failed.");
    }

    direc_abs=dirto;

    control_response(control_fd,OK);
}

void server_get(int data_fd,int control_fd,char* command){

    char filename[N];
    get_content(filename,command,strlen(PUT));

    /**int status=0;
    waiting for START from client
    if(recv(control_fd,&status, sizeof(status),0)<0){
        printf("receive status START failed.\n");
        return;
    }**/

    /**get code from client**/
    int ret_code=get_command_retcode(control_fd);
    printf("server get retcode:%d\n",ret_code);

    FILE *fd = NULL;
    if ((fd = fopen(filename, "w")) == NULL) {
        printf("can't open file:%s\n", filename);
    }

    if(ret_code==STOP){
        char buffer[N];
        memset(buffer,0,N);

        receive_data_once(data_fd,buffer,N);

        printf("%s\n",buffer);
    }
    else if(ret_code==START) {
        char buffer[N];
        memset(buffer, 0, N);
        int size = 0;
        /**receive data from client**/
        while ((size = recv(data_fd, buffer, N, 0)) > 0) {
            printf("get file content:%s\n",buffer);
            fwrite(buffer, 1, size, fd);
        }
    }
    /**waiting for OK from client**/
    /**
    if(recv(control_fd,&status, sizeof(status),0)<0){
        printf("receive status START failed.\n");
        return;
    }**/
    fclose(fd);
}

void server_put(int data_fd,int control_fd,char* command){

    char filename[N];
    get_content(filename,command,3);

    FILE* fd=NULL;
    char buffer[N];
    memset(buffer,0,N);

    control_response(control_fd,START);

    if((fd=fopen(filename,"r"))==NULL){//invalid filename,send error message.
        sprintf(buffer,"FOUND ERROR:invalid filename:%s",filename);

        if(send_data_once(data_fd,buffer)<0){
            error_exit("send error_filename failed.");
        }
    }else{//correct filename,send file.
        size_t len;
        do{
            if((len=fread(buffer,1,N,fd))<0){
                printf("read filename failed.\n");
            }

            //printf("send file content:%s\n",buffer);
            if(send(data_fd,buffer,len,0)<0){
                error_exit("send file failed.\n");
            }
        }while(len>0);
    }
    control_response(control_fd,OK);
    fclose(fd);
}

int get_command_retcode(int control_fd){
    char s_retcode[N];
    memset(s_retcode,0,N);

    if(recv(control_fd,s_retcode,N,0)<0){
        error_exit("receive reply code failed.");
    }

    //printf("get retcode first time:%s\n",s_retcode);
    return atoi(s_retcode);
}

void server_rm(int data_fd,int control_fd,char* command){

    char filename[N];
    get_content(filename,command,2);

    printf("get filename %s\n",filename);

    char message[N];
    memset(message,0,N);
    if(remove(filename)==-1){//remove failed.
        char* buf;
        sprintf(buf,"can't remove file:%s",filename);
        strcat(message,buf);
        free(buf);
    }
    else {//change successfully.
        strcat(message, "remove ");
        strcat(message, filename);
        strcat(message, " successfully.");
    }
    printf("message:%s\n",message);

    control_response(control_fd,START);

    if(send_data_once(data_fd,message)<0){
        error_exit("send pwd failed.");
    }

    control_response(control_fd,OK);
}