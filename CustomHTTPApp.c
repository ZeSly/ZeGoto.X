/*********************************************************************
 *
 *  Application to Demo HTTP2 Server
 *  Support for HTTP2 module in Microchip TCP/IP Stack
 *	 -Implements the application 
 *	 -Reference: RFC 1002
 *
 *********************************************************************
 * FileName:        CustomHTTPApp.c
 * Dependencies:    TCP/IP stack
 * Processor:       PIC18, PIC24F, PIC24H, dsPIC30F, dsPIC33F, PIC32
 * Compiler:        Microchip C32 v1.05 or higher
 *					Microchip C30 v3.12 or higher
 *					Microchip C18 v3.30 or higher
 *					HI-TECH PICC-18 PRO 9.63PL2 or higher
 * Company:         Microchip Technology, Inc.
 *
 * Software License Agreement
 *
 * Copyright (C) 2002-2010 Microchip Technology Inc.  All rights
 * reserved.
 *
 * Microchip licenses to you the right to use, modify, copy, and
 * distribute:
 * (i)  the Software when embedded on a Microchip microcontroller or
 *      digital signal controller product ("Device") which is
 *      integrated into Licensee's product; or
 * (ii) ONLY the Software driver source files ENC28J60.c, ENC28J60.h,
 *		ENCX24J600.c and ENCX24J600.h ported to a non-Microchip device
 *		used in conjunction with a Microchip ethernet controller for
 *		the sole purpose of interfacing with the ethernet controller.
 *
 * You should refer to the license agreement accompanying this
 * Software for additional information regarding your rights and
 * obligations.
 *
 * THE SOFTWARE AND DOCUMENTATION ARE PROVIDED "AS IS" WITHOUT
 * WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT
 * LIMITATION, ANY WARRANTY OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * MICROCHIP BE LIABLE FOR ANY INCIDENTAL, SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF
 * PROCUREMENT OF SUBSTITUTE GOODS, TECHNOLOGY OR SERVICES, ANY CLAIMS
 * BY THIRD PARTIES (INCLUDING BUT NOT LIMITED TO ANY DEFENSE
 * THEREOF), ANY CLAIMS FOR INDEMNITY OR CONTRIBUTION, OR OTHER
 * SIMILAR COSTS, WHETHER ASSERTED ON THE BASIS OF CONTRACT, TORT
 * (INCLUDING NEGLIGENCE), BREACH OF WARRANTY, OR OTHERWISE.
 *
 *
 * Author               Date    Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Elliott Wood     	6/18/07	Original
 ********************************************************************/

#include "telescope_movement_commands.h"

#define __CUSTOMHTTPAPP_C

#include "TCPIPConfig.h"

#if defined(STACK_USE_HTTP2_SERVER)

#include "TCPIP_Stack/TCPIP.h"
#include "main.h"		// Needed for SaveAppConfig() prototype
#include "USB/usb.h"

#include "utils.h"
#include "mount.h"
#include "inputs.h"
#include "ra_motor.h"
#include "dec_motor.h"

#include "get_telescope_information.h"
#include "rtcc.h"
#include <math.h>
#include "gps.h"
#include "mount.h"
#include "reticule.h"
#include <ctype.h>

/****************************************************************************
  Section:
        Function Prototypes and Memory Globalizers
 ***************************************************************************/
#if defined(HTTP_USE_POST)
#if defined(STACK_USE_HTTP_APP_RECONFIG)
extern APP_CONFIG AppConfig;
static HTTP_IO_RESULT HTTPPostConfig(void);
#endif
static HTTP_IO_RESULT HTTPPostMount(void);
#endif

// Sticky status message variable.
// This is used to indicated whether or not the previous POST operation was 
// successful.  The application uses these to store status messages when a 
// POST operation redirects.  This lets the application provide status messages
// after a redirect, when connection instance data has already been lost.
static BOOL lastFailure = FALSE;
static int lastResultMsg;

/****************************************************************************
  Section:
        GET Form Handlers
 ***************************************************************************/

/*****************************************************************************
  Function:
        HTTP_IO_RESULT HTTPExecuteGet(void)
	
  Internal:
        See documentation in the TCP/IP Stack API or HTTP2.h for details.
 ***************************************************************************/
