//
//  Server.cpp
//  MyHTTP
//
//  Created by Orlando‘s Mac on 2024/9/8.
//

#include "Server.hpp"



void Server::startUp(){
    //创建服务器套接字
    int server_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(server_socket == -1){
        perror("Socket");
        exit(1);
    }
    
    //设置端口复用
    int opt = 1;
    int ret = setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    if (ret == -1){
        perror("set port opt");
        exit(1);
    }
    
    //配置服务器端的网络地址
    struct sockaddr_in server_addr;
    //统一重制为0
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;//ipv4
    server_addr.sin_port  = htons(port_);//设置端口
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    
    //绑定套接字
    if(bind(server_socket,(struct sockaddr*) &server_addr, sizeof(server_addr)) < 0){
        perror("bind");
        exit(1);
    }
    
    //动态分配port when port == 0
    socklen_t nameLen = sizeof(server_addr);
    if(port_ == 0){
        if(getsockname(server_socket, (struct sockaddr*) &server_addr, &nameLen) < 0){
            perror("dynamic port");
            exit(1);
        }
        
        port_ = ntohs(server_addr.sin_port);
    }
    
    serverSocket_ = server_socket;
    //创建监听队列
    if(listen(server_socket, 5) < 0){
        perror("listen");
        exit(1);
    }
    
    std::cout << "Server started, listening:" << port_ << std::endl << std::endl;
    
    //创建线程池
    ThreadPool(5);
    
    acceptConnection();
    
}


void Server::acceptConnection(){
    struct sockaddr_in client_addr;
    socklen_t client_aadr_len = sizeof(client_addr);
    
    while (1) {
        //阻塞式等待连接
        int clientSocket =  accept(serverSocket_, (struct sockaddr*)&client_addr, &client_aadr_len);
        if (clientSocket < 0) {
            perror("Accept");
            exit(1);
        }
        
        //分发任务
        threadPool_.enqueue([this, clientSocket](){return this->handleClient(clientSocket);});
        
    }
}

void Server::handleClient(int clientSocket){
    Connection connection(clientSocket);
    connection.analyse();
    connection.handleRequst();
}
