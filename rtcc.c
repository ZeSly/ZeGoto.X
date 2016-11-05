/*********************************************************************
 *
 *      ~ ZeGoto ~
 *
 *  I2C access to the MCP79401 Real-Time Clock/Calendar with
 *  SRAM and Protected EEPROM MAC address
 *
 *********************************************************************
 * FileName:        rtcc.c
 * Processor:       PIC24FJ256GB106
 * Compiler:        Microchip XC16 v1.21 or higher
 *
 * Copyright © 2014 Sylvain Girard
 *
 * Author               Date        Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Sylvain Girard   	3 mai 2014 Creation
 ********************************************************************/

/* Device header file */
#include <xc.h>

#include "mount.h"
#include "GenericTypeDefs.h"
#include <math.h>

typedef BOOL XEE_RESULT;
#define XEE_SUCCESS FALSE

#include "rtcc.h"

#define MAC_CONTROL     0xAE
#define RTCC_CONTROL    0xDE

/* taken from in i2ceeprom.c */
#define READ	(0x01)
#define WRITE	(0x00)

#define I2CStart()      EEPROM_I2CCONbits.SEN = 1
#define I2CRestart()    EEPROM_I2CCONbits.RSEN = 1
#define I2CIdle()   while(EEPROM_I2CCONbits.SEN || EEPROM_I2CCONbits.RSEN || \
                        EEPROM_I2CCONbits.PEN || EEPROM_I2CCONbits.RCEN || \
                        EEPROM_I2CCONbits.ACKEN || EEPROM_I2CSTATbits.TRSTAT)
#define I2CStop()       EEPROM_I2CCONbits.PEN = 1

/* Implemented in i2ceeprom.c */
unsigned char I2CGet(void);
char I2CPut(unsigned char data_out);
XEE_RESULT XEEWrite(BYTE val);
XEE_RESULT XEEEndWrite(void);

/******************************************************************************
 * Function:        XEE_RESULT RTCCBeginWrite(BYTE address)
 * PreCondition:    I2C initialized
 * Input:           Nono
 * Output:          None
 * Side Effects:    None
 * Overview:        send first address to write to the RTCC
 *****************************************************************************/
XEE_RESULT RTCCBeginWrite(BYTE address)
{
    do
    {
        I2CIdle();
        I2CStart();
        while (EEPROM_I2CCONbits.SEN)
            ;

        // Send "control" byte with device address to slave
        if (I2CPut(RTCC_CONTROL | WRITE))
            return !XEE_SUCCESS;
    }
    while (EEPROM_I2CSTATbits.ACKSTAT);

    // Send address bytes
    if (I2CPut(address))
        return !XEE_SUCCESS;

    return XEE_SUCCESS;
}

/******************************************************************************
 * Function:        void RTCCWriteArray(BYTE address, BYTE *val, WORD wLen)
 * PreCondition:    I2C initialized
 * Input:           Nono
 * Output:          None
 * Side Effects:    None
 * Overview:        Write an array of byte in the RTCC
 *****************************************************************************/
void RTCCWriteArray(BYTE address, BYTE *val, WORD wLen)
{
    RTCCBeginWrite(address++);
    while (wLen--)
    {
        XEEWrite(*val++);
    }
    XEEEndWrite();
}

/******************************************************************************
 * Function:        XEE_RESULT RTCCBeginRead(BYTE address)
 * PreCondition:    I2C initialized
 * Input:           Nono
 * Output:          None
 * Side Effects:    None
 * Overview:        send first address to read to the RTCC
 *****************************************************************************/
XEE_RESULT RTCCBeginRead(BYTE i2c_control, BYTE address)
{
    if (RTCCBeginWrite(address) != XEE_SUCCESS)
        return !XEE_SUCCESS;

    I2CIdle();
    I2CRestart();
    while (EEPROM_I2CCONbits.RSEN);

    // Send "control" byte with device address to slave
    if (I2CPut(i2c_control | READ))
        return !XEE_SUCCESS;

    I2CIdle();
    return XEE_SUCCESS;
}

/******************************************************************************
 * Function:        BOOL RTCCReadArray(BYTE address, BYTE *buffer, WORD length)
 * PreCondition:    I2C initialized
 * Input:           Nono
 * Output:          None
 * Side Effects:    None
 * Overview:        Read an array of byte in the RTCC
 *****************************************************************************/
BOOL RTCCReadArray(BYTE address, BYTE *buffer, WORD length)
{
    XEE_RESULT r;

    r = RTCCBeginRead(RTCC_CONTROL, address++);
    if (r != XEE_SUCCESS)
        return FALSE;

    while (length) /* Receive the number of bytes specified by length */
    {
        *buffer = I2CGet(); /* save byte received */
        buffer++;
        length--;
        if (length == 0) /* If last char, generate NACK sequence */
        {
            EEPROM_I2CCONbits.ACKDT = 1;
            EEPROM_I2CCONbits.ACKEN = 1;
        }
        else /* For other chars,generate ACK sequence */
        {
            EEPROM_I2CCONbits.ACKDT = 0;
            EEPROM_I2CCONbits.ACKEN = 1;
        }
        while (EEPROM_I2CCONbits.ACKEN == 1)
            ; /* Wait till ACK/NACK sequence is over */
    }

    I2CStop();

    return TRUE;
}

