#include "common.h"

const size_t Packet::DATASIZE;

// move index to first non-whitespace character
int seek_whitespaces(string const &str, int &index) {
    size_t n = str.size();
    while (index < (int) n && isspace(str[index])) {
        ++index;
    }
    return SUCCESS;
}

// move index to first non-whitespace char whose is not "character"
int seek_character(string const &str, int &index, char character) {
    int ret = ERROR;
    seek_whitespaces(str, index);   
    if (index < (int) str.size() && str[index] == character) {
        ++index;
        ret = SUCCESS;
    }
    return ret;
}

// Read number from current position
int seek_number(string const &str, int &index, int &number) {
    size_t n = str.size();
    size_t len = 0;
    number = 0;
    seek_whitespaces(str, index);
    while (index < (int) n && isdigit(str[index])) {
        char ch = str[index];
        number = number * 10 + (ch - '0');
        len += 1;
        index += 1;
    }
    return len?SUCCESS: ERROR;
}

int seek_username(string const &str, int &index, string &username) {
    size_t n = str.size();
    size_t len = 0;
    username.clear();
    seek_whitespaces(str, index);
    while (index < (int) n && islower(str[index])) {
        username.push_back(str[index]);
        len++;
        index++;
    }
    return len? SUCCESS: ERROR;
}

int inputline_split(
    string const& inputline, 
    string &return_username,
    vector <pair<int, int>> &return_timeslice) {

    string username;
    vector <pair<int, int>> timeslice;
    string str = inputline;
    int index = 0;
    int success = SUCCESS;
    
    success &= seek_username(str, index, username);
    success &= seek_character(str, index, ';');
    success &= seek_character(str, index, '[');

    while ((success == SUCCESS) && seek_character(str, index, ']') == ERROR) {
        int number1;
        int number2;
        if (seek_character(str, index, ',') == ERROR) {
            if (!timeslice.empty()) {
                success = ERROR;
            }
        }

        success &= seek_character(str, index, '[');
        success &= seek_number(str, index, number1);
        success &= seek_character(str, index, ',');
        success &= seek_number(str, index, number2);
        success &= seek_character(str, index, ']');

        if (success) {
            timeslice.push_back({number1, number2});
        }
    }

    if (success) {
        return_username = username;
        return_timeslice = timeslice;
    }

    return success;
}

map<string, vector<pair<int, int>>>
readusernames_fromFile(string const &filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Cannot open file\n";
        exit(1);
    }
    map<string, vector<pair<int,int>>> ret;
    string line;
    while (getline(file, line)) {
        string username;
        vector<pair<int, int>> timeslices;
        if (line.empty()) {
        }
        else if (inputline_split(line, username, timeslices) == ERROR) {
            cerr << "Cannot extract username, timeslices from input line\n";
            exit(1);
        }
        else {
            ret[username] = timeslices;
        }
    }
    return ret;
}

void timelist_pack(TimeList const &timelist, char **ret_buf, size_t *ret_len) {
    uint64_t numstamps = timelist.size();
    size_t buflen = sizeof(numstamps) + numstamps * sizeof(int) * 2;
    char *buf = new char[buflen];
    memset(buf, 0, buflen);
    size_t pos = 0;
    memcpy(buf + pos, (void*)&numstamps, sizeof(numstamps));
    pos += sizeof(numstamps);
    for (TimeStamp const &timestamp: timelist) {
        for (int t : {timestamp.first, timestamp.second}) {
            memcpy(buf + pos, (void*)&t, sizeof(t));
            pos += sizeof(t);
        }
    }
    *ret_buf = buf;
    *ret_len = buflen;
}

TimeList timelist_unpack(char *buf, size_t len) {
    TimeList timelist;
    if (len >= sizeof(uint64_t)) {
        size_t nbytes = 0;
        uint64_t numstamps;
        memcpy((void*)&numstamps, buf + nbytes, sizeof(numstamps));
        nbytes += sizeof(numstamps);
        for (int i = 0; i < (int) numstamps; ++i) {
            int values[2];
            for (int index : {0, 1}) {
                memcpy((void*)&values[index], buf + nbytes, sizeof(int));
                nbytes += sizeof(int);
            }
            timelist.push_back({values[0], values[1]});
        }
    }
    return timelist;
}

