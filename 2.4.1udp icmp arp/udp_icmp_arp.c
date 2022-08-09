//
// Created by 68725 on 2022/7/19.
//
#include <stdio.h>
#include <sys/poll.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define NETMAP_WITH_LIBS

#include <net/netmap_user.h>
#include <string.h>

#pragma pack(1)
#define ETH_ADDR_LENGTH 6
#define PROTO_IP 0x0800
#define PROTO_ARP 0x0806
#define PROTO_RARP    0x0835
#define PROTP_UPD 17
#define PROTO_ICMP    1

struct ethhdr {
    unsigned char h_dst[ETH_ADDR_LENGTH];
    unsigned char h_src[ETH_ADDR_LENGTH];
    unsigned short h_proto;
};

struct iphdr {
    unsigned char hdrlen: 4,
            version: 4;
    unsigned char tos;
    unsigned short totlen;
    unsigned short id;
    unsigned short flag_offset;
    unsigned char ttl;
    unsigned char type;
    unsigned short check;
    unsigned int sip;
    unsigned int dip;
};

struct ippkt {
    struct ethhdr eh; //14
    struct iphdr ip; //20
};

struct udphdr {
    unsigned short sport;
    unsigned short dport;
    unsigned short length;
    unsigned short check;
};

struct udppkt {
    struct ethhdr eh; //14
    struct iphdr ip; //20
    struct udphdr udp;//8
    unsigned char data[0];
};

struct arphdr {
    unsigned short h_type;
    unsigned short h_proto;

    unsigned char h_addrlen;
    unsigned char h_protolen;

    unsigned short oper;

    unsigned char smac[ETH_ADDR_LENGTH];
    unsigned int sip;
    unsigned char dmac[ETH_ADDR_LENGTH];
    unsigned int dip;
};

struct arppkt {
    struct ethhdr eh;
    struct arphdr arp;
};

int str2mac(char *mac, char *str) {
    char *p = str;
    unsigned char value = 0x0;
    int i = 0;
    while (p != '\0') {
        if (*p == ':') {
            mac[i++] = value;
            value = 0x0;
        }
        else {
            unsigned char temp = *p;
            if (temp <= '9' && temp >= '0') {
                temp -= '0';
            }
            else if (temp <= 'f' && temp >= 'a') {
                temp -= 'a';
                temp += 10;
            }
            else if (temp <= 'F' && temp >= 'A') {
                temp -= 'A';
                temp += 10;
            }
            else {
                break;
            }
            value <<= 4;
            value |= temp;
        }
        p++;
    }
    mac[i] = value;//a
    return 0;
}

void echo_udp_pkt(struct udppkt *udp, struct udppkt *udp_rt) {
    memcpy(udp_rt, udp, sizeof(struct udppkt));
    memcpy(udp_rt->eh.h_dst, udp->eh.h_src, ETH_ADDR_LENGTH);
    memcpy(udp_rt->eh.h_src, udp->eh.h_dst, ETH_ADDR_LENGTH);
    udp_rt->ip.sip = udp->ip.dip;
    udp_rt->ip.dip = udp->ip.sip;
    udp_rt->udp.sport = udp->udp.dport;
    udp_rt->udp.dport = udp->udp.sport;
}

void echo_arp_pkt(struct arppkt *arp, struct arppkt *arp_rt, char *mac) {
    memcpy(arp_rt, arp, sizeof(struct arppkt));
    memcpy(arp_rt->eh.h_dst, arp->eh.h_src, ETH_ADDR_LENGTH);//以太网首部填入目的 mac
    str2mac(arp_rt->eh.h_src, mac);//以太网首部填入源mac
    arp_rt->eh.h_proto = arp->eh.h_proto;//以太网协议还是arp协议
    arp_rt->arp.h_addrlen = 6;
    arp_rt->arp.h_protolen = 4;//aa
    arp_rt->arp.oper = htons(2); // ARP响应
    str2mac(arp_rt->arp.smac, mac);//arp报文填入源mac
    arp_rt->arp.sip = arp->arp.dip; // arp报文填入发送端 ip
    memcpy(arp_rt->arp.dmac, arp->arp.smac, ETH_ADDR_LENGTH);//arp报文填入目的 mac
    arp_rt->arp.dip = arp->arp.sip; // arp报文填入目的 ip
}

