
#ifndef ETHERNET_H
#define ETHERNET_H

#include <NativeEthernet.h>
#include <NativeEthernetUdp.h>

extern EthernetUDP udp;
extern EthernetClient client;

FLASHMEM bool EthernetInit();

#endif // ETHERNET_H