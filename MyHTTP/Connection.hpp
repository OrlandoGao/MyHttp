//
//  Connection.hpp
//  MyHTTP
//
//  Created by Orlando‘s Mac on 2024/9/8.
//

#ifndef Connection_hpp
#define Connection_hpp

#include "Header.h"


class Connection {
    int clientSocket_;
    std::string requstMethod_;//请求方法
    std::string requstPath_;//请求路径
    
    
public:
    //分析连接请求
    void analyse();
    //处理请求
    void handleRequst();
    //从套接字读取一行
    int get_line(int sock, char* buff, int size);
    
    Connection() = default;
    Connection(int clientSocket);
    ~Connection();
};

#endif /* Connection_hpp */