HTTP_IO_RESULT HTTPExecuteGet(void)
{
    BYTE *ptr;
    BYTE filename[20];

    // Load the file name
    // Make sure BYTE filename[] above is large enough for your longest name
    MPFSGetFilename(curHTTP.file, filename, 20);

    // If its the forms.htm page
    if (!memcmppgm2ram(filename, "mount.htm", 9))
    {
        // Seek out each of the two LED strings, and if it exists set the LED states
        ptr = HTTPGetROMArg(curHTTP.data, (ROM BYTE *) "invertedra");
        if (ptr)
        {
            Mount.Config.RADefaultDirection = (*ptr == '1');
        }
        else
        {
            Mount.Config.RADefaultDirection = 0;
        }
        Mount.WestDirection = Mount.Config.RADefaultDirection;
        Mount.EastDirection = !Mount.Config.RADefaultDirection;
        RAChangeDirection();

        ptr = HTTPGetROMArg(curHTTP.data, (ROM BYTE *) "inverteddec");
        if (ptr)
        {
            Mount.Config.DecDefaultDirection = (*ptr == '1');
        }
        else
        {
            Mount.Config.DecDefaultDirection = 0;
        }
        Mount.NorthDirection = Mount.Config.DecDefaultDirection;
        Mount.SouthDirection = !Mount.Config.DecDefaultDirection;
        SaveMountConfig(&Mount.Config);
    }

        // If it's the LED updater file
    else if (!memcmppgm2ram(filename, "leds.cgi", 8))
    {
        // Determine which LED to toggle
        ptr = HTTPGetROMArg(curHTTP.data, (ROM BYTE *) "led");

        // Toggle the specified LED
        switch (*ptr)
        {
        case '1':
            if (!OC5CON1bits.OCM) ReticuleOn();
            else ReticuleOff();
            break;
        case '2':
            LED2_IO ^= 1;
            break;
        }
    }

        // If it's the BTN updater file
    else if (!memcmppgm2ram(filename, "btns.cgi", 8))
    {
        // Determine which btn is pressed or released
        ptr = HTTPGetROMArg(curHTTP.data, (ROM BYTE *) "btn");

        switch (*ptr)
        {
        case '3':
            if (*(ptr + 1) == '1')
            {
                PadState.PAD_S3 = 1;
                MoveWest();
            }
            else
            {
                PadState.PAD_S3 = 0;
                Halt();
            }
            break;
        case '4':
            if (*(ptr + 1) == '1')
            {
                PadState.PAD_S4 = 1;
                MoveEast();
            }
            else
            {
                PadState.PAD_S4 = 0;
                Halt();
            }
            break;
        case '5':
            if (*(ptr + 1) == '1')
            {
                PadState.PAD_S5 = 1;
                MoveNorth();
            }
            else
            {
                PadState.PAD_S5 = 0;
                Halt();
            }
            break;
        case '6':
            if (*(ptr + 1) == '1')
            {
                PadState.PAD_S6 = 1;
                MoveSouth();
            }
            else
            {
                PadState.PAD_S6 = 1;
                Halt();
            }
            break;
        }
    }

    return HTTP_IO_DONE;
}

/****************************************************************************
  Section:
        POST Form Handlers
 ***************************************************************************/
#if defined(HTTP_USE_POST)

/*****************************************************************************
  Function:
        HTTP_IO_RESULT HTTPExecutePost(void)
	
  Internal:
        See documentation in the TCP/IP Stack API or HTTP2.h for details.
 ***************************************************************************/
HTTP_IO_RESULT HTTPExecutePost(void)
{
    // Resolve which function to use and pass along
    BYTE filename[20];

    // Load the file name
    // Make sure BYTE filename[] above is large enough for your longest name
    MPFSGetFilename(curHTTP.file, filename, sizeof (filename));

#if defined(STACK_USE_HTTP_APP_RECONFIG)
    if (!memcmppgm2ram(filename, "protect/config.htm", 18))
        return HTTPPostConfig();
#endif
    if (!memcmppgm2ram(filename, "mount.htm", 9))
        return HTTPPostMount();

    return HTTP_IO_DONE;
}

#endif

/*****************************************************************************
  Function:
        static HTTP_IO_RESULT HTTPPostConfig(void)

  Summary:
        Processes the configuration form on config/index.htm

  Description:
        Accepts configuration parameters from the form, saves them to a
        temporary location in RAM, then eventually saves the data to EEPROM or
        external Flash.
	
        When complete, this function redirects to config/reboot.htm, which will
        display information on reconnecting to the board.

        This function creates a shadow copy of the AppConfig structure in
        RAM and then overwrites incoming data there as it arrives.  For each
        name/value pair, the name is first read to curHTTP.data[0:5].  Next, the
        value is read to newAppConfig.  Once all data has been read, the new
        AppConfig is saved back to EEPROM and the browser is redirected to
        reboot.htm.  That file includes an AJAX call to reboot.cgi, which
        performs the actual reboot of the machine.
	
        If an IP address cannot be parsed, too much data is POSTed, or any other
        parsing error occurs, the browser reloads config.htm and displays an error
        message at the top.

  Precondition:
        None

  Parameters:
        None

  Return Values:
        HTTP_IO_DONE - all parameters have been processed
        HTTP_IO_NEED_DATA - data needed by this function has not yet arrived
 ***************************************************************************/
#if defined(STACK_USE_HTTP_APP_RECONFIG)

