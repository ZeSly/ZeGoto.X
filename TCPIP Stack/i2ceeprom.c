#define __I2CEEPROM_C

#include "GenericTypeDefs.h"
#include "Compiler.h"
#include "HardwareProfile.h"
#include "XEEPROM.h"

#include <i2c.h>

#define EEPROM_CONTROL  0xA0
#define FSCL    100000

#define READ	(0x01)
#define WRITE	(0x00)

#define I2CStart()      EEPROM_I2CCONbits.SEN = 1
#define I2CRestart()    EEPROM_I2CCONbits.RSEN = 1
#define I2CIdle()   while(EEPROM_I2CCONbits.SEN || EEPROM_I2CCONbits.RSEN || \
                        EEPROM_I2CCONbits.PEN || EEPROM_I2CCONbits.RCEN || \
                        EEPROM_I2CCONbits.ACKEN || EEPROM_I2CSTATbits.TRSTAT)
#define I2CStop()       EEPROM_I2CCONbits.PEN = 1

unsigned char I2CGet(void)
{
    while (EEPROM_I2CCON & 0x1F)
        ;
    EEPROM_I2CCONbits.RCEN = 1;
    while (EEPROM_I2CCONbits.RCEN)
        ;
    EEPROM_I2CSTATbits.I2COV = 0;
    return EEPROM_I2CRCV;
}

char I2CPut(unsigned char data_out)
{
    EEPROM_I2CTRN = data_out;

    if(EEPROM_I2CSTATbits.IWCOL)        /* If write collision occurs,return -1 */
        return -1;

    while (EEPROM_I2CSTATbits.TRSTAT)
        ;

    return 0;
}

/*********************************************************************
 * Function:        void XEEInit(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Initialize I2C module to communicate to serial
 *                  EEPROM.
 *
 * Note:            Code sets I2c clock to 100kHz with Fcy = 16MHz
 ********************************************************************/
void XEEInit(void)
{
    EEPROM_I2CCON = 0x8000;
    EEPROM_I2CBRG = 37;
}

/*********************************************************************
* Function:        XEE_RESULT XEEBeginWrite(XEE_ADDR address)
*
* PreCondition:    XEEInit() is already called.
*
* Input:           address     - address to be set
*
* Output:          XEE_SUCCESS if successful
*                  other value if failed.
*
* Side Effects:    None
*
* Overview:        Modifies internal address counter of EEPROM.
*
* Note:            This function does not release the I2C bus.
*                  User must close XEEClose() after this function
*                  is called.
********************************************************************/
DWORD WriteAddress;
BYTE WriteCount;

XEE_RESULT XEEBeginWrite(DWORD address)
{
    do
    {
        I2CIdle();
        I2CStart();
        while (EEPROM_I2CCONbits.SEN)
            ;

        // Send "control" byte with device address to slave
        if (I2CPut(EEPROM_CONTROL | WRITE))
            return !XEE_SUCCESS;
    }
    while(EEPROM_I2CSTATbits.ACKSTAT);

    // Send address bytes
    if(I2CPut(((WORD_VAL*)&address)->v[1]))
        return !XEE_SUCCESS;

    if(I2CPut(((WORD_VAL*)&address)->v[0]))
        return !XEE_SUCCESS;

    WriteAddress = address;
    WriteCount = 32;

    return XEE_SUCCESS;
}

XEE_RESULT XEEWrite(BYTE val)
{
    if (I2CPut(val))
        return !XEE_SUCCESS;

    return XEE_SUCCESS;
}

XEE_RESULT XEEEndWrite(void)
{
    I2CStop();
    while (EEPROM_I2CCONbits.PEN)
        ;

    return XEE_SUCCESS;
}

