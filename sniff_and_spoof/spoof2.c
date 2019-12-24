#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <linux/udp.h>
#include <arpa/inet.h> 
#include <netinet/ip.h>
#include <sys/ioctl.h>
#include <linux/if.h>
#include <netinet/ether.h>

#define PCKT_LEN 8192
#define FAKE_IP "7.7.7.7"
#define FAKE_PORT "9999"
#define TEST_IP "140.120.15.159"
#define TEST_PORT "7777"


unsigned short csum(unsigned short *buf, int nwords)
{
    unsigned long sum;
  
    for(sum=0; nwords>0; nwords--)
    sum += *buf++;
    sum = (sum >> 16) + (sum &0xffff);
    sum += (sum >> 16);
    return (unsigned short)(~sum);
}

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

struct pseudo_header
{
	u_int32_t source_address;
	u_int32_t dest_address;
	u_int8_t placeholder;
	u_int8_t protocol;
	u_int16_t udp_length;
};

int main()
{
    unsigned short src_addr = inet_addr(FAKE_IP);
    unsigned short src_port = atoi(FAKE_PORT);
    // for test
    unsigned short dst_addr = inet_addr(TEST_IP);
    unsigned short dst_port = atoi(TEST_PORT);

    // data
    int datalen = 4;
    char data[4] = "test";

    char buffer_send[PCKT_LEN];
    struct iphdr *ip = (struct iphdr *) buffer_send;
    struct udphdr *udp = (struct udphdr *) (buffer_send + sizeof(struct iphdr));
    // rest buffer
    memset(buffer_send, 0, PCKT_LEN);

    struct sockaddr_in sin;


    // create a raw socket with UDP protocol
    int fd = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);
    if (fd < 0) {
        perror("socket error");
    }
    printf("OK: fd raw socket is created.\n");


    // ----receive----
    unsigned char buffer[ETH_FRAME_LEN];
    struct ifreq ethreq;
    struct ethhdr *peth;
    struct iphdr *pip;
    struct udphdr *pudp;
    char *ptemp;
    struct sockaddr_in* pipaddr;
    char ipbuf[INET_ADDRSTRLEN];
    int count = 0;

    while(count < 1) {
        int r = recvfrom(fd, buffer, ETH_FRAME_LEN, 0, NULL, NULL);
        ptemp = buffer;
        // peth = (struct ethhdr *)ptemp;    // ethernet header
        // ptemp += sizeof(struct ethhdr);
        pip = (struct iphdr *)ptemp;    // ipheader
        ptemp += sizeof(struct iphdr);
        pudp = (struct udphdr *)ptemp;
        
        
        if (pip->protocol == IPPROTO_UDP) {
            printf("---------------------\n");
            printf("incoming\n");
            printf("packet %d:\n", count+1);
            printf("IP->protocol = %s\n", (pip->protocol)?"UDP":"error");
            printf("IP->src_ip = %s\n", inet_ntop(AF_INET, &pip->saddr, ipbuf, sizeof(ipbuf))); 
            printf("IP->des_ip = %s\n", inet_ntop(AF_INET, &pip->daddr, ipbuf, sizeof(ipbuf))); 
            printf("Src_port = %hu\n", ntohs(pudp->source));
            printf("Dst_port = %hu\n", ntohs(pudp->dest));
            count++;
        }
    }

    // ----send----
    // modify data
    memcpy (buffer_send + sizeof(struct iphdr) + sizeof(struct udphdr), 
            data, datalen * sizeof (uint8_t));
    
    int one = 1;
    const int *val = &one;
    if(setsockopt(fd, IPPROTO_IP, IP_HDRINCL, val, sizeof(one)) < 0) {
        perror("setsockopt() error");
        exit(2);
    }
    printf("OK: socket option IP_HDRINCL is set.\n");

    // set to TEST_IP for test
    sin.sin_family = AF_INET;
    sin.sin_port = htons(dst_port);
    sin.sin_addr.s_addr = dst_addr;

    // IP header
    ip->ihl      = pip->ihl;
    ip->version  = pip->version;
    ip->tos      = pip->tos;
    ip->tot_len  = sizeof(struct iphdr) + sizeof(struct udphdr) + sizeof(data);
    ip->id       = pip->id;
    ip->ttl      = pip->ttl;
    ip->protocol = pip->protocol;
    ip->saddr = inet_addr(FAKE_IP);
    ip->daddr = pip->daddr;
    ip->check = csum((unsigned short *)buffer_send, sizeof(struct iphdr));
    
    // UDP header
    udp->source = htons(src_port);
    udp->dest = pudp->dest;
    udp->len = htons(sizeof(struct udphdr) + sizeof(data));
    // udp->check = csum((unsigned short *)buffer_send, sizeof(struct udphdr) + sizeof(data));
    struct pseudo_header psh;
    psh.source_address = inet_addr(FAKE_IP);
	psh.dest_address = sin.sin_addr.s_addr;
	psh.placeholder = 0;
	psh.protocol = IPPROTO_UDP;
	psh.udp_length = htons(sizeof(struct udphdr) + strlen(data) );
    char *pseudogram;
    int psize = sizeof(struct pseudo_header) + sizeof(struct udphdr) + strlen(data);
	pseudogram = malloc(psize);
    memcpy(pseudogram , (char*) &psh , sizeof (struct pseudo_header));
    memcpy(pseudogram + sizeof(struct pseudo_header) , udp , sizeof(struct udphdr) + strlen(data));
    udp->check = csum( (unsigned short*) pseudogram , psize);

    printf("---------------------\n");
    printf("outgoing\n");
    printf("packet %d:\n", count+1);
    printf("IP->protocol = %s\n", (ip->protocol)?"UDP":"error");
    printf("IP->src_ip = %s\n", inet_ntop(AF_INET, &ip->saddr, ipbuf, sizeof(ipbuf))); 
    printf("IP->des_ip = %s\n", inet_ntop(AF_INET, &ip->daddr, ipbuf, sizeof(ipbuf))); 
    printf("Src_port = %hu\n", ntohs(udp->source));
    printf("Dst_port = %hu\n", ntohs(udp->dest));

    if (sendto(fd, buffer_send, ip->tot_len, 0, (struct sockaddr *)&sin, sizeof(sin)) < 0)
    {
        perror("sendto()");
        exit(3);
    }
    printf("OK: packet is sent.\n");
        

    return 0;
}
