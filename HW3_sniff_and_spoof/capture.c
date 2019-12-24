#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/ether.h>
#include <linux/if.h>
#include <linux/ip.h>
#include <arpa/inet.h>

int main() {
    unsigned char buffer[ETH_FRAME_LEN];
    struct ifreq ethreq;
    struct ethhdr *peth;
    struct iphdr *pip;
    char *ptemp;
    int ip_counter = 0, arp_counter = 0, rarp_counter = 0, 
        tcp_counter = 0, udp_counter = 0, icmp_counter = 0, igmp_counter = 0;

    int fd = socket(PF_PACKET,SOCK_RAW,htons(ETH_P_ALL));  // create raw socket

    // get current flags
    strncpy(ethreq.ifr_name,"eno2",IFNAMSIZ);
    if(ioctl(fd,SIOCGIFFLAGS,&ethreq) == -1) {
        perror("ioctl");exit(1);
    }
    // start PROMISC
    ethreq.ifr_flags |= IFF_PROMISC;
    if(ioctl(fd,SIOCSIFFLAGS,&ethreq) == -1) {
        perror("ioctl");exit(1);
    }

    // capture packet
    for(int i = 0; i < 200; i++) {
        int r = recvfrom(fd, buffer, ETH_FRAME_LEN, 0, NULL, NULL);
        ptemp = buffer;
        peth = (struct ethhdr *)ptemp; //ethernet header
        switch(ntohs(peth->h_proto)) 
        {
            case 0x0800:
                ip_counter++;
                break;
            case 0x0806:
                arp_counter++;
                break;
            case 0x8035:
                rarp_counter++;
                break;
        }
        ptemp += sizeof(struct ethhdr);
        pip = (struct iphdr *)ptemp;
        switch(pip->protocol) 
        {
            case IPPROTO_TCP:
                tcp_counter++;
                break;
            case IPPROTO_UDP:
                udp_counter++;
                break;
            case IPPROTO_ICMP:
                icmp_counter++;
                break;
            case IPPROTO_IGMP:
                igmp_counter++;
                break;
        }
    }

    // terminate PROMISC
    ethreq.ifr_flags &= ~IFF_PROMISC;
    if(ioctl(fd,SIOCSIFFLAGS,&ethreq) == -1) {
        perror("ioctl");exit(1);
    }

    printf("-----statistics-----\n");
    printf("ip_counter: %d\n", ip_counter);
    printf("arp_counter: %d\n", arp_counter);
    printf("rarp_counter: %d\n", rarp_counter);
    printf("tcp_counter: %d\n", tcp_counter);
    printf("udp_counter: %d\n", udp_counter);
    printf("icmp_counter: %d\n", icmp_counter);
    printf("igmp_counter: %d\n", igmp_counter);
    printf("-----finish--------\n");

    return 0;
}