static HTTP_IO_RESULT HTTPPostConfig(void)
{
    APP_CONFIG newAppConfig;
    BYTE *ptr;
    BYTE i;

    // Check to see if the browser is attempting to submit more data than we
    // can parse at once.  This function needs to receive all updated
    // parameters and validate them all before committing them to memory so that
    // orphaned configuration parameters do not get written (for example, if a
    // static IP address is given, but the subnet mask fails parsing, we
    // should not use the static IP address).  Everything needs to be processed
    // in a single transaction.  If this is impossible, fail and notify the user.
    // As a web devloper, if you add parameters to AppConfig and run into this
    // problem, you could fix this by to splitting your update web page into two
    // seperate web pages (causing two transactional writes).  Alternatively,
    // you could fix it by storing a static shadow copy of AppConfig someplace
    // in memory and using it instead of newAppConfig.  Lastly, you could
    // increase the TCP RX FIFO size for the HTTP server.  This will allow more
    // data to be POSTed by the web browser before hitting this limit.
    if (curHTTP.byteCount > TCPIsGetReady(sktHTTP) + TCPGetRxFIFOFree(sktHTTP))
        goto ConfigFailure;

    // Ensure that all data is waiting to be parsed.  If not, keep waiting for
    // all of it to arrive.
    if (TCPIsGetReady(sktHTTP) < curHTTP.byteCount)
        return HTTP_IO_NEED_DATA;


    // Use current config in non-volatile memory as defaults
#if defined(EEPROM_CS_TRIS) || defined(EEPROM_I2CCON)
    XEEReadArray(sizeof (NVM_VALIDATION_STRUCT), (BYTE*) & newAppConfig, sizeof (newAppConfig));
#elif defined(SPIFLASH_CS_TRIS)
    SPIFlashReadArray(sizeof (NVM_VALIDATION_STRUCT), (BYTE*) & newAppConfig, sizeof (newAppConfig));
#endif

    // Start out assuming that DHCP is disabled.  This is necessary since the
    // browser doesn't submit this field if it is unchecked (meaning zero).
    // However, if it is checked, this will be overridden since it will be
    // submitted.
    newAppConfig.Flags.bIsDHCPEnabled = 0;


    // Read all browser POST data
    while (curHTTP.byteCount)
    {
        // Read a form field name
        if (HTTPReadPostName(curHTTP.data, 6) != HTTP_READ_OK)
            goto ConfigFailure;

        // Read a form field value
        if (HTTPReadPostValue(curHTTP.data + 6, sizeof (curHTTP.data) - 6 - 2) != HTTP_READ_OK)
            goto ConfigFailure;

        // Parse the value that was read
        if (!strcmppgm2ram((char*) curHTTP.data, (ROM char*) "ip"))
        {// Read new static IP Address
            if (!StringToIPAddress(curHTTP.data + 6, &newAppConfig.MyIPAddr))
                goto ConfigFailure;

            newAppConfig.DefaultIPAddr.Val = newAppConfig.MyIPAddr.Val;
        }
        else if (!strcmppgm2ram((char*) curHTTP.data, (ROM char*) "gw"))
        {// Read new gateway address
            if (!StringToIPAddress(curHTTP.data + 6, &newAppConfig.MyGateway))
                goto ConfigFailure;
        }
        else if (!strcmppgm2ram((char*) curHTTP.data, (ROM char*) "sub"))
        {// Read new static subnet
            if (!StringToIPAddress(curHTTP.data + 6, &newAppConfig.MyMask))
                goto ConfigFailure;

            newAppConfig.DefaultMask.Val = newAppConfig.MyMask.Val;
        }
        else if (!strcmppgm2ram((char*) curHTTP.data, (ROM char*) "dns1"))
        {// Read new primary DNS server
            if (!StringToIPAddress(curHTTP.data + 6, &newAppConfig.PrimaryDNSServer))
                goto ConfigFailure;
        }
        else if (!strcmppgm2ram((char*) curHTTP.data, (ROM char*) "dns2"))
        {// Read new secondary DNS server
            if (!StringToIPAddress(curHTTP.data + 6, &newAppConfig.SecondaryDNSServer))
                goto ConfigFailure;
        }
        else if (!strcmppgm2ram((char*) curHTTP.data, (ROM char*) "mac"))
        {
            // Read new MAC address
            WORD w;
            BYTE i;

            ptr = curHTTP.data + 6;

            for (i = 0; i < 12u; i++)
            {// Read the MAC address

                // Skip non-hex bytes
                while (*ptr != 0x00u && !(*ptr >= '0' && *ptr <= '9') && !(*ptr >= 'A' && *ptr <= 'F') && !(*ptr >= 'a' && *ptr <= 'f'))
                    ptr++;

                // MAC string is over, so zeroize the rest
                if (*ptr == 0x00u)
                {
                    for (; i < 12u; i++)
                        curHTTP.data[i] = '0';
                    break;
                }

                // Save the MAC byte
                curHTTP.data[i] = *ptr++;
            }

            // Read MAC Address, one byte at a time
            for (i = 0; i < 6u; i++)
            {
                ((BYTE*) & w)[1] = curHTTP.data[i * 2];
                ((BYTE*) & w)[0] = curHTTP.data[i * 2 + 1];
                newAppConfig.MyMACAddr.v[i] = hexatob(*((WORD_VAL*) & w));
            }
        }
        else if (!strcmppgm2ram((char*) curHTTP.data, (ROM char*) "host"))
        {// Read new hostname
            FormatNetBIOSName(&curHTTP.data[6]);
            memcpy((void*) newAppConfig.NetBIOSName, (void*) curHTTP.data + 6, 16);
        }
        else if (!strcmppgm2ram((char*) curHTTP.data, (ROM char*) "dhcp"))
        {// Read new DHCP Enabled flag
            if (curHTTP.data[6] == '1')
                newAppConfig.Flags.bIsDHCPEnabled = 1;
        }
    }


    // All parsing complete!  Save new settings and force a reboot
    SaveAppConfig(&newAppConfig);

    // Set the board to reboot and display reconnecting information
    strcpypgm2ram((char*) curHTTP.data, "/protect/reboot.htm?");
    memcpy((void*) (curHTTP.data + 20), (void*) newAppConfig.NetBIOSName, 16);
    curHTTP.data[20 + 16] = 0x00; // Force null termination
    for (i = 20; i < 20u + 16u; i++)
    {
        if (curHTTP.data[i] == ' ')
            curHTTP.data[i] = 0x00;
    }
    curHTTP.httpStatus = HTTP_REDIRECT;

    return HTTP_IO_DONE;


ConfigFailure:
    lastFailure = TRUE;
    strcpypgm2ram((char*) curHTTP.data, "/protect/config.htm");
    curHTTP.httpStatus = HTTP_REDIRECT;

    return HTTP_IO_DONE;
}

