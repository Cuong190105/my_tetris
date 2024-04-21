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

        //Stores sockets connected to a client
        vector<SOCKET> clientSocket;

        //Holds messages that will be sent to each client
        vector<string> msgToEachClient;

        //Holds messages from each clients
        vector<string> clientMsg;

        //Stores this server IP address
        string svAddress;
    public:
        Server();
        ~Server();

        //Shuts down the server & notify other clients about it
        void closeServer();

        //Returns a specific client's socket
        SOCKET getClientSocket( int client );

        //Returns the number of clients connecting to the server
        int getClientNum();

        //Returns svAddress
        string getIPAddressString();
        
        //Initializes Winsock & create listen socket
        bool createServer();

        //Accepts connect requests
        bool acceptConnection();

        //Broadcasts this server's info to clients so that they can find and connect to
        void broadcastInvitation();

        //Sends all messages stored in msgToEachClient to the corresponding client
        void sendToClient();

        //Receives messages from clients
        void receive();

        //Creates messages for a client and stores it for the next delivery
        void makeMsg( string msg, int client );

        //Ping all the clients to check if any of them are disconnected
        void pingClient();

        //Shuts down disconnected client's sockets
        void closeClientSocket( int client );

        //Retrieves the messages received
        string getMsg( int client );

};

class Client
{
    private:
        WSADATA wsaData;
        SOCKET connectSocket;
        string recvMsg;
        bool connected;
        int position;
    public:
        //Holds the received servers' info during scanning for available servers.
        vector<string> address;
        vector<string> serverName;
        
        Client();
        ~Client();

        //Scans for available servers
        void searchServer();

        //Connects to a server selected from the server list above
        void connectToServer( int serverNum );

        //Returns the position of this player in the lobby
        int getPosition();

        //Sends messages to the server
        void sendToServer(string sendString);

        //Receives messages from the server
        void receive();

        //Retrieves the stored incoming messages
        string getMsg();

        //Closes sockets & disconnects from server
        void closeSocket();

        //Check if the player is connecting to any server.
        bool isConnected();

        //Ping the server to check if it is still connected
        void pingServer();
};

extern Server server;
extern Client client;

#endif