/******************************************************************************
 * Function:        XEE_RESULT RTCCBeginWrite(BYTE address)
 * PreCondition:    I2C initialized
 * Input:           Nono
 * Output:          None
 * Side Effects:    None
 * Overview:        send first address to write to the RTCC
 *****************************************************************************/
BOOL RTCCReadMacAddress(BYTE *MacAddress)
{
    BYTE address = 0xF2;
    BYTE length;
    XEE_RESULT r;

    r = RTCCBeginRead(MAC_CONTROL, address++);
    if (r != XEE_SUCCESS)
        return FALSE;

    length = 6;
    while (length) /* Receive the number of bytes specified by length */
    {
        *MacAddress = I2CGet(); /* save byte received */
        MacAddress++;
        length--;
        if (length == 0) /* If last char, generate NACK sequence */
        {
            EEPROM_I2CCONbits.ACKDT = 1;
            EEPROM_I2CCONbits.ACKEN = 1;
        }
        else /* For other chars,generate ACK sequence */
        {
            EEPROM_I2CCONbits.ACKDT = 0;
            EEPROM_I2CCONbits.ACKEN = 1;
        }
        while (EEPROM_I2CCONbits.ACKEN == 1)
            ; /* Wait till ACK/NACK sequence is over */
    }

    I2CStop();

    return TRUE;

}

void RTCCInit()
{
    RTCWKDAYbits rtcwkday;
    RTCSECbits rtcsec;

    //if (RTCCReadArray(RTCSEC, &rtcsec.b, 1) == TRUE)
    {
        rtcsec.b = 0;
        rtcsec.ST = 1;

        RTCCWriteArray(RTCSEC, &rtcsec.b, 1);
    }

    RTCCReadArray(RTCWKDAY, (BYTE *) & rtcwkday, 1);
    if (rtcwkday.VBATEN == 0)
        rtcwkday.VBATEN = 1;
    RTCCWriteArray(RTCWKDAY, (BYTE *) & rtcwkday, 1);
}

/******************************************************************************
 * Function:        BOOL RTCCGetTimekeeping(RTCCMapTimekeeping *timekeeping)
 * PreCondition:    I2C initialized
 * Input:           pointer to a RTCCMapTimekeeping structure
 * Output:          None
 * Side Effects:    None
 * Overview:        Get the timekeeping register from the RTCC
 *****************************************************************************/
BOOL RTCCGetTimekeeping(RTCCMapTimekeeping *timekeeping)
{
    BYTE rtccmap[8];

    if (RTCCReadArray(0, rtccmap, sizeof (rtccmap)) == TRUE)
    {
        timekeeping->rtcsec.b = rtccmap[0];
        timekeeping->rtcmin.b = rtccmap[1];
        timekeeping->rtchour.b = rtccmap[2];
        timekeeping->rtcwkday.b = rtccmap[3];
        timekeeping->rtcdate.b = rtccmap[4];
        timekeeping->rtcmth.b = rtccmap[5];
        timekeeping->rtcyear.b = rtccmap[6];
        return TRUE;
    }

    return FALSE;
}

/******************************************************************************
 * Function:        BOOL RTCCSetTimekeeping(RTCCMapTimekeeping *timekeeping)
 * PreCondition:    I2C initialized
 * Input:           pointer to a RTCCMapTimekeeping structure
 * Output:          None
 * Side Effects:    None
 * Overview:        Set the timekeeping register in the RTCC and activate the
 *                  oscillator
 *****************************************************************************/
BOOL RTCCSetTimekeeping(RTCCMapTimekeeping *timekeeping)
{
    BYTE rtccmap[7];

    // clering ST bit before writing new date
    RTCSECbits rtcsec;
    if (RTCCReadArray(RTCSEC, &rtcsec.b, 1) == TRUE)
    {
        rtcsec.ST = 0;

        RTCCWriteArray(RTCSEC, &rtcsec.b, 1);

        // waiting for OSCRUN bit to clear
        RTCWKDAYbits rtcwkday;
        do
        {
            RTCCReadArray(RTCWKDAY, &rtcwkday.b, 1);
        }
        while (rtcwkday.OSCRUN);

        // writing new time
        timekeeping->rtcsec.ST = 0;
        rtccmap[0] = timekeeping->rtcsec.b;
        rtccmap[1] = timekeeping->rtcmin.b;
        rtccmap[2] = timekeeping->rtchour.b;
        timekeeping->rtcwkday.OSCRUN = 0;
        timekeeping->rtcwkday.PWRFAIL = 0;
        timekeeping->rtcwkday.VBATEN = 1;
        rtccmap[3] = timekeeping->rtcwkday.b;
        rtccmap[4] = timekeeping->rtcdate.b;
        rtccmap[5] = timekeeping->rtcmth.b;
        rtccmap[6] = timekeeping->rtcyear.b;
        RTCCWriteArray(0, rtccmap, sizeof (rtccmap));

        // setting ST bit after writing new date
        timekeeping->rtcsec.ST = 1;
        RTCCWriteArray(RTCSEC, &timekeeping->rtcsec.b, 1);
        return TRUE;
    }
    return FALSE;
}

