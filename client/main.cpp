#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string>


class Client {
private:
    struct sockaddr_in server_addr;
    int serverFD;
    const char *serverAddr = "127.0.0.1";
    int _bufSize;
public:
    Client() {
        _bufSize = 16*1024;
        if ((serverFD = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
            perror("socket");
            exit(-1);
        }
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(7004);
        server_addr.sin_addr.s_addr = inet_addr(serverAddr);
        if (connect(serverFD, (struct sockaddr *) &server_addr, sizeof(struct sockaddr_in)) == -1) {
            perror("connect error");
            exit(-1);
        }
    }
    void run(){
        int clientFD;
        if ((clientFD = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
            perror("socket");
            exit(-1);
        }
        sockaddr_in webserver_addr;
        webserver_addr.sin_family = AF_INET;
        webserver_addr.sin_port = htons(8002);//web server port
        webserver_addr.sin_addr.s_addr = inet_addr("127.0.0.1");  //web server host
        if (connect(clientFD, (struct sockaddr *) &webserver_addr, sizeof(struct sockaddr_in)) == -1) {
            perror("connect error");
            exit(-1);
        }
        while(1){
            std::string result = _recv(serverFD); // server 来第一次
            _send(clientFD,result); // 发送到webserver
            result = _recv(clientFD);//response 200 ok
            puts("request");
            puts(result.c_str());
            puts("response");
            puts(result.c_str());
            _send(serverFD,result); // 给server response 200 ok

            result = _recv(clientFD);// 从web 拿到第二次response

            puts("response");
            puts(result.c_str());
            _send(serverFD,result);// 发给server 拿到第二次response
        }
        close(clientFD);
    }
    ~Client() {
        close(serverFD);
    }
    [[nodiscard]] std::string _recv(int conn) const {
        char *buf = new char[this->_bufSize];
        std::string data;
        while (recv(conn, buf, this->_bufSize, 0) == this->_bufSize) {
            data += buf;
        }
        data += buf;
        delete[] buf;
        return data;
    }

    void _send(int conn, const std::string &data) const {
        char *buf = new char[this->_bufSize];
        int i;
        for (i = 0; i < data.size(); i += this->_bufSize) {
            if(i+_bufSize>data.size()){
                strcpy(buf, data.substr(i, data.size() - i).c_str());
                send(conn, buf, strlen(buf) * sizeof(char), 0);
                break;
            }
            strcpy(buf, data.substr(i, this->_bufSize).c_str());
            send(conn, buf, this->_bufSize, 0);
        }
        delete[] buf;
    }
};

int main() {
    Client c;
    c.run();
//    int i=0;
//    int sockfd2, numbytes2;
//    char buf2[MAXDATASIZE];
//    struct sockaddr_in server_addr2;
//    if ((sockfd2 = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
//        perror("socket");
//        exit(1);
//    }
//
//    server_addr2.sin_family = AF_INET;
//    server_addr2.sin_port = htons(5005);
//    server_addr2.sin_addr.s_addr = inet_addr(SERVER_IP);
//    bzero(&(server_addr2.sin_zero), sizeof(server_addr2.sin_zero));
//    if (connect(sockfd2, (struct sockaddr *) &server_addr2, sizeof(struct sockaddr_in)) == -1) {
//        perror("connect error");
//        exit(1);
//    }
//    char *msg = "123456789\n";
//    while (1) {
//        printf("%d",i);
//        recv(sockfd, buf, sizeof (buf), 0);
//        puts(buf);
//        send(sockfd2, buf, strlen(buf)*sizeof (char), 0);
//
////        memset(buf,0,sizeof (buf));
//
//        recv(sockfd2, buf, sizeof (buf), 0);
//        puts("==============>>>>>");
//        puts(buf);
//        puts("<<<<<==============");
//        send(sockfd, buf, strlen(buf)*sizeof (char), 0);
//        puts(buf);
//        i++;
//    }
    return 0;
}