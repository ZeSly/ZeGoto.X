/*********************************************************************
 *
 *      ~ OpenGoto ~
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
#include "TCPIP Stack/TCPIP.h"

#include <string.h>
#include <stdlib.h>

#undef atof
extern double atof(const char *);

// Defines which port the server will listen on
#define GPS_SERVER_PORT	9300

#define USE_AND_OR /* To enable AND_OR mask setting */
#include "uart.h"
#include "gps.h"

#define GPS_BAUDRATE    9600ul
#define GPS_BUFFERSIZE  128

void GPSStart()
{
    unsigned long ubrg;

    memset(&GPS, 0, sizeof(GPS));

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
}

char GPSRxData[2][GPS_BUFFERSIZE];
int frame_rcv = 0;
int frame_complete = -1;

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
    if (end_ptr == sizeof(buf_rx))
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
        GPS.Latitute[i++] = *f++;
        GPS.Latitute[i++] = *f++;
        GPS.Latitute[i++] = '°';
        strcpy(GPS.Latitute + i, f);
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
    char *fields[16];
    char checksum, chk;
    int i, j;

    if (frame_complete == 0 || frame_complete == 1)
    {
        strncpy(frame, GPSRxData[frame_complete], GPS_BUFFERSIZE);
        i = 0;
        j = 0;
        chk = 0;
        fields[j++] = frame;
        while (i < GPS_BUFFERSIZE && j < 16)
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
            GPSDecodeUTCTime(fields[i++]);
            GPSDecodeLatitude(fields[i++]);
            GPS.NSIndicator = fields[i++][0];
            GPSDecodeLongitude(fields[i++]);
            GPS.EWIndicator = fields[i++][0];
            GPS.PositionFixIndicator = fields[i++][0];
            GPS.SatellitesUsed = atoi(fields[i++]);
            i++;
//            GPS.MSLAltitude = atof(fields[i++]);
            strncpy(GPS.MSLAltitude,fields[i], sizeof (GPS.MSLAltitude));
        }

        else if (strcmp("$GPGLL", fields[0]) == 0)
        {
            GPSDecodeLatitude(fields[i++]);
            GPS.NSIndicator = fields[i++][0];
            GPSDecodeLongitude(fields[i++]);
            GPS.EWIndicator = fields[i++][0];
            GPSDecodeUTCTime(fields[i++]);
            GPS.Status = fields[i][0];
        }

        else if (strcmp("$GPRMC", fields[0]) == 0)
        {
            GPSDecodeUTCTime(fields[i++]);
            GPS.Status = fields[i++][0];
            GPSDecodeLatitude(fields[i++]);
            GPS.NSIndicator = fields[i++][0];
            GPSDecodeLongitude(fields[i++]);
            GPS.EWIndicator = fields[i++][0];
            GPSDecodeDate(fields[i + 2]);

        }

        frame_complete = -1;
    }
}

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
