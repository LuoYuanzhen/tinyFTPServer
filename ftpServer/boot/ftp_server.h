//
// Created by luoyuanzhen on 2020/6/17.
//

#ifndef TINYFTPSERVER_FTP_SERVER_H
#define TINYFTPSERVER_FTP_SERVER_H

#include "../../util_socket.h"

#define db_num 2;
char* db_username[]={"luoyuanzhen","admin"};
char* db_password[]={"123","admin"};

struct user{
    char username[N];
    char password[N];
};

void ftp_server_start(int control_fd,int hit);

int request_login(int control_fd);

int check_user(struct user u);

int get_command(int control_fd,char* command);

int check_command(char* command);

int server_choose_cway(int control_fd,int hit);

int server_data_open(int control_fd,int hits);

int server_data_connect(int control_fd,int client_port);

void server_help(int data_fd,int control_fd);

void server_pwd(int data_fd,int control_fd,char* direc_abs);

void server_dir(int data_fd,int control_fd,char* direc_abs);

void server_cd(int data_fd,int control_fd,char* command,char* direc_abs);

void server_get(int data_fd,int control_fd,char* command);

void server_put(int data_fd,int control_fd,char* command);

void server_rm(int data_fd,int control_fd,char* command);

int get_command_retcode(int control_fd);

#endif //TINYFTPSERVER_FTP_SERVER_H
