#ifndef __COMMON_H__
#define __COMMON_H__

#define USCID               (678)
#define ServerA_UdpPort     (21000+USCID)
#define ServerB_UdpPort     (22000+USCID)
#define ServerM_UdpPort     (23000+USCID)
#define ServerM_TcpPort     (24000+USCID)
#define HOSTNAME            ("127.0.0.1")

#define PAYLOAD_SIZE        (512)
#define USERNAME_MAXSIZE    (25)
#define SUCCESS             (1)
#define ERROR               (0)

#include <string>
#include <vector>
#include <map>
#include <utility>
#include <fstream>
#include <iostream>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <map>
#include <ctype.h>
#include <memory>
#include <algorithm>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <error.h>
#include <unistd.h>


using namespace std;

using TimeList = vector <pair<int, int>>;
using TimeStamp = pair<int, int>;

// Only used for UDP.
// Data is boken down into packets and sent.
// Attach them back on the receiver side.
class Packet {
public:
    static const size_t DATASIZE = PAYLOAD_SIZE - (sizeof(uint64_t) * 3);
    uint64_t    sequence_number;    // packet number
    uint64_t    count;              // Total number of packets (original data is broken down into packets)
    uint64_t    length;             // length of content inside data buffer
    char        data[DATASIZE];
    static uint64_t packetnumber() {
        static uint64_t id = 0;
        return id++;
    }
    void get(char *buf) const {
        size_t n = 0;
        memcpy(buf + n, (void*)&sequence_number, sizeof(sequence_number));
        n += sizeof(sequence_number);
        memcpy(buf + n, (void*)&count, sizeof(count));
        n += sizeof(count);
        memcpy(buf + n, (void*)&length, sizeof(length));
        n += sizeof(length);
        memcpy(buf + n, data, DATASIZE);
    }
    void load(char *buf) {
        size_t n = 0;
        memcpy((void*)&sequence_number, buf + n, sizeof(sequence_number));
        n += sizeof(sequence_number);
        memcpy((void*)&count, buf + n, sizeof(count));
        n += sizeof(count);
        memcpy((void*)&length, buf + n, sizeof(length));
        n += sizeof(length);
        memcpy(data, buf + n, DATASIZE);
    }
};

class UdpConnection {
public:
    UdpConnection(const struct addrinfo *info);
    ~UdpConnection();
    void send(const struct addrinfo *info, char *buf, size_t len);
    void recv(int sender_port, char **ret_buf, size_t *ret_len);
private:
    map <int, vector<Packet>> received;
    bool recv_frombuffer(int sender_port, char **ret_buf, size_t *ret_len);
    int sockfd;
};


class TcpConnection {
public:
    ~TcpConnection();
    TcpConnection(int clientfd);    // client mode
    TcpConnection(const struct addrinfo *serveinfo, bool client = false);    // server mode
    int accept();
    int filedescriptor() { return sockfd; }
    void send(char *buf, size_t len);
    void recv(char **ret_buf, size_t *ret_len);
private:
    void _send(char *buf, size_t len);
    void _recv(char *buf, size_t len);
    int sockfd;
};

int inputline_split(
    string const& inputline, 
    string &return_username,
    vector <pair<int, int>> &return_timeslice);


map<string, vector<pair<int, int>>> readusernames_fromFile(string const &filename);

struct addrinfo* addrinfo(const char *hostname, int portno, int ai_family, int ai_socktype);

const struct addrinfo* ServerM_Tcp_AddrInfo();
const struct addrinfo* ServerM_Udp_AddrInfo();
const struct addrinfo* ServerA_Udp_AddrInfo();
const struct addrinfo* ServerB_Udp_AddrInfo();

// Time slices algorithm
bool overlapping(TimeStamp const &lhs, TimeStamp const &rhs, TimeStamp &result);
TimeList schedule(TimeList const &lhs, TimeList const &rhs);

// Send and recv usernames
void usernames_send(TcpConnection &connection, vector <string> const &usernames);
void usernames_send(UdpConnection &connection, const struct addrinfo *info, vector <string> const &usernames);
vector <string> usernames_recv(UdpConnection &connection, int port);
vector <string> usernames_recv(TcpConnection &client);

// Send and receive timelist
TimeList timelist_recv(TcpConnection &client);
TimeList timelist_recv(UdpConnection &connection, int port); // Port is the sender's port
void timelist_send(UdpConnection &connection, const struct addrinfo *info, TimeList const &timelist);
void timelist_send(TcpConnection &connection, TimeList const &timelist);

// Convert to string representation
string timelist_tostring(TimeList const &timelist);
string usernames_tostring(vector <string> const &usernames);

// Encode/decode and store the usernames to char array
void usernames_pack(vector <string> const &usernames, char **ret_buf, size_t *ret_len);
vector <string> usernames_unpack(char *buf, size_t len);

// Encode/decode timelist to char array
void timelist_pack(TimeList const &timelist, char **ret_buf, size_t *ret_len);
TimeList timelist_unpack(char *buf, size_t len);


#endif