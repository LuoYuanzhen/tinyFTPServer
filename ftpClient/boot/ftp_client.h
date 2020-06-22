//
// Created by luoyuanzhen on 2020/6/17.
//

#ifndef TINYFTPSERVER_FTP_CLIENT_H
#define TINYFTPSERVER_FTP_CLIENT_H

#include "../../util_socket.h"

int response_login();

int get_command_retcode();

int client_choose_cway(int port);

int client_data_open(int port);

int client_data_connect();

void client_gets(int data_fd);//?,dir,pwd,cd

void client_getf(int data_fd,char* command);

void client_putf(int data_fd,char* command);

void see_connect_way(int port);

#endif //TINYFTPSERVER_FTP_CLIENT_H
