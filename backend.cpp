#include "backend.h"

void backend(const struct addrinfo* info,
            string const &name,
            string const &filename) {

    int port = ntohs(((struct sockaddr_in *)info->ai_addr)->sin_port);

    UdpConnection server(info);

    printf("%s is up and running using UDP on port %d.\n", name.c_str(), port);

    // Read usernames from file and send to main udp server
    map<string, vector<pair<int, int>>> store = readusernames_fromFile(filename);
    vector <string> usernames;
    for (const auto &it : store) {
        usernames.push_back(it.first);
    }
    usernames_send(server, ServerM_Udp_AddrInfo(), usernames);
    printf("%s finished sending a list of usernames to Main Server.\n", name.c_str());

    while (1) {

        vector <string> query_usernames = usernames_recv(server, ServerM_UdpPort);
        printf("%s received the usernames from Main Server using UDP over port %d.\n",name.c_str(), port);

        // Run the algorithm
        TimeList result;
        for (int i = 0; i < (int) query_usernames.size(); ++i) {
            TimeList timelist = store[query_usernames[i]];
            if (i) {
                timelist = schedule(result, timelist);
            }
            result = timelist;
        }

        cout << "Found the intersection result: "
             << timelist_tostring(result)
             << " for " << usernames_tostring(query_usernames)
             << "." << endl;

        timelist_send(server, ServerM_Udp_AddrInfo(), result);
        printf("%s finished sending the response to Main Server.\n", name.c_str());
        fflush(stdout);
    }

}