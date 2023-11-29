
#ifndef ETHERNET_H
#define ETHERNET_H

#include <NativeEthernet.h>
#include <NativeEthernetUdp.h>

EthernetUDP udp;

FLASHMEM bool EthernetInit();

#endif // ETHERNET_H