#endif //(use_post)

/*****************************************************************************
  Function:
        static HTTP_IO_RESULT HTTPPostMount(void)

  Summary:
        Processes the configuration form mount.htm

  Description:
        Accepts configuration parameters from the form, saves them to a
        temporary location in RAM, then eventually saves the data to EEPROM or
        external Flash.

        When complete, this function redirects to config/reboot.htm, which will
        display information on reconnecting to the board.

        This function creates a shadow copy of the AppConfig structure in
        RAM and then overwrites incoming data there as it arrives.  For each
        name/value pair, the name is first read to curHTTP.data[0:5].  Next, the
        value is read to newAppConfig.  Once all data has been read, the new
        AppConfig is saved back to EEPROM and the browser is redirected to
        reboot.htm.  That file includes an AJAX call to reboot.cgi, which
        performs the actual reboot of the machine.

        If an IP address cannot be parsed, too much data is POSTed, or any other
        parsing error occurs, the browser reloads config.htm and displays an error
        message at the top.

  Precondition:
        None

  Parameters:
        None

  Return Values:
        HTTP_IO_DONE - all parameters have been processed
        HTTP_IO_NEED_DATA - data needed by this function has not yet arrived
 ***************************************************************************/
static HTTP_IO_RESULT HTTPPostMount(void)
{
    mountconfig_t newMountConfig;
    char *ptr;

    lastFailure = TRUE; // always true for displaying the result of the mount config
    lastResultMsg = 1; // default error message

    // Check to see if the browser is attempting to submit more data than we
    // can parse at once.  This function needs to receive all updated
    // parameters and validate them all before committing them to memory so that
    // orphaned configuration parameters do not get written (for example, if a
    // static IP address is given, but the subnet mask fails parsing, we
    // should not use the static IP address).  Everything needs to be processed
    // in a single transaction.  If this is impossible, fail and notify the user.
    // As a web devloper, if you add parameters to AppConfig and run into this
    // problem, you could fix this by to splitting your update web page into two
    // seperate web pages (causing two transactional writes).  Alternatively,
    // you could fix it by storing a static shadow copy of AppConfig someplace
    // in memory and using it instead of newAppConfig.  Lastly, you could
    // increase the TCP RX FIFO size for the HTTP server.  This will allow more
    // data to be POSTed by the web browser before hitting this limit.
    if (curHTTP.byteCount > TCPIsGetReady(sktHTTP) + TCPGetRxFIFOFree(sktHTTP))
        goto ConfigFailure;

    // Ensure that all data is waiting to be parsed.  If not, keep waiting for
    // all of it to arrive.
    if (TCPIsGetReady(sktHTTP) < curHTTP.byteCount)
        return HTTP_IO_NEED_DATA;

    // Use current config
    memcpy(&newMountConfig, &Mount.Config, sizeof (newMountConfig));

    // Read all browser POST data
    while (curHTTP.byteCount)
    {
        // Read a form field name
        if (HTTPReadPostName(curHTTP.data, 12) != HTTP_READ_OK)
            goto ConfigFailure;

        // Read a form field value
        if (HTTPReadPostValue(curHTTP.data + 12, sizeof (curHTTP.data) - 12 - 2) != HTTP_READ_OK)
            goto ConfigFailure;

        // Parse the value that was read
        if (!strcmppgm2ram((char*) curHTTP.data, (ROM char*) "nbmaxstep"))
        {
            newMountConfig.NbStepMax = strtoul((char*) curHTTP.data + 12, &ptr, 10);
            if (newMountConfig.NbStepMax == 0) goto ConfigFailure;
        }
        else if (!strcmppgm2ram((char*) curHTTP.data, (ROM char*) "period"))
        {
            newMountConfig.SideralPeriod = strtoul((char*) curHTTP.data + 12, &ptr, 10);
            if (newMountConfig.SideralPeriod == 0) goto ConfigFailure;
        }
        else if (!strcmppgm2ram((char*) curHTTP.data, (ROM char*) "maxrate"))
        {
            newMountConfig.MaxSpeed = strtoul((char*) curHTTP.data + 12, &ptr, 10);
            if (newMountConfig.MaxSpeed == 0) goto ConfigFailure;
        }
        else if (!strcmppgm2ram((char*) curHTTP.data, (ROM char*) "centerrate"))
        {
            newMountConfig.CenteringSpeed = strtoul((char*) curHTTP.data + 12, &ptr, 10);
            if (newMountConfig.CenteringSpeed == 0) goto ConfigFailure;
        }
        else if (!strcmppgm2ram((char*) curHTTP.data, (ROM char*) "guiderate"))
        {
            char *p = (char*) curHTTP.data + 12;
            if (!isdigit(*p)) goto ConfigFailure;
            newMountConfig.GuideSpeed = (*p++ -'0') * 10;
            if (*p++ == '.')
            {
                if (!isdigit(*p)) goto ConfigFailure;
                newMountConfig.GuideSpeed += (*p - '0');
            }
        }
    }

    if (!DecIsMotorStop() || !RAIsMotorStop())
    {
        lastResultMsg = 2;
        goto ConfigFailure;
    }

    // All parsing complete!  Save new settings and force a reboot
    memcpy(&Mount.Config, &newMountConfig, sizeof (Mount.Config));
    SaveMountConfig(&newMountConfig);

    RAStop();
    RAMotorInit();
    DecMotorInit();
    RAStart();

    strcpypgm2ram((char*) curHTTP.data, "/mount.htm");
    lastResultMsg = 0;
    curHTTP.httpStatus = HTTP_REDIRECT;

    return HTTP_IO_DONE;


ConfigFailure:
    strcpypgm2ram((char*) curHTTP.data, "/mount.htm");
    curHTTP.httpStatus = HTTP_REDIRECT;

    return HTTP_IO_DONE;
}

