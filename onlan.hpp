#ifndef onlan_hpp
#define onlan_hpp
#include <cstring>
#include <iostream>
#include <vector>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <unistd.h>
using namespace std;

extern struct matchInfo
{
    string serverName;
    int gameMode;
    int maxPlayers;
    int lvlSpd;
    int winCount;
} mInfo;

struct playerInfo
{
    string name;
    string address;
    bool ready;
};

extern vector<playerInfo> playerList;

extern bool isHost;

extern const int MAX_CONNECTIONS;
const int BUFFER_SIZE = 1024;
extern string playerName;
extern string tmpName;

class Server
{
    private:
        WSADATA wsaData;
        addrinfo *result, hints;
        SOCKET ListenSocket;
        vector<SOCKET> clientSocket;
        vector<string> msgToEachClient;
        vector<string> clientMsg;
    public:
        Server();
        ~Server();
        void closeServer();
        SOCKET getClientSocket( int client );
        int getClientNum();
        bool createServer();
        bool acceptConnection();

        void sendToClient();

        void receive();

        void makeMsg( string msg, int client );
        void closeClientSocket( int client );
        string getMsg( int client );

};

class Client
{
    private:
        WSADATA wsaData;
        addrinfo *result, *ptr, hints;
        SOCKET connectSocket;
        string recvMsg;
        bool connected;
        int position;
    public:
        vector<string> address;
        vector<string> serverName;
        Client();
        ~Client();
        void connectToServer();
        int getPosition();
        void sendToServer(string sendString);
        void receive();
        string getMsg();
        void closeSocket();
        bool isConnected();
};

extern Server server;
extern Client client;

#endif