#include "UDPSocket.h"
#include <ws2tcpip.h>
#include <iostream>
#include <thread>
#include <mutex>

using namespace std;


#define SERVER "127.0.0.1"
#define PORT 8888
#define BUFLEN 512


#define THREAD_LOCK std::mutex
#define THREADFUNCVAR void*
#define THREADFUNCARGS void*
#define THREADVAR std::thread


THREAD_LOCK recv_lock;
bool quitnow = false;


void LockThread(THREAD_LOCK& lock) {
    lock.lock();
}

void UnlockThread(THREAD_LOCK& lock) {
    lock.unlock(); 
}

void InitThreadLock(THREAD_LOCK& lock) {

}


void ReceiveMessagesThread(UDPSocket* client_sock, sockaddr_in* si_other) {
    int slen = sizeof(sockaddr_in);
    char buf[BUFLEN] = { 0 };

    while (!quitnow) {
        memset(buf, 0, BUFLEN); 

        LockThread(recv_lock); 
        int bytesReceived = client_sock->RecvDatagram(buf, BUFLEN, (struct sockaddr*)si_other, &slen);
        UnlockThread(recv_lock); 

        if (bytesReceived > 0) {
            cout << "\n[Server]: " << buf << endl;
        }
        else if (bytesReceived == SOCKET_ERROR) {
            cerr << "Error receiving message: " << WSAGetLastError() << endl;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));  
    }
}


int main(int argc, char* argv[]) {
    struct sockaddr_in si_other;
    int slen = sizeof(si_other);
    char message[BUFLEN] = { 0 };

    UDPSocket client_sock;
    InitThreadLock(recv_lock); 


    memset((char*)&si_other, 0, sizeof(si_other));
    si_other.sin_family = AF_INET;
    si_other.sin_port = htons((argc > 1) ? atoi(argv[1]) : PORT);
    inet_pton(AF_INET, (argc > 2) ? argv[2] : SERVER, &si_other.sin_addr);

    cout << "Client started. Connected to server " << SERVER << " on port " << PORT << endl;


    THREADVAR recvThread(ReceiveMessagesThread, &client_sock, &si_other);


    while (!quitnow) {
        cout << "Enter message: ";
        LockThread(recv_lock); 
        cin.getline(message, BUFLEN);
        client_sock.SendDatagram(message, (int)strlen(message), (struct sockaddr*)&si_other, slen);
        UnlockThread(recv_lock);  

        if (string(message) == "exit") {
            quitnow = true;
        }
    }

  
    recvThread.join();

    return 0;
}
