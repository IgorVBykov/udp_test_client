#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "aux.h"

#define TARGET_PORT 5555
#define SOURCE_PORT 4444
#define PACKET_MAX_LENGTH 4096
#define MAX_S_IP_LENGTH 16
#define UDP_HEADER_LENGTH 8

int main(int argc, char **argv)
{
    char packet[PACKET_MAX_LENGTH];
    char udpData[] = "Zzzzzzzzzz";
    char inputData[8192];
    char *pudpPayload;
    char sourceIP[] = "192.168.1.50";
    char *ppsdPacket;
    struct IPHeader *pipHdr = (struct IPHeader *)packet;
    struct UDPHeader *pudpHdr = (struct UDPHeader *)(packet + sizeof(struct IPHeader));
    struct UDPPseudoHDR ppsdHeader;
    struct sockaddr_in serverAddr;
    socklen_t serverAddrLen;
    int addrStatus;
    size_t packetSize, pseudoPacketSize, dataLength;

    if(argc != 2) {
        printf("Wrong usage: udp_test_client target IP\n");
        exit(1);
    }
    memset(&serverAddr, 0, sizeof(serverAddr));
    memset(inputData, 0, sizeof(inputData));
    if( (addrStatus = inet_pton(AF_INET, argv[1], &serverAddr.sin_addr.s_addr)) == -1) {
        printf("Internal error");
        exit(1);
    }
    if(addrStatus == 0) {
        printf("Wrong IP address format\n");
        exit(1);
    }
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(TARGET_PORT);
    serverAddrLen = sizeof(serverAddr);
    // Открываем сырой IP-сокет
    int udpOutFD = socket (AF_INET, SOCK_RAW, IPPROTO_RAW);
    if(udpOutFD == -1)
    {
        // Ошибка открытия сокета
        perror("Can't create raw socket!");
        exit(1);
    }
    int udpInFD = socket (AF_INET, SOCK_RAW, IPPROTO_UDP);
    if(udpOutFD == -1)
    {
        // Ошибка открытия сокета
        perror("Can't create raw socket!");
        exit(1);
    }
    // Формируем заголовок IP и вычисляем его чексумму
    dataLength = sizeof(udpData) - 1;
    pipHdr->ip_vhl = 0x45;
    pipHdr->ip_tos = 0;
    pipHdr->ip_len = htons(sizeof(struct IPHeader) + sizeof(struct UDPHeader) + dataLength);
    pipHdr->ip_id = 0;
    pipHdr->ip_off = 0x0040;
    pipHdr->ip_ttl = 0x40;
    pipHdr->ip_p = IPPROTO_UDP;
    pipHdr->ip_sum = 0;
    inet_pton(AF_INET, sourceIP, &pipHdr->ip_src.s_addr);
    pipHdr->ip_dst.s_addr = serverAddr.sin_addr.s_addr;
    // Формируем заголовок UDP
    pudpHdr->uh_sport = htons(SOURCE_PORT);
    pudpHdr->uh_dport = htons(TARGET_PORT);
    pudpHdr->uh_length = htons(UDP_HEADER_LENGTH + dataLength);
    pudpHdr->uh_crc = 0;
    // Формируем псевдозаголовок для вычисления контрольной суммы
    inet_pton(AF_INET, sourceIP, &ppsdHeader.ip_src);
    ppsdHeader.ip_dst.s_addr = serverAddr.sin_addr.s_addr;
    ppsdHeader.zeroes = 0;
    ppsdHeader.protocol = IPPROTO_UDP;
    pseudoPacketSize = sizeof(struct UDPPseudoHDR) + sizeof(struct UDPHeader) + dataLength;
    ppsdHeader.length = htons(sizeof(struct UDPHeader) + dataLength);
    packetSize = sizeof(struct UDPHeader) + sizeof(struct IPHeader) + dataLength;
    ppsdPacket = malloc(pseudoPacketSize);
    memcpy(ppsdPacket, (char*)&ppsdHeader , sizeof (struct UDPPseudoHDR));
    memcpy(ppsdPacket + sizeof(struct UDPPseudoHDR) , pudpHdr , sizeof(struct UDPHeader));
    memcpy(ppsdPacket + sizeof(struct UDPPseudoHDR) + sizeof(struct UDPHeader), udpData, dataLength);
    pudpHdr->uh_crc = calcCsum((u_int16_t*)ppsdPacket , pseudoPacketSize);
    memcpy(packet + sizeof(struct IPHeader) + sizeof(struct UDPHeader), udpData, dataLength);
    int one = 1;
    const int *val = &one;
    if (setsockopt (udpOutFD, IPPROTO_IP, IP_HDRINCL, val, sizeof (one)) < 0)
    {
        perror("Error setting IP_HDRINCL");
        exit(0);
    }
    int i;
    // Посылаем пакет
    i = sendto(udpOutFD, packet, packetSize,  0, (struct sockaddr*)&serverAddr, serverAddrLen);
    if(i < 0)
    {
        perror("Can't' send packet\n");
    }
    // Пакет отправлен
    else
    {
        printf ("Packet Sent. Length : %d \n" , ntohs(pipHdr->ip_len));
    }
    // Получаем пакет
    i = recvfrom(udpInFD, inputData, sizeof(inputData), 0, (struct sockaddr*)&serverAddr, &serverAddrLen);
    if(i < 0) {
        perror("Can't' read raw socket!\n");
    }
    pipHdr = (struct IPHeader*)inputData;
    pudpPayload = inputData + IP_HL(pipHdr)*4 + UDP_HEADER_LENGTH;
    printf("%s\n", pudpPayload);
    close(udpOutFD);
    close(udpInFD);
    free(ppsdPacket);
    exit(0);
}

