/*********************************************************************
 *
 *      ~ ZeGoto ~
 *
 *  TODO add description
 *
 *********************************************************************
 * FileName:        gps.c
 * Processor:       PIC24FJ256GB106
 * Compiler:        Microchip XC16 v1.21 or higher
 *
 * Copyright © 2014 Sylvain Girard
 *
 * Author               Date        Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Sylvain Girard       17 mai 2014 Creation
 ********************************************************************/

/* Device header file */
#include <xc.h>
#include "HardwareProfile.h"
#include "TCPIPConfig.h"
#include "TCPIP_Stack/TCPIP.h"
#include "rtcc.h"

#include <string.h>
#include <stdlib.h>
#include <ctype.h>

// Defines which port the server will listen on
#define GPS_SERVER_PORT	9300

#define USE_AND_OR /* To enable AND_OR mask setting */
#include "uart.h"
#include "mount.h"
#include "gps.h"

#define GPS_BAUDRATE    9600ul

int frame_rcv = 0;
int frame_complete = -1;
char GPSRxData[2][GPS_BUFFERSIZE];

void GPSStart()
{
    unsigned long ubrg;

    memset(&GPS, 0, sizeof (GPS));

    ubrg = GetInstructionClock() / (16ul * GPS_BAUDRATE) - 1ul;

    IFS0bits.U1RXIF = 0;
    IFS0bits.U1TXIF = 0;

    IPC2bits.U1RXIP = 2; // priority
    IEC0bits.U1RXIE = 1; // enable Rx interrupt

    U1BRG = (unsigned int) ubrg;
    U1STA = UART_TX_ENABLE;
    U1MODE = UART_EN;
    while (U1STAbits.URXDA == 1) // Clear Buffer
    {
        char Temp;
        Temp = U1RXREG;
    }
    IFS0bits.U1RXIF = 0; // clear interrupt flag

    GPS.ON = 1;
    GPS.Available = 0;
    GPS.Forward = 0;
    //    frame_complete = 0;
    //    strcpy(GPSRxData[frame_complete], "$GPGSV,3,1,12,24,79,359,22,12,55,241,33,15,42,176,44,17,30,053,25*7E");
    //    GPSDecodeFrame();
    //
    //    frame_complete = 1;
    //    strcpy(GPSRxData[frame_complete], "$GPGSV,3,2,12,25,17,239,28,18,15,254,,14,15,317,,22,15,291,15*7B");
    //    GPSDecodeFrame();
    //
    //    frame_complete = 0;
    //    strcpy(GPSRxData[frame_complete], "$GPGSV,3,3,12,26,11,152,35,04,00,103,,33,33,208,38,42,42,666,88*4A");
    //    GPSDecodeFrame();
}

static volatile unsigned char start_ptr = 0;
static volatile unsigned char end_ptr = 0;
char buf_rx[GPS_BUFFERSIZE];

void __attribute__((interrupt, no_auto_psv)) _U1RXInterrupt(void)
{
    static int receiving_frame = 0;
    static int receiving_end = 0;
    static int i;
    char c;

    U1RX_Clear_Intr_Status_Bit;
    while (!U1STAbits.URXDA)
        ;
    c = U1RXREG & 0xFF;

    buf_rx[end_ptr] = c;
    end_ptr++;
    if (end_ptr == sizeof (buf_rx))
        end_ptr = 0;

    if (c == '$')
    {
        receiving_frame = 1;
        i = 0;
    }
    if (receiving_frame)
    {
        GPSRxData[frame_rcv][i++] = c;
        if (c == '*')
        {
            receiving_end = 2;
            receiving_frame = 0;
        }
    }
    else if (receiving_end)
    {
        GPSRxData[frame_rcv][i++] = c;
        receiving_end--;
        if (receiving_end == 0)
        {
            GPSRxData[frame_rcv][i] = '\0';
            frame_complete = frame_rcv;
            frame_rcv = frame_complete ? 0 : 1;
        }
    }
}