void HTTPPrint_result(void)
{
    if (lastResultMsg == 0)
        TCPPutROMString(sktHTTP, (ROM BYTE*) "ok");
    else
        TCPPutROMString(sktHTTP, (ROM BYTE*) "fail");
}

void HTTPPrint_result_text(void)
{
    switch (lastResultMsg)
    {
    case 0:
        TCPPutROMString(sktHTTP, (ROM BYTE*) "New mechanical mount setting saved.");
        break;
    case 1:
        TCPPutROMString(sktHTTP, (ROM BYTE*) "ERROR: A field was unparsable or too much data was POSTed. Try again.");
        break;
    case 2:
        TCPPutROMString(sktHTTP, (ROM BYTE*) "ERROR: Unable to change mechanical mount setting while moving. Wait for the motor to stop and try again");
        break;
    }
}

/****************************************************************************
  Section:
        Dynamic Variable Callback Functions
 ***************************************************************************/

/*****************************************************************************
  Function:
        void HTTPPrint_varname(void)
	
  Internal:
        See documentation in the TCP/IP Stack API or HTTP2.h for details.
 ***************************************************************************/

void HTTPPrint_builddate(void)
{
    curHTTP.callbackPos = 0x01;
    if (TCPIsPutReady(sktHTTP) < strlenpgm((ROM char*) __DATE__" "__TIME__))
        return;

    curHTTP.callbackPos = 0x00;
    TCPPutROMString(sktHTTP, (ROM void*) __DATE__" "__TIME__);
}

void HTTPPrint_version(void)
{
    TCPPutROMString(sktHTTP, (ROM void*) VERSION);
}

void HTTPPrint_tcpipversion(void)
{
    TCPPutROMString(sktHTTP, (ROM void*) TCPIP_STACK_VERSION);
}

