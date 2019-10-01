#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define Server_PortNumber 5555
#define Server_Address "192.168.161.137"

int main()
{
    struct sockaddr_in address;
    int sock, byte_sent;
    char buffer[100] = "hello\0";

    // create socket
    sock = socket(PF_INET, SOCK_DGRAM, 0);
    if (sock < 0)
        printf("Error creating socker\n");
    // fillin struct
    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_port = htons(Server_PortNumber);
    address.sin_addr.s_addr = inet_addr(Server_Address);

    int address_length = sizeof(address);
    
    
    // send packet
    printf("Packet content: ");
    scanf("%[^\n]", buffer);
    byte_sent = sendto(sock, buffer, sizeof(buffer),
                       0, (struct sockaddr *)&address, address_length);
    if (byte_sent < 0) printf("Error sending packet\n");

    //receive
    int byte_recv, client_address, client_address_length;
    char buffer2[100] = "";
    byte_recv = recvfrom(sock, buffer2, sizeof(buffer),
                         0, (struct sockaddr *)&client_address, &client_address_length);
    printf("%s\n", buffer2);
    if (byte_recv < 0) printf("Error recving packet\n");

    close(sock);
    return 0;
}