void usernames_pack(vector <string> const &usernames, char **ret_buf, size_t *ret_len) {
    uint64_t numusers = usernames.size();
    size_t buflen = sizeof(numusers) + usernames.size() * USERNAME_MAXSIZE;
    char *buf = new char[buflen];
    memset(buf, 0, buflen);
    size_t pos = 0;
    memcpy(buf + pos, (void*)&numusers, sizeof(numusers));
    pos += sizeof(numusers);
    for (string const &username: usernames) {
        memcpy(buf + pos, username.c_str(), username.size());
        pos += USERNAME_MAXSIZE;
    }
    *ret_buf = buf;
    *ret_len = buflen;
}

vector <string> usernames_unpack(char *buf, size_t len) {
    vector <string> usernames;
    if (len >= sizeof(uint64_t)) {
        size_t nbytes = 0;
        uint64_t numusers;
        memcpy((void*)&numusers, buf + nbytes, sizeof(numusers));
        nbytes += sizeof(numusers);
        for (int i = 0; i < (int) numusers; ++i) {
            string username(buf + nbytes);
            nbytes += USERNAME_MAXSIZE;
            usernames.push_back(username);
        }
    }
    return usernames;
}


void timelist_send(TcpConnection &connection, TimeList const &timelist) {
    char *buf;
    size_t len;
    timelist_pack(timelist, &buf, &len);
    connection.send(buf, len);
    delete[] buf;
}

void timelist_send(UdpConnection &connection, const struct addrinfo *info, TimeList const &timelist) {
    char *buf;
    size_t len;
    timelist_pack(timelist, &buf, &len);
    connection.send(info, buf, len);
    delete[] buf;
}

void usernames_send(TcpConnection &connection, vector <string> const &usernames) {
    char *buf;
    size_t len;
    usernames_pack(usernames, &buf, &len);
    connection.send(buf, len);
    delete[] buf;
}

void usernames_send(UdpConnection &connection, const struct addrinfo *info, vector <string> const &usernames) {
    char *buf;
    size_t len;
    usernames_pack(usernames, &buf, &len);
    connection.send(info, buf, len);
    delete[] buf;
}


vector <string> usernames_recv(UdpConnection &connection, int port) {
    char *buf = NULL;
    size_t len = 0;
    connection.recv(port, &buf, &len);
    vector <string> usernames = usernames_unpack(buf, len);
    delete[] buf;
    return usernames;
}

vector <string> usernames_recv(TcpConnection &client) {
    char *buf = NULL;
    size_t len = 0;
    client.recv(&buf, &len);
    vector <string> usernames = usernames_unpack(buf, len);
    delete[] buf;
    return usernames;  
}

TimeList timelist_recv(UdpConnection &connection, int port) {
    char *buf = NULL;
    size_t len = 0;
    connection.recv(port, &buf, &len);
    TimeList timelist = timelist_unpack(buf, len);
    delete[] buf;
    return timelist;
}

TimeList timelist_recv(TcpConnection &client) {
    char *buf = NULL;
    size_t len = 0;
    client.recv(&buf, &len);
    TimeList timelist = timelist_unpack(buf, len);
    delete[] buf;
    return timelist;  
}

void print_usernames(string const &name, vector <string> const &usernames) {
    cout << name << endl;
    for (string const &username : usernames) {
        cout << "Username: " << username << endl;
    }
    cout << endl;
}

struct addrinfo*
addrinfo(const char *hostname, int portno, int ai_family, int ai_socktype) {
    struct addrinfo hints;
    struct addrinfo *servinfo;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = ai_family;
    hints.ai_socktype = ai_socktype;
    char port[1024];
    snprintf(port, 1024, "%d", portno);
    int status = getaddrinfo(hostname, port, &hints, &servinfo);
    if (status == -1) {
        perror("getaddrinfo");
        exit(1);
    }
    return servinfo;
}

const struct addrinfo* ServerM_Tcp_AddrInfo() {
    static unique_ptr<struct addrinfo, void(*)(struct addrinfo*)> info(nullptr, freeaddrinfo);
    static int first = 1;
    if (first) {
        first = 0;
        info.reset(addrinfo(HOSTNAME, ServerM_TcpPort, AF_INET, SOCK_STREAM));
    }
    return info.get();
}