gps_t GPS;

/******************************************************************************
 * Function:        int DMSToDec(double *dec, char *str)
 * PreCondition:    None
 * Input:           double *dec : pointer to the decimal number destination
 *                  char *str : string source
 * Output:          int : 1 valid, 0 invalid
 * Side Effects:    None
 * Overview:        Convert a degreee, minutes, second string from a string to
 *                  decimal
 *                  If the convertion fail, *dec is not modified
 *****************************************************************************/
int DMSToDec(double *dec, char *str)
{
    double degrees;
    double minutes;
    double seconds;
    double sign = 1.0;
    int r = 0;


    if (*str == '-')
    {
        sign = -1.0;
        str++;
    }
    else if (*str == '+')
    {
        str++;
    }


    degrees = (double) (*str++ -'0');
    while (isdigit(*str))
    {
        degrees *= 10.0;
        degrees += (double) (*str++ -'0');

    }
    if (degrees > 0.0 && degrees < 360.0)
    {
        str++; // skip the seperator
        minutes = (double) (*str++ -'0') * 10.0;
        minutes += (double) (*str++ -'0');

        seconds = 0.0;

        if (*str == '.') // decimal seperator from GPS
        {
            str++; // skip the seperator
            minutes += (double) (*str++ -'0') * 0.1;
            minutes += (double) (*str++ -'0') * 0.01;
            minutes += (double) (*str++ -'0') * 0.001;
            minutes += (double) (*str++ -'0') * 0.0001;
        }
        else if (*str != '#') // end of a LX200 command
        {
            str++; // skip the seperator
            seconds = (double) (*str++ -'0') * 10.0;
            seconds += (double) (*str++ -'0');
            //            LX200Precise = TRUE;
        }

        *dec = degrees + minutes / 60.0 + seconds / 3600.0;
        *dec *= sign;
        r = 1;
    }

    return r;
}

void GPSDecodeUTCTime(char *f)
{
    int i = 0;

    if (*f)
    {
        GPS.UTCTime[i++] = *f++;
        GPS.UTCTime[i++] = *f++;
        GPS.UTCTime[i++] = ':';
        GPS.UTCTime[i++] = *f++;
        GPS.UTCTime[i++] = *f++;
        GPS.UTCTime[i++] = ':';
        strcpy(GPS.UTCTime + i, f);
    }
}

void GPSDecodeLatitude(char *f)
{
    int i = 0;

    if (*f)
    {
        GPS.Latitude[i++] = *f++;
        GPS.Latitude[i++] = *f++;
        GPS.Latitude[i++] = '°';
        strcpy(GPS.Latitude + i, f);
    }
}

void GPSDecodeLongitude(char *f)
{
    int i = 0;

    if (*f)
    {
        GPS.Longitude[i++] = *f++;
        GPS.Longitude[i++] = *f++;
        GPS.Longitude[i++] = *f++;
        GPS.Longitude[i++] = '°';
        strcpy(GPS.Longitude + i, f);
    }
}

void GPSDecodeDate(char *f)
{
    int i = 0;

    if (*f)
    {
        GPS.Date[i++] = *f++;
        GPS.Date[i++] = *f++;
        GPS.Date[i++] = '/';
        GPS.Date[i++] = *f++;
        GPS.Date[i++] = *f++;
        GPS.Date[i++] = '/';
        GPS.Date[i++] = *f++;
        GPS.Date[i++] = *f++;
        GPS.Date[i] = '\0';
    }
}

/******************************************************************************
 * Function:        void GPSRead(char *buffer)
 * PreCondition:    UART1 initialized and running for GPS data reception
 * Input:           None
 * Output:          None
 * Overview:        Read available data in GPS reception buffer
 *****************************************************************************/
