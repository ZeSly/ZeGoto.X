/*********************************************************************
 *
 *	UPNP Client
 *  Module for Microchip TCP/IP Stack
 * http://www.microchip.com/forums/m529284.aspx
 *
 *********************************************************************
 * FileName:        upnp.c
 * Dependencies:    UDP
 * Processor:       PIC18, PIC24F, PIC24H, dsPIC30F, dsPIC33F, PIC32
 * Compiler:        Microchip C32 v1.05 or higher
 *					Microchip C30 v3.12 or higher
 *					Microchip C18 v3.30 or higher
 *					HI-TECH PICC-18 PRO 9.63PL2 or higher
 ********************************************************************/

#define __UPNP_C

#include "TCPIPConfig.h"

#if defined(STACK_USE_UPNP)

#include "TCPIP_Stack/TCPIP.h"

#include "main.h"
#include "upnp.h"

//#define WITH_DATE
//#define UPNP11

#if defined(WITH_DATE)
#include <time.h>
#endif

#define HTTP_XML_HEADER "" \
"HTTP/1.1 200 OK\r\n" \
"CONTENT-TYPE: text/xml;charset=\"utf-8\"\r\n" \
"CONTENT-LENGTH: %i\r\n"

//"<?xml version=\"1.0\" encoding=\"utf-8\"?>\r\n"

#define DEVICE_XML_BODY_PART1 "" \
"<?xml version=\"1.0\"?>\r\n" \
"<root xmlns=\"urn:schemas-upnp-org:device-1-0\">\r\n" \
"	<specVersion>\r\n" \
"		<major>1</major>\r\n" \
"		<minor>0</minor>\r\n" \
"	</specVersion>\r\n" \
"	<device>\r\n" \
"		<deviceType>urn:schemas-upnp-org:device:Basic:1</deviceType>\r\n" \
"		<friendlyName>OpenGoto</friendlyName>\r\n" \
"		<manufacturer>Sylvain Girard</manufacturer>\r\n" \
"		<manufacturerURL>http://www.zesly.net</manufacturerURL>\r\n" \
"		<modelName>OpenGoto</modelName>\r\n" \
"		<modelDescription>OpenGoto Equatorial mount controller</modelDescription>\r\n" \
"		<modelNumber>OpenGoto "VERSION"</modelNumber>\r\n" \
"		<modelURL>http://www.zesly.net</modelURL>\r\n" \
"		<serialNumber>ZESLY-123456</serialNumber>\r\n"

#define DEVICE_XML_BODY_PART2 "" \
"		<UDN>uuid:%s</UDN>\r\n"

/*#define DEVICE_XML_BODY_PART3 "" \
"		<serviceList>\r\n" \
"			<service>\r\n" \
"				<serviceType>urn:schemas-upnp-org:service:SwitchPower:1</serviceType>\r\n" \
"				<serviceId>urn:upnp-org:serviceId:SwitchPower</serviceId>\r\n" \
"				<SCPDURL>/switchpower.xml</SCPDURL>\r\n" \
"				<controlURL>/control</controlURL>\r\n" \
"				<eventSubURL>/event</eventSubURL>\r\n" \
"			</service>\r\n" \
"		</serviceList>\r\n"*/

#define DEVICE_XML_BODY_PART4 "" \
"		<presentationURL>http://%s</presentationURL>\r\n"

#define DEVICE_XML_BODY_PART5 "" \
"	</device>\r\n" \
"</root>"

//};
//const BYTE SERVICE1_XML_BODY[] =
//{
#define SERVICE1_XML_BODY "" \
"<?xml version=\"1.0\"?>\r\n" \
"<scpd xmlns=\"urn:schemas-upnp-org:service-1-0\">\r\n" \
"	<specVersion>\r\n" \
"		<major>1</major>\r\n" \
"		<minor>0</minor>\r\n" \
"	</specVersion>\r\n" \
"	<actionList>\r\n" \
"		<action>\r\n" \
"			<name>SETBLUE</name>\r\n" \
"			<argumentList>\r\n" \
"				<argument>\r\n" \
"					<name>LEVEL</name>\r\n" \
"					<relatedStateVariable>LEVEL</relatedStateVariable>\r\n" \
"					<direction>in</direction>\r\n" \
"				</argument>\r\n" \
"			</argumentList>\r\n" \
"		</action>\r\n" \
"		<action>\r\n" \
"			<name>GETBLUE</name>\r\n" \
"			<argumentList>\r\n" \
"				<argument>\r\n" \
"					<name>LEVEL</name>\r\n" \
"					<relatedStateVariable>LEVEL</relatedStateVariable>\r\n" \
"					<direction>out</direction>\r\n" \
"				</argument>\r\n" \
"			</argumentList>\r\n" \
"		</action>\r\n" \
"	</actionList>\r\n" \
"	<serviceStateTable>\r\n" \
"		<stateVariable sendEvents=\"no\">\r\n" \
"			<name>LEVEL</name>\r\n" \
"			<dataType>ui1</dataType>\r\n" \
"			<allowedValueRange>\r\n" \
"				<minimum>0</minimum>\r\n" \
"				<maximum>255</maximum>\r\n" \
"				<step>1</step>\r\n" \
"			</allowedValueRange>\r\n" \
" 		</stateVariable>\r\n" \
"	</serviceStateTable>\r\n" \
"</scpd>"
//};