void HTTPPrint_usbversion(void)
{
    char usb_version[8];

    sprintf(usb_version, "v%d.%d.%d", USB_MAJOR_VER, USB_MINOR_VER, USB_DOT_VER);
    TCPPutString(sktHTTP, (void*) usb_version);
}


ROM BYTE HTML_UP_ARROW[] = "up";
ROM BYTE HTML_DOWN_ARROW[] = "dn";

void HTTPPrint_btn(WORD num)
{
    // Determine which button
    switch (num)
    {
    case 1:
        num = PadState.PAD_S1;
        break;
    case 2:
        num = PadState.PAD_S2;
        break;
    case 3:
        num = PadState.PAD_S3;
        break;
    case 4:
        num = PadState.PAD_S4;
        break;
    case 5:
        num = PadState.PAD_S5;
        break;
    case 6:
        num = PadState.PAD_S6;
        break;
    case 7:
        num = PadState.PAD_SWITCH;
        break;
    default:
        num = 0;
    }

    // Print the output
    TCPPutROMString(sktHTTP, (num ? HTML_UP_ARROW : HTML_DOWN_ARROW));
    return;
}

void HTTPPrint_led(WORD num)
{
    // Determine which LED
    switch (num)
    {
    case 1:
        num = OC5CON1bits.OCM ? 1 : 0;
        break;
    case 2:
        num = LED2_IO;
        break;
    default:
        num = 0;
    }

    // Print the output
    TCPPut(sktHTTP, (num ? '1' : '0'));
    return;
}

void HTTPPrint_ledSelected(WORD num, WORD state)
{
    // Determine which LED to check
    switch (num)
    {
    case 1:
        num = OC5CON1bits.OCM ? 1 : 0;
        break;
    case 2:
        num = LED2_IO;
        break;
    default:
        num = 0;
    }

    // Print output if TRUE and ON or if FALSE and OFF
    if ((state && num) || (!state && !num))
        TCPPutROMString(sktHTTP, (ROM BYTE*) "SELECTED");
    return;
}

extern APP_CONFIG AppConfig;

void HTTPPrintIP(IP_ADDR ip)
{
    BYTE digits[4];
    BYTE i;

    for (i = 0; i < 4u; i++)
    {
        if (i)
            TCPPut(sktHTTP, '.');
        uitoa(ip.v[i], digits);
        TCPPutString(sktHTTP, digits);
    }
}

void HTTPPrint_config_hostname(void)
{
    TCPPutString(sktHTTP, AppConfig.NetBIOSName);
    return;
}

void HTTPPrint_config_dhcpchecked(void)
{
    if (AppConfig.Flags.bIsDHCPEnabled)
        TCPPutROMString(sktHTTP, (ROM BYTE*) "checked");
    return;
}

void HTTPPrint_config_ip(void)
{
    HTTPPrintIP(AppConfig.MyIPAddr);
    return;
}

void HTTPPrint_config_gw(void)
{
    HTTPPrintIP(AppConfig.MyGateway);
    return;
}

void HTTPPrint_config_subnet(void)
{
    HTTPPrintIP(AppConfig.MyMask);
    return;
}

void HTTPPrint_config_dns1(void)
{
    HTTPPrintIP(AppConfig.PrimaryDNSServer);
    return;
}

void HTTPPrint_config_dns2(void)
{
    HTTPPrintIP(AppConfig.SecondaryDNSServer);
    return;
}

void HTTPPrint_config_mac(void)
{
    BYTE i;

    if (TCPIsPutReady(sktHTTP) < 18u)
    {//need 17 bytes to write a MAC
        curHTTP.callbackPos = 0x01;
        return;
    }

    // Write each byte
    for (i = 0; i < 6u; i++)
    {
        if (i)
            TCPPut(sktHTTP, ':');
        TCPPut(sktHTTP, btohexa_high(AppConfig.MyMACAddr.v[i]));
        TCPPut(sktHTTP, btohexa_low(AppConfig.MyMACAddr.v[i]));
    }

    // Indicate that we're done
    curHTTP.callbackPos = 0x00;
    return;
}

void HTTPPrint_reboot(void)
{
    // This is not so much a print function, but causes the board to reboot
    // when the configuration is changed.  If called via an AJAX call, this
    // will gracefully reset the board and bring it back online immediately
    Reset();
}

void HTTPPrint_rebootaddr(void)
{// This is the expected address of the board upon rebooting
    TCPPutString(sktHTTP, curHTTP.data);
}

void HTTPPrint_status_fail(void)
{
    if (lastFailure)
        TCPPutROMString(sktHTTP, (ROM BYTE*) "block");
    else
        TCPPutROMString(sktHTTP, (ROM BYTE*) "none");
    lastFailure = FALSE;
}

void HTTPPrint_rightascension(void)
{
    char rightascension[16];

    GetRAString(RA.StepPosition, TRUE, rightascension);
    rightascension[2] = 'h';
    rightascension[5] = 'm';
    rightascension[8] = 's';
    rightascension[9] = '\0';
    TCPPutString(sktHTTP, (BYTE *) rightascension);
}

