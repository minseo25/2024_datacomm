/*
*   tested on Ubuntu 22.04
*   g++ -o client client.cpp
*   ./client [port]  OR  ./client (default port: 50000)
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

int main(int argc, char** argv) {
    sockaddr_in server_addr;
    int sockfd, port = 50000;
    socklen_t server_addr_len;

    if(argc>=2) {
        port = atoi(argv[1]);
    }

    if((sockfd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        error_handling("socket error\n");
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr_len = sizeof(server_addr);

    if(connect(sockfd, (struct sockaddr *)&server_addr, server_addr_len) < 0) {
        close(sockfd);
        error_handling("connect error\n");
    }

    queue<string> user_inputs;
    string user_input;
    int num_inputs;
    bool closeConnect = false;
    char* buf = (char *)calloc(MAX_BUF_SIZE, sizeof(char)); // initialize to 0

    while(true) {
        while(true) {
            // get user inputs
            getline(cin, user_input);

            if(user_input.compare("Q") == 0) {
                break;
            }
            if(user_input.compare("bye") == 0) {
                closeConnect = true;
                break;
            }
            user_input += "\n";
            user_inputs.push(user_input);
        }
        if(closeConnect) {
            break;
        }

        num_inputs = user_inputs.size();

        // send all messages
        socket_send(sockfd, SEND);
        while(!user_inputs.empty()) {
            user_input = user_inputs.front();
            user_inputs.pop();
            socket_send(sockfd, user_input);
        }
        socket_send(sockfd, RECV);
        
        // receive all replies
        for(int i=0; i<num_inputs; i++) {
            socket_recv(sockfd, buf);
            cout << "[CLIENT] " << buf;
            memset(buf, 0, MAX_BUF_SIZE);
        }
    }

    socket_send(sockfd, ECHO_CLOSE);
    socket_recv(sockfd, buf);
    cout << "[CONNECTION CLOSED]\n";

    free(buf);
    close(sockfd);

    return 0;
}