#if defined UPNP11
static unsigned int BUO_COUNTER = 0;
#endif

extern NODE_INFO remoteNode;


const BYTE multicast_IP[4] =
{
    239,
    255,
    255,
    250

};
const BYTE multicast_MAC[6] =
{
    0x01,
    0x00,
    0x5E,
    0x00,
    0x00,
    0x00
};

char HOST[] = "000.000.000.000";
char localipstring[] = "000.000.000.000";

char upnp_id[] = "00000000-0000-0000-0000-000000000000";

BYTE uuid_test(const char *uuid) //uuid: xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx
{
    unsigned int i;

    for (i = strlen(uuid); i != 0; i--)
    {
        if ((*uuid != '0') && (*uuid != '-'))
        {
            return 1;
        }
        uuid++;
    }

    return 0;
}

unsigned long generate_id(void)
{
    DWORD_VAL temp;
    DWORD tick = TickGet();
    temp.v[3] = AppConfig.MyMACAddr.v[2];
    temp.v[2] = AppConfig.MyMACAddr.v[3];
    temp.v[1] = AppConfig.MyMACAddr.v[4];
    temp.v[0] = AppConfig.MyMACAddr.v[5];
    srand(temp.Val + tick);
    return temp.Val + rand();
}

void uuid_generate(char *uuid) //uuid: xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxx
{
    unsigned long r;
    unsigned int a, b, c, d, e, f, g, h;

    r = generate_id();
    a = (r >> 0) & 0xFFFF;
    b = (r >> 16) & 0xFFFF;

    r = generate_id();
    c = (r >> 0) & 0xFFFF;
    d = (r >> 16) & 0xFFFF;

    r = generate_id();
    e = (r >> 0) & 0xFFFF;
    f = (r >> 16) & 0xFFFF;

    r = generate_id();
    g = (r >> 0) & 0xFFFF;
    h = (r >> 16) & 0xFFFF;


    //sprintf(uuid, "%04X-%02X-%02X-%02X-%04X%02X", (a&0xFFFF), (b&0xFF), (c&0xFF), (d&0xFF), (e&0xFFFF), (f&0xFF));
    sprintf(uuid, "%04x%04x-%04x-%04x-%04x-%04x%04x%04x", (a & 0xFFFF), (b & 0xFFFF), (c & 0xFFFF), (d & 0xFFFF), (e & 0xFFFF), (f & 0xFFFF), (g & 0xFFFF), (h & 0xFFFF));

    return;
}

/*********************************************************************
 * Function:        void SSDPAnnounce(void)
 *
 * Summary:         Transmits an Announce packet.
 *
 * PreCondition:    Stack is initialized(); IP address by DHCP
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        AnnounceIP opens a UDP socket and transmits a 
 *					broadcast packet to port 1900.  If a computer is
 *					on the same subnet and a utility is looking for 
 *					packets on the UDP port, it will receive the 
 *					broadcast.  For this application, it is used to 
 *					announce the change of this board's IP address.
 *					The messages can be viewed with the MCHPDetect.exe
 *					program.
 *
 * Note:            A UDP socket must be available before this 
 *					function is called.  It is freed at the end of 
 *					the function.  MAX_UDP_SOCKETS may need to be 
 *					increased if other modules use UDP sockets.
 ********************************************************************/
