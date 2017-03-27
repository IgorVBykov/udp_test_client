#include <stdlib.h>
#include <stdio.h>
#include "aux.h"

u_int16_t calcCsum(const u_int16_t *packet, int length)
{
    uint32_t cksum = 0;
    while(length > 1)
    {
        cksum += *packet++;
        length -= 2;
    }
    if(length) {
        cksum += *(u_char*)packet;
    }
    cksum = (cksum >> 16) + (cksum & 0xffff);
    cksum += (cksum >>16);
    cksum = ~cksum;
    return (uint16_t)(cksum);
}