double DateToJulianDay(datetime_t *datetime)
{
    double year = datetime->year;
    double month = datetime->month;
    if (month < 3)
    {
        year -= 1;
        month += 12;
    }

    double c = floor(year / 100);
    double b = 2 - c + floor(c / 4);
    double t = (double) datetime->hour / 24.0 + (double) datetime->minute / 1440.0 + (double) datetime->second / 86400.0;
    double jj = floor(365.25 * (year + 4716)) + floor(30.6001 * (month + 1)) + (double) datetime->day + t + b - 1524.5;
    return jj;
}

void JulianDayToDate(double jj, datetime_t *datetime)
{
    double z = floor(jj + 0.5);
    double alp = floor((z - 1867216.25) / 36525.25);
    double a = z + 1 + alp - floor(alp / 4);
    double b = a + 1524;
    double c = floor((b - 122.1) / 365.25);
    double d = floor(365.25 * c);
    double e = floor((b - d) / 30.6001);
    double f = (jj + 0.5) - z;

    double jdec = b - d - floor(30.6001 * e) + f;

    datetime->day = floor(jdec);
    datetime->month = e < 13.5 ? e - 1 : e - 13;
    datetime->year = datetime->month >= 2 ? c - 4716 : c - 4715;
    datetime->hour = floor(f * 24);
    double mindec = (f * 24 - datetime->hour) * 60;
    datetime->minute = floor(mindec);
    datetime->second = floor((mindec - datetime->minute) * 60);
}

double JulianDay;

BOOL GetUTCDateTime(datetime_t *datetime)
{
    BOOL ret;
    RTCCMapTimekeeping tk;

    ret = RTCCGetTimekeeping(&tk);
    if (ret == TRUE)
    {
        datetime->year = tk.rtcyear.YRTEN * 10 + tk.rtcyear.YRONE + 2000;
        datetime->month = tk.rtcmth.MTHTEN * 10 + tk.rtcmth.MTHONE;
        datetime->day = tk.rtcdate.DATETEN * 10 + tk.rtcdate.DATEONE;
        datetime->hour = tk.rtchour.HRTEN * 10 + tk.rtchour.HRONE;
        datetime->minute = tk.rtcmin.MINTEN * 10 + tk.rtcmin.MINONE;
        datetime->second = tk.rtcsec.SECTEN * 10 + tk.rtcsec.SECONE;
    }
    return ret;
}

BOOL GetLocalDateTime(datetime_t *datetime)
{
    BOOL ret;
    RTCCMapTimekeeping tk;

    ret = RTCCGetTimekeeping(&tk);
    if (ret == TRUE)
    {
        datetime->year = tk.rtcyear.YRTEN * 10 + tk.rtcyear.YRONE + 2000;
        datetime->month = tk.rtcmth.MTHTEN * 10 + tk.rtcmth.MTHONE;
        datetime->day = tk.rtcdate.DATETEN * 10 + tk.rtcdate.DATEONE;
        datetime->hour = tk.rtchour.HRTEN * 10 + tk.rtchour.HRONE;
        datetime->minute = tk.rtcmin.MINTEN * 10 + tk.rtcmin.MINONE;
        datetime->second = tk.rtcsec.SECTEN * 10 + tk.rtcsec.SECONE;

        JulianDay = DateToJulianDay(datetime);
        JulianDay += Mount.Config.UTCOffset / 24.0;
        JulianDayToDate(JulianDay, datetime);
    }
    return ret;
}

BOOL SetUTCDateTime(datetime_t *datetime)
{
    RTCCMapTimekeeping tk;

    tk.rtcyear.YRTEN = (datetime->year - 2000) / 10;
    tk.rtcyear.YRONE = datetime->year % 10;

    tk.rtcmth.MTHTEN = datetime->month / 10;
    tk.rtcmth.MTHONE = datetime->month % 10;

    tk.rtcdate.DATETEN = datetime->day / 10;
    tk.rtcdate.DATEONE = datetime->day % 10;

    tk.rtchour.B12_24 = 0; // The RTCC wil always be set in 24h format
    tk.rtchour.HRTEN = datetime->hour / 10;
    tk.rtchour.HRONE = datetime->hour % 10;

    tk.rtcmin.MINTEN = datetime->minute / 10;
    tk.rtcmin.MINONE = datetime->minute % 10;

    tk.rtcsec.SECTEN = datetime->second / 10;
    tk.rtcsec.SECONE = datetime->second % 10;

    return RTCCSetTimekeeping(&tk);
}

BOOL SetLocalDateTime(datetime_t *datetime)
{
    JulianDay = DateToJulianDay(datetime);
    JulianDay -= Mount.Config.UTCOffset / 24.0;
    JulianDayToDate(JulianDay, datetime);

    return SetUTCDateTime(datetime);
}
