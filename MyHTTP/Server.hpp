//
//  Server.hpp
//  MyHTTP
//
//  Created by Orlando‘s Mac on 2024/9/8.
//

#ifndef Server_hpp
#define Server_hpp

#include "Header.h"
#include "ThreadPool.hpp"
#include "Connection.hpp"


class Server {
    //线程池
    ThreadPool threadPool_;
    //客户端socket
    int serverSocket_;
    //客户端port
    int port_ = 80;
    
public:
    //开启服务器
    void startUp();
    //接收客户连接
    void acceptConnection();
    //处理客户请求
    void handleClient(int client);
    
    Server(int port) : port_(port), serverSocket_(-1), threadPool_(5) {}
    ~Server() = default;
    
};


#endif /* Server_hpp */
