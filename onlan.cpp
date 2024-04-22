#include "onlan.hpp"
#include <thread>
#include <atomic>
#include <windows.h>
#include <map>
#include <iphlpapi.h>
using namespace std;

#pragma comment(lib, "ws2_32")

#define DEFAULT_PORT "27015"
#define DEFAULT_BUFLEN 512
const int MAX_CONNECTIONS = 3;
string playerName = "";
string tmpName = "";
bool isHost = false;
const char delimiter = '\x1F';
const char endMsg = '\x1D';
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
    for (int i = 0; i < clientSocket.size(); i++)
    {
        makeMsg("0quit", i);
    }
    sendToClient();
    if ( result != NULL )
    {
        freeaddrinfo(result);
        result = NULL;
    }
    closesocket(ListenSocket);
    for (int i = 0; i < clientSocket.size(); i++)
    {
        shutdown(clientSocket[i], SD_BOTH);
        closesocket(clientSocket[i]);
    }
    WSACleanup();
    msgToEachClient.clear();
    clientMsg.clear();
    playerList.clear();
}

SOCKET Server::getClientSocket( int client )
{
    return clientSocket[client];
}

int Server::getClientNum()
{
    return clientSocket.size();
}

string Server::getIPAddressString()
{
    return svAddress;
}

