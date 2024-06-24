/*
*   tested on Ubuntu 22.04
*   g++ -o server server.cpp
*   ./server [port]  OR  ./server (default port: 50000)
*/
#include <cstring>
#include <iostream>
#include <queue>
#include <netinet/in.h>     // 
#include <arpa/inet.h>      // inet_addr, sockaddr_in, htons, htonl, INADDR_ANY, ... , etc
#include <sys/socket.h>     // socket, AF_INET, sockaddr, ... , etc
#include <unistd.h>         // read, write, close
using namespace std;

#define MAX_BUF_SIZE 1025
#define SEND "SEND"
#define RECV "RECV"
#define ECHO_CLOSE "Echo_CLOSE"

void error_handling(string msg) {
    cerr << msg << "\n";
    exit(1);
}

// wrapper function with error handling
void socket_send(int sockfd, string s) {
    char *buf = (char *)calloc(MAX_BUF_SIZE, sizeof(char));
    strcpy(buf, s.c_str());

    if(write(sockfd, buf, MAX_BUF_SIZE) <= 0) { // send(sockfd, s.c_str(), s_len, 0)
        close(sockfd);
        error_handling("recv error\n");
    }
    free(buf);
}
void socket_recv(int sockfd, char* buf) {
    if(read(sockfd, buf, MAX_BUF_SIZE) <= 0) { // recv(sockfd, buf, s_len, 0)
        close(sockfd);
        error_handling("recv error\n");
    }
}

int main(int argc, char **argv){
    // create socket
    int server_sockfd, sockfd, port = 50000;
    sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len;

    if(argc>=2) {
        port = atoi(argv[1]);
    }
    
    if((server_sockfd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        error_handling("socket error\n");
    }
    
    // set socket option: can reallocate <ip, port> to the new socket immediately after closing the socket
    int opt = 1;
    if(setsockopt(server_sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        close(server_sockfd);
        error_handling("setsockopt error\n");
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    if(bind(server_sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        close(server_sockfd);
        error_handling("bind error\n");
    }

    if(listen(server_sockfd, 5) < 0) {
        close(server_sockfd);
        error_handling("listen error\n");
    }
    client_addr_len = sizeof(client_addr);

    if((sockfd = accept(server_sockfd, (struct sockaddr *)&client_addr, &client_addr_len)) < 0) {
        close(server_sockfd);
        error_handling("accept error\n");
    }

    char* buf = (char *)calloc(MAX_BUF_SIZE, sizeof(char));
    queue<string> user_inputs;
    string user_input;
    bool closeConnect = false;

    int cnt = 0;
    while(true) {
        // waits until client sends "SEND", "Echo_CLOSE"
        while(true) {
            memset(buf, 0, MAX_BUF_SIZE);
            socket_recv(sockfd, buf);

            if(strcmp(buf, SEND) == 0) {
                break;
            }
            if(strcmp(buf, ECHO_CLOSE) == 0) {
                closeConnect = true;
                break;
            }
        }
        if(closeConnect) {
            break;
        }

        // recv messages from client
        while(true) {
            memset(buf, 0, MAX_BUF_SIZE);
            socket_recv(sockfd, buf);

            if(strcmp(buf, RECV) == 0) {
                break;
            }
            cout << "[SERVER] " << buf;
            user_inputs.push(string(buf));
        }
        
        // send messages to client
        while(!user_inputs.empty()) {
            user_input = user_inputs.front();
            user_inputs.pop();
            socket_send(sockfd, user_input);
        }
    }
    socket_send(sockfd, ECHO_CLOSE);

    free(buf);
    close(sockfd);
    close(server_sockfd);
    cout << "[CONNECTION CLOSED]\n";

    return 0;
}