#include <iostream>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <thread>
#include <vector>
#include <algorithm>
#include <cstring>
#include <map>

using namespace std;

#define SPEC_LENGTH 3
#define MAX_NICK_LENGTH 33
#define MAX_MSG_LENGTH 2048

vector<string> users;
map<int, bool>* state;

void echo(vector<int> const* socks, char const* msg, const int size){
    for(int i = 0; i < socks->size(); ++i){
        if((*state)[(*socks)[i]])
            send((*socks)[i], msg, size, 0);
    }
}

void receiver(vector<int> const* socks, const int id){
    const int sock = (*socks)[id];
    char msg[MAX_MSG_LENGTH], smsg[MAX_MSG_LENGTH];
    while(recv(sock, msg, sizeof(msg), 0) > 0){
        int cid;
        for(int i = 0; i < socks->size(); ++i){
            if((*socks)[i] == sock){
                cid = i;
                break;
            }
        }
        
        strcpy(smsg, users[cid].c_str());
        strcat(smsg, ": ");
        strcat(smsg, msg);

        for(int i = 0; i < socks->size(); ++i){
            if((*socks)[i] != sock && (*state)[(*socks)[i]])
                send((*socks)[i], smsg, MAX_MSG_LENGTH, 0);
        }
    }

    (*state)[sock] = false;
    char glhf[MAX_NICK_LENGTH + 23];
    strcpy(glhf, "Info: ");             // 6
    strcat(glhf, users[id].c_str());    // MAX_NICK
    strcat(glhf, " has disconnected");  // 17
    echo(socks, glhf, sizeof(glhf));
    cout << glhf << endl;
}

int main(){ 
    state = new map<int, bool>();
    int sockfd = socket(AF_INET, SOCK_STREAM, 0), sockfd_accept;
    if(sockfd == -1){
        cout << "Error: socket\n";
        return 0;
    }

    sockaddr_in addr, gaddr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(0);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if(bind(sockfd, (sockaddr*) &addr, sizeof(addr)) == -1){
        cout << "Error: socket\n";
        return 0;
    }

    struct sockaddr_in sin;
    socklen_t len = sizeof(sin);
    if (getsockname(sockfd, (struct sockaddr *)&sin, &len) == -1){
        cout << "Error: getsockname\n";
        return 0;
    }
    else{
        cout << "Info: server's port is " << ntohs(sin.sin_port) << "\n";
    }

    listen(sockfd, 0);
    vector<int>* socks = new vector<int>();

    int cnt = 0;
    while(true){
        sockfd_accept = accept(sockfd, NULL, NULL);

        if(sockfd_accept == -1){
        cout << "Error: accept\n";
        }else{
            char nickname[MAX_NICK_LENGTH];
            int recvres = recv(sockfd_accept, nickname, sizeof(nickname), 0);
            if(!recvres || recvres == -1){
                cout << "Error: an attempt to connect was made\n";
                close(sockfd_accept);
                continue;
            }

            users.push_back(nickname);
            socks->push_back(sockfd_accept);
            (*state)[sockfd_accept] = true;
            
            char welcome[MAX_NICK_LENGTH + 20];
            strcpy(welcome, "Info: ");  // 6
            strcat(welcome, nickname);  // MAX_NICK
            strcat(welcome, " has connected"); // 14
            echo(socks, welcome, sizeof(welcome));
            cout << welcome << endl;

            thread t(receiver, socks, cnt++);
            t.detach();
        }
    }

    for(int i = 0; i < socks->size(); ++i)
        close((*socks)[i]);
    
    // delete socks;
    // close(sockfd);
    // delete state;
    return 0;
}