bool Server::createServer()
{
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    //Get local IP adress
    char hostName[256];
    if (gethostname(hostName, sizeof(hostName)) == SOCKET_ERROR)  { WSACleanup(); return false; }
    hostent* hostEntry = gethostbyname(hostName);
    if (hostEntry == nullptr) { WSACleanup(); return false; }
    in_addr* ipAddress = reinterpret_cast<in_addr*>(hostEntry->h_addr_list[0]);
    svAddress = inet_ntoa(*ipAddress);

    //Create listen socket
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
    result = NULL;
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

void Server::broadcastInvitation()
{
    //Get the subnet mask
    vector<string> subnetMaskList;
    IP_ADAPTER_INFO adapterInfo[16];
    ULONG bufferSize = sizeof(adapterInfo);
    if (GetAdaptersInfo(adapterInfo, &bufferSize) == ERROR_SUCCESS) {
        IP_ADAPTER_INFO* adapter = adapterInfo;
        while (adapter) {
            string subnetMask = adapter->IpAddressList.IpMask.String;
            if (subnetMask[0] != '0') subnetMaskList.push_back(subnetMask);
            adapter = adapter->Next;
        }
    } else {
        std::cerr << "Error getting adapter information." << std::endl;
    }

    //Create broatcast socket
    SOCKET broadcast = socket( AF_INET, SOCK_DGRAM, 0 );
    if ( broadcast == INVALID_SOCKET ) return;
    char i = 1;
    if ( setsockopt( broadcast, SOL_SOCKET, SO_BROADCAST, &i, sizeof(i) ) ) return;
    string msg = mInfo.serverName;

    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(27015);

    //Calculate the broadcast address
    //Convert IP address string to number
    unsigned int decIP = 0;
    int tmp = 0;
    for (char c : svAddress)
    {
        if ( c == '.' ) {decIP = decIP * 256 + tmp; tmp = 0;}
        else tmp = tmp * 10 + (c - '0');
    }
    decIP = decIP * 256 + tmp; tmp = 0; 

    for ( int i = 0; i < subnetMaskList.size(); i++)
    {
        //Convert subnet mask string to number
        unsigned int decMask = 0;
        for (char c : subnetMaskList[i])
        {
            if ( c == '.' ) {decMask = decMask * 256 + tmp; tmp = 0;}
            else tmp = tmp * 10 + (c - '0');
        }
        decMask = decMask * 256 + tmp;
        unsigned int bc = decIP | (~decMask);

        string bcAddr = "";
        for ( int part = 0; part < 4; part++)
        {
            bcAddr = (part < 3 ? "." : "") +to_string( bc % 256 ) + bcAddr;
            bc /= 256;
        }

        //Broadcast
        addr.sin_addr.s_addr = inet_addr(bcAddr.c_str());
        sendto( broadcast, msg.c_str(), msg.length(), 0, (sockaddr*)&addr, sizeof(addr) );
    }
    closesocket( broadcast );
}

bool Server::acceptConnection()
{
    //Create socket for accepting connection
    SOCKET ClientSocket = INVALID_SOCKET;
    bool success = true;
    sockaddr_in addr;
    ClientSocket = accept(ListenSocket, (sockaddr*)&addr, NULL );
    
    if (ClientSocket == INVALID_SOCKET) success = false;
    else {
        //Make it non blocking socket
        unsigned long b = 1;
        ioctlsocket(ClientSocket, FIONBIO, &b);
        int len = sizeof(sockaddr_in);
        getpeername(ClientSocket, (sockaddr*)&addr, &len);
        clientSocket.push_back(ClientSocket);

        // int buf = 16;
        // char tmp[buf];
        // memset(&tmp, '\0', buf);
        // int info = recv(ClientSocket, tmp, buf, 0);

        //Receive new client info: Name & address, then push into playerList
        receive();
        string tmp = getMsg( getClientNum() - 1 );

        playerList.push_back(playerInfo{tmp, inet_ntoa(addr.sin_addr), false});

        //Send to new client server's info: match settings, other players' info.
        Sleep(10);
        string serverInfo = mInfo.serverName + delimiter + to_string(mInfo.gameMode) + delimiter + to_string(mInfo.maxPlayers) + delimiter + to_string(mInfo.lvlSpd) + delimiter + to_string(mInfo.winCount);
        for (int i = 0; i < playerList.size(); i++)
        {
            serverInfo += delimiter + playerList[i].name + delimiter + playerList[i].address + delimiter + to_string(playerList[i].ready);
        }
        makeMsg(serverInfo, getClientNum() - 1);
        sendToClient();
    }
    return success;
}

void Server::sendToClient()
{
    for (int i = clientSocket.size() - 1; i > -1; i--)
    {
        if ( msgToEachClient[i].length() != 0 )
        {
            int info = send( clientSocket[i], msgToEachClient[i].c_str(), msgToEachClient[i].length(), 0 );
            if ( info == SOCKET_ERROR )
                closeClientSocket(i);
            else msgToEachClient[i] = "";
        }
    }
}

void Server::receive()
{
    for (int i = 0; i < clientSocket.size(); i++)
    {
        char tmp[BUFFER_SIZE];
        memset(&tmp, 0, BUFFER_SIZE);
        int info = recv( clientSocket[i], tmp, BUFFER_SIZE, 0 );
        if ( info > 0 && strcmp(tmp, "ping\x1E") != 0 )
        {
            clientMsg[i] = tmp;
        }
        else if (info == SOCKET_ERROR && WSAGetLastError() != WSAEWOULDBLOCK )
        {
            cout << WSAGetLastError() << " " << "Socket closed" << endl;
            closeClientSocket(i);
            continue;
        }
    }

    // Check if a player send quit command
    for (int i = clientSocket.size() - 1; i > -1; i--)
    {
        if ( clientMsg[i] == "quit" )
        {
            char tmp[] = "4";
            send(clientSocket[i], tmp, strlen(tmp), 0);
            closeClientSocket(i);
        }
    }
}

void Server::makeMsg( string msg, int client )
{
    msgToEachClient[client] = msg + endMsg;
}

string Server::getMsg( int client )
{   
    int endMsgPos = 0;
    string res = "";
    for ( int i = 0; i < clientMsg[client].length(); i++ )
    {
        if ( clientMsg[client][i] == endMsg ) {endMsgPos = i + 1; break;}
        else res += clientMsg[client][i];
    }
    clientMsg[client] = clientMsg[client].substr(endMsgPos);
    return res;
}

void Server::pingClient()
{
    for (int i = clientSocket.size() - 1; i > -1; i--) makeMsg( "ping", i );
    sendToClient();
}

void Server::closeClientSocket( int client )
{
    shutdown(clientSocket[client], SD_BOTH);
    closesocket(clientSocket[client]);
    clientSocket.erase(clientSocket.begin() + client);
    clientMsg.erase(clientMsg.begin() + client);
    msgToEachClient.erase(msgToEachClient.begin() + client);
    playerList.erase(playerList.begin() + client + 1);

    //Notify other players about the disconnected player
    for (int i = 0; i < playerList.size(); i++)
    {
        makeMsg(to_string(client) + "quit", i);
    }
    sendToClient();
    clientMsg.push_back("");
    msgToEachClient.push_back("");
}

Client::Client()
{
    connected = false;
    connectSocket = INVALID_SOCKET;
    position = -1;
}

Client::~Client() { closeSocket(); }

bool Client::isConnected() { return connected; }


void Client::connectToServer( int serverNum )
{
    //Initializing winsock & create connect socket
    WSAStartup(MAKEWORD(2, 2), &wsaData);
    sockaddr_in svAddr;
    svAddr.sin_family = AF_INET;
    svAddr.sin_addr.s_addr = inet_addr(address[serverNum].c_str());
    svAddr.sin_port = htons(stoi(DEFAULT_PORT));
    connectSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP );
    if (connectSocket == INVALID_SOCKET) { closeSocket(); return; }
    if (connect(connectSocket, (sockaddr*)&svAddr, sizeof(svAddr)) == SOCKET_ERROR) {closeSocket(); return;}
    unsigned long b = 1; ioctlsocket(connectSocket, FIONBIO, &b);
    connected = true;
    
    //Sending this client info & receiving server info
    sendToServer(playerName);
    Sleep(50);
    string tmp = "";
    while ( tmp == "" )
    {
        receive();
        tmp = getMsg();
        Sleep(5);
    }
    int part = 0;
    vector<string> info(17, "");
    for (int i = 0; i < tmp.length(); i++)
    {
        if ( tmp[i] == delimiter ) part++;
        else info[part] += tmp[i];
    }
    mInfo = { info[0], stoi(info[1]), stoi(info[2]), stoi(info[3]), stoi(info[4]) };
    for (int i = 0; i < 4 && !info[5 + i * 3].empty(); i++)
    {
        playerList.push_back( playerInfo { info[5 + i * 3], info[6 + i * 3], (info[7 + i * 3] == "1") } );
    }
    position = playerList.size() - 1;
}