void HTTPPrint_declination(void)
{
    char declination[16];

    GetDecString(Dec.StepPosition, TRUE, declination);
    declination[3] = '°';
    declination[6] = '\'';
    declination[9] = '\'';
    declination[10] = '\'';
    declination[11] = '\0';
    TCPPutString(sktHTTP, (BYTE *) declination);
}

void HTTPPrint_wikiskycoord(void)
{
    char wikiskycoord[32];
    double ra, dec;

    ra = (double) RA.StepPosition * 24.0 / (double) Mount.Config.NbStepMax;
    dec = (double) Dec.StepPosition * 360.0 / (double) Mount.Config.NbStepMax;

    sprintf(wikiskycoord, "ra=%f&de=%f", ra, dec);
    TCPPutString(sktHTTP, (BYTE *) wikiskycoord);
}

void HTTPPrint_datetime(void)
{
    RTCCMapTimekeeping Timekeeping;
    char datetime[32];
    BYTE i = 4;

    RTCCGetTimekeeping(&Timekeeping);

    strcpy(datetime, "UTC ");

    // 0123456789012345678
    // dd/mm/yyyy hh:mn:ss
    datetime[i++] = Timekeeping.rtcdate.DATETEN + '0';
    datetime[i++] = Timekeeping.rtcdate.DATEONE + '0';
    datetime[i++] = '/';
    datetime[i++] = Timekeeping.rtcmth.MTHTEN + '0';
    datetime[i++] = Timekeeping.rtcmth.MTHONE + '0';
    datetime[i++] = '/';
    datetime[i++] = '2';
    datetime[i++] = '0';
    datetime[i++] = Timekeeping.rtcyear.YRTEN + '0';
    datetime[i++] = Timekeeping.rtcyear.YRONE + '0';
    datetime[i++] = ' ';
    if (Timekeeping.rtchour.B12_24)
    {
        // 12-hour format
        datetime[i++] = Timekeeping.rtchour.HRTEN0 + '0';
        datetime[i++] = Timekeeping.rtchour.HRONE + '0';
    }
    else
    {
        // 24-hour format
        datetime[i++] = Timekeeping.rtchour.HRTEN + '0';
        datetime[i++] = Timekeeping.rtchour.HRONE + '0';
    }
    datetime[i++] = ':';
    datetime[i++] = Timekeeping.rtcmin.MINTEN + '0';
    datetime[i++] = Timekeeping.rtcmin.MINONE + '0';
    datetime[i++] = ':';
    datetime[i++] = Timekeeping.rtcsec.SECTEN + '0';
    datetime[i++] = Timekeeping.rtcsec.SECONE + '0';
    if (Timekeeping.rtchour.B12_24)
    {
        datetime[i++] = ' ';
        datetime[i++] = Timekeeping.rtchour.AM_PM ? 'A' : 'P';
        datetime[i++] = 'M';
    }
    datetime[i++] = '\0';

    TCPPutString(sktHTTP, (BYTE *) datetime);
}

void HTTPPrint_azimuthcoord(void)
{
    char str[256];
    char *p;
    double Azimuth, Altitude;
    datetime_t datetime;
    double st;

    ComputeAzimuthalCoord(&Altitude, &Azimuth);

    p = str;
    p += sprintf(p, "Azimut :");
    p += Dec2DMS(Azimuth, p);
    p += sprintf(p, " Altitude :");
    p += Dec2DMS(Altitude, p);
    p += sprintf(p, "\n<br/><br/>Julian day: %f<br/>\n", JulianDay);
    GetLocalDateTime(&datetime);
    p += sprintf(p, "Local time: %02d/%02d/%04d %02d:%02d:%02d<br/>\n",
                 datetime.day, datetime.month, datetime.year,
                 datetime.hour, datetime.minute, datetime.second);
    st = ComputeSideralTime();
    p += sprintf(p, "Sideral time: ");
    p += Dec2HMS(st, p);
    p += sprintf(p, "<br/>\n");

    TCPPutString(sktHTTP, (BYTE *) str);
}

#include "gps.h"

