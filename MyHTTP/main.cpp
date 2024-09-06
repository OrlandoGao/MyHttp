//
//  main.cpp
//  MyHTTP
//
//  Created by Orlando‘s Mac on 2024/9/4.
//

#include "Header.h"
#include "ThreadPool.hpp"

#define print(buff) printf("[%s - %d] buff:%s \n", __func__, __LINE__, buff)
#define buffSize 1024

//报错并退出
void error_die(const char* s){
    perror(s);
    exit(1);
}


//实现网络初始化 返回一个套接字（server）
int startUp(unsigned short *port){
    //创建套接字
    int server_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(server_socket == -1){
        error_die("Socket");
    }
    
    //设置端口复用
    int opt = 1;
    int ret = setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    if (ret == -1){
        error_die("Set socket opt");
    }
    
    //配置服务器端的网络地址
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port  = htons(*port);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    
    //绑定套接字
    if(bind(server_socket,(struct sockaddr*) &server_addr, sizeof(server_addr)) < 0){
        error_die("Bind");
    }
    
    //动态分配port when port == 0
    socklen_t nameLen = sizeof(server_addr);
    if(*port == 0){
        if(getsockname(server_socket, (struct sockaddr*) &server_addr, &nameLen) < 0){
            error_die("Dynamic_port");
        }
        
        *port = ntohs(server_addr.sin_port);
    }
    
    //创建监听队列
    if(listen(server_socket, 5) < 0){
        error_die("Listen");
    }
    
    
    return server_socket;
}


//从sock套接字读一行,返回读取的字符个数
int get_line(int sock, char* buff, int size){
    char ch = '0';
    int i = 0;
    while (i < size - 1 &&  ch != '\n') {
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


//向客户返回错误页面
void unimplement(int client){
    
}



//向客户返回网页不存在
void not_found(int client){
    char buff[4096];
    
    strcpy(buff, "HTTP/1.1 404 Not Found\r\n");
    send(client, buff, strlen(buff), 0);
    
    strcpy(buff, "Server: OrlandoHttpd/0.1\r\n");
    send(client, buff, strlen(buff), 0);
    
    strcpy(buff, "Content-type: text/html\r\n");
    send(client, buff, strlen(buff), 0);
    
    strcpy(buff, "\r\n");
    send(client, buff, strlen(buff), 0);
    

    
}


//读取请求文件格式
const char* get_contect_type(const char* fileName){
    //找到最后一个'.'
    const char* ext = strrchr(fileName, '.');
    
    if (ext != NULL) {
            if (strcmp(ext, ".html") == 0 || strcmp(ext, ".htm") == 0) {
                return "text/html";
            } else if (strcmp(ext, ".jpg") == 0 || strcmp(ext, ".jpeg") == 0) {
                return "image/jpeg";
            } else if (strcmp(ext, ".png") == 0) {
                return "image/png";
            } else if (strcmp(ext, ".css") == 0) {
                return "text/css";
            } else if (strcmp(ext, ".js") == 0) {
                return "application/javascript";
            } else if (strcmp(ext, ".gif") == 0) {
                return "image/gif";
            }
        }
    //默认返回2进制流
    return "application/octet-stream";
}

//发送相应的文件头信息
void headers(int client, const char* contect_type){
    char buff[1024];
    
    sprintf(buff, "HTTP/1.1 200 OK\r\n");
    send(client, buff, strlen(buff), 0);
    sprintf(buff, "Content-type: %s\r\n",contect_type);
    send(client, buff, strlen(buff), 0);
    sprintf(buff, "\r\n");
    send(client, buff, strlen(buff), 0);

}


//向客户提交文件
void send_file(int client, const char* fileName){
    //打开文件
    char buff[buffSize];
    int resource = open(fileName, O_RDONLY);
    if (resource == -1) {
        not_found(client);
    }
    else {
        //正式发送资源给浏览器
        //获取资源类型
        const char* contect_type = get_contect_type(fileName);
        
        //发送报文头
        headers(client, contect_type);
        
        //发送请求的资源
        ssize_t bytes_read, bytes_send;
        while ((bytes_read = read(resource, buff, sizeof(buff))) > 0) {
            bytes_send = send(client, buff, bytes_read, 0);
            if (bytes_send == -1) {
                error_die("send");
            }
        }
        if (bytes_read == -1) {
            error_die("read failure");
        }
        
        close(resource);
        printf("Has sent %s (%dB) to browser\n", fileName, bytes_send);
    }
}


//处理客户请求的线程函数
void accept_request(int arg){
    char buff[buffSize];
    int client = arg;
    int bytes_read = get_line(client, buff, buffSize);
    if(bytes_read <= 0){
        error_die("read first line");
    }
    
    //读取方法和路径
    char method[16], path[256];
    sscanf(buff, "%s %s", method, path);
    
    //检查方法
    if (strcasecmp(method, "GET") && strcasecmp(method, "POST")) {
        unimplement(client);
        return;
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
        not_found(client);
    }
    else {
    // 发送文件
        send_file(client, full_path);
    }
    close(client);
    
    
    return;
}


int main() {
    unsigned short port = 80;
    int server_sock = startUp(&port);
    std::cout << "http server start, listening port : " << port << std::endl;
    
    struct sockaddr_in client_addr;
    socklen_t client_aadr_len = sizeof(client_addr);
    
    ThreadPool pool(10);
    
    while (1) {
        //阻塞式等待
        int client_sock =  accept(server_sock, (struct sockaddr*)&client_addr, &client_aadr_len);
        if (client_sock < 0) {
            error_die("Accept");
        }
        
        //将任务推进线程池的任务队列
        pool.enqueue([client_sock](){return accept_request(client_sock);});
        
    }
    
    close(server_sock);
    return 0;
}
