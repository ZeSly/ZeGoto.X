#ifndef _UPNP_H_
#define _UPNP_H_


//----- DEFINES -----
#define SSDP_PORT   (1900)
#define UPNP_PORT   (8080)

//----- PROTOTYPES -----
void SSDPAnnounce(void);
void SSDPDiscoveryTask(void);
void UPNPServer(void);

#endif //_UPNP_H_