void SSDPAnnounce(void)
{
    UDP_SOCKET MySocket;
    NODE_INFO LocalNode;
    char buf[65];

    int i, counter;

    DWORD StartingTickTime = 0;
    DWORD random_delay = 0;

    if (uuid_test(upnp_id) == 0)
    {
        uuid_generate(upnp_id);
    }

    sprintf(localipstring, "%i.%i.%i.%i", AppConfig.MyIPAddr.v[0], AppConfig.MyIPAddr.v[1], AppConfig.MyIPAddr.v[2], AppConfig.MyIPAddr.v[3]);

#if defined UPNP11
    BUO_COUNTER++;
#endif

    // Open UDP socket for broadcast on SSDP port 1900!!

    LocalNode.IPAddr.v[0] = multicast_IP[0];
    LocalNode.IPAddr.v[1] = multicast_IP[1];
    LocalNode.IPAddr.v[2] = multicast_IP[2];
    LocalNode.IPAddr.v[3] = multicast_IP[3];

    LocalNode.MACAddr.v[0] = multicast_MAC[0];
    LocalNode.MACAddr.v[1] = multicast_MAC[1];
    LocalNode.MACAddr.v[2] = multicast_MAC[2];
    LocalNode.MACAddr.v[3] = (multicast_IP[1] & 0x7F);
    LocalNode.MACAddr.v[4] = multicast_IP[2];
    LocalNode.MACAddr.v[5] = multicast_IP[3];

    //MySocket = UDPOpen(0, &LocalNode, SSDP_PORT);
    MySocket = UDPOpenEx((DWORD)(PTR_BASE) &LocalNode, UDP_OPEN_NODE_INFO, 0, SSDP_PORT);

    // Abort operation if no UDP sockets are available
    // If this ever happens, incrementing MAX_UDP_SOCKETS in
    // StackTsk.h may help (at the expense of more global memory
    // resources).
    if (MySocket == INVALID_UDP_SOCKET)
        return;

    // Advertise 3 times due to unreliable nature of UDP
    for (counter = 0; counter < 3; counter++)
    {
        /*Add random delay of 100ms max*/
        random_delay = ((LFSRRand() % (100) * (TICK_SECOND / 1000)));
        StartingTickTime = TickGet();
        while ((TickGet() - StartingTickTime) < (random_delay));

        // Expose all interfaces
        for (i = 0; i < 6; i++)
        {
            while (!UDPIsPutReady(MySocket));

            UDPPutROMString((ROM BYTE*) "NOTIFY * HTTP/1.1\r\n");

            sprintf(buf, "HOST: %i.%i.%i.%i:%i\r\n", LocalNode.IPAddr.v[0], LocalNode.IPAddr.v[1], LocalNode.IPAddr.v[2], LocalNode.IPAddr.v[3], SSDP_PORT);
            UDPPutString((void*) buf);

            UDPPutROMString((ROM BYTE*) "CACHE-CONTROL: max-age=1800\r\n");

            sprintf(buf, "LOCATION: http://%s:%i/device.xml\r\n", localipstring, UPNP_PORT);
            UDPPutString((void*) buf);

            if (i == 0)
            {
                UDPPutROMString((ROM BYTE*) "NT: upnp:rootdevice\r\n");
            }
            else if ((i == 1) || (i == 3))
            {
                sprintf(buf, "NT: uuid:%s", upnp_id);
                UDPPutString((void*) buf);
                UDPPutROMString((ROM BYTE*) "\r\n");
            }
            else if ((i == 2) || (i == 4))
            {
                UDPPutROMString((ROM BYTE*) "NT: urn:schemas-upnp-org:device:BinaryLight:1\r\n");
            }
            else if (i == 5)
            {
                UDPPutROMString((ROM BYTE*) "NT: urn:schemas-upnp-org:service:SwitchPower:1\r\n");
            }


            UDPPutROMString((ROM BYTE*) "NTS: ssdp:alive\r\n");

#if defined UPNP11
            sprintf(buf, "SERVER: MICROCHIP/5.3 UPnP/1.1 %s/1.0", MY_DEFAULT_HOST_NAME);
#else
            sprintf(buf, "SERVER: MICROCHIP/5.3 UPnP/1.0 %s/1.0", MY_DEFAULT_HOST_NAME);
#endif
            UDPPutString((void*) buf);
            UDPPutROMString((ROM BYTE*) "\r\n");

            if (i == 0)
            {
                sprintf(buf, "USN: uuid:%s::upnp:rootdevice\r\n", upnp_id);
                UDPPutString((void*) buf);
            }
            else if ((i == 1) || (i == 3))
            {
                sprintf(buf, "USN: uuid:%s\r\n", upnp_id);
                UDPPutString((void*) buf);
            }
            else if ((i == 2) || (i == 4))
            {
                sprintf(buf, "USN: uuid:%s", upnp_id);
                UDPPutString((void*) buf);
                UDPPutROMString((ROM BYTE*) "::urn:schemas-upnp-org:device:BinaryLight:1\r\n");
            }
            else if (i == 5)
            {
                sprintf(buf, "USN: uuid:%s", upnp_id);
                UDPPutString((void*) buf);
                UDPPutROMString((ROM BYTE*) "::urn:schemas-upnp-org:service:SwitchPower:1\r\n");
            }

            //UDPPutROMString((ROM BYTE*)"Content-Length: 0\r\n");

#if defined UPNP11
            sprintf(buf, "BOOTID.UPNP.ORG: %i\r\n", BUO_COUNTER);
            UDPPutString((void*) buf);
            sprintf(buf, "CONFIGID.UPNP.ORG: 12345\r\n");
            UDPPutString((void*) buf);
#endif

            UDPPut('\r');
            UDPPut('\n');

            // Send the packet
            UDPFlush();

            UDPDiscard();

        }
    }


    // Close the socket so it can be used by other modules
    UDPClose(MySocket);

#if defined(STACK_USE_UART)	
    putrsUART("SSDP announcement ready !!!!\r\n");
#endif

}

