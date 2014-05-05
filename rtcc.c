/*********************************************************************
 *
 *      ~ OpenGoto ~
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
#include "GenericTypeDefs.h"

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
 * Function:        void RTCCInit()
 * PreCondition:    I2C initialized
 * Input:           Nono
 * Output:          None
 * Side Effects:    None
 * Overview:        Set RTCC VBATEN bits to activate battery backup
 *****************************************************************************/
void RTCCInit()
{
    RTCWKDAYbits rtcwkday;

    RTCCReadArray(RTCWKDAY, (BYTE *) &rtcwkday, 1);
    if (rtcwkday.VBATEN == 0)
        rtcwkday.VBATEN = 1;
    RTCCWriteArray(RTCWKDAY, (BYTE *) &rtcwkday, 1);
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