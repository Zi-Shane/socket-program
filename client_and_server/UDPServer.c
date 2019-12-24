#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PortNumber 5555

int main()
{
    struct sockaddr_in address, client_address;
    int sock, byte_recv, byte_sent;
    char buffer[100];

    // create socket
    sock = socket(PF_INET, SOCK_DGRAM, 0);
    if (sock < 0) printf("Error creating socket\n");
    // init struct
    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_port = htons(PortNumber);
    address.sin_addr.s_addr = INADDR_ANY;
    // bind
    if (bind(sock, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        printf("Error binding\n");
        close(sock);
    }

    int address_length = sizeof(address);
    
    // continuing up
    while (1)
    {
        // receive packet
        bzero(buffer, sizeof(buffer));
        byte_recv = recvfrom(sock, buffer, sizeof(buffer),
                             0, (struct sockaddr *)&address, &address_length);
        if (byte_recv < 0) printf("Error recving packet\n");
        printf("Received packet: %s", buffer);

        // response "welcome"
        sprintf(buffer, "welcome!");
        byte_sent = sendto(sock, buffer, sizeof(buffer),
                               0, (struct sockaddr *)&address, address_length);
        if (byte_sent < 0) printf("Error sending packet\n");
    }

    return 0;
}
