#pragma comment(lib, "ws2_32.lib")
#include <WinSock2.h>
#include <iostream>
#include <string>
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma warning(disable: 4996)

SOCKET Connection;

enum Packet {
    P_ChatMessage,
    P_SetName
};

bool ProcessPacket(Packet packettype) {
    switch (packettype) {
    case P_ChatMessage:
    {
        int msg_size = 0;
        recv(Connection, (char*)&msg_size, sizeof(int), NULL);
        char* msg = new char[msg_size + 1];
        msg[msg_size] = '\0';
        recv(Connection, msg, msg_size, NULL);
        int name_size = 0;
        recv(Connection, (char*)&name_size, sizeof(int), NULL);
        char* name = new char[name_size + 1];
        name[name_size] = '\0';
        recv(Connection, name, name_size, NULL);
        std::cout << name << ": " << msg << std::endl;
        delete[] msg;
        break;
    }
    case P_SetName:
    {
        int msg_size = 0;
        recv(Connection, (char*)&msg_size, sizeof(int), NULL);
        char* msg = new char[msg_size + 1];
        msg[msg_size] = '\0';
        recv(Connection, msg, msg_size, NULL);
        std::cout << msg << " connected!" << std::endl;
        delete[] msg;
        break;
    }
    default:
        std::cout << "Unrecognized packet: " << packettype << std::endl;
        break;
    }
    return true;
}

void ServerHandler() {
    Packet packettype;
    int result;
    while (true) {
        result = recv(Connection, (char*)&packettype, sizeof(Packet), NULL);
        if (result == SOCKET_ERROR) {
            std::cout << "Server disconnected!" << std::endl;
            break;
        }
        if (!ProcessPacket(packettype)) {
            break;
        }
    }
    closesocket(Connection);
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

    Connection = socket(AF_INET, SOCK_STREAM, NULL);
    if (connect(Connection, (SOCKADDR*)&addr, sizeof(addr))) {
        std::cout << "Error: failed connect to server." << std::endl;
        return 1;
    }
    std::cout << "Connected!" << std::endl;

    CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)ServerHandler, NULL, NULL, NULL);

    std::string msg;
    Packet messagetype = P_ChatMessage;
    Packet nametype = P_SetName;
    int msg_size = 0;
    std::cout << "Enter your name:" << std::endl;
    std::getline(std::cin, msg);
    msg_size = msg.size();
    send(Connection, (char*)&nametype, sizeof(Packet), NULL);
    send(Connection, (char*)&msg_size, sizeof(int), NULL);
    send(Connection, msg.c_str(), msg_size, NULL);
    //myname = msg;
    while (true) {
        std::getline(std::cin, msg);
        msg_size = msg.size();
        send(Connection, (char*)&messagetype, sizeof(Packet), NULL);
        send(Connection, (char*)&msg_size, sizeof(int), NULL);
        send(Connection, msg.c_str(), msg_size, NULL);
        Sleep(10);
    }
}

