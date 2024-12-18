#include <iostream>
#include <cstring>
#include <string>
#include <thread>
#include <ws2tcpip.h>
#include <winsock2.h>
#include <mutex>
#include "UDPServer.h"

#define PORT 8888  
#define BUFLEN 512
#pragma comment(lib, "ws2_32.lib")


std::mutex recv_lock; 
bool quitnow = false;


void LockThread(std::mutex& lock) {
    lock.lock();  
}


void UnlockThread(std::mutex& lock) {
    lock.unlock();  
}


void handleClient(UDPServer& server) {
    struct sockaddr_in si_other;
    int slen = sizeof(si_other);
    char buf[BUFLEN];
    char ipStr[INET_ADDRSTRLEN];

    while (!quitnow) {
        int receivedBytes = server.RecvDatagram(buf, BUFLEN, (struct sockaddr*)&si_other, &slen);
        if (receivedBytes > 0) {
            buf[receivedBytes] = '\0'; 
            inet_ntop(AF_INET, &si_other.sin_addr, ipStr, INET_ADDRSTRLEN);

    
            LockThread(recv_lock);

            std::cout << "Received packet from " << ipStr << ":" << ntohs(si_other.sin_port) << std::endl;
            std::cout << "Data: " << buf << std::endl;

            UnlockThread(recv_lock);

          
            std::string response = "Received message: " + std::string(buf);
            std::cout << "Sending response: " << response << std::endl;

            int sentBytes = server.SendDatagram(response.c_str(), response.length(), (struct sockaddr*)&si_other, slen);
            if (sentBytes == -1) {
                std::cerr << "Failed to send response. Error code: " << WSAGetLastError() << std::endl;
            }

       
            std::string prompt = "Enter a text: ";
            server.SendDatagram(prompt.c_str(), prompt.length(), (struct sockaddr*)&si_other, slen);
        }
        else {
            std::cerr << "Failed to receive data. Error code: " << WSAGetLastError() << std::endl;
        }
    }
}

int main() {
    WSADATA wsa;
    SOCKET serverSock;
    struct sockaddr_in serverAddr, clientAddr;
    char buffer[BUFLEN];
    int slen = sizeof(clientAddr);


    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        std::cerr << "WSAStartup failed with error: " << WSAGetLastError() << std::endl;
        return 1;
    }


    serverSock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (serverSock == INVALID_SOCKET) {
        std::cerr << "Socket creation failed with error: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return 1;
    }


    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

  
    if (bind(serverSock, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Bind failed with error: " << WSAGetLastError() << std::endl;
        closesocket(serverSock);
        WSACleanup();
        return 1;
    }

    std::cout << "Server listening on port " << PORT << "...\n";

  
    UDPServer server(serverSock);
    std::thread clientHandler(handleClient, std::ref(server));

    
    while (!quitnow) {
        std::string userInput;
        LockThread(recv_lock);
        std::getline(std::cin, userInput);  
        const char* response = userInput.c_str();
        sendto(serverSock, response, userInput.length(), 0, (struct sockaddr*)&clientAddr, slen);
        UnlockThread(recv_lock); 
    }


    quitnow = true;
    clientHandler.join();  

    closesocket(serverSock);
    WSACleanup();
    return 0;
}
