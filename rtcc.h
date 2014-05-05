/*********************************************************************
 *
 *      ~ OpenGoto ~
 *
 *  I2C access to the MCP79401 Real-Time Clock/Calendar with
 *  SRAM and Protected EEPROM MAC address
 *
 *********************************************************************
 * FileName:        rtcc.h
 * Processor:       PIC24FJ256GB106
 * Compiler:        Microchip XC16 v1.21 or higher
 *
 * Copyright © 2014 Sylvain Girard
 *
 * Author               Date        Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Sylvain Girard   	3 mai 2014 Creation
 ********************************************************************/

#ifndef RTCC_H
#define	RTCC_H

#include "GenericTypeDefs.h"
#include "HardwareProfile.h"

typedef union
{
    BYTE b;

    struct
    {
        unsigned SECONE : 4;
        unsigned SECTEN : 3;
        unsigned ST : 1;
    };
} RTCSECbits;

typedef union
{
    BYTE b;

    struct
    {
        unsigned MINONE : 4;
        unsigned MINTEN : 3;
    };
} RTCMINbits;

typedef union
{
    BYTE b;

    struct
    {
        unsigned HRONE : 4;
        unsigned HRTEN : 2;
        unsigned B12_24 : 1;
    };

    struct
    {
        unsigned HRONE : 4;
        unsigned HRTEN0 : 1;
        unsigned AM_PM : 1;
        unsigned B12_24 : 1;
    };
} RTCHOURbits;

typedef union
{
    BYTE b;

    struct
    {
        unsigned WKDAY : 3;
        unsigned VBATEN : 1;
        unsigned PWRFAIL : 1;
        unsigned OSCRUN : 1;
    };
} RTCWKDAYbits;

typedef union
{
    BYTE b;

    struct
    {
        unsigned DATEONE : 4;
        unsigned DATEEN : 2;
    };
} RTCDATEbits;

typedef union
{
    BYTE b;

    struct
    {
        unsigned MTHONE : 4;
        unsigned MTHEN0 : 1;
        unsigned LPYR : 1;
    };
} RTCMTHbits;

typedef union
{
    BYTE b;

    struct
    {
        unsigned YRONE : 4;
        unsigned YRTEN : 4;
    };
} RTCYEARbits;

typedef union
{
    BYTE b;

    struct
    {
        unsigned SQWFS : 2;
        unsigned CRSTRIM : 1;
        unsigned EXTOSC : 1;
        unsigned ALM0EN : 1;
        unsigned ALM1EN : 1;
        unsigned SQWEN : 1;
        unsigned OUT : 1;
    };
} CONTROLbits;

typedef union
{
    BYTE b;

    struct
    {
        unsigned TRIMVAL : 7;
        unsigned SIGN : 1;
    };
} OSCTRIMbits;

typedef struct
{
    RTCSECbits rtcsec;
    RTCMINbits rtcmin;
    RTCHOURbits rtchour;
    RTCWKDAYbits rtcwkday;
    RTCDATEbits rtcdate;
    RTCMTHbits rtcmth;
    RTCYEARbits rtcyear;
    CONTROLbits rtccontrol;
    OSCTRIMbits osctrim;
    //BYTE eeunlock;
} RTCCMapTimekeeping;

#define RTCSEC      0x00
#define RTCMIN      0x01
#define RTCHOUR     0x02
#define RTCWKDAY    0x03
#define RTCDATE     0x04
#define RTCMTH      0x05
#define RTCYEAR     0x06
#define CONTROL     0x07
#define OSCTRIM     0x08
#define EEUNLOCK    0x09

#define RTCC_RAM    0x20

void RTCCWriteArray(BYTE address, BYTE *val, WORD wLen);
BOOL RTCCReadArray(BYTE address, BYTE *buffer, WORD length);
void RTCCInit();
BOOL RTCCReadMacAddress(BYTE *MacAddress);

#endif	/* RTCC_H */
