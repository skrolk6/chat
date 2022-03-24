#pragma comment(lib, "ws2_32.lib")
#include <WinSock2.h>
#include <iostream>
#include <vector>

#pragma warning(disable: 4996)

enum Packet {
    P_ChatMessage,
    P_SetName
};

std::vector <std::pair <SOCKET, std::string>> Connections;
int Counter = 0;

bool ProccessPacket(int index, Packet packettype) {
    switch (packettype) {
    case P_ChatMessage:
    {
        int msg_size = 0;
        int name_size = 0;
        recv(Connections[index].first, (char*)&msg_size, sizeof(int), NULL);
        char* msg = new char[msg_size + 1];
        msg[msg_size] = '\0';
        recv(Connections[index].first, msg, msg_size, NULL);
        for (int i = 0; i < Counter; ++i) {
            if (i == index) {
                continue;
            }
            Packet msgtype = P_ChatMessage;
            send(Connections[i].first, (char*)&msgtype, sizeof(Packet), NULL);
            send(Connections[i].first, (char*)&msg_size, sizeof(int), NULL);
            send(Connections[i].first, msg, msg_size, NULL);
            name_size = Connections[index].second.size();
            send(Connections[i].first, (char*)&name_size, sizeof(int), NULL);
            send(Connections[i].first, Connections[index].second.c_str(), name_size, NULL);
        }
        delete[] msg;
        break;
    }
    case P_SetName:
    {
        int msg_size = 0;
        recv(Connections[index].first, (char*)&msg_size, sizeof(int), NULL);
        char* msg = new char[msg_size + 1];
        msg[msg_size] = '\0';
        recv(Connections[index].first, msg, msg_size, NULL);
        std::cout << msg << " connected!" << std::endl;
        Connections[index].second = msg;
        for (int i = 0; i < Counter; ++i) {
            if (i == index) {
                continue;
            }
            Packet msgtype = P_SetName;
            send(Connections[i].first, (char*)&msgtype, sizeof(Packet), NULL);
            send(Connections[i].first, (char*)&msg_size, sizeof(int), NULL);
            send(Connections[i].first, msg, msg_size, NULL);
        }
        delete[] msg;
        break;
    }
    default:
        std::cout << "Unrecognized packet: " << packettype << std::endl;
        break;
    }
    return true;
}

void ClientHandler(int index) {
    Packet packettype;
    int result;
    while (true) {
        result = recv(Connections[index].first, (char*)&packettype, sizeof(Packet), NULL);
        if (result == SOCKET_ERROR) {
            if (Connections[index].second.size() > 0) {
                std::cout << Connections[index].second << " disconnected!" << std::endl;
            }
            else {
                std::cout << "Unknown client disconnected!" << std::endl;
            }
            break;
        }
        else if (!ProccessPacket(index, packettype)) {
            break;
        }
    }
    closesocket(Connections[index].first);
}

int main(int argc, char* argv[])
{
    WSAData wsaData;
    WORD DLLVersion = MAKEWORD(2, 1);
    if (WSAStartup(DLLVersion, &wsaData)) {
        std::cout << "Error" << std::endl;
        exit(1);
    }

    SOCKADDR_IN addr;
    int sizeofaddr = sizeof(addr);
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr.sin_port = htons(1111);
    addr.sin_family = AF_INET;

    SOCKET sListen = socket(AF_INET, SOCK_STREAM, NULL);
    bind(sListen, (SOCKADDR*)&addr, sizeof(addr));
    listen(sListen, SOMAXCONN);

    SOCKET newConnection;
    std::pair <SOCKET, std::string> client;
    while (Connections.size() < 100) {
        newConnection = accept(sListen, (SOCKADDR*)&addr, &sizeofaddr);

        if (newConnection) {
            client.first = newConnection;
            Connections.push_back(client);
            CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)ClientHandler, (LPVOID)(Counter), NULL, NULL);
            Counter++;
        }
        else {
            std::cout << "Failed to connect!" << std::endl;
        }
    }
    system("pause");
    return 0;
}

