#include <iostream>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <thread>
#include <vector>
#include <algorithm>
#include <cstring>

#include "inpset.h"

using namespace std;

#define SPEC_LENGTH         2   // ": "
#define MAX_NICK_LENGTH     33
#define MAX_MSG_LENGTH      2048
#define FULL_MSG_LENGTH     (MAX_MSG_LENGTH - MAX_NICK_LENGTH - SPEC_LENGTH - 1)

string inpStr;  // current message
void receiver(int sockfd, bool* status){
    char msg[MAX_MSG_LENGTH];
    while(recv(sockfd, msg, sizeof(msg), 0) > 0){
        bool spaceFlag = false;
        const int width = get_term_width();

        int buffSize = (strlen(msg) / width) * width + width + 1;
        char strs[buffSize];
        memset(&strs, ' ', sizeof(strs));
        strcpy(strs, msg);
        strs[strlen(msg)] = ' ';
        strs[buffSize - 1] = '\0';

        if(inpStr.size() > 0){
            for(int i = 0; i < (inpStr.size() - 1) / width; ++i)
                cout << "\x1b[A";
            cout << "\r";
        }
        
        cout << strs << "\n";
        cout << inpStr;
        cout.flush();
    }

    cout << "Error: the connection was lost\n";
    *status = false;
}

bool is_valid_nick(string str){
    if(str.size() < 1 || str.size() > MAX_NICK_LENGTH-1)
        return false;

    for(int i = 0; i < str.size(); ++i){
        if(str[i] >= 48 && str[i] <= 57 
        || str[i] >= 65 && str[i] <= 90
        || str[i] >= 97 && str[i] <= 122)
            continue;
        else
            return false;
    }

    return true;
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
        getline(cin, nickname); // skip first \n

        cout << "Enter your nickname: ";
        while(getline(cin, nickname) && !is_valid_nick(nickname)){
            cout << "Info: enter >= 1 & <= " << MAX_NICK_LENGTH-1 << " numbers or letters: ";
        }

        if(send(sockfd, nickname.c_str(), MAX_NICK_LENGTH, 0) == -1){
            cout << "Error: sending nickname\n";
            return 0;
        }
    }

    bool* status = new bool(true); // active app
    thread t(receiver, sockfd, status);
    t.detach();

    char ch;
    initTermios(false);
    while(*status){
        ch = getchar();

        // ASCII table, 32 = space, 126 = ~
        if(ch >= 32 && ch <= 126){
            cout << ch;
            inpStr.push_back(ch);
        }
        else if(ch == 10){  // enter
            cout << "\n";
            string s(inpStr);
            inpStr.clear();

            if (!s.size()){
                continue;
            }else if(s == ":d" || !*status)
                break;

            if(send(sockfd, s.c_str(), FULL_MSG_LENGTH+1, 0) == -1){
                cout << "Error: while sending the message\n";
            }
        }
        else if(ch == 127){
            cout << "\b \b";
            inpStr.pop_back();
        }
        else{
            if(inpStr.size() + 1 > FULL_MSG_LENGTH)
                cout << "\b";
            inpStr.push_back(ch);
        }
    }

    // resetTermios();
    // delete status;
    // close(sockfd);
    return 0;
}