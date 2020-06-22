//
// Created by luoyuanzhen on 2020/6/17.
//

#ifndef TINYFTPSERVER_UTIL_SOCKET_H
#define TINYFTPSERVER_UTIL_SOCKET_H

#include <arpa/inet.h>
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define N 512//max size of every command.

/**default port about client and server.**/
#define default_client_port 1209
#define default_server_port 1209

/**default host ip address about server**/
#define server_host "127.0.0.1"

/**response codes on control link:**/
#define READY 101
#define ADMIT 230
#define REFUSE 404

#define PASV 227
#define PORT 228
#define START 100
#define OK 110

#define STOP 120
/**command code for server:**/
#define QUIT_C 401//quit
#define HELP_C 505//?
#define DIR_C 300//cd,pwd,dir
#define FILE_C 600//put,get
#define WAY_C 700//change way.
#define RM_C 800
/**request command on control link:**/
#define QUIT "quit"
#define HELP "?"
#define CD "cd"
#define MDIR "dir"
#define PWD "pwd"
#define PUT "put"
#define GET "get"

#define CWAY "cway"
#define RM "rm"

#define PORT_WAY "port"
#define PASV_WAY "pasv"

void error_exit(char* error);

int create_listen_socket(int port);

int accept_client_fd(int fd_listen);

int create_connect_socket(int port,char* host);

int control_response(int socket_fd,int code);

int receive_data_once(int fd,char* buffer,int size);

void getstd(char* buffer,int size);

int send_data_once(int fd,char* buffer);

void copy_string(char* a,char* b);

void get_content(char* content,char* command,int c_len);
#endif //TINYFTPSERVER_UTIL_SOCKET_H