void GPSDecodeFrame()
{
    char frame[GPS_BUFFERSIZE];
    char *fields[32];
    char checksum, chk;
    int i, j;

    if (frame_complete == 0 || frame_complete == 1)
    {
        strncpy(frame, GPSRxData[frame_complete], GPS_BUFFERSIZE);
        i = 0;
        j = 0;
        chk = 0;
        fields[j++] = frame;
        while (i < GPS_BUFFERSIZE && j < 32)
        {
            chk += frame[i];
            if (frame[i] == ',')
            {
                frame[i] = '\0';
                fields[j++] = &frame[i + 1];
            }
            else if (frame[i] == '*')
            {
                char *e;
                frame[i] = '\0';
                checksum = (char) strtol(&frame[i + 1], &e, 16);
            }
            i++;
        }

        i = 1;
        if (strcmp("$GPGGA", fields[0]) == 0)
        {
            GPS.Available = 1;
            GPSDecodeUTCTime(fields[i++]);
            GPSDecodeLatitude(fields[i++]);
            GPS.NSIndicator = fields[i++][0];
            GPSDecodeLongitude(fields[i++]);
            GPS.EWIndicator = fields[i++][0];
            GPS.PositionFixIndicator = fields[i++][0];
            GPS.SatellitesUsed = atoi(fields[i++]);
            i++;
            strncpy(GPS.MSLAltitude, fields[i], sizeof (GPS.MSLAltitude));
        }

        else if (strcmp("$GPGLL", fields[0]) == 0)
        {
            GPS.Available = 1;
            GPSDecodeLatitude(fields[i++]);
            GPS.NSIndicator = fields[i++][0];
            GPSDecodeLongitude(fields[i++]);
            GPS.EWIndicator = fields[i++][0];
            GPSDecodeUTCTime(fields[i++]);
            GPS.Status = fields[i][0];
        }

        else if (strcmp("$GPRMC", fields[0]) == 0)
        {
            GPS.Available = 1;
            GPSDecodeUTCTime(fields[i++]);
            GPS.Status = fields[i++][0];
            GPSDecodeLatitude(fields[i++]);
            GPS.NSIndicator = fields[i++][0];
            GPSDecodeLongitude(fields[i++]);
            GPS.EWIndicator = fields[i++][0];
            GPSDecodeDate(fields[i + 2]);

        }
        else if (strcmp("$GPGSV", fields[0]) == 0)
        {
            i++; // skip Total number of messages
            int MessageNumber = fields[i++][0] - '0';
            int o = (MessageNumber - 1) * 4;
            int j;

            //o = 0;
            GPS.SatellitesInView = atoi(fields[i++]);
            for (j = 0; j < 4 && j + o < GPS.SatellitesInView; j++)
            {
                strcpy(GPS.Satellites[j + o].Id, fields[i++]);
                GPS.Satellites[j + o].Elevation = atoi(fields[i++]);
                GPS.Satellites[j + o].Azimuth = atoi(fields[i++]);
                GPS.Satellites[j + o].SNR = atoi(fields[i++]);
            }
            GPS.Available = 1;
        }

        if (GPS.ON && GPS.PositionFixIndicator > '0')
        {
            double Longitude = Mount.Config.Longitude;
            double Latitude = Mount.Config.Latitude;
            double Elevation = Mount.Config.Elevation;

            if (GPS.Latitude[0])
            {
                DMSToDec(&Latitude, GPS.Latitude);
                if (GPS.NSIndicator == 'S')
                {
                    Latitude = -Latitude;
                }
            }
            if (GPS.Longitude[0])
            {
                DMSToDec(&Longitude, GPS.Longitude);
                if (GPS.EWIndicator == 'E')
                {
                    Longitude = -Longitude;
                }
            }
            if (GPS.MSLAltitude[0])
            {
                int z;

                Elevation = 0;
                for (z = 0; GPS.MSLAltitude[z] && GPS.MSLAltitude[z] != '.'; z++)
                {
                    Elevation *= 10;
                    Elevation += GPS.MSLAltitude[z] - '0';
                }
                if (GPS.MSLAltitude[z] == '.')
                {
                    double f = 10;
                    for (z += 1; GPS.MSLAltitude[z]; z++)
                    {
                        Elevation += (GPS.MSLAltitude[z] - '0') / 10;
                        f *= 10;
                    }
                }

            }

            Mount.Config.Longitude = Longitude;
            Mount.Config.Latitude = Latitude;
            Mount.Config.Elevation = Elevation;

            if (GPS.UTCTime[0] && GPS.Date[0])
            {
                datetime_t utctime, systemdate;
                int p;

                p = 0;
                utctime.day = (GPS.Date[p++] - '0') * 10;
                utctime.day += (GPS.Date[p++] - '0');
                p++;
                utctime.month = (GPS.Date[p++] - '0') * 10;
                utctime.month += (GPS.Date[p++] - '0');
                p++;
                utctime.year = (GPS.Date[p++] - '0') * 10;
                utctime.year += (GPS.Date[p++] - '0');
                utctime.year += 2000;

                p = 0;
                utctime.hour = (GPS.UTCTime[p++] - '0') * 10;
                utctime.hour += (GPS.UTCTime[p++] - '0');
                p++;
                utctime.minute = (GPS.UTCTime[p++] - '0') * 10;
                utctime.minute += (GPS.UTCTime[p++] - '0');
                p++;
                utctime.second = (GPS.UTCTime[p++] - '0') * 10;
                utctime.second += (GPS.UTCTime[p++] - '0');

                GetUTCDateTime(&systemdate);
                if (memcmp(&utctime, &systemdate, sizeof(utctime)) != 0)
                {
                    SetUTCDateTime(&utctime);
                }
            }

        }
    }

    frame_complete = -1;
}

