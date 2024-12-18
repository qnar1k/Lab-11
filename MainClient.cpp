#include "UDPSocket.h"
#include <stdio.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#ifdef CLIENT
int main(int argc, char* argv[]) {
    struct sockaddr_in si_other;
    int slen = sizeof(si_other);
    char buf[BUFLEN];
    char message[BUFLEN];

    unsigned short srv_port = 0;

    // Create socket
    UDPSocket client_sock;

    // Setup address structure
    memset((char*)&si_other, 0, sizeof(si_other));
    si_other.sin_family = AF_INET;

    // Convert IP address to a wide string (if necessary)
    wchar_t wide_ip[256];
    size_t outSize;
    mbstowcs_s(&outSize, wide_ip, sizeof(wide_ip) / sizeof(wide_ip[0]), SERVER, strlen(SERVER) + 1);

    // Check if we are passing command-line arguments for server address and port
    if (1 == argc) {
        si_other.sin_port = htons(PORT);
        if (InetPtonW(AF_INET, wide_ip, &si_other.sin_addr) <= 0) {  // Use InetPtonW
            printf("Invalid address/Address not supported\n");
            return 1;
        }
        printf("Server - addr=%s , port=%d\n", SERVER, PORT);
    }
    else if (2 == argc) {
        si_other.sin_port = htons(atoi(argv[1]));
        mbstowcs_s(&outSize, wide_ip, sizeof(wide_ip) / sizeof(wide_ip[0]), SERVER, strlen(SERVER) + 1);
        if (InetPtonW(AF_INET, wide_ip, &si_other.sin_addr) <= 0) {  // Use InetPtonW
            printf("Invalid address/Address not supported\n");
            return 1;
        }
        printf("Server - addr=%s , port=%d\n", SERVER, atoi(argv[1]));
    }
    else {
        si_other.sin_port = htons(atoi(argv[2]));
        mbstowcs_s(&outSize, wide_ip, sizeof(wide_ip) / sizeof(wide_ip[0]), argv[1], strlen(argv[1]) + 1);
        if (InetPtonW(AF_INET, wide_ip, &si_other.sin_addr) <= 0) {  // Use InetPtonW
            printf("Invalid address/Address not supported\n");
            return 1;
        }
        printf("Server - addr=%s , port=%d\n", argv[1], atoi(argv[2]));
    }

    // Start communication loop
    while (1) {
        printf("\nEnter message: ");
        gets_s(message, BUFLEN);

        // Send the message to server
        printf("Sending message: %s\n", message);  // Debug: print the message being sent
        client_sock.SendDatagram(message, (int)strlen(message), (struct sockaddr*)&si_other, slen);

        // Clear the buffer before receiving a response
        memset(buf, '\0', BUFLEN);

        // Receive the server's response (blocking call)
        client_sock.RecvDatagram(buf, BUFLEN, (struct sockaddr*)&si_other, &slen);

        // Print the server's response
        printf("Server Response: %s\n", buf);
    }

    return 0;
}
#endif
