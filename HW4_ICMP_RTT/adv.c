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
    struct icmp *picmp = (struct icmp *) buffer;
    struct iphdr *ip_hdr_recv;
    struct icmp *icmp_hdr_recv;
    char *ptemp;
    unsigned int t1, t2, t3, t4;
    unsigned int maxRTT = 0, minRTT = 0, TotRTT = 0, curRTT = 0;

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
        gettimeofday(&start, NULL);
        memset(buffer, 0, sizeof(buffer));
        // init icmp header
        picmp->icmp_type = ICMP_TIMESTAMP;
        picmp->icmp_code = 0;
        picmp->icmp_hun.ih_idseq.icd_id = 8;
        picmp->icmp_hun.ih_idseq.icd_seq = i+1;
        picmp->icmp_dun.id_ts.its_otime = (start.tv_sec % (24*60*60)) * 1000 + start.tv_usec / 1000;
        picmp->icmp_cksum = checksum((unsigned short *)buffer, sizeof(struct icmp));

        // send icmp
        int s = sendto(fd, buffer, sizeof(struct icmp), 0, (struct sockaddr*)&addr, sizeof(addr));
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
        icmp_hdr_recv = (struct icmp *)ptemp;
        // ip_hdr_recv = (struct iphdr *)buffer;
        // icmp_hdr_recv = (struct icmp *)(buffer + (ip_hdr_recv->ihl)*4);

        t1 = icmp_hdr_recv->icmp_dun.id_ts.its_otime;
        t2 = icmp_hdr_recv->icmp_dun.id_ts.its_rtime;
        t3 = icmp_hdr_recv->icmp_dun.id_ts.its_ttime;
        t4 = (end.tv_sec % (24*60*60)) * 1000 + end.tv_usec / 1000;
        curRTT = (t4 - t3) + (t2 - t1);
        minRTT = minRTT == 0 ? curRTT : minRTT;
        maxRTT = maxRTT < curRTT ? curRTT : maxRTT;
        minRTT = minRTT > curRTT ? curRTT : minRTT;
        TotRTT += curRTT;

        struct sockaddr_in sa;
        char ipbuf[INET_ADDRSTRLEN];

        printf("replyfrom = %s, icmp_type = %u, icmp_code = %u, icmp_seq = %hu\n otime = %u, rtime = %u, ttime = %u, ftime = %u, RTT: %u\n", 
        inet_ntop(AF_INET, &ip_hdr_recv->saddr, ipbuf, sizeof(ipbuf)), 
        icmp_hdr_recv->icmp_type, 
        icmp_hdr_recv->icmp_code, 
        icmp_hdr_recv->icmp_hun.ih_idseq.icd_seq, 
        t1, 
        t2, 
        t3, 
        t4, 
        curRTT
        );

    }

    printf("\n");
    printf("Max RTT: %ums, Min RTT: %ums, Avg RTT: %.3fms\n", maxRTT, minRTT, (float)TotRTT/10);

    return 0;
}