const struct addrinfo* ServerM_Udp_AddrInfo() {
    static unique_ptr<struct addrinfo, void(*)(struct addrinfo*)> info(nullptr, freeaddrinfo);
    static int first = 1;
    if (first) {
        first = 0;
        info.reset(addrinfo(HOSTNAME, ServerM_UdpPort, AF_INET, SOCK_DGRAM));
    }
    return info.get();
}

const struct addrinfo* ServerA_Udp_AddrInfo() {
    static unique_ptr<struct addrinfo, void(*)(struct addrinfo*)> info(nullptr, freeaddrinfo);
    static int first = 1;
    if (first) {
        first = 0;
        info.reset(addrinfo(HOSTNAME, ServerA_UdpPort, AF_INET, SOCK_DGRAM));
    }
    return info.get();
}

const struct addrinfo* ServerB_Udp_AddrInfo() {
    static unique_ptr<struct addrinfo, void(*)(struct addrinfo*)> info(nullptr, freeaddrinfo);
    static int first = 1;
    if (first) {
        first = 0;
        info.reset(addrinfo(HOSTNAME, ServerB_UdpPort, AF_INET, SOCK_DGRAM));
    }
    return info.get();
}

UdpConnection::UdpConnection(const struct addrinfo *info) 
    : sockfd(-1) {

    if ((sockfd = socket(info->ai_family, info->ai_socktype, info->ai_protocol)) == -1) {
        perror("socket");
        exit(1);
    }
    if (bind(sockfd, info->ai_addr, info->ai_addrlen) == -1) {
        close(sockfd);
        sockfd = -1;
        perror("UdpServer::UdpServer::bind");
        exit(1);
    }
}

UdpConnection::~UdpConnection() {
    if (sockfd != -1) {
        close(sockfd);
    }
}

void UdpConnection::send(const struct addrinfo *info, char *buf, size_t len) {
    vector <Packet> packets;
    while (len) {
        size_t datasize = min(Packet::DATASIZE, len);
        Packet packet;
        packet.length = datasize;
        memcpy(packet.data, buf, datasize);
        len -= datasize;
        buf += datasize;
        packets.push_back(packet);
    }

    for (int i = 0; i < (int) packets.size(); ++i) {
        Packet &packet = packets[i];
        packet.sequence_number = i;
        packet.count = (int) packets.size();
        char buf[PAYLOAD_SIZE];
        packet.get(buf);
        ssize_t nbytes = sendto(sockfd, buf, PAYLOAD_SIZE, 0, info->ai_addr, info->ai_addrlen);
        if (nbytes != PAYLOAD_SIZE) {
            fprintf(stderr, "Send data = %ld bytes\n", nbytes);
            perror("packet_sendpacket::sendto");
            exit(1);
        }
        if (recvfrom(sockfd, buf, PAYLOAD_SIZE, 0, NULL, NULL) == -1) {
            perror("packet_sendpacket: recvfrom");
            exit(1);
        }
    }    
}


void UdpConnection::recv(int sender_port, char **ret_buf, size_t *ret_len) {
    while (!recv_frombuffer(sender_port, ret_buf, ret_len)) {
        char buf[PAYLOAD_SIZE];
        struct sockaddr_storage their_addr;
        socklen_t addr_len = sizeof(their_addr);
        memset(&their_addr, 0, addr_len);
        ssize_t nbytes;
        nbytes = recvfrom(sockfd, buf, PAYLOAD_SIZE, 0, (struct sockaddr *)&their_addr, &addr_len);
        if (nbytes != PAYLOAD_SIZE) {
            fprintf(stderr, "packet_recv::recvfrom received: %ld bytes\n", nbytes);
            perror("packet_recv::recvfrom");
            exit(1);
        }
        Packet packet;
        packet.load(buf);
        int key = ntohs(((struct sockaddr_in *)((struct sockaddr*)&their_addr))->sin_port);
        received[key].push_back(packet);
        nbytes = sendto(sockfd, buf, PAYLOAD_SIZE, 0, (struct sockaddr*)&their_addr, addr_len);
        if (nbytes != PAYLOAD_SIZE) {
            fprintf(stderr, "packet_recv::sendto sent %ld bytes\n", nbytes);
            perror("packet_recv::sendto");
            exit(1);
        }
    }
}