void HTTPPrint_gpsdata(void)
{
    char str[256];
    char *p;

    TCPFlush(sktHTTP);
    p = str;
    p += sprintf(p, "Setting : %f %f %.1fm<br/>\n", Mount.Config.Latitude, Mount.Config.Longitude, Mount.Config.Elevation);
    if (GPS.Available)
    {
        if (GPS.Status == 'A')
        {
            p += sprintf(p, "Latitude: %s %c ", GPS.Latitude, GPS.NSIndicator);
            p += sprintf(p, "Longitude: %s %c <br/>\n", GPS.Longitude, GPS.EWIndicator);
            p += sprintf(p, "Altitude: %s <br/>\n", GPS.MSLAltitude);
        }
        else
        {
            p += sprintf(p, "Position not valid<br/>\n");
        }
        p += sprintf(p, "Position fix indicator: ");
        switch (GPS.PositionFixIndicator)
        {
        case '0':
            p += sprintf(p, "0, position fix unavailable<br/>\n");
            break;
        case '1':
            p += sprintf(p, "1, valid position fix, SPS mode<br/>\n");
            break;
        case '2':
            p += sprintf(p, "2, valid position fix, differential GPS mode<br/>\n");
            break;
        case '3':
            p += sprintf(p, "3, GPS PPS Mode, fix valid<br/>\n");
            break;
        case '4':
            p += sprintf(p, "4, Real Time Kinematic. System used in RTK mode with fixed integers<br/>\n");
            break;
        case '5':
            p += sprintf(p, "5, Float RTK. Satellite system used in RTK mode. Floating integers<br/>\n");
            break;
        case '6':
            p += sprintf(p, "6, Estimated(dead reckoning) Mode<br/>\n");
            break;
        case '7':
            p += sprintf(p, "7, Manual Input Mode<br/>\n");
            break;
        case '8':
            p += sprintf(p, "8, Simulator Mode<br/>\n");
            break;
        default:
            p += sprintf(p, "No GPS<br/>\n");
            break;
        }
        p += sprintf(p, "UTC %s %s<br/>", GPS.Date, GPS.UTCTime);
    }
    else
    {
        p += sprintf(p, "No GPS<br/>\n");
    }
    TCPPutString(sktHTTP, (BYTE *) str);
}

void HTTPPrint_gpssatellitesinview(void)
{
    char str[4];

    sprintf(str, "%d", GPS.SatellitesInView);
    TCPPutString(sktHTTP, (BYTE *) str);
}

void HTTPPrint_gpssatellitesused(void)
{
    char str[4];

    sprintf(str, "%d", GPS.SatellitesUsed);
    TCPPutString(sktHTTP, (BYTE *) str);
}

void HTTPPrint_svggpssignal()
{
    char str[256];
    char *p;
    int i;

    p = str;
    *p = '\0';
    for (i = 0; i < GPS.SatellitesInView; i++)
    {
        p += sprintf(p, "%s,%d,%d,%d,",
                     GPS.Satellites[i].Id,
                     GPS.Satellites[i].Elevation,
                     GPS.Satellites[i].Azimuth,
                     GPS.Satellites[i].SNR);
    }
    if (i > 0) *--p = '\0';
    TCPPutString(sktHTTP, (BYTE *) str);
}

void HTTPPrint_mountconfig_inverteddec(void)
{
    if (Mount.NorthDirection)
        TCPPutROMString(sktHTTP, (ROM BYTE*) "checked");
    return;
}

void HTTPPrint_mountconfig_invertedra(void)
{
    if (Mount.WestDirection)
        TCPPutROMString(sktHTTP, (ROM BYTE*) "checked");
    return;
}

void HTTPPrint_mountconfig_nbmaxstep(void)
{
    char str[16];

    sprintf(str, "%li", Mount.Config.NbStepMax);
    TCPPutString(sktHTTP, (BYTE *) str);
}

void HTTPPrint_mountconfig_sideralperiod(void)
{
    char str[16];

    sprintf(str, "%lu", Mount.Config.SideralPeriod);
    TCPPutString(sktHTTP, (BYTE *) str);
}

void HTTPPrint_mountconfig_maxrate(void)
{
    char str[16];

    sprintf(str, "%u", Mount.Config.MaxSpeed);
    TCPPutString(sktHTTP, (BYTE *) str);
}

void HTTPPrint_mountconfig_centeringrate(void)
{
    char str[16];

    sprintf(str, "%u", Mount.Config.CenteringSpeed);
    TCPPutString(sktHTTP, (BYTE *) str);
}

void HTTPPrint_mountconfig_guidingrate(void)
{
    char str[16];

    sprintf(str, "%.1f", (double) Mount.Config.GuideSpeed / 10.0);
    TCPPutString(sktHTTP, (BYTE *) str);
}

void HTTPPrint_sideofpier(void)
{
    switch (Mount.SideOfScope)
    {
    case PIER_WEST:
        TCPPutROMString(sktHTTP, (ROM BYTE*) "Scope is on <em>west</em> side of mount,");
        break;
    case PIER_EAST:
        TCPPutROMString(sktHTTP, (ROM BYTE*) "Scope is on <em>east</em> side of mount,");
        break;
    }

    switch (Mount.SideOfPier)
    {
    case PIER_EAST :
        TCPPutROMString(sktHTTP, (ROM BYTE*) " poll <i>east</i>");
        break;
    case PIER_WEST :
        TCPPutROMString(sktHTTP, (ROM BYTE*) " poll <i>west</i>");
        break;
    }
}

#endif
