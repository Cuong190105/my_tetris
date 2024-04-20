#include "onlan.hpp"
#include <thread>
#include <atomic>
#include <windows.h>
using namespace std;

#pragma comment(lib, "ws2_32")

#define DEFAULT_PORT "27015"
#define DEFAULT_BUFLEN 512
const int MAX_CONNECTIONS = 3;
string playerName = "";
string tmpName = "";
bool isHost = false;

struct matchInfo mInfo;

vector<playerInfo> playerList;

Server server;
Client client;

Server::Server()
{
    result = NULL;
    ZeroMemory(&hints, sizeof (hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;
    ListenSocket = INVALID_SOCKET;
    msgToEachClient = vector<string>(MAX_CONNECTIONS, "");
    clientMsg = vector<string>(MAX_CONNECTIONS, "");
}

Server::~Server() { closeServer(); }

void Server::closeServer()
{
    freeaddrinfo(result);
    result = NULL;
    closesocket(ListenSocket);
    for (int i = 0; i < clientSocket.size(); i++)
    {
        shutdown(clientSocket[i], SD_BOTH);
        closesocket(clientSocket[i]);
    }
    WSACleanup();
}

SOCKET Server::getClientSocket( int client )
{
    return clientSocket[client];
}

int Server::getClientNum()
{
    return clientSocket.size();
}

bool Server::createServer()
{
    WSAStartup(MAKEWORD(2, 2), &wsaData);
    if ( getaddrinfo( NULL, DEFAULT_PORT, &hints, &result ) == 0 )
    {
        ListenSocket = socket( result->ai_family, result->ai_socktype, result->ai_protocol );
    }
    if (ListenSocket == INVALID_SOCKET)
    {
        cout << "Error at socket(): " << WSAGetLastError() << endl;
        freeaddrinfo(result);
        return false;
    }
    int i = 1;
    setsockopt(ListenSocket, SOL_SOCKET, SO_REUSEADDR, (char *)&i, sizeof(i));

    int info = bind( ListenSocket, result->ai_addr, (int)result->ai_addrlen );
    if (info == SOCKET_ERROR)
    {
        cout << "bind failed with error: " << WSAGetLastError() << endl;
        closesocket(ListenSocket);
    }

    freeaddrinfo(result);

    if ( listen( ListenSocket, SOMAXCONN ) == SOCKET_ERROR )
    {
        cout << "Listen failed with error: " << WSAGetLastError() << endl;
        closesocket(ListenSocket);
        return false;
    }
    unsigned long b = 1;
    ioctlsocket(ListenSocket, FIONBIO, &b);
    return true;
}

bool Server::acceptConnection()
{
    SOCKET ClientSocket = INVALID_SOCKET;
    bool success = true;
    sockaddr *addr;
    ClientSocket = accept(ListenSocket, addr, NULL );
    
    if (ClientSocket == INVALID_SOCKET) success = false;
    else {
        unsigned long b = 1;
        ioctlsocket(ClientSocket, FIONBIO, &b);
        Sleep(10);
        int buf = 16;
        char tmp[buf];
        memset(&tmp, '\0', buf);
        int info = recv(ClientSocket, tmp, buf, 0);
        sockaddr_in *addr_in = (sockaddr_in*)addr;
        clientSocket.push_back(ClientSocket);
        playerList.push_back(playerInfo{tmp, inet_ntoa(addr_in->sin_addr), false});
    }
    return success;
}

void Server::sendToClient()
{
    for (int i = 0; i < clientSocket.size(); i++)
    {
        if ( msgToEachClient[i].length() != 0 )
        {
            cout << msgToEachClient[i].c_str() << endl;
            if ( send( clientSocket[i], msgToEachClient[i].c_str(), msgToEachClient[i].length(), 0 ) == SOCKET_ERROR )
            {
                closeClientSocket(i);
            }
            else
            {
                msgToEachClient[i] = "";
            }
        }
    }
}

void Server::receive()
{
    for (int i = 0; i < clientSocket.size(); i++)
    {
        // unsigned long b = 1;
        // ioctlsocket(clientSocket[i], FIONBIO, &b);
        char tmp[BUFFER_SIZE];
        memset(&tmp, 0, BUFFER_SIZE);
        int info = recv( clientSocket[i], tmp, BUFFER_SIZE, 0 );
        if ( info > 0 )
        {
            clientMsg[i] = tmp;
        }
        else if ( WSAGetLastError() != WSAEWOULDBLOCK )
        {
            cout << WSAGetLastError() << " " << "Socket closed" << endl;
            closeClientSocket(i);
            continue;
        }
    }
    for (int i = clientSocket.size() - 1; i > -1; i--)
    {
        if ( clientMsg[i] == "quit" )
        {
            cout << "User " << clientSocket[i] << " disconnected" << endl;
            char tmp[] = "4";
            send(clientSocket[i], tmp, strlen(tmp), 0);
            closeClientSocket(i);
        }
    }
}

void Server::makeMsg( string msg, int client )
{
    msgToEachClient[client] = msg;
}

string Server::getMsg( int client )
{
    string tmp = clientMsg[client];
    clientMsg[client] = "";
    return tmp;
}

void Server::closeClientSocket( int client )
{
    shutdown(clientSocket[client], SD_BOTH);
    closesocket(clientSocket[client]);
    clientSocket.erase(clientSocket.begin() + client);
    clientMsg.erase(clientMsg.begin() + client);
    msgToEachClient.erase(msgToEachClient.begin() + client);
    clientMsg.push_back("");
    msgToEachClient.push_back("");
}

Client::Client()
{
    result = NULL;
    ptr = NULL;
    ZeroMemory( &hints, sizeof(hints) );
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    connected = false;
    connectSocket = INVALID_SOCKET;
    position = -1;
}

Client::~Client() { closeSocket(); }

bool Client::isConnected() { return connected; }

void Client::connectToServer()
{
    WSAStartup(MAKEWORD(2, 2), &wsaData);
    if ( getaddrinfo( NULL, DEFAULT_PORT, &hints, &result ) == 0 )
    {
        for (ptr = result; ptr != NULL; ptr = ptr->ai_next)
        {
            connectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
            if (connectSocket == INVALID_SOCKET) { cout << "Failed to create socket" << endl; continue;}
            int info = connect(connectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
            if (info == SOCKET_ERROR) {cout << "Failed to connect" << endl; continue;}
            else
            {
                connected = true;
                unsigned long b = 1; ioctlsocket(connectSocket, FIONBIO, &b);
                send(connectSocket, playerName.c_str(), playerName.size(), 0);
                return;
            }
        }
    }
    closeSocket();
}

void Client::sendToServer(string sendString)
{
    int info = send( connectSocket, sendString.c_str(), sendString.length(), 0 );
    if (info == SOCKET_ERROR)
    {
        closeSocket();
    }
}

void Client::receive()
{
    char tmp[BUFFER_SIZE];
    memset(&tmp, 0, BUFFER_SIZE);
    int info = recv( connectSocket, tmp, BUFFER_SIZE, 0 );
    if ( info == SOCKET_ERROR && WSAGetLastError() != WSAEWOULDBLOCK )
    {
        closesocket(connectSocket);
        WSACleanup();
    }
    else
    {
        recvMsg = tmp;
    }
}

string Client::getMsg()
{
    string res = recvMsg;
    recvMsg = "";
    return res;
}

void Client::closeSocket()
{
    connected = false;
    shutdown( connectSocket, SD_BOTH );
    closesocket(connectSocket);
    freeaddrinfo(result);
    result = NULL;
    if (ptr != NULL)
    {
        freeaddrinfo(result);
        ptr = NULL;
    }
    WSACleanup();
}

int Client::getPosition() { return position; }

// atomic<bool> stop;

// void SvRun()
// {
//     Server sv;
//     cout << "Starting server" << endl;
//     if ( sv.createServer() )
//     {
//         while (!stop)
//         {
//             if ( sv.getClientNum() < MAX_CONNECTIONS )
//             {
//                 bool acpStatus = sv.acceptConnection();
//                 if (acpStatus)
//                 {
//                     cout << "New connection: " << sv.getClientSocket( sv.getClientNum() - 1 ) << endl;
//                 }
//             }
//             Sleep( 10 );
//             if ( sv.getClientNum() != 0 )
//             {
//                 sv.receive();
//                 for(int i = 0; i < sv.getClientNum(); i++)
//                 {
//                     string msg = sv.getMsg( i );
//                     if (msg.length() > 0)
//                     {
//                         cout << "User " << sv.getClientSocket(i) << " sent: " << msg << endl;
//                         sv.makeMsg( to_string(msg.length()), i);
//                     }
//                 }
//                 sv.sendToClient();
//             }
//         }
//     }
// }
// int main()
// {
//     string input;
//     cout << "Be server or client?(S/c): ";
//     getline(cin, input);
//     char i = input[0];
//     if ( toupper(i) == 'S') {
//         stop = false;
//         thread th (SvRun);
//         char input;
//         do{
//             cin >> input;
//         } while ( input != 'q');
//         stop = true;
//         th.join();
//         // Server sv;
//         // cout << "Starting server" << endl;
//         // if ( sv.createSocket() )
//         // {
//         //     if ( sv.bindSocket() )
//         //     {
//         //         if ( sv.initialListener() )
//         //         {
//         //             while (!stop)
//         //             {
//         //                 if ( sv.getClientNum() < MAX_CONNECTIONS )
//         //                 {
//         //                     bool acpStatus = sv.acceptConnection();
//         //                     if (acpStatus)
//         //                     {
//         //                         cout << "New connection: " << sv.getClientSocket( sv.getClientNum() - 1 ) << endl;
//         //                     }
//         //                 }
//         //                 if ( sv.getClientNum() != 0 )
//         //                 {
//         //                     sv.receive();
//         //                     for(int i = 0; i < sv.getClientNum(); i++)
//         //                     {
//         //                         string msg = sv.getMsg( i );
//         //                         if (msg.length() > 0)
//         //                         {
//         //                             cout << "User " << sv.getClientSocket(i) << " sent: " << msg << endl;
//         //                             sv.makeMsg( to_string(msg.length()), i);
//         //                         }
//         //                     }
//         //                     sv.sendToClient();
//         //                 }
//         //             }
//         //         }
//         //     }
//         // }
//     } else {
//         Client cl;
//         bool terminate = false;
//         while (!terminate)
//         {
//             cout << "Connecting" << endl;
//             while ( !cl.isConnected() ) cl.connectToServer();
//             cout << "Connected" << endl;
//             stop = false;
//             while (!stop)
//             {
//                 string data;
//                 cout << "Type to send: ";
//                 getline(cin, data);
//                 cl.sendToServer( data );
//                 if (data == "quit") {stop = true; cl.closeSocket();}
//                 Sleep(10);
//                 if ( cl.isConnected() )
//                 {
//                     cl.receive();
//                     cout << "Received from server: " << cl.getMsg() << endl;
//                 }
//             }
//             string ch;
//             cout << "Disconnected. Find a new server?(Y/n) " << endl;
//             getline(cin, ch);
//             if (ch[0] == 'n') terminate = true;
//         }
//     }
//     return 0;
// }