/*****************************************************************************
  Function:
    XEE_RESULT XEEWriteArray(BYTE *val, WORD wLen)

  Summary:
    Writes an array of bytes to the EEPROM part.

  Description:
    This function writes an array of bytes to the EEPROM at the address
    specified when XEEBeginWrite() was called.  Page boundary crossing is
    handled internally.

  Precondition:
    XEEInit() was called once and XEEBeginWrite() was called.

  Parameters:
    vData - The array to write to the next memory location
    wLen - The length of the data to be written

  Returns:
    None

  Remarks:
    The internal write cache is flushed at completion, so it is unnecessary
    to call XEEEndWrite() after calling this function.  However, if you do
    so, no harm will be done.
  ***************************************************************************/
void XEEWriteArray(BYTE *val, WORD wLen)
{
    while(wLen--)
    {
        XEEWrite(*val++);
        WriteCount--;
        //if (WriteCount == 0)
        {
            XEEEndWrite();
            WriteAddress += 1;
            XEEBeginWrite(WriteAddress);
        }
    }

    XEEEndWrite();
}

/*********************************************************************
 * Function:        XEE_RESULT XEEBeginRead(DWORD address)
 *
 * PreCondition:    None
 *
 * Input:           address - Address at which read is to be performed.
 *
 * Output:          XEE_SUCCESS
 *
 * Side Effects:    None
 *
 * Overview:        Sets internal address counter to given address.
 *
 * Note:            None
 ********************************************************************/
XEE_RESULT XEEBeginRead(DWORD address)
{
    if (XEEBeginWrite(address) != XEE_SUCCESS)
        return !XEE_SUCCESS;

    I2CIdle();
    I2CRestart();
    while(EEPROM_I2CCONbits.RSEN );

    // Send "control" byte with device address to slave
    if (I2CPut(EEPROM_CONTROL | READ))
        return !XEE_SUCCESS;

    I2CIdle();
    return XEE_SUCCESS;
}

/*BYTE XEERead(void)
{
    BYTE b = I2CGet();

    EEPROM_I2CCONbits.ACKDT = 0;
    EEPROM_I2CCONbits.ACKEN = 1;
    while (EEPROM_I2CCONbits.ACKEN)
        ;

    return b;
}

XEE_RESULT XEEEndRead(void)
{
    I2CGet();

    EEPROM_I2CCONbits.ACKDT = 1;
    EEPROM_I2CCONbits.ACKEN = 1;
    while (EEPROM_I2CCONbits.ACKEN)
        ;

    return XEE_SUCCESS;
}*/

/*********************************************************************
 * Function:        XEE_RESULT XEEReadArray(DWORD address,
 *                                          BYTE *buffer,
 *                                          WORD length)
 *
 * PreCondition:    XEEInit() is already called.
 *
 * Input:           address     - Address from where array is to be read
 *                  buffer      - Caller supplied buffer to hold the data
 *                  length      - Number of bytes to read.
 *
 * Output:          XEE_SUCCESS
 *
 * Side Effects:    None
 *
 * Overview:        Reads desired number of bytes in sequential mode.
 *                  This function performs all necessary steps
 *                  and releases the bus when finished.
 *
 * Note:            None
 ********************************************************************/
XEE_RESULT XEEReadArray(DWORD address,
                        BYTE *buffer,
                        WORD length)
{
    XEE_RESULT r;

    r = XEEBeginRead(address++);
    if (r != XEE_SUCCESS)
        return r;

    while(length)                    /* Receive the number of bytes specified by length */
    {
        *buffer = I2CGet();             /* save byte received */
        buffer++;
        length--;
        if(length == 0)              /* If last char, generate NACK sequence */
        {
            EEPROM_I2CCONbits.ACKDT = 1;
            EEPROM_I2CCONbits.ACKEN = 1;
        }
       else                         /* For other chars,generate ACK sequence */
        {
            EEPROM_I2CCONbits.ACKDT = 0;
            EEPROM_I2CCONbits.ACKEN = 1;
        }
        while(EEPROM_I2CCONbits.ACKEN == 1)
            ;    /* Wait till ACK/NACK sequence is over */
    }

    I2CStop();
    
    return r;
}
