#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstdlib>
#include <thread>
#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <pthread.h>

int _bufSize;
int _maxConnNum;
int _listenFD;

int _serverPort;
sockaddr_in _serverAddr;

sockaddr_in _clientAddr;
int _clientConn;
socklen_t _clientAddrLen;
std::string _recvBuff;
std::string _sendBuff;
std::mutex sendM,recvM;
class Server {
private:
public:

    Server() {
        _bufSize = 1024 * 16;
        _maxConnNum = 1024;
        _serverPort = 7004;
        _listenFD = socket(AF_INET, SOCK_STREAM, 0);
        if (_listenFD == -1) {
            puts("Error: socket\n");
            exit(-1);
        }
        _serverAddr.sin_family = AF_INET;
        _serverAddr.sin_port = htons(_serverPort);
        _serverAddr.sin_addr.s_addr = INADDR_ANY;
        if (bind(_listenFD, (struct sockaddr *) &_serverAddr, sizeof(struct sockaddr_in)) == -1) {
            puts("Error: bind\n");
            exit(-1);
        }
        if (listen(_listenFD, _maxConnNum) == -1) {
            puts("Error: listen\n");
            exit(-1);
        }
        puts("server start successfully");
    }

    static std::string _recv(int conn) {
        char *buf = new char[_bufSize];
        std::string data;
        while (recv(conn, buf, _bufSize, 0) == _bufSize) {
            data += buf;
        }
        data += buf;
        delete[] buf;
        return data;
    }

    static void _send(int conn, const std::string &data) {
        char *buf = new char[_bufSize];
        int i;
        for (i = 0; i < data.size(); i += _bufSize) {
            if (i + _bufSize > data.size()) {
                strcpy(buf, data.substr(i, data.size() - i).c_str());
                send(conn, buf, strlen(buf) * sizeof(char), 0);
                break;
            }
            strcpy(buf, data.substr(i, _bufSize).c_str());
            send(conn, buf, _bufSize, 0);
        }
        delete[] buf;
    }

    static void sendToClient() {
        while (true) {
            sendM.lock();
            if (_sendBuff != "") {
                _send(_clientConn, _sendBuff);
                _sendBuff = "";
            }
            sendM.unlock();
        }
    }

    static void recvFromClient() {
        while (true) {
            recvM.lock();
            _recvBuff += _recv(_clientConn);
            recvM.unlock();
        }
    }

    static void tran() {
    }

    void clientConn() {
        _clientAddrLen = sizeof(_clientAddr);
        _clientConn = accept(_listenFD, (struct sockaddr *) &_clientAddr, &_clientAddrLen);
        if (_clientConn < 0) {
            exit(-1);
        }
        puts("client connected");
    }
    static void test(int conn){
        recvM.lock();
        _send(conn, _recvBuff);
        _recvBuff = "";
        recvM.unlock();
}
    static void browser(){
        while (true) {
            int conn;
            sockaddr_in clientAddr{};
            socklen_t clientAddrLen = sizeof(clientAddr);
            conn = accept(_listenFD, (struct sockaddr *) &clientAddr, &clientAddrLen);
            if (conn < 0) {
                exit(-1);
            }
            sendM.lock();
            _sendBuff += _recv(conn); // 浏览器第一次请求
            sendM.unlock();
            std::thread th(test,conn);
            th.detach();
            sleep(1);
            close(conn);
        }
}
    void run() {
        clientConn();
        std::thread th(sendToClient);
        std::thread th2(recvFromClient);
        std::thread th3(tran);
        std::thread th4(browser);
        th.join();
        th2.join();
        th3.join();
        th4.join();

    }

    ~Server() {
        close(_listenFD);
    }
};


int main() {
    Server s;
    s.run();
    return 0;
}