/*********************************************************************
 * Function:        void DiscoveryTask(void)
 *
 * Summary:         Announce callback task.
 *
 * PreCondition:    Stack is initialized()
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Recurring task used to listen for Discovery
 *                  messages on the specified ANNOUNCE_PORT.  These
 *                  messages can be sent using the Microchip Device
 *                  Discoverer tool. If one is received, this
 *                  function will transmit a reply.
 *
 * Note:            A UDP socket must be available before this 
 *					function is called.  It is freed at the end of 
 *					the function.  MAX_UDP_SOCKETS may need to be 
 *					increased if other modules use UDP sockets.
 ********************************************************************/
void SSDPDiscoveryTask(void)
{

    static enum
    {
        DISCOVERY_HOME,
        DISCOVERY_LISTEN,
        DISCOVERY_REQUEST_WAITING,
        DISCOVERY_REQUEST_RESPONSE,
        DISCOVERY_DISABLED
    } DiscoverySM = DISCOVERY_HOME;

    static enum
    {
        RES_NONE,
        RES_ROOT,
        RES_UUID,
        RES_DEVICE,
        RES_SERVICE
    } SearchResponse = RES_NONE;

    static UDP_SOCKET MySocket = INVALID_UDP_SOCKET;
    //UDP_SOCKET	MySendSocket = INVALID_UDP_SOCKET;
    UINT8 mcast_addr[6];

    WORD wMaxGet;

    static BYTE ResponseCounter = 0;
    static DWORD StartingTickTime = 0;
    static WORD random_delay = 0;
    static BYTE MX = 0;

    char buf[150];
    char * STpos;
    char * ptr;

    if (uuid_test(upnp_id) == 0)
    {
        uuid_generate(upnp_id);
    }

    switch (DiscoverySM)
    {
    case DISCOVERY_HOME:

        // Wait for DHCP to finish !!

#if defined(STACK_USE_DHCP_CLIENT) && defined(STACK_USE_UPNP)
    {
        static DWORD dwTimer = 0;

        // Wait until DHCP module is finished
        if (AppConfig.Flags.bIsDHCPEnabled)
        {
            if (!DHCPIsBound(0))
            {
                dwTimer = TickGet();
                return;
            }
        }

        // Wait an additional half second after DHCP is finished to let the announce module and any other stack state machines to reach normal operation
        if (TickGet() - dwTimer < TICK_SECOND / 2)
            return;
    }
#endif

        sprintf(localipstring, "%i.%i.%i.%i", AppConfig.MyIPAddr.v[0], AppConfig.MyIPAddr.v[1], AppConfig.MyIPAddr.v[2], AppConfig.MyIPAddr.v[3]);

        // Calculate MAC to enable reception of packets !!
        mcast_addr[0] = multicast_MAC[0];
        mcast_addr[1] = multicast_MAC[1];
        mcast_addr[2] = multicast_MAC[2];
        mcast_addr[3] = (multicast_IP[1] & 0x7F);
        mcast_addr[4] = multicast_IP[2];
        mcast_addr[5] = multicast_IP[3];

#if /* PIC32MX6XX/7XX Internal Ethernet controller */ (defined(__PIC32MX__) && defined(_ETH) && !defined(ENC100_INTERFACE_MODE) && !defined(ENC_CS_TRIS) && !defined(WF_CS_TRIS)) || \
			/* ENC424J600/624J600 */						  defined(ENC100_INTERFACE_MODE) || \
			/* ENC28J60 */									  defined(ENC_CS_TRIS) || \
			/* PIC18F97J60 family internal Ethernet controller with C18 compiler */ (defined(__18F97J60) || defined(__18F96J65) || defined(__18F96J60) || defined(__18F87J60) || defined(__18F86J65) || defined(__18F86J60) || defined(__18F67J60) || defined(__18F66J65) || defined(__18F66J60) || \
			/* PIC18F97J60 family internal Ethernet controller HI-TECH PICC-18 compiler */ defined(_18F97J60) ||  defined(_18F96J65) ||  defined(_18F96J60) ||  defined(_18F87J60) ||  defined(_18F86J65) ||  defined(_18F86J60) ||  defined(_18F67J60) ||  defined(_18F66J65) ||  defined(_18F66J60))
        SetRXHashTableEntry(*((MAC_ADDR*) mcast_addr));
#elif /* MRF24WB0M */ defined(WF_CS_TRIS)
        WF_SetMultiCastFilter(WF_MULTICAST_FILTER_2, mcast_addr);
#else
#error Must call appropraite API to enable multicast packet reception in the network controller.
#endif

        // Open a UDP socket for inbound and outbound transmission
        // Since we expect to only receive broadcast packets and
        // only send unicast packets directly to the node we last
        // received from, the remote NodeInfo parameter can be anything

        MySocket = UDPOpen(SSDP_PORT, NULL, INVALID_UDP_SOCKET);
        //MySocket = UDPOpen(SSDP_PORT, NULL, 3911);

        if (MySocket == INVALID_UDP_SOCKET)
            return;

        DiscoverySM = DISCOVERY_LISTEN;

        break;

    case DISCOVERY_LISTEN:

        wMaxGet = UDPIsGetReady(MySocket);
        if (!wMaxGet) return;

        // See if this is a discovery query. Do not respond to NOTIFY
        UDPGetArray((BYTE*) buf, 8);
        if (memcmppgm2ram(buf, (ROM void*) "M-SEARCH", 8))
        {
            UDPDiscard();
            return;
        }

#if defined(STACK_USE_UART)
        putrsUART("M-SEARCH detected on port 1900 !!!!\r\n");
#endif

        wMaxGet = UDPIsGetReady(MySocket);
        if (!wMaxGet) return;

        if (wMaxGet > (sizeof (buf) - 1)) wMaxGet = (sizeof (buf) - 1);
        UDPGetArray((BYTE*) buf, wMaxGet);
        //buf[wMaxGet]='\0';
        buf[wMaxGet] = 0;

        UDPDiscard();

        STpos = NULL;
        STpos = strstr(buf, (ROM void*) "HOST:");
        if (STpos == NULL) STpos = strstr(buf, (ROM void*) "Host:");
        if (STpos != NULL)
        {
            STpos += 5;
            if (*STpos == ' ') STpos++;
            ptr = memchr(STpos, ':', 17);
            if (ptr != NULL)
            {
                memcpy(HOST, STpos, ptr - STpos);
                HOST[ptr - STpos] = 0;
                // Multicast ??
                if (memcmppgm2ram(HOST, (ROM void*) "239.255.255.250", 15) == 0)
                {
#if defined(STACK_USE_UART)	
                    //putrsUART("Got you multicast IP-host !!!!\r\n");
#endif
                }
                else
                    if (memcmp(HOST, localipstring, strlen(localipstring)) == 0)
                {
#if defined(STACK_USE_UART)	
                    //putrsUART("Got you unicast IP-host !!!!\r\n");
#endif
                }
                else return;
            }
        }

        SearchResponse = RES_NONE;

        STpos = NULL;
        STpos = strstr(buf, (ROM void*) "ST:");
        if (STpos != NULL)
        {
            STpos += 3;
            if (*STpos == ' ') STpos++;
            if (memcmppgm2ram(STpos, (ROM void*) "upnp:rootdevice", 15) == 0) SearchResponse = RES_ROOT;
            else
                if (memcmppgm2ram(STpos, (ROM void*) "root", 4) == 0) SearchResponse = RES_ROOT;
                //else
                //if ( (memcmppgm2ram(STpos, (ROM void*)"uuid:", 5) == 0) && (memcmp((char*)(STpos+5),upnp_id,strlen(upnp_id))) ) SearchResponse = RES_UUID;
            else
                if ((memcmppgm2ram(STpos, (ROM void*) "uuid:", 5) == 0) && (strstr(STpos + 5, upnp_id) != NULL)) SearchResponse = RES_UUID;
            else
                if (memcmppgm2ram(STpos, (ROM void*) "urn:schemas-upnp-org:device:BinaryLight:1", 41) == 0) SearchResponse = RES_DEVICE;
            else
                if (memcmppgm2ram(STpos, (ROM void*) "urn:schemas-upnp-org:service:SwitchPower:1", 42) == 0) SearchResponse = RES_SERVICE;

        }

        if (SearchResponse == RES_NONE)
            return;

        // Find response time
        MX = 0;
        STpos = NULL;
        STpos = strstr(buf, (ROM void*) "MX:");
        STpos += 3;
        if (*STpos == ' ') STpos++;
        while ((*STpos >= '0') && (*STpos <= '9'))
        {
            MX *= 10;
            MX += *STpos - '0';
            STpos++;
        }
        if (MX > 5) MX = 5;
        if (MX == 0) MX = 1;

        ResponseCounter = 0;

        // Change the destination to the unicast address of the last received packet
        memcpy((void*) &UDPSocketInfo[MySocket].remote.remoteNode, (const void*) &remoteNode, sizeof (remoteNode));
        //UDPSocketInfo[MySocket].localPort = 3911;
        //MySendSocket = UDPOpen(0, &remoteNode, UDPSocketInfo[MySocket].remotePort);

        DiscoverySM = DISCOVERY_REQUEST_WAITING;

#if defined(STACK_USE_UART)
        putrsUART("SSDP discovery request received !!!!\r\n");
#endif

        break;

    case DISCOVERY_REQUEST_WAITING:

        // random delay between 0 and MX seconds
        if (SearchResponse == RES_UUID) MX = 1;
        random_delay = ((LFSRRand() % (MX * 1000) * (TICK_SECOND / 1000)));
        StartingTickTime = TickGet(); // The time we started the waiting.

        DiscoverySM = DISCOVERY_REQUEST_RESPONSE;

        break;

    case DISCOVERY_REQUEST_RESPONSE:

        if ((TickGet() - StartingTickTime) < (random_delay))
        {
            //Not Completed the delay proposed
            return;
        }

        //while(!UDPIsPutReady(MySendSocket));
        while (!UDPIsPutReady(MySocket));

        UDPPutROMString((ROM BYTE*) "HTTP/1.1 200 OK\r\n");

        UDPPutROMString((ROM BYTE*) "CACHE-CONTROL: max-age=1800\r\n");

        UDPPutROMString((ROM BYTE*) "EXT:\r\n");

        sprintf(buf, "LOCATION: http://%s:%i/device.xml\r\n", localipstring, UPNP_PORT);
        UDPPutString((void*) buf);

#if defined UPNP11
        sprintf(buf, "SERVER: MICROCHIP/5.3 UPnP/1.1 %s/1.0", MY_DEFAULT_HOST_NAME);
#else
        sprintf(buf, "SERVER: MICROCHIP/5.3 UPnP/1.0 %s/1.0", MY_DEFAULT_HOST_NAME);
#endif
        UDPPutString((void*) buf);
        UDPPutROMString((ROM BYTE*) "\r\n");

        if (SearchResponse == RES_ROOT)
        {
            UDPPutROMString((ROM BYTE*) "ST: upnp:rootdevice\r\n");
            sprintf(buf, "USN: uuid:%s::upnp:rootdevice\r\n", upnp_id);
            UDPPutString((void*) buf);
        }
        else if (SearchResponse == RES_UUID)
        {
            sprintf(buf, "ST: uuid:%s\r\n", upnp_id);
            UDPPutString((void*) buf);
            sprintf(buf, "USN: uuid:%s\r\n", upnp_id);
            UDPPutString((void*) buf);
        }
        else if (SearchResponse == RES_DEVICE)
        {
            UDPPutROMString((ROM BYTE*) "ST: urn:schemas-upnp-org:device:BinaryLight:1\r\n");
            sprintf(buf, "USN: uuid:%s", upnp_id);
            UDPPutString((void*) buf);
            UDPPutROMString((ROM BYTE*) "::urn:schemas-upnp-org:device:BinaryLight:1\r\n");
        }
        else if (SearchResponse == RES_SERVICE)
        {
            UDPPutROMString((ROM BYTE*) "ST: urn:schemas-upnp-org:service:SwitchPower:1\r\n");
            sprintf(buf, "USN: uuid:%s", upnp_id);
            UDPPutString((void*) buf);
            UDPPutROMString((ROM BYTE*) "::urn:schemas-upnp-org:service:SwitchPower:1\r\n");
        }

#if defined UPNP11
        sprintf(buf, "BOOTID.UPNP.ORG: %i\r\n", BUO_COUNTER);
        UDPPutString((void*) buf);
        sprintf(buf, "CONFIGID.UPNP.ORG: 12345\r\n");
        UDPPutString((void*) buf);
#else
        //UDPPutROMString((ROM BYTE*)"CONTENT-LENGTH: 0\r\n");
#endif

        UDPPut('\r');
        UDPPut('\n');

        // Send the packet
        UDPFlush();

#if defined(STACK_USE_UART)
        putrsUART("SSDP discovery request response send !!!!\r\n");
#endif

        // Listen for other discovery requests
        ResponseCounter++;
        if (ResponseCounter < 0)
        {
            DiscoverySM = DISCOVERY_REQUEST_WAITING;
        }
        else
        {
            //UDPClose(MySendSocket);
            DiscoverySM = DISCOVERY_LISTEN;
        }

        break;

    case DISCOVERY_DISABLED:
        UDPClose(MySocket);
        break;
    }

}

