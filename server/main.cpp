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

class Server {
private:
    int _bufSize;
    int _maxConnNum;
    int _listenFD;

    int _serverPort;
    sockaddr_in _serverAddr;

    int _clientPort;
    sockaddr_in _clientAddr;
    int _clientConn;
    socklen_t _clientAddrLen;
public:

    Server() {
        this->_bufSize = 1024 * 16;
        this->_maxConnNum = 1024;
        this->_serverPort = 7005;
        this->_listenFD = socket(AF_INET, SOCK_STREAM, 0);
        if (this->_listenFD == -1) {
            puts("Error: socket\n");
            exit(-1);
        }
        this->_serverAddr.sin_family = AF_INET;
        this->_serverAddr.sin_port = htons(this->_serverPort);
        this->_serverAddr.sin_addr.s_addr = INADDR_ANY;
        if (bind(this->_listenFD, (struct sockaddr *) &this->_serverAddr, sizeof(struct sockaddr_in)) == -1) {
            puts("Error: bind\n");
            exit(-1);
        }
        if (listen(this->_listenFD, this->_maxConnNum) == -1) {
            puts("Error: listen\n");
            exit(-1);
        }
        puts("server start successfully");
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

    void *check() {
        char *_buf = new char[_bufSize];
        if (recv(_clientConn, _buf, this->_bufSize, 0) < 0) {
            puts("break off");
        } else { puts("connect"); }
    }

    void _send(int conn, const std::string &data) const {
        char *_buf = new char[this->_bufSize];
        int i;
        for (i = 0; i < data.size(); i += this->_bufSize) {
            if (i + _bufSize > data.size()) {
                strcpy(_buf, data.substr(i, data.size() - i).c_str());
                send(conn, _buf, strlen(_buf) * sizeof(char), 0);
                break;
            }
            strcpy(_buf, data.substr(i, this->_bufSize).c_str());
            send(conn, _buf, this->_bufSize, 0);
        }
        delete[] _buf;
    }

    void clientConn() {
        this->_clientAddrLen = sizeof(this->_clientAddr);
        this->_clientConn = accept(this->_listenFD, (struct sockaddr *) &this->_clientAddr, &this->_clientAddrLen);
        if (this->_clientConn < 0) {
            exit(-1);
        }
        puts("client connected");
//        std::thread th(check());
    }

    void run() {
        clientConn();
        while (true) {
            int conn;
            sockaddr_in clientAddr{};
            socklen_t clientAddrLen = sizeof(clientAddr);
            conn = accept(this->_listenFD, (struct sockaddr *) &clientAddr, &clientAddrLen);
            if (conn < 0) {
                exit(-1);
            }
            std::string result = _recv(conn);
            puts(result.c_str());

            _send(_clientConn, result);
            result = _recv(_clientConn);

            _send(conn, result);
            close(conn);
        }
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