struct icmphdr {
    unsigned char type;
    unsigned char code;
    unsigned short check;
    unsigned short identifier;
    unsigned short seq;
    unsigned char data[32];
};

struct icmppkt {
    struct ethhdr eh;
    struct iphdr ip;
    struct icmphdr icmp;
};

unsigned short in_cksum(unsigned short *addr, int len) {
    register int nleft = len;
    register unsigned short *w = addr;
    register int sum = 0;
    unsigned short answer = 0;
    while (nleft > 1) {
        sum += *w++;
        nleft -= 2;
    }
    if (nleft == 1) {
        *(u_char *) (&answer) = *(u_char *) w;
        sum += answer;
    }
    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);
    answer = ~sum;
    return (answer);
}

void echo_icmp_pkt(struct icmppkt *icmp, struct icmppkt *icmp_rt) {
    memcpy(icmp_rt, icmp, sizeof(struct icmppkt));
    icmp_rt->icmp.type = 0x0; //
    icmp_rt->icmp.code = 0x0; //
    icmp_rt->icmp.check = 0x0;
    icmp_rt->ip.sip = icmp->ip.dip;
    icmp_rt->ip.dip = icmp->ip.sip;
    memcpy(icmp_rt->eh.h_dst, icmp->eh.h_src, ETH_ADDR_LENGTH);
    memcpy(icmp_rt->eh.h_src, icmp->eh.h_dst, ETH_ADDR_LENGTH);
    icmp_rt->icmp.check = in_cksum((unsigned short *) &icmp_rt->icmp, sizeof(struct icmphdr));
}

int main() {
    struct nm_pkthdr h;
    struct nm_desc *nmr = nm_open("netmap:ens33", NULL, 0, NULL);
    if (nmr == NULL) {
        return -1;
    }
    printf("open ens33 seccess\n");
    struct pollfd pfd = {0};
    pfd.fd = nmr->fd;
    pfd.events = POLLIN;
    while (1) {
        printf("new data coming!\n");
        int ret = poll(&pfd, 1, -1);
        if (ret < 0) {
            continue;
        }
        if (pfd.revents & POLLIN) {
            unsigned char *stream = nm_nextpkt(nmr, &h);
            struct ethhdr *eh = (struct ethhdr *) stream;
            if (ntohs(eh->h_proto) == PROTO_IP) {
                struct ippkt *iph = (struct ippkt *) stream;
                if (iph->ip.type == PROTP_UPD) {
                    struct udppkt *udp = (struct udppkt *) stream;
                    int udplength = ntohs(udp->udp.length);
                    udp->data[udplength - 8] = '\0';
                    printf("udp ---> %s\n", udp->data);
                    struct udppkt udp_rt;
                    echo_udp_pkt(udp, &udp_rt);
                    nm_inject(nmr, &udp_rt, sizeof(struct udppkt));
                }
                else if (iph->ip.type == PROTO_ICMP) {
                    struct icmppkt *icmp = (struct icmppkt *) stream;
                    printf("icmp ---------- --> %d, %x\n", icmp->icmp.type, icmp->icmp.check);
                    if (icmp->icmp.type == 0x08) {
                        struct icmppkt icmp_rt = {0};
                        echo_icmp_pkt(icmp, &icmp_rt);
                        nm_inject(nmr, &icmp_rt, sizeof(struct icmppkt));
                    }
                }
            }
            else if (ntohs(eh->h_proto) == PROTO_ARP) {
                struct arppkt *arp = (struct arppkt *) stream;
                struct arppkt arp_rt;
                if (arp->arp.dip == inet_addr("192.168.109.100")) {
                    echo_arp_pkt(arp, &arp_rt, "00:0c:29:1b:18:20");
                    nm_inject(nmr, &arp_rt, sizeof(arp_rt));
                    printf("arp ret\n");
                }
            }
        }
    }
    nm_close(nmr);
}
//gcc -o main main.c -I /netmap/sys/
//insmod netmap.ko