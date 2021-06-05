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
    int serverFD, numbytes;
    char *buf;
    const char *serverAddr = "127.0.0.1";
    int _bufSize;
public:
    Client() {
        _bufSize = 16*1024;
        buf = new char[_bufSize];
        if ((serverFD = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
            perror("socket");
            exit(-1);
        }
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(7005);
        server_addr.sin_addr.s_addr = inet_addr(serverAddr);
        if (connect(serverFD, (struct sockaddr *) &server_addr, sizeof(struct sockaddr_in)) == -1) {
            perror("connect error");
            exit(-1);
        }
    }
    void run(){
        while(1){
            puts(_recv(serverFD).c_str());
            _send(serverFD,"received info !!!!");
        }
    }
    ~Client() {
        close(serverFD);
    }
    [[nodiscard]] std::string _recv(int conn) const {
        char *_buf = new char[this->_bufSize];
        std::string data;
        while (recv(conn, _buf, this->_bufSize, 0) == this->_bufSize) {
            data += _buf;
        }
        data += _buf;
        delete[] _buf;
        return data;
    }

    void _send(int conn, const std::string &data) const {
        char *_buf = new char[this->_bufSize];
        int i;
        for (i = 0; i < data.size(); i += this->_bufSize) {
            if(i+_bufSize>data.size()){
                strcpy(_buf, data.substr(i, data.size() - i).c_str());
                send(conn, _buf, strlen(_buf) * sizeof(char), 0);
                break;
            }
            strcpy(_buf, data.substr(i, this->_bufSize).c_str());
            send(conn, _buf, this->_bufSize, 0);
        }
        delete[] _buf;
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