void GPSon()
{
    GPS.ON = 1;
}

void GPSoff()
{
    GPS.ON = 0;
    GPS.Available = 0;
    GPS.Forward = 0;
}

void GPSforward()
{
    GPS.Forward = 1;
}

char *GPSGetFrame()
{
    if (frame_complete == 0 || frame_complete == 1)
    {
        return GPSRxData[frame_complete];
    }
    
    return NULL;
}

#if defined(USE_GENERIC_TCP_SERVER_GPS)

/******************************************************************************
 * Function:        void GPSTCPServer(void)
 * PreCondition:    UART1 initialized and running for GPS data reception
 * Input:           None
 * Output:          None
 * Overview:        Read available data in GPS reception buffer
 *****************************************************************************/
void GPSTCPServer(void)
{
    unsigned char AppBuffer[64];
    int n, m, wMaxPut;
    static TCP_SOCKET MySocket;

    m = 0;

    static enum _TCPServerState
    {
        SM_HOME = 0,
        SM_LISTENING,
        SM_CLOSING,
    } TCPServerState = SM_HOME;

    switch (TCPServerState)
    {
    case SM_HOME:
        // Allocate a socket for this server to listen and accept connections on
        MySocket = TCPOpen(0, TCP_OPEN_SERVER, GPS_SERVER_PORT, TCP_PURPOSE_GENERIC_TCP_SERVER);
        if (MySocket == INVALID_SOCKET)
            return;

        TCPServerState = SM_LISTENING;
        break;

    case SM_LISTENING:
        // See if anyone is connected to us
        if (!TCPIsConnected(MySocket))
            return;

        // Figure out how many bytes have been received and how many we can transmit.
        wMaxPut = TCPIsPutReady(MySocket); // Get TCP TX FIFO free space

        // Get data in the GPS buffer
        for (n = 0; n < wMaxPut && start_ptr != end_ptr; n++)
        {
            AppBuffer[n] = buf_rx[start_ptr++];
            if (start_ptr == sizeof (buf_rx)) start_ptr = 0;
        }

        // Transfer the data out of our local processing buffer and into the TCP TX FIFO.
        TCPPutArray(MySocket, AppBuffer, n);

        // No need to perform any flush.  TCP data in TX FIFO will automatically transmit itself after it accumulates for a while.  If you want to decrease latency (at the expense of wasting network bandwidth on TCP overhead), perform and explicit flush via the TCPFlush() API.

        break;

    case SM_CLOSING:
        // Close the socket connection.
        TCPClose(MySocket);

        TCPServerState = SM_HOME;
        break;
    }
}

