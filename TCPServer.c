#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <unistd.h> 
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>


#define PortNumber 5555

int main() {
    struct sockaddr_in server_addr, client_addr;
    int sock, byte_recv, server_addr_length, client_addr_length, recfd;
    server_addr_length = sizeof(server_addr);
    client_addr_length = sizeof(client_addr);
    
    char buffer[100] = "welcome\n", buffer2[100] = {};

    sock = socket(PF_INET, SOCK_STREAM, 0);
    if (sock < 0) printf("Error creating socket\n");

    bzero(&server_addr, server_addr_length);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PortNumber);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sock, (struct sockaddr *)&server_addr, server_addr_length) == -1) {
        printf("Error binding\n");
        close(sock);
    }

    if (listen(sock, 5) == -1) {
        printf("Listen failed\n");
        close(sock);
    }

    while(1) {
        if ((recfd = accept(sock, (struct sockaddr *)&client_addr, &client_addr_length)) == -1) {
            printf("accept failed\n");
            close(sock);
        }
        send(recfd, buffer, sizeof(buffer), 0);
        byte_recv = recv(recfd, buffer2, sizeof(buffer2), 0);
        if (byte_recv < 0) printf("Error recving packet\n");
        printf("Received packet: %s\n", buffer2);
    }
    return 0;
}
