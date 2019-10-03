#include <stdio.h>
#include <stdlib.h>
#include <string.h>  
#include <unistd.h> 
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>


#define Server_PortNumber 5555
#define Server_Address "192.168.161.137"

int main() {
    struct sockaddr_in server_addr;
    int sock, byte_sent, server_addr_length = sizeof(server_addr);

    char buffer[100], buffer2[100];
    
    struct timeval start;
    struct timeval end;
    unsigned long diff;

    // create socket
    sock = socket(PF_INET, SOCK_STREAM, 0);
    if (sock < 0) printf("Error creating socket\n");
    // init struct
    bzero(&server_addr, server_addr_length);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(Server_PortNumber);
    server_addr.sin_addr.s_addr = inet_addr(Server_Address);
    // establish 3-way
    if (connect(sock, (struct sockaddr *)&server_addr, server_addr_length) == -1) {
        printf("connect fail\n");
        close(sock);
    }
    // printf("input: ");
    // scanf("%[^\n]", buffer);
    int len = read(STDIN_FILENO, buffer, sizeof(buffer));

    gettimeofday(&start, NULL);    // record send time
    byte_sent = send(sock, buffer, len, 0);
    if (byte_sent < 0) printf("Error sending packet\n");
    gettimeofday(&end, NULL);    // record end time
    // latency
    diff = 1000000*(end.tv_sec-start.tv_sec)+end.tv_usec-start.tv_usec;

    recv(sock, buffer, sizeof(buffer), 0);
    
    printf("%s", buffer);
    printf("The latency is %ld (ms)\n", diff);
    printf("throughput: %f (bps)\n", (byte_sent*8) / (double)diff * 1000);

    close(sock);
    return 0;
}
