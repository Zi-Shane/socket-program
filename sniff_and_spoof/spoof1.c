#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <linux/if_ether.h>
#include <linux/ip.h>
#include <linux/udp.h>
#include <arpa/inet.h> 

unsigned short csum(unsigned short *buf, int nwords)
{
    unsigned long sum;
  
    for(sum=0; nwords>0; nwords--)
    sum += *buf++;
    sum = (sum >> 16) + (sum &0xffff);
    sum += (sum >> 16);
    return (unsigned short)(~sum);
}

int main(int argc, char const *argv[])
{
    if (argc != 5) {
        printf("Error: Invalid parameters!\n");
        printf("Usage: %s <source IP> <source port> <target IP> <target port>\n", argv[0]);
        exit(1);
    }

    unsigned short src_port, dst_port;
    unsigned short src_addr, dst_addr;
    src_addr = inet_addr(argv[1]);
    dst_addr = inet_addr(argv[3]);
    src_port = atoi(argv[2]);
    dst_port = atoi(argv[4]);

    int fd;
    char buffer[ETH_FRAME_LEN];
    struct iphdr *ip = (struct iphdr *) buffer;
    struct udphdr *udp = (struct udphdr *) (buffer + sizeof(struct iphdr));

    struct sockaddr_in sin;
    int one = 1;
    const int *val = &one;

    // reset buffer
    memset(buffer, 0, ETH_FRAME_LEN);

    // create a raw socket
    fd = socket(PF_INET, SOCK_RAW, IPPROTO_UDP);
    if (fd < 0) {
        perror("socket error");
    }

    // inform the kernel do not fill up the packet structure, we will build our own
    if(setsockopt(fd, IPPROTO_IP, IP_HDRINCL, val, sizeof(one)) < 0) {
        perror("setsockopt error");
    }

    // IP header
    ip->ihl      = 5;
    ip->version  = 4;
    ip->tos      = 16;
    ip->tot_len  = sizeof(struct iphdr) + sizeof(struct udphdr);
    ip->id       = htons(54321);
    ip->ttl      = 64;
    ip->protocol = IPPROTO_UDP;
    ip->saddr = src_addr;
    ip->daddr = dst_addr;

    // UDP header
    udp->source = htons(src_port);
    udp->dest = htons(dst_port);
    udp->len = htons(sizeof(struct udphdr));
    // ip->check = csum((unsigned short *)buffer, sizeof(struct iphdr) + sizeof(struct udphdr));
    ip->check = csum((unsigned short *)buffer, sizeof(struct iphdr) + sizeof(struct udphdr));
    
    // socket send
    sin.sin_family = AF_INET;
    sin.sin_port = htons(dst_port);
    sin.sin_addr.s_addr = dst_addr;

    for (int i = 0; i < 5; i++)
    {
        if (sendto(fd, buffer, ip->tot_len, 0, (struct sockaddr *)&sin, sizeof(sin)) < 0)
        {
            perror("sendto");
        }
        printf("OK: %d packet is sent.\n", i+1);
    }

    close(fd);
    return 0;
}
