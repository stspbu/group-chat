#include <iostream>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <thread>
#include <vector>
#include <algorithm>

using namespace std;

#define SPEC_LENGTH 2   // ": "
#define MAX_NICK_LENGTH 33
#define MAX_MSG_LENGTH 2048

void receiver(int sockfd, bool* status){
    char msg[MAX_MSG_LENGTH];
    while(recv(sockfd, msg, sizeof(msg), 0) > 0){
        cout << msg << "\n";
    }

    cout << "Error: the connection was lost\n";
    *status = false;
}

int main(){
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd == -1){
        cout << "Error: socket\n";
        return 0;
    }

    int port;
    cout << "Enter server's port: ";
    cin >> port;

    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    if(connect(sockfd, (sockaddr*)&addr, sizeof(addr)) == -1){
        cout << "Error: connect\n";
        return 0;
    }else{
        string nickname;
        cout << "Enter your nickname: ";
        while(cin >> nickname && (nickname.size() < 1 || nickname.size() > MAX_NICK_LENGTH - 1)){
            cout << "Info: enter at least 1 and no more than " << MAX_NICK_LENGTH-1 << " characters\n";
        }

        if(send(sockfd, nickname.c_str(), MAX_NICK_LENGTH, 0) == -1){
            cout << "Error: sending nickname\n";
            return 0;
        }
    }

    bool* status = new bool(true); // active app
    thread t(receiver, sockfd, status);
    t.detach();

    string s;
    while(status){
        getline(cin, s);
        
        if (!s.size())
            continue;
        else if(s.size() > MAX_MSG_LENGTH - 1 - MAX_NICK_LENGTH - 1 - SPEC_LENGTH){
            cout << "Error: " << MAX_MSG_LENGTH - MAX_NICK_LENGTH - 1 << " chars is the limit\n";
            continue;
        }else if(s == ":d" || !*status)
            break;

        if(send(sockfd, s.c_str(), MAX_MSG_LENGTH, 0) == -1){
            cout << "Error: your message hasn't sent\n";
        }
    }

    delete status;
    close(sockfd);
    return 0;
}