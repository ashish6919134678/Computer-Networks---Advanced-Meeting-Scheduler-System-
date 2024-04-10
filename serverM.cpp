#include  <iostream>
#include "common.h"
#include <stdio.h>

int main() {
    TcpConnection tcpserver(ServerM_Tcp_AddrInfo());
    UdpConnection udpserver(ServerM_Udp_AddrInfo()); 

    printf("Main Server is up and running.\n");

    map <int, vector<string>> backend_usernames;
    backend_usernames[ServerA_UdpPort] = usernames_recv(udpserver, ServerA_UdpPort);
    printf("Main Server received the username list from server A using UDP over port %d.\n", ServerM_UdpPort);
    backend_usernames[ServerB_UdpPort] = usernames_recv(udpserver, ServerB_UdpPort);
    printf("Main Server received the username list from server B using UDP over port %d.\n", ServerM_UdpPort);

    TcpConnection client(tcpserver.accept());

    while (1) {
        vector <string> client_usernames = usernames_recv(client);
        printf("Main Server received the request from the client using TCP over port %d.\n", ServerM_TcpPort);

        vector <string> backend1_usernames;
        vector <string> backend2_usernames;
        vector <string> notfound_usernames;

        for (const string &query: client_usernames) {
            int index = -1;
            for (const auto &store: backend_usernames) {
                for (const string &username : store.second) {
                    if (index == -1 && query == username) {
                        index = store.first;
                        break;
                    }
                }
            }
            if (index == -1) {
                notfound_usernames.push_back(query);
            }
            else if (index == ServerA_UdpPort) {
                backend1_usernames.push_back(query);
            }
            else {
                backend2_usernames.push_back(query);
            }
        }

        if (!notfound_usernames.empty()) {
            string users = usernames_tostring(notfound_usernames);
            printf("%s do not exist. Send a reply to the client.\n", users.c_str());
        }

        if (!backend1_usernames.empty()) {
            string users = usernames_tostring(backend1_usernames);
            printf("Found %s located at Server A. Send to Server A.\n", users.c_str());
        }

        if (!backend2_usernames.empty()) {
            string users = usernames_tostring(backend2_usernames);
            printf("Found %s located at Server B. Send to Server B.\n", users.c_str());
        }

        usernames_send(client, notfound_usernames);

        bool b1ok = !backend1_usernames.empty();
        bool b2ok = !backend2_usernames.empty();

        TimeList backend1_results, backend2_results;

        if (b1ok) {
            usernames_send(udpserver, ServerA_Udp_AddrInfo(), backend1_usernames);
            backend1_results = timelist_recv(udpserver, ServerA_UdpPort);
            printf("Main Server received from server A the intersection result using UDP over port %d:\n", ServerM_UdpPort);
            cout << timelist_tostring(backend1_results) << ".\n";
        }

        if (b2ok) {
            usernames_send(udpserver, ServerB_Udp_AddrInfo(), backend2_usernames);
            backend2_results = timelist_recv(udpserver, ServerB_UdpPort);
            printf("Main Server received from server B the intersection result using UDP over port %d:\n", ServerM_UdpPort);
            cout << timelist_tostring(backend2_results) << ".\n";
        }

        // send result between two intersection results to client
        TimeList result;
        if (b1ok && b2ok) {
            result = schedule(backend1_results, backend2_results);
        }
        else if (b1ok) {
            result = backend1_results;
        }
        else {
            result = backend2_results;
        }

        printf("Found the intersection between the results from server A and B:\n");
        cout << timelist_tostring(result) << ".\n";
        timelist_send(client, result);
        printf("Main Server sent the result to the client.\n");
    }
}

