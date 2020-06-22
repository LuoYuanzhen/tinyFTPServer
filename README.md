# tinyFTPServer

This is a C vesion tiny server &amp; client using FTP.
这是一个C语言版的使用FTP协议的小型客户-服务端.

## files list/文件列表:

      /ftpClient/boot:    ftp_client.c
                          ftp_client.h
      /ftpServer/boot:    ftp_server.c
                          ftp_server.h
      /util_socket.c
      /util_socket.h

## support function/支持的功能:

      - '?'               see all the support commands./查看服务端支持的指令
      - 'dir'             see the list of files in the directory./查看当前服务端目录下的所有文件
      - 'pwd'             see location directory./查看当前所处的服务端目录
      - 'cd <dirctory>'   cd to the directory./转到<dirctory>目录下
      - 'get <filename>'  get the file from server./从服务端下载文件到客户端
      - 'put <filename>'  put the file to server./客户端上传文件到服务端
      - 'cway'            see the data connection way (port or pasv)./查看当前的数据连接方式(port主动方式或者pasv被动方式)
      - 'pasv'            set the pasv data connection way ./设置当前数据连接方式为pasv方式
      - 'port'            set the port data connection way ./设置当前数据连接方式为port方式
      
## how to use/怎样使用:

      - start ftp server:(运行ftp服务端)
      cd /ftpServer/boot:gcc -o ftp_server ftp_server.c ../../util_socket.c
                         ./ftp_server
      - start ftp client:(运行ftp客户端)
      cd /ftpClient/boot:gcc -o ftp_client ftp_client.c ../../util_socket.c
                         ./ftp_client PORT
                         (PORT is port number,using for the client port way.)
                         (PORT为端口号,用于主动连接方式下客户端提供的监听端口号)
      
      login in username:admin
               password:admin
               