void Client::sendToServer(string sendString)
{
    int info = send( connectSocket, (sendString + endMsg).c_str(), sendString.length(), 0 );
    if (info == SOCKET_ERROR)
    {
        closeSocket();
    }
}

void Client::searchServer()
{
    //Flushes server list
    serverName.clear();
    address.clear();

    //Creates socket for receiving broadcast messages
    WSAStartup(MAKEWORD(2, 2), &wsaData);
    SOCKET search = socket( AF_INET, SOCK_DGRAM, 0 );
    char i = 1;
    if (setsockopt( search, SOL_SOCKET, SO_BROADCAST, &i, sizeof(i)) == SOCKET_ERROR)
    {
        closesocket(search);
        return;
    }
    sockaddr_in addr;
    char recvMsg[BUFFER_SIZE];
    addr.sin_family = AF_INET;
    addr.sin_port = htons(27015);
    addr.sin_addr.s_addr = INADDR_ANY;
    if (bind(search, (sockaddr*)&addr, sizeof(sockaddr_in)) < 0)
    {
        closesocket(search);
        return;
    }
    unsigned long b = 1;
    ioctlsocket(search, FIONBIO, &b);
    int len = sizeof(sockaddr_in);
    int cycle = 0;
    map<string, string> svList;

    //Loop to scan all available servers in case their messages fails to reach the client
    while (cycle < 250)
    {
        memset(&recvMsg, 0, BUFFER_SIZE);
        int info = recvfrom( search, recvMsg, BUFFER_SIZE, 0, (sockaddr *)&addr, &len );
        if (info == SOCKET_ERROR && WSAGetLastError() != WSAEWOULDBLOCK)
        {
            cout << WSAGetLastError() << " Broadcast listener error" << endl;
            break;
        }
        else if (strlen(recvMsg) > 0)
        {
            string svName = recvMsg;
            string add = inet_ntoa(addr.sin_addr);
            svList[add] = svName;
        }
        Sleep(2);
        cycle++;
    }

    //Fills the server list with gathered info
    for (map<string, string>::iterator it = svList.begin(); it != svList.end(); it++)
    {
        address.push_back(it->first);
        serverName.push_back(it->second);
    }

    //Closes broadcast socket
    closesocket(search);
    WSACleanup();
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
    else if ( strcmp(tmp, "ping\x1E") != 0 )
    {
        recvMsg += tmp;
    }
}

string Client::getMsg()
{
    //Marks the position of the EndMsg character. The message before the mark will be returned
    int endMsgPos = 0;
    string res = "";
    for ( int i = 0; i < recvMsg.length(); i++ )
    {
        if ( recvMsg[i] == endMsg ) {endMsgPos = i + 1; break;}
        else res += recvMsg[i];
    }

    //Trims the returned message to keep only the unread messages.
    recvMsg = recvMsg.substr(endMsgPos);
    
    return res;
}

void Client::pingServer()
{
    sendToServer("ping");
}

void Client::closeSocket()
{
    connected = false;
    playerList.clear();
    sendToServer( "quit" );
    shutdown( connectSocket, SD_BOTH );
    closesocket(connectSocket);
    WSACleanup();
}

int Client::getPosition() { return position; }


//Code for running this file as a separate app for testing
// atomic<bool> stop;

// void SvRun()
// {
//     Server sv;
//     cout << "Starting server" << endl;
//     mInfo.serverName = "abc";
//     mInfo.maxPlayers = 1;
//     mInfo.gameMode = 6;
//     mInfo.lvlSpd = 15;
//     mInfo.winCount = 10;
//     if ( sv.createServer() )
//     {
//         while (!stop)
//         {
//             sv.broadcastInvitation();
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
//             cout << "Finding server..." << endl;
//             cl.searchServer();
//             cout << "Server List:" << endl;
//             for (int i = 0; i < cl.address.size(); i++)
//             {
//                 cout << cl.address[i] << ": " << cl.serverName[i] << endl;
//             }
//             stop = false;
//             int choice = -1;
//             int attempt = 0;
//             while (choice < 0 || choice >= cl.address.size())
//             {
//                 cout << "Enter server num: ";
//                 cin >> choice;
//             }
//             while (!cl.isConnected() && attempt <= 5) {cl.connectToServer(choice); attempt++;}
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