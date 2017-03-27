#ifndef AUX_H
#define AUX_H
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

struct IPHeader {
    u_char  ip_vhl;                 /* version << 4 | header length >> 2 */
    u_char  ip_tos;                 /* type of service */
    u_short ip_len;                 /* total length */
    u_short ip_id;                  /* identification */
    u_short ip_off;                 /* fragment offset field */
    #define IP_RF 0x8000            /* reserved fragment flag */
    #define IP_DF 0x4000            /* dont fragment flag */
    #define IP_MF 0x2000            /* more fragments flag */
    #define IP_OFFMASK 0x1fff       /* mask for fragmenting bits */
    u_char  ip_ttl;                 /* time to live */
    u_char  ip_p;                   /* protocol */
    u_short ip_sum;                 /* checksum */
    struct  in_addr ip_src,ip_dst;  /* source and dest address */
};

#define IP_HL(ip)               (((ip)->ip_vhl) & 0x0f)
#define IP_V(ip)                (((ip)->ip_vhl) >> 4)

// Total length: 8
struct UDPHeader {
    u_short uh_sport;
    u_short uh_dport;
    u_short uh_length;
    u_short uh_crc;
};
// Total length: 12
struct UDPPseudoHDR {
    struct in_addr ip_src;
    struct in_addr ip_dst;
    uint8_t zeroes;
    uint8_t protocol;
    uint16_t length;
};

u_int16_t calcCsum(const u_int16_t *packet, int length);

#endif
