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
#include <linux/tcp.h>
#include <linux/filter.h>

void print_mac(unsigned char *mac) 
{
    printf("%02x:%02x:%02x:%02x:%02x:%02x\n",
        (unsigned char) mac[0],
        (unsigned char) mac[1],
        (unsigned char) mac[2],
        (unsigned char) mac[3],
        (unsigned char) mac[4],
        (unsigned char) mac[5]);
}

int main() {
    unsigned char buffer[ETH_FRAME_LEN];
    struct ifreq ethreq;
    struct ethhdr *peth;
    struct iphdr *pip;
    struct tcphdr *ptcp;
    char *ptemp;
    struct sockaddr_in* pipaddr;
    char ipbuf[INET_ADDRSTRLEN];

    // create raw socket
    int fd = socket(PF_PACKET,SOCK_RAW,htons(ETH_P_ALL));

    // get current flags
    strncpy(ethreq.ifr_name,"eno2",IFNAMSIZ);

    // get interfce mac address
    // unsigned char *if_mac;
    // ioctl(fd, SIOCGIFHWADDR, &ethreq);
    // if_mac = (unsigned char *)ethreq.ifr_hwaddr.sa_data;

    // get interfce ip address"
    if (ioctl(fd,SIOCGIFADDR,&ethreq)==-1) {
        perror("ioctl");exit(1);
    }
    pipaddr = (struct sockaddr_in*)&ethreq.ifr_addr;
    unsigned int if_ipaddr = pipaddr->sin_addr.s_addr;

    // start PROMISC
    if(ioctl(fd,SIOCGIFFLAGS,&ethreq) == -1) {
        perror("ioctl");exit(1);
    }
    ethreq.ifr_flags |= IFF_PROMISC;
    if(ioctl(fd,SIOCSIFFLAGS,&ethreq) == -1) {
        perror("ioctl");exit(1);
    }

    int count = 0;
    // capture packet
    while(count < 5) {
        int r = recvfrom(fd, buffer, ETH_FRAME_LEN, 0, NULL, NULL);
        ptemp = buffer;
        peth = (struct ethhdr *)ptemp;    // ethernet header
        ptemp += sizeof(struct ethhdr);
        pip = (struct iphdr *)ptemp;    // ipheader
        ptemp += sizeof(struct iphdr);
        ptcp = (struct tcphdr *)ptemp;
        
        
        if (pip->daddr != if_ipaddr && pip->saddr != if_ipaddr && pip->protocol == IPPROTO_TCP) {
            printf("---------------------\n");
            printf("packet %d:\n", count+1);
            printf("Source MAC Address : ");print_mac(peth->h_source);
            printf("Destination MAC Address : ");print_mac(peth->h_dest);
            printf("IP->protocol = %s\n", (pip->protocol)?"TCP":"error");
            printf("IP->src_ip = %s\n", inet_ntop(AF_INET, &pip->saddr, ipbuf, sizeof(ipbuf))); 
            printf("IP->des_ip = %s\n", inet_ntop(AF_INET, &pip->daddr, ipbuf, sizeof(ipbuf))); 
            printf("Src_port = %hu\n", ntohs(ptcp->source));
            printf("Dst_port = %hu\n", ntohs(ptcp->dest));
            count++;
        }
    }

    // terminate PROMISC
    ethreq.ifr_flags &= ~IFF_PROMISC;
    if(ioctl(fd,SIOCSIFFLAGS,&ethreq) == -1) {
        perror("ioctl");exit(1);
    }

    return 0;
}
