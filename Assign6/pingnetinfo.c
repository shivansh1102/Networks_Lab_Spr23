#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>
#include <netinet/ip.h>
#include <netinet/in.h>
#include <netinet/ip_icmp.h>
#include <fcntl.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <poll.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>


void copyn(char *dest, char *src, int n)
{
    int i = 0;
    for (i = 0; i < n; ++i)
    {
        dest[i] = src[i];
    }
    dest[i] = '\0';
}

char *getProtocol(int protocol)
{
    if (protocol == IPPROTO_TCP)
        return "TCP";
    else if (protocol == IPPROTO_UDP)
        return "UDP";
    else if (protocol == IPPROTO_ICMP)
        return "ICMP";
    else
        return "Unknown";
}

unsigned short checksum(void *b, int len)
{
    unsigned short *buf = b;
    unsigned int sum = 0;
    unsigned short result;

    for (sum = 0; len > 1; len -= 2)
        sum += *buf++;
    if (len == 1)
        sum += *(unsigned char *)buf;
    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    result = ~sum;
    return result;
}

int main(int argc, const char *argv[])
{

    if (argc <= 3)
    {
        printf("Please provide all fields\n");
        exit(1);
    }

    struct hostent *host;
    host = gethostbyname(argv[1]);

    int n = atoi(argv[2]);

    int T = atoi(argv[3]);

    if (host == NULL)
    {
        perror("gethostbyname");
        exit(1);
    }

    // get host ip address
    struct in_addr **addr_list = (struct in_addr **)host->h_addr_list;
    struct in_addr *host_ip = addr_list[0];

    // create socket
    int sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sockfd < 0)
    {
        perror("socket");
        exit(1);
    }

    // set socket options
    int one = 1;
    setsockopt(sockfd, IPPROTO_IP, IP_HDRINCL, &one, sizeof(one));

    char packet[1000];
    memset(packet, 0, sizeof(packet));

    struct sockaddr_in host_addr;
    host_addr.sin_family = host->h_addrtype;
    host_addr.sin_port = 0;
    host_addr.sin_addr.s_addr = host_ip->s_addr;

    struct ip *iph = (struct ip *)packet;

    // set ip header
    iph->ip_v = IPVERSION;
    iph->ip_hl = sizeof(struct ip) >> 2;
    iph->ip_id = 0;
    iph->ip_len = 0;
    iph->ip_off = htons(0);
    iph->ip_ttl = 2;
    iph->ip_p = IPPROTO_ICMP;
    iph->ip_src.s_addr = 0;
    iph->ip_dst.s_addr = host_ip->s_addr;
    iph->ip_sum = 0;

    int ttl = 1;

    char *data = (char *)(packet + sizeof(struct ip) + sizeof(struct icmp));

    int packets_sent = 0;

    double prev_d1 = 0, prev_d2 = 0;
    int done = 0;

    char sendData[1000];
    memset(sendData, 0, sizeof(sendData));

    while (1)
    {
        iph->ip_ttl = ttl;
        if(ttl>30)break;

        // send five zero length packets to confirm next hop
        struct icmp *icmph = (struct icmp *)(packet + sizeof(struct ip));
        icmph->icmp_type = ICMP_ECHO;
        icmph->icmp_code = 0;
        icmph->icmp_id = htons(100);

        int gotReply = 0;
        double maxRTT1 = 0.0, maxRTT2 = 0.0, minRTT1 = 100000000.0, minRTT2 = 100000000.0;

        for(int i=0;i<5;++i){

            icmph->icmp_seq = htons(packets_sent);
            icmph->icmp_cksum = 0;
            icmph->icmp_cksum = checksum(icmph, sizeof(struct icmp));


            // print icmp header in nice format
            printf("Sent ICMP packet: Type: %2d, Code: %2d, DataSize: %5d, Checksum: %7d, Id: %4d,  Seq: %6d\n", icmph->icmp_type, icmph->icmp_code, 0, icmph->icmp_cksum, ntohs(icmph->icmp_id), ntohs(icmph->icmp_seq));

            int sent = sendto(sockfd, packet, sizeof(struct ip) + sizeof(struct icmp), 0, (struct sockaddr *)&host_addr, sizeof(host_addr));
            if (sent < 0)
            {
                perror("sendto");
                exit(1);
            }

            packets_sent++;

            while(1){
                struct sockaddr_in recv_addr;
                socklen_t recv_addr_len = sizeof(recv_addr);

                char recv_packet[1000];
                memset(recv_packet, 0, sizeof(recv_packet));

                struct pollfd fd_set[1];
                fd_set[0].fd = sockfd;
                fd_set[0].events = POLLIN;

                int timeout = 2000;

                int poll_ret = poll(fd_set, 1, timeout);

                if (poll_ret <= 0)
                {
                    break;;
                }

                int recv = recvfrom(sockfd, recv_packet, sizeof(recv_packet), 0, (struct sockaddr *)&recv_addr, &recv_addr_len);
                if (recv < 0)
                {
                    perror("recvfrom");
                    exit(1);
                }
                gotReply++;

                struct icmp *recv_icmph = (struct icmp *)(recv_packet + sizeof(struct ip));

                // print icmp header in nice format
                printf("Recv ICMP packet: Type: %2d, Code: %2d, DataSize: %5d, Checksum: %7d, Id: %4d,  Seq: %6d\n", recv_icmph->icmp_type, recv_icmph->icmp_code, (int)(recv - sizeof(struct ip) - sizeof(struct icmp)), icmph->icmp_cksum, ntohs(recv_icmph->icmp_id), ntohs(recv_icmph->icmp_seq));

                if(recv_icmph->icmp_type == ICMP_ECHOREPLY){
                    done = 1;
                    printf("Hop: %2d\t IP: %s\n", ttl, inet_ntoa(recv_addr.sin_addr));
                    printf("----------------------------------------------------------------------------------------------------\n");
                    break;
                }
                else if(recv_icmph->icmp_type != ICMP_TIME_EXCEEDED){
                    struct ip *data_ip = (struct ip *)(&(((char *)(recv_icmph))[8]));

                    // received ip header
                    printf("Received IP Header\n");
                    printf("Version: %d\n", data_ip->ip_v);
                    printf("Header Length: %d\n", data_ip->ip_hl);
                    printf("Protocol: %s\n", getProtocol(data_ip->ip_p));
                    printf("Total Length: %d\n", ntohs(data_ip->ip_len));
                    printf("Source IP: %s\n", inet_ntoa(data_ip->ip_src));
                    printf("Destination IP: %s\n", inet_ntoa(data_ip->ip_dst));

                    // print respective protocol header or print unknown protocol
                    if (data_ip->ip_p == IPPROTO_TCP)
                    {
                        struct tcphdr *data_tcp = (struct tcphdr *)(&(((char *)(data_ip))[data_ip->ip_hl << 2]));
                        printf("Received TCP Header\n");
                        printf("Source Port: %d\n", ntohs(data_tcp->source));
                        printf("Destination Port: %d\n", ntohs(data_tcp->dest));
                    }
                    else if (data_ip->ip_p == IPPROTO_UDP)
                    {
                        struct udphdr *data_udp = (struct udphdr *)(&(((char *)(data_ip))[data_ip->ip_hl << 2]));
                        printf("Received UDP Header\n");
                        printf("Source Port: %d\n", ntohs(data_udp->source));
                        printf("Destination Port: %d\n", ntohs(data_udp->dest));
                    }
                    else
                    {
                        printf("Received Unknown Protocol\n");
                    }
                    printf("----------------------------------------------------------------------------------------------------\n");

                }
                else{

                    printf("----------------------------------------------------------------------------------------------------\n");
                    break;
                }
                

            }
            sleep(1);
        }


        if(gotReply){
            // send n zero length packets to find latency with T
            double rtt = 0.0;
            gotReply = 0;

            for(int i=0;i<n;++i){

                icmph->icmp_seq = htons(packets_sent);
                icmph->icmp_cksum = 0;
                icmph->icmp_cksum = checksum(icmph, sizeof(struct icmp));

                int correctPacket = 0;

                // print icmp header in nice format
                printf("Sent ICMP packet: Type: %2d, Code: %2d, DataSize: %5d, Checksum: %7d, Id: %4d,  Seq: %6d\n", icmph->icmp_type, icmph->icmp_code,0,icmph->icmp_cksum, ntohs(icmph->icmp_id), ntohs(icmph->icmp_seq));

                // calculate time
                struct timeval t1, t2;
                gettimeofday(&t1, NULL);
                int sent = sendto(sockfd, packet, sizeof(struct ip) + sizeof(struct icmp), 0, (struct sockaddr *)&host_addr, sizeof(host_addr));

                if (sent < 0)
                {
                    perror("sendto");
                    exit(1);
                }

                packets_sent++;

                struct sockaddr_in recv_addr;
                socklen_t recv_addr_len = sizeof(recv_addr);

                char recv_packet[1000];

                memset(recv_packet, 0, sizeof(recv_packet));

                struct pollfd fd_set[1];
                fd_set[0].fd = sockfd;
                fd_set[0].events = POLLIN;

                int timeout = 2000;

                while(1){
                    int poll_ret = poll(fd_set, 1, timeout);

                    if (poll_ret <= 0)
                    {
                        break;
                    }

                    int recv = recvfrom(sockfd, recv_packet, sizeof(recv_packet), 0, (struct sockaddr *)&recv_addr, &recv_addr_len);
                    // get final time
                    
                    gettimeofday(&t2, NULL);
                    if (recv < 0)
                    {
                        perror("recvfrom");
                        exit(1);
                    }

                    struct icmp *recv_icmph = (struct icmp *)(recv_packet + sizeof(struct ip));

                    // print icmp header in nice format
                    printf("Recv ICMP packet: Type: %2d, Code: %2d, DataSize: %5d, Checksum: %7d, Id: %4d,  Seq: %6d\n", recv_icmph->icmp_type, recv_icmph->icmp_code, (int)(recv - sizeof(struct ip) - sizeof(struct icmp)), icmph->icmp_cksum, ntohs(recv_icmph->icmp_id), ntohs(recv_icmph->icmp_seq));

                    if(recv_icmph->icmp_type == ICMP_TIME_EXCEEDED){
                        printf("Hop: %2d\t IP: %s\n", ttl, inet_ntoa(recv_addr.sin_addr));
                        gotReply++;
                        break;
                    }
                    else if(recv_icmph->icmp_type == ICMP_ECHOREPLY){
                        printf("Hop: %2d\t IP: %s\n", ttl, inet_ntoa(recv_addr.sin_addr));
                        gotReply++;
                        break;
                    }
                    else{
                        struct ip *data_ip = (struct ip *)(&(((char *)(recv_icmph))[8]));

                        // received ip header
                        printf("Received IP Header\n");
                        printf("Version: %d\n", data_ip->ip_v);
                        printf("Header Length: %d\n", data_ip->ip_hl);
                        printf("Protocol: %s\n", getProtocol(data_ip->ip_p));
                        printf("Total Length: %d\n", ntohs(data_ip->ip_len));
                        printf("Source IP: %s\n", inet_ntoa(data_ip->ip_src));
                        printf("Destination IP: %s\n", inet_ntoa(data_ip->ip_dst));

                        // print respective protocol header or print unknown protocol
                        if (data_ip->ip_p == IPPROTO_TCP)
                        {
                            struct tcphdr *data_tcp = (struct tcphdr *)(&(((char *)(data_ip))[data_ip->ip_hl << 2]));
                            printf("Received TCP Header\n");
                            printf("Source Port: %d\n", ntohs(data_tcp->source));
                            printf("Destination Port: %d\n", ntohs(data_tcp->dest));
                        }
                        else if (data_ip->ip_p == IPPROTO_UDP)
                        {
                            struct udphdr *data_udp = (struct udphdr *)(&(((char *)(data_ip))[data_ip->ip_hl << 2]));
                            printf("Received UDP Header\n");
                            printf("Source Port: %d\n", ntohs(data_udp->source));
                            printf("Destination Port: %d\n", ntohs(data_udp->dest));
                        }
                        else
                        {
                            printf("Received Unknown Protocol\n");
                        }
                        correctPacket = 1;
                    }
                }

                if(!correctPacket){
                    double curr = 0.0;
                    curr += (t2.tv_sec - t1.tv_sec) * 1000.0;
                    curr += (t2.tv_usec - t1.tv_usec) / 1000.0;
                    rtt += curr;
                    if(maxRTT1<curr){
                        maxRTT1 = curr;
                    }
                    if(minRTT1>curr){
                        minRTT1 = curr;
                    }
                }
                printf("----------------------------------------------------------------------------------------------------\n");

                sleep(T);
            }

            if(gotReply)rtt /= gotReply;
            double latency = 0.0;
            double bandwidth = 0.0;

            double rtt2 = 0.0;

            // send n packets with data to find bandwidth with T
            gotReply = 0;
            for(int i=0;i<n;++i){

                icmph->icmp_seq = htons(packets_sent);
                copyn(data, sendData, 100);
                icmph->icmp_cksum = 0;
                icmph->icmp_cksum = checksum(icmph, sizeof(struct icmp) + 100);

                // print icmp header in nice format
                printf("Sent ICMP packet: Type: %2d, Code: %2d, DataSize: %5d, Checksum: %7d, Id: %4d,  Seq: %6d\n", icmph->icmp_type, icmph->icmp_code, 100, icmph->icmp_cksum, ntohs(icmph->icmp_id), ntohs(icmph->icmp_seq));

                // calculate time
                struct timeval t1, t2;
                gettimeofday(&t1, NULL);
                int sent = sendto(sockfd, packet, sizeof(struct ip) + sizeof(struct icmp) + 100, 0, (struct sockaddr *)&host_addr, sizeof(host_addr));

                if (sent < 0)
                {
                    perror("sendto");
                    exit(1);
                }

                packets_sent++;

                struct sockaddr_in recv_addr;
                socklen_t recv_addr_len = sizeof(recv_addr);

                char recv_packet[1000];

                memset(recv_packet, 0, sizeof(recv_packet));

                int correctPacket = 0;
                while(1){
                    struct pollfd fd_set[1];
                    fd_set[0].fd = sockfd;
                    fd_set[0].events = POLLIN;


                    int timeout = 2000;

                    int poll_ret = poll(fd_set, 1, timeout);

                    if (poll_ret <= 0)
                    {
                        break;
                    }

                    int recv = recvfrom(sockfd, recv_packet, sizeof(recv_packet), 0, (struct sockaddr *)&recv_addr, &recv_addr_len);
                    // get final time
                    gettimeofday(&t2, NULL);
                    if (recv < 0)
                    {
                        perror("recvfrom");
                        exit(1);
                    }
                    

                    // struct ip *recv_iph = (struct ip *)recv_packet;
                    struct icmp *recv_icmph = (struct icmp *)(recv_packet + sizeof(struct ip));

                    // print icmp header in nice format
                    printf("Recv ICMP packet: Type: %2d, Code: %2d, DataSize: %5d, Checksum: %7d, Id: %4d,  Seq: %6d\n", recv_icmph->icmp_type, recv_icmph->icmp_code, (int)(recv - sizeof(struct ip) - sizeof(struct icmp)), icmph->icmp_cksum, ntohs(recv_icmph->icmp_id), ntohs(recv_icmph->icmp_seq));

                    if(recv_icmph->icmp_type == ICMP_TIME_EXCEEDED){
                        printf("Hop: %2d\t IP: %s\n", ttl, inet_ntoa(recv_addr.sin_addr));
                        gotReply++;
                        break;
                    }
                    else if(recv_icmph->icmp_type == ICMP_ECHOREPLY){
                        printf("Hop: %2d\t IP: %s\n", ttl, inet_ntoa(recv_addr.sin_addr));
                        gotReply++;
                        break;
                    }
                    else{
                        struct ip *data_ip = (struct ip *)(&(((char *)(recv_icmph))[8]));

                        // received ip header
                        printf("Received IP Header\n");
                        printf("Version: %d\n", data_ip->ip_v);
                        printf("Header Length: %d\n", data_ip->ip_hl);
                        printf("Protocol: %s\n", getProtocol(data_ip->ip_p));
                        printf("Total Length: %d\n", ntohs(data_ip->ip_len));
                        printf("Source IP: %s\n", inet_ntoa(data_ip->ip_src));
                        printf("Destination IP: %s\n", inet_ntoa(data_ip->ip_dst));

                        // print respective protocol header or print unknown protocol
                        if (data_ip->ip_p == IPPROTO_TCP)
                        {
                            struct tcphdr *data_tcp = (struct tcphdr *)(&(((char *)(data_ip))[data_ip->ip_hl << 2]));
                            printf("Received TCP Header\n");
                            printf("Source Port: %d\n", ntohs(data_tcp->source));
                            printf("Destination Port: %d\n", ntohs(data_tcp->dest));
                        }
                        else if (data_ip->ip_p == IPPROTO_UDP)
                        {
                            struct udphdr *data_udp = (struct udphdr *)(&(((char *)(data_ip))[data_ip->ip_hl << 2]));
                            printf("Received UDP Header\n");
                            printf("Source Port: %d\n", ntohs(data_udp->source));
                            printf("Destination Port: %d\n", ntohs(data_udp->dest));
                        }
                        else
                        {
                            printf("Received Unknown Protocol\n");
                        }
                        correctPacket = 1;
                    }
                }

                if(!correctPacket){
                    double curr = 0.0;
                    curr += (t2.tv_sec - t1.tv_sec) * 1000.0;
                    curr += (t2.tv_usec - t1.tv_usec) / 1000.0;
                    rtt2 += curr;
                    if(maxRTT2<curr){
                        maxRTT2 = curr;
                    }
                    if(minRTT2>curr){
                        minRTT2 = curr;
                    }
                }
                printf("----------------------------------------------------------------------------------------------------\n");
                sleep(T);
            }

            if(gotReply)rtt2/=gotReply;

            // using min and max
            latency = (maxRTT1 - prev_d1)/2.0;
            if(latency<0)latency = -latency;

            double latencydash = (maxRTT2 - prev_d2)/2.0;
            if(latencydash<0)latencydash = -latencydash;

            bandwidth = 100.0 / (latencydash - latency);
            bandwidth/=1000.0;

            if(bandwidth<0)bandwidth = -bandwidth;

            prev_d1 = minRTT1;
            prev_d2 = minRTT2;

            printf("Bandwidth: %f MB/s, Latency: %f ms\n", bandwidth, latency);
        }
        else{
            printf("No reply\n");
        }
        printf("****************************************************************************************************\n");
        if(done){
            break;
        }
    
        ttl++;
    }
}