#elif defined(STACK_USE_BERKELEY_API)

#define PORTNUM 9300
#define MAX_CLIENT (3) // Maximum number of simultanous connections accepted by the server.

/*********************************************************************
 * Function:        void BerkeleyTCPServerGPS(void)
 *
 * PreCondition:    Stack is initialized
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        None
 *
 * Note:            None
 ********************************************************************/
void BerkeleyTCPServerGPS(void)
{
    static SOCKET bsdServerSocket;
    static SOCKET ClientSock[MAX_CLIENT];
    struct sockaddr_in addr;
    struct sockaddr_in addRemote;
    int addrlen = sizeof (struct sockaddr_in);
    char TCP_In_Buffer[15];
    char TCP_Out_Buffer[15];
    int length;
    int i, n;

    static enum
    {
        BSD_INIT = 0,
        BSD_CREATE_SOCKET,
        BSD_BIND,
        BSD_LISTEN,
        BSD_OPERATION
    } BSDServerState = BSD_INIT;

    switch (BSDServerState)
    {
    case BSD_INIT:
        // Initialize all client socket handles so that we don't process
        // them in the BSD_OPERATION state
        for (i = 0; i < MAX_CLIENT; i++)
            ClientSock[i] = INVALID_SOCKET;

        BSDServerState = BSD_CREATE_SOCKET;
        // No break needed

    case BSD_CREATE_SOCKET:
        // Create a socket for this server to listen and accept connections on
        bsdServerSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (bsdServerSocket == INVALID_SOCKET)
            return;

        BSDServerState = BSD_BIND;
        // No break needed

    case BSD_BIND:
        // Bind socket to a local port
        addr.sin_port = PORTNUM;
        addr.sin_addr.S_un.S_addr = IP_ADDR_ANY;
        if (bind(bsdServerSocket, (struct sockaddr*) &addr, addrlen) == SOCKET_ERROR)
            return;

        BSDServerState = BSD_LISTEN;
        // No break needed

    case BSD_LISTEN:
        if (listen(bsdServerSocket, MAX_CLIENT) == 0)
            BSDServerState = BSD_OPERATION;

        // No break.  If listen() returns SOCKET_ERROR it could be because
        // MAX_CLIENT is set to too large of a backlog than can be handled
        // by the underlying TCP socket count (TCP_PURPOSE_BERKELEY_SERVER
        // type sockets in TCPIPConfig.h).  However, in this case, it is
        // possible that some of the backlog is still handleable, in which
        // case we should try to accept() connections anyway and proceed
        // with normal operation.

    case BSD_OPERATION:
        for (i = 0; i < MAX_CLIENT; i++)
        {
            // Accept any pending connection requests, assuming we have a place to store the socket descriptor
            if (ClientSock[i] == INVALID_SOCKET)
                ClientSock[i] = accept(bsdServerSocket, (struct sockaddr*) &addRemote, &addrlen);

            // If this socket is not connected then no need to process anything
            if (ClientSock[i] == INVALID_SOCKET)
                continue;

            for (n = 0; n < sizeof (TCP_In_Buffer) && start_ptr != end_ptr; n++)
            {
                TCP_In_Buffer[n] = buf_rx[start_ptr++];
                if (start_ptr == sizeof (buf_rx)) start_ptr = 0;
            }
            send(ClientSock[i], TCP_In_Buffer, n, 0);

            length = recv(ClientSock[i], TCP_Out_Buffer, sizeof (TCP_Out_Buffer), 0);

            if (length > 0)
            {
                if (TCP_Out_Buffer[0] == 27) // Escape received
                {
                    closesocket(ClientSock[i]);
                    ClientSock[i] = INVALID_SOCKET;
                }
            }
        }
        break;

    default:
        return;
    }
    return;
}

#endif
