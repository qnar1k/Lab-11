#include <stdio.h>
#include "UDPServer.h"
#include <winsock2.h>
#include <ws2tcpip.h>  // Include for inet_ntop()

#define BUFLEN 1024  // Max length of buffer

int main(int argc, char* argv[]) {
    struct sockaddr_in si_other;
    unsigned short srvport;
    int slen;
    char buf[BUFLEN];
    char msg[BUFLEN];

    srvport = (1 == argc) ? PORT : atoi(argv[1]);

    // Initialize server socket
    UDPServer server(srvport);
    slen = sizeof(si_other);

    printf("Waiting for data...\n");

    // Keep listening for data
    while (1) {
        memset(buf, '\0', BUFLEN);  // Clear buffer before receiving data

        // Receive data (blocking call)
        server.RecvDatagram(buf, BUFLEN, (struct sockaddr*)&si_other, &slen);

        // Use inet_ntop to convert the binary IP address to a text representation
        char ipStr[INET_ADDRSTRLEN];  // Create a buffer to store the IP string
        inet_ntop(AF_INET, &si_other.sin_addr, ipStr, sizeof(ipStr));  // Use inet_ntop instead of inet_ntoa

        // Print received data and client info
        printf("Received packet from %s:%d\n", ipStr, ntohs(si_other.sin_port));
        printf("Data: %s\n", buf);

        // Ask server operator to provide a reply
        printf("\nAnswer: ");
        gets_s(msg, BUFLEN);

        // Send response back to client
        server.SendDatagram(msg, (int)strlen(msg), (struct sockaddr*)&si_other, slen);
    }

    return 0;
}
