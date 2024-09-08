//
//  Connection.cpp
//  MyHTTP
//
//  Created by Orlando‘s Mac on 2024/9/8.
//

#include "Connection.hpp"




int Connection::get_line(int sock, char *buff, int maxSize){
    char ch = '0';
    int i = 0;
    while (i < maxSize - 1 &&  ch != '\n') {
        int n = recv(sock, &ch, 1, 0);
        if (n > 0) {
            if (ch == '\r') {
                n = recv(sock, &ch, 1, MSG_PEEK);
                if (n > 0 && ch == '\n') {
                    recv(sock, &ch, 1, 0);
                }
                else{
                    ch = '\n';
                }
            }
            buff[i++] = ch;
        }
        else {
            ch = '\n';
        }
    }
    
    buff[i] = 0;//'\0'
    return i;
}

Connection::Connection(int clientSocket){
    clientSocket_ = clientSocket;
}

void Connection::analyse(){
    char buff[1024];
    int bytes_read = get_line(clientSocket_, buff, 1024);
    if(bytes_read < 0){
        perror("bytes_read");
        exit(1);
    }
    
    //分析方法和路径
    char method[16], path[256];
    sscanf(buff, "%s %s", method, path);
    
    //检查方法
    if (strcasecmp(method, "GET") && strcasecmp(method, "POST")) {
        requstMethod_ = "notSupport";
    }
    else{
        requstMethod_ = method;
    }
    
    
    //检查路径
    if(strcmp(path, "/") == 0){
        strcpy(path, "/index.html");
    }
    //拼接完整路径
    char full_path[512];
    snprintf(full_path, sizeof(full_path), "htdocs%s", path);
    
    //检查文件是否存在
    struct stat file_stat;
    if (stat(full_path, &file_stat) == -1) {
        requstPath_ = "hrdocs/404.html";
    }
    else{
        requstPath_ = full_path;
    }
    std::cout << "Get requst from socket:" << clientSocket_ << std::endl;
    std::cout << requstMethod_ << " " << requstPath_ << std::endl << std::endl;
}

void Connection::handleRequst(){
    std::ifstream file(requstPath_, std::ios::binary);
    if (!file) {
        perror("File open");
        exit(1);
    }
    
    const size_t bufferSize = 1024;
    char buffer[bufferSize];
    size_t count = 0;
    while (file.read(buffer, bufferSize)) {
        count += send(clientSocket_, buffer, file.gcount(), 0);
    }
    // 发送剩余的部分（如果有的话）
    if (file.gcount() > 0) {
        count += send(clientSocket_, buffer, file.gcount(), 0);
    }
    
    
    std::cout << "Have sent " << count << "B to " << clientSocket_ << std::endl << std::endl;
    
    file.close();
}

Connection::~Connection(){
    std::cout << "Close " << clientSocket_ << " connection" << std::endl << std::endl;
    close(clientSocket_);
}
