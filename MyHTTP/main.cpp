//
//  main.cpp
//  MyHTTP
//
//  Created by Orlando‘s Mac on 2024/9/4.
//

#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <strings.h>

#define print(buff) printf("[%s - %d] buff:%s \n", __func__, __LINE__, buff)

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
    char buff[1024];
    
    strcpy(buff, "HTTP/1.1 404 Not Found\r\n");
    send(client, buff, strlen(buff), 0);
    
    strcpy(buff, "Server: OrlandoHttpd/0.1\r\n");
    send(client, buff, strlen(buff), 0);
    
    strcpy(buff, "Content-type: text/html\r\n");
    send(client, buff, strlen(buff), 0);
    
    strcpy(buff, "\r\n");
    send(client, buff, strlen(buff), 0);
    
    //发送404
    sprintf(buff,
        "<html>                                         \
            <title>Not Found</title>                    \
            <body>                                      \
                <h2>The resource is unavailable</h2>    \
            </body>                                     \
        </html>");
    send(client, buff, sizeof(buff), 0);
}


//发送相应的文件头信息
void headers(int client){
    char buff[1024];
    
    strcpy(buff, "HTTP/1.1 200 OK\r\n");
    send(client, buff, strlen(buff), 0);
    strcpy(buff, "Server: OrlandoHttpd/0.1\r\n");
    send(client, buff, strlen(buff), 0);
    strcpy(buff, "Content-type: text/html\r\n");
    send(client, buff, strlen(buff), 0);
    strcpy(buff, "\r\n");
    send(client, buff, strlen(buff), 0);
    
}

//发送客户请求的文件
void cat(int client, FILE* resource){
    char buff[4096];
    int count = 0;
    while (1) {
        int ret =  fread(buff, sizeof(char), sizeof(buff), resource);
        if (ret <= 0) {
            break;
        }
        send(client, buff, ret, 0);
        count += ret;
    }
    printf("sent %dB to browser\n", count);
}

//向客户提交文件
void server_file(int clinet, const char* fileName){
    int num = 1;
    char buff[1024];
    while (num > 0 && strcmp(buff, "\n")) {
        num = get_line(clinet, buff, sizeof(buff));
    }
    
    //以文本方式打开
    FILE *resource = fopen(fileName, "r");
    if (resource == NULL) {
        not_found(clinet);
    }
    else {
        //正式发送资源给浏览器
        //发送报文头
        headers(clinet);
        //发送请求的资源
        cat(clinet, resource);
        
        printf("文件发送完毕!\n");
        
    }
    fclose(resource);
}


//处理客户请求的线程函数
void* accept_request(void* arg){
    char buff[1024];//1k
    int clinet = *(int*)arg;
    int num = get_line(clinet, buff, sizeof(buff));
    print(buff);
    
    //读取方法
    char method[255];
    int j = 0, i = 0;//j是buff的指针
    while (!isspace(buff[j]) && i < sizeof(method) - 1) {
        method[i++] = buff[j++];
    }
    method[i] = 0;//'\0'
    print(method);
    
    //检查方法(GET,POST) 待扩展
    if (strcasecmp(method, "GET") && strcasecmp(method, "POST")) {
        unimplement(clinet);
        return 0;
    }
    
    //解析资源文件的路径
    char url[255];//存放 url
    i= 0;
    //跳过空格
    while (isspace(buff[j]) && j < sizeof(buff)) {
        ++j;
    }
    
    while (!isspace(buff[j]) && i < sizeof(url) - 1 && j < sizeof(buff)) {
        url[i++] = buff[j++];
    }
    url[i] = 0;
    print(url);
    
    char path[512] = "";
    sprintf(path, "htdocs%s",url);
    if (path[strlen(path) - 1] == '/') {
        strcat(path, "index.html");
    }
    print(path);
    
    struct stat status;
    if (stat(path, &status) == -1) {
        perror("stat");
        // 请求包剩余数据
        while (num > 0 && strcmp(buff, "\n")) {
            num = get_line(clinet, buff, sizeof(buff));
        }
        not_found(clinet);
    }
    else {
        if (S_ISDIR(status.st_mode)) {
            strcat(path, "/index.html");
        }
        
        server_file(clinet, path);
        
    }

    close(clinet);
    
    return NULL;
}


int main() {
    unsigned short port = 80;
    int server_sock = startUp(&port);
    std::cout << "http server start, listening port : " << port << std::endl;
    
    struct sockaddr_in client_addr;
    socklen_t client_aadr_len = sizeof(client_addr);
    
    while (1) {
        //阻塞式等待
        int client_sock =  accept(server_sock, (struct sockaddr*)&client_addr, &client_aadr_len);
        if (client_sock < 0) {
            error_die("Accept");
        }
        
        //使用client_sock对用户访问
        pthread_t thread;
        pthread_create(&thread, NULL, accept_request, &client_sock);
    }
    
    close(server_sock);
    return 0;
}
