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

int main()
{
    struct sockaddr_in address;
    int sock, byte_sent, byte_recv;
    char buffer[100];

    struct timeval start;
    struct timeval end;
    unsigned long diff;

    // create socket
    sock = socket(PF_INET, SOCK_DGRAM, 0);
    if (sock < 0) printf("Error creating socker\n");
    // init struct
    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_port = htons(Server_PortNumber);
    address.sin_addr.s_addr = inet_addr(Server_Address);

    int address_length = sizeof(address);
    
    // printf("Packet content: ");
    // scanf("%[^\n]", buffer);
    int len = read(STDIN_FILENO,buffer,sizeof(buffer));

    // send packet
    gettimeofday(&start, NULL);   // record send time
    byte_sent = sendto(sock, buffer, len,
                       0, (struct sockaddr *)&address, address_length);
    if (byte_sent < 0) printf("Error sending packet\n");
    gettimeofday(&end, NULL);   // record end time
    // latency
    diff = 1000000*(end.tv_sec-start.tv_sec)+end.tv_usec-start.tv_usec;
    // receive
    byte_recv = recvfrom(sock, buffer, sizeof(buffer),
                         0, (struct sockaddr *)&address, &address_length);
    if (byte_recv < 0) printf("Error recving packet\n");
    
    printf("%s\n", buffer);
    
    // throughput
    printf("The latency is %ld (ms)\n", diff);
    printf("throughput: %.4f (Mbps)\n", (byte_sent*8)/(double)diff);

    close(sock);
    return 0;
}
