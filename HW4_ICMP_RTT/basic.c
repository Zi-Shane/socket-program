#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip_icmp.h>
#include <netinet/ip.h>
#include <sys/time.h>
#include <stddef.h>
#include <arpa/inet.h>
#include <string.h>

#define BUFFER_SIZE 1024

unsigned short checksum(unsigned short *buf, int bufsz){
    unsigned long sum = 0xffff;

    while(bufsz > 1){
        sum += *buf;
        buf++;
        bufsz -= 2;
    }

    if(bufsz == 1)
        sum += *(unsigned char*)buf;

    sum = (sum & 0xffff) + (sum >> 16);
    sum = (sum & 0xffff) + (sum >> 16);

    return ~sum;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Error: Invalid parameters!\n");
        printf("Usage: %s <target IP>\n", argv[0]);
        exit(1);
    }

    char buffer[BUFFER_SIZE];
    int fd;
    unsigned short dst_addr;
    struct timeval start;
    struct timeval end;
    struct sockaddr_in addr;
    struct icmphdr *icmp = (struct icmphdr *) buffer;
    struct iphdr *ip_hdr_recv;
    struct icmphdr *icmp_hdr_recv;
    char *ptemp;
    unsigned long diff;
    float maxRTT = 0, minRTT = 0, TotRTT = 0, curRTT = 0;

    addr.sin_family = PF_INET;  // IPv4
    addr.sin_addr.s_addr = inet_addr(argv[1]);

    // create socket
    fd = socket(PF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (fd < 0) {
        perror("socket");
        exit(-1);
    }

    for (int i = 0; i < 10; i++)
    {
        memset(buffer, 0, sizeof(buffer));
        // init icmp header
        icmp->type = ICMP_ECHO;
        icmp->code = 0;
        icmp->un.echo.id = 8;
        icmp->un.echo.sequence = i+1;
        icmp->checksum = checksum((unsigned short *)buffer, sizeof(struct icmphdr));

        gettimeofday(&start, NULL);

        // send icmp
        int s = sendto(fd, buffer, sizeof(struct icmphdr), 0, (struct sockaddr*)&addr, sizeof(addr));
        if (s < 1) {
            perror("sendto");
            exit(1);
        }
        // printf("sended an ICMP packet to %s\n", argv[1]);

        // reveive icmp
        memset(buffer, 0, sizeof(buffer));
        int r = recvfrom(fd, buffer, sizeof(buffer), 0, NULL, NULL);
            if (r < 1) {
            perror("recvfrom");
            exit(1);
        }

        gettimeofday(&end, NULL);

        ptemp = buffer;
        ip_hdr_recv = (struct iphdr *)ptemp;
        ptemp += sizeof(struct iphdr);
        icmp_hdr_recv = (struct icmphdr *)ptemp;
        // ip_hdr_recv = (struct iphdr *)buffer;
        // icmp_hdr_recv = (struct icmphdr *)(buffer + (ip_hdr_recv->ihl)*4);

        struct sockaddr_in sa;
        char ipbuf[INET_ADDRSTRLEN];
        diff = 1000000*(end.tv_sec-start.tv_sec)+end.tv_usec-start.tv_usec;
        curRTT = (float)diff/1000;
        // init minRTT
        minRTT = minRTT == 0 ? curRTT : minRTT;

        printf("replyfrom = %s, icmp_type = %u, icmp_code = %u, icmp_seq = %hu, RTT: %.2fms\n",
        inet_ntop(AF_INET, &ip_hdr_recv->saddr, ipbuf, sizeof(ipbuf)),
        icmp_hdr_recv->type,
        icmp_hdr_recv->code,
        icmp_hdr_recv->un.echo.sequence,
        curRTT
        );

        maxRTT = maxRTT < curRTT ? curRTT : maxRTT;
        minRTT = minRTT > curRTT ? curRTT : minRTT;
        TotRTT += curRTT;
    }
    printf("\n");
    printf("Max RTT: %.2fms, Min RTT: %.2fms, Avg RTT: %.2fms\n", maxRTT, minRTT, TotRTT/10);

    return 0;
}
