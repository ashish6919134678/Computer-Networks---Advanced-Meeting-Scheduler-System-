#include <iostream>
#include <string>
#include <string.h>
#include <sys/socket.h>
#include "common.h"

vector <string> string_split(string const &str, const char *delimeter = " \t\n") {
    char *duplicate = strdup(str.c_str());
    vector <string> tokens;
    if (duplicate) {
        char *token = strtok(duplicate, delimeter);
        while (token) {
            tokens.push_back(token);
            token = strtok(NULL, delimeter);
        }
    }
    free(duplicate);
    return tokens;
}

int sockfd_toPortNumber(int fd) {
    struct sockaddr addr;
    socklen_t len = sizeof(addr);
    if (getsockname(fd, &addr, &len) == -1) {
        perror("sockfd_toPortNumber: getsockname");
        exit(1);
    }
    return ntohs(((struct sockaddr_in*)&addr)->sin_port);
}

int main() {

    TcpConnection client(ServerM_Tcp_AddrInfo(), true);

    const int port = sockfd_toPortNumber(client.filedescriptor());

    printf("Client is up and running.\n");

    string line;

    while (cin) {
        cout << "Please enter the usernames to check schedule availability:\n";
        getline(cin, line);
        vector <string> usernames = string_split(line);
        if (!usernames.empty()) {
            usernames_send(client, usernames);
            printf("Client finished sending the usernames to Main Server.\n");
            vector <string> notfound = usernames_recv(client);
            if (!notfound.empty()) {
                string users = usernames_tostring(notfound);
                printf("Client received the reply from the Main Server using TCP over port %d:\n", port);
                printf("%s do not exist.\n", users.c_str());
            }
            
            TimeList result = timelist_recv(client);
            string timelist_str = timelist_tostring(result);
            vector <string> filtered;
            for (string const &query : usernames) {
                if (find(notfound.begin(), notfound.end(), query) == notfound.end()) {
                    filtered.push_back(query);
                }
            }
            string joined = usernames_tostring(filtered);

            printf("Client received the reply from the Main Server using TCP over port %d:\n", port);
            printf("Time intervals %s works for %s.\n", timelist_str.c_str(), joined.c_str());
        }
        printf("-----Start a new request-----\n");
    }
}