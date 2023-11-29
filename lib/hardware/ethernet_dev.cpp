
#include "ethernet_dev.h"

#include <EEPROM.h>

#include "eeprom_dev.h"


FLASHMEM bool EthernetInit()
{
    uint32_t beginWait = millis();
    uint8_t mac[6];
    bool retval = true;
    Serial.print ("\nEthernet init ");

    EEPreadNBuf (eepAdMyMAC, mac, 6);

    if (EEPROM.read (eepAdDHCPEnabled))
    {
        Serial.print ("via DHCP... ");

        uint16_t DHCPTimeout, DHCPRespTO;
        EEPROM.get (eepAdDHCPTimeout, DHCPTimeout);
        EEPROM.get (eepAdDHCPRespTO, DHCPRespTO);
        if (Ethernet.begin (mac, DHCPTimeout, DHCPRespTO) == 0)
        {
            Serial.println ("*Failed!*");
            // Check for Ethernet hardware present
            if (Ethernet.hardwareStatus() == EthernetNoHardware)
                Serial.println ("Ethernet HW was not found.");
            else
                if (Ethernet.linkStatus() == LinkOFF)
                    Serial.println ("Ethernet cable is not connected.");
            retval = false;
        }
        else
        {
            Serial.println ("passed.");
        }
    }
    else
    {
        Serial.println ("using Static");
        uint32_t ip, dns, gateway, subnetmask;
        EEPROM.get (eepAdMyIP, ip);
        EEPROM.get (eepAdDNSIP, dns);
        EEPROM.get (eepAdGtwyIP, gateway);
        EEPROM.get (eepAdMaskIP, subnetmask);
        Ethernet.begin (mac, ip, dns, gateway, subnetmask);
    }

    Serial.printf ("Took %d mS\nIP: ", (millis() - beginWait));
    Serial.println (Ethernet.localIP());
    return retval;
}