void UPNPServer(void)
{
    WORD i;
    BYTE AppBuffer[150];
    WORD wMaxGet;
    int length;
    static TCP_SOCKET MySocket;

    static enum _TCPServerState
    {
        SM_HOME = 0,
        SM_LISTENING,
        SM_SENDING,
        SM_CLOSING
    } TCPServerState = SM_HOME;

    static enum _UPNPMode
    {
        UPNP_NONE,
        UPNP_DEVICE,
        UPNP_SERVICE1,
        UPNP_SERVICE2,
        UPNP_SERVICE3
    } UPNPMode = UPNP_NONE;

    static BOOL WithLanguage = FALSE;

#if defined(WITH_DATE)
    time_t dwTime;
    struct tm *newtime;
#endif

    if (uuid_test(upnp_id) == 0)
    {
        uuid_generate(upnp_id);
    }

    switch (TCPServerState)
    {
    case SM_HOME:
        // Allocate a socket for this server to listen and accept connections on
        MySocket = TCPOpen(0, TCP_OPEN_SERVER, UPNP_PORT, TCP_PURPOSE_GENERIC_TCP_SERVER);
        if (MySocket == INVALID_SOCKET)
            return;

        TCPServerState = SM_LISTENING;

        break;

    case SM_LISTENING:
        // See if anyone is connected to us
        if (!TCPIsConnected(MySocket))
            return;

        // Process all bytes that we can
        wMaxGet = TCPIsGetReady(MySocket); // Get TCP RX FIFO byte count

        if (!wMaxGet)
            return;

        if (wMaxGet > (sizeof (AppBuffer) - 1)) wMaxGet = (sizeof (AppBuffer) - 1);
        TCPGetArray(MySocket, AppBuffer, wMaxGet);
        //AppBuffer[wMaxGet]=0;

        if (memcmppgm2ram(AppBuffer, (ROM void*) "GET /device.xml", 15) == 0)
        {
            UPNPMode = UPNP_DEVICE;
        }
//        else if (memcmppgm2ram(AppBuffer, (ROM void*) "GET /switchpower.xml", 20) == 0)
//        {
//            UPNPMode = UPNP_SERVICE1;
//        }
        else UPNPMode = UPNP_NONE;

        if (strstr((char*) AppBuffer, (ROM void*) "ACCEPT-LANGUAGE") != NULL) WithLanguage = TRUE;
        else WithLanguage = FALSE;

        TCPDiscard(MySocket);

        if (UPNPMode == UPNP_NONE)
            return;

        TCPServerState = SM_SENDING;

        break;

    case SM_SENDING:

#if defined(WITH_DATE)
        //The number of seconds since the Epoch.  (Default 01-Jan-1970 00:00:00)
        dwTime = SNTPGetUTCSeconds();
        newtime = gmtime(&dwTime);
#if defined(STACK_USE_UART)
        sprintf((char*) AppBuffer, "UTC time = %s\r\n", asctime(newtime));
        putrsUART((char*) AppBuffer);
#endif
#endif

        if (UPNPMode == UPNP_DEVICE)
        {
#if defined(STACK_USE_UART)
            putrsUART("Got you device !!!!!!!!\r\n");
#endif
            i = strlen((char*) DEVICE_XML_BODY_PART1);
            i += sprintf((char*) AppBuffer, DEVICE_XML_BODY_PART2, upnp_id);
            //i += strlen((char*) DEVICE_XML_BODY_PART3);
            i += sprintf((char*) AppBuffer, DEVICE_XML_BODY_PART4, localipstring);
            i += strlen((char*) DEVICE_XML_BODY_PART5);
            length = sprintf((char*) AppBuffer, HTTP_XML_HEADER, i);
            if (length > TCPIsPutReady(MySocket)) TCPFlush(MySocket);
            TCPPutArray(MySocket, AppBuffer, length);
#if defined(WITH_DATE)
            //length=sprintf((char*)AppBuffer,"DATE: %s\r\n",i,asctime(newtime));
            length = sprintf((char*) AppBuffer, "DATE: %s\r\n", "MON, 18 Nov 2010 23:59:59 GMT");
            if (length > TCPIsPutReady(MySocket)) TCPFlush(MySocket);
            TCPPutArray(MySocket, AppBuffer, length);
#endif
            if (WithLanguage) TCPPutROMString(MySocket, (ROM BYTE*) "CONTENT-LANGUAGE: en\r\n");
            TCPPutROMString(MySocket, (ROM BYTE*) "\r\n");
            if (i > TCPIsPutReady(MySocket)) TCPFlush(MySocket);
            TCPPutROMArray(MySocket, DEVICE_XML_BODY_PART1, strlen((char*) DEVICE_XML_BODY_PART1));
            length = sprintf((char*) AppBuffer, DEVICE_XML_BODY_PART2, upnp_id);
            TCPPutArray(MySocket, AppBuffer, length);
            //TCPPutROMArray(MySocket, DEVICE_XML_BODY_PART3, strlen((char*) DEVICE_XML_BODY_PART3));
            length = sprintf((char*) AppBuffer, DEVICE_XML_BODY_PART4, localipstring);
            TCPPutArray(MySocket, AppBuffer, length);
            TCPPutROMArray(MySocket, DEVICE_XML_BODY_PART5, strlen((char*) DEVICE_XML_BODY_PART5));
        }
//        else if (UPNPMode == UPNP_SERVICE1)
//        {
//#if defined(STACK_USE_UART)
//            putrsUART("Got you service !!!!!!!!!!!!!!!!!!!!!!\r\n");
//#endif
//            i = strlenpgm(SERVICE1_XML_BODY);
//            length = sprintf((char*) AppBuffer, HTTP_XML_HEADER, i);
//            if (length > TCPIsPutReady(MySocket)) TCPFlush(MySocket);
//            TCPPutArray(MySocket, AppBuffer, length);
//#if defined(WITH_DATE)
//            //length=sprintf((char*)AppBuffer,"DATE: %s\r\n",i,asctime(newtime));
//            length = sprintf((char*) AppBuffer, "DATE: %s\r\n", "MON, 18 Nov 2010 23:59:59 GMT");
//            if (length > TCPIsPutReady(MySocket)) TCPFlush(MySocket);
//            TCPPutArray(MySocket, AppBuffer, length);
//#endif
//            if (WithLanguage) TCPPutROMString(MySocket, (ROM BYTE*) "CONTENT-LANGUAGE: en\r\n");
//            TCPPutROMString(MySocket, (ROM BYTE*) "\r\n");
//            if (i > TCPIsPutReady(MySocket)) TCPFlush(MySocket);
//            TCPPutROMArray(MySocket, SERVICE1_XML_BODY, i);
//        }

        //TCPFlush(MySocket);

        TCPServerState = SM_CLOSING;

        break;


    case SM_CLOSING:
        // Close the socket connection.
        TCPClose(MySocket);

        TCPServerState = SM_HOME;
        break;
    }
}


#endif //#if defined(STACK_USE_UPNP)