bool UdpConnection::recv_frombuffer(int sender_port, char **ret_buf, size_t *ret_len) {
    vector <Packet> &packets = received[sender_port];
    if (packets.empty() || packets.size() < packets.back().count) {
        return false;
    }
    size_t totalbytes = 0;
    for (const Packet &packet : packets) {
        totalbytes += packet.length;
    }
    sort(packets.begin(), packets.end(), [](const Packet &lhs, const Packet &rhs){
        return lhs.sequence_number < rhs.sequence_number;
    });
    char *buf = new char[totalbytes];
    size_t len = 0;
    for (const Packet &packet : packets) {
        memcpy(buf + len, packet.data, packet.length);
        len += packet.length;
    }
    *ret_buf = buf;
    *ret_len = len;
    packets.clear();
    return true;
}

TcpConnection::TcpConnection(const struct addrinfo *info, bool client)
    : sockfd(-1)
{
    if ((sockfd = socket(info->ai_family, info->ai_socktype, info->ai_protocol)) == -1) {
        perror("TcpServer::TcpServer::socket");
        exit(1);
    }

    if (!client) {
        int yes = 1;
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
            perror("TcpServer::TcpServer::setsockopt");
            exit(1);
        }

        if (bind(sockfd, info->ai_addr, info->ai_addrlen) == -1) {
            perror("TcpServer::TcpServer::bind");
            exit(1);
        }

        if (listen(sockfd, 10) == -1) {
            perror("TcpServer::TcpServer: listen");
            exit(1);
        }
    }
    else {
        if (connect(sockfd, info->ai_addr, info->ai_addrlen) == -1) {
            perror("TcpServer::TcpServer: connect");
            exit(1);
        }
    }
}

TcpConnection::TcpConnection(int clientfd)
    : sockfd(clientfd) 
{}

int TcpConnection::accept() {
    int fd = ::accept(sockfd, NULL, NULL);
    if (fd == -1) {
        perror("TcpServer::accept: accept");
        exit(1);
    }
    return fd;
}

TcpConnection::~TcpConnection() {
    if (sockfd != -1) {
        close(sockfd);
    }
}

void TcpConnection::send(char *buf, size_t len) {
    _send((char *)&len, sizeof(len));
    _send(buf, len);
}

void TcpConnection::recv(char **ret_buf, size_t *ret_len) {
    size_t len = 0;
    _recv((char*)&len, sizeof(len));
    char *buf = new char[len];
    _recv(buf, len);
    *ret_buf = buf;
    *ret_len = len; 
}

void TcpConnection::_send(char *buf, size_t len) {
    while (len) {
        ssize_t nbytes = write(sockfd, buf, len);
        if (nbytes == -1) {
            if (errno == EINTR) {
                continue;
            }
            perror("TcpServer::_send");
            exit(1);
        }
        else {
            len -= nbytes;
            buf += nbytes;
        }
    }
}

void TcpConnection::_recv(char *buf, size_t len) {
    while (len) {
        ssize_t nbytes = read(sockfd, buf, len);
        if (nbytes == -1 && errno != EINTR) {
            perror("TcpServer::_recv: read");
            exit(1);
        }
        else if (nbytes == 0) {
            break;
        }
        else {
            len -= nbytes;
            buf += nbytes;
        }
    }
}

string usernames_tostring(vector <string> const &usernames) {
    string ret;
    for (int i = 0; i < (int)usernames.size(); ++i) {
        if (i) {
            ret += ", ";
        }
        ret += usernames[i];
    }
    return ret;
}


bool overlapping(TimeStamp const &lhs, TimeStamp const &rhs, TimeStamp &result) {
    int _min = std::max(lhs.first, rhs.first);
    int _max = std::min(lhs.second, rhs.second);
    if (_min < _max) {
        result.first = _min;
        result.second = _max;
        return true;
    }
    return false;
}

TimeList schedule(TimeList const &lhs, TimeList const &rhs) {
    TimeList result;
    for (int i = 0; i < (int) lhs.size(); ++i) {
        for (int j = 0; j < (int) rhs.size(); ++j) {
            TimeStamp timestamp;
            if (overlapping(lhs[i], rhs[j], timestamp)) {
                result.push_back(timestamp);
            }
        }
    }
    return result;
}

string timelist_tostring(TimeList const &timelist) {
    string result = "";
    result += "[";
    for (int i = 0; i < (int)timelist.size(); ++i) {
        if (i) {
            result += ",";
        }
        result += "[";
        result += to_string(timelist[i].first);
        result += ",";
        result += to_string(timelist[i].second);
        result += "]";
    }
    result += "]";
    return result;
}


