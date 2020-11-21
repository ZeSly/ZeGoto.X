/*********************************************************************
 *
 *      ~ ZeGoto ~
 *
 *  Processing LX200 commands
 *
 *********************************************************************
 * FileName:        lx200_protocol.c
 * Processor:       PIC24FJ256GB106
 * Compiler:        Microchip XC16 v1.21 or higher
 *
 * Copyright © 2014 Sylvain Girard
 *
 * Author               Date        Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Sylvain Girard   	2 mai 2014 Creation
 ********************************************************************/

/* Device header file */
#include <xc.h>

#include <TCPIP_Stack/TCPIP.h>
#include <TCPIP_Stack/Helpers.h>

#include "GenericTypeDefs.h"
#include "HardwareProfile.h"
#include "main.h"
#include "Compiler.h"
#include <string.h>

#include "get_telescope_information.h"
#include "telescope_movement_commands.h"
#include "slew_rate_commands.h"
#include "telescope_set_commands.h"
#include "gps.h"
#include "home_position_commands.h"
#include "reticule.h"
#include "tracking_commands.h"
#include "mount.h"
#include "ra_motor.h"

char LX200String[32];
char LX200Response[64];

void GetTelescopeFirmwareDate()
{
    strcpy(LX200Response, __DATE__"#");
}

void GetTelescopeFirmwareNumber()
{
    strcpy(LX200Response, VERSION"#");
}

void GetTelescopeFirmwareTime()
{
    strcpy(LX200Response, __TIME__"#");
}

void GetTelescopeProductName()
{
    strcpy(LX200Response, "ZeGoto#");
}

void GetPowerVoltage()
{
    // POWER_SENSE is on AN10 
    double ADCValue;
    
    AD1PCFGL = 0xFBFF;  // AN10 as analog, all other pins are digital
    AD1CON1 = 0x0000;   // SAMP bit = 0 ends sampling and starts converting
    AD1CHS = 0x000A;    // Connect AN10 as S/H+ input
    
    AD1CSSL = 0;
    AD1CON3 = 0x0002; // Manual Sample, Tad = 3Tcy
    AD1CON2 = 0;
    AD1CON1bits.ADON = 1; // turn ADC ON

    AD1CON1bits.SAMP = 1; // start sampling...
    Delay10us(1); // Ensure the correct sampling time has elapsed
    // before starting conversion.
    AD1CON1bits.SAMP = 0; // start converting
    while (!AD1CON1bits.DONE)
    {
    }; // conversion done?
    ADCValue = ADC1BUF0; // yes then get ADC value
    ADCValue *= 3.3 * 9.5 + 0.5;   // Vref * voltage_divider + D1 voltage drop
    ADCValue /= 1023.0;
    
    sprintf(LX200Response, "%f#", ADCValue);
}

static APP_CONFIG newAppConfig;
static BOOL StartnewAppConfig = TRUE;

void GetIPString(IP_ADDR ip, char *Str_P)
{
    int i;

    for (i = 0; i < 4u; i++)
    {
        if (i)
        {
            *Str_P++ = '.';
        }
        uitoa(ip.v[i], (BYTE*) Str_P);
        Str_P += strlen(Str_P);
    }
    *Str_P++ = '#';
    *Str_P = '\0';
}

void GetIPConfig()
{
    if (strncmp(LX200String + 3, "ip", 2) == 0)
    {
        GetIPString(AppConfig.MyIPAddr, LX200Response);
    }
    else if (strncmp(LX200String + 3, "gw", 2) == 0)
    {
        GetIPString(AppConfig.MyGateway, LX200Response);
    }
    else if (strncmp(LX200String + 3, "sub", 3) == 0)
    {
        GetIPString(AppConfig.MyMask, LX200Response);
    }
    else if (strncmp(LX200String + 3, "dns1", 4) == 0)
    {
        GetIPString(AppConfig.PrimaryDNSServer, LX200Response);
    }
    else if (strncmp(LX200String + 3, "dns2", 4) == 0)
    {
        GetIPString(AppConfig.SecondaryDNSServer, LX200Response);
    }
    else if (strncmp(LX200String + 3, "dhcp", 4) == 0)
    {
        LX200Response[0] = AppConfig.Flags.bIsDHCPEnabled ? '1' : '0';
        LX200Response[1] = '#';
        LX200Response[2] = '\0';
    }
    else if (strncmp(LX200String + 3, "host", 4) == 0)
    {
        gethostname(LX200Response, 16);
        strcat(LX200Response, "#");
    }
}

void SetIPConfig()
{
    int i;

    if (StartnewAppConfig == TRUE)
    {
        // Use current config in non-volatile memory as defaults
#if defined(EEPROM_CS_TRIS) || defined(EEPROM_I2CCON)
        XEEReadArray(sizeof (NVM_VALIDATION_STRUCT), (BYTE*) & newAppConfig, sizeof (newAppConfig));
#elif defined(SPIFLASH_CS_TRIS)
        SPIFlashReadArray(sizeof (NVM_VALIDATION_STRUCT), (BYTE*) & newAppConfig, sizeof (newAppConfig));
#endif
        StartnewAppConfig = FALSE;
    }

    for (i = 3 ; LX200String[i] != '\0' ; i++)
    {
        if (LX200String[i] == '#')
        {
            LX200String[i] = '\0';
            break;
        }
    }

    if (strncmp(LX200String + 3, "ip", 2) == 0)
    {
        if (!StringToIPAddress((BYTE*) LX200String + 5, &newAppConfig.MyIPAddr))
            goto ConfigFailure;

        newAppConfig.DefaultIPAddr.Val = newAppConfig.MyIPAddr.Val;
    }
    else if (strncmp(LX200String + 3, "gw", 2) == 0)
    {
        if (!StringToIPAddress((BYTE*) LX200String + 5, &newAppConfig.MyGateway))
            goto ConfigFailure;
    }
    else if (strncmp(LX200String + 3, "sub", 3) == 0)
    {
        if (!StringToIPAddress((BYTE*) LX200String + 6, &newAppConfig.MyMask))
            goto ConfigFailure;
    }
    else if (strncmp(LX200String + 3, "dns1", 4) == 0)
    {
        if (!StringToIPAddress((BYTE*) LX200String + 7, &newAppConfig.PrimaryDNSServer))
            goto ConfigFailure;
    }
    else if (strncmp(LX200String + 3, "dns2", 4) == 0)
    {
        if (!StringToIPAddress((BYTE*) LX200String + 7, &newAppConfig.SecondaryDNSServer))
            goto ConfigFailure;
    }
    else if (strncmp(LX200String + 3, "dhcp", 4) == 0)
    {
        if (LX200String[7] == '1')
            newAppConfig.Flags.bIsDHCPEnabled = 1;
        else
            newAppConfig.Flags.bIsDHCPEnabled = 0;
    }
    else if (strncmp(LX200String + 3, "host", 4) == 0)
    {
        FormatNetBIOSName((BYTE*) LX200String + 7);
        memcpy((void*) newAppConfig.NetBIOSName, (void*) LX200String + 7, 16);
    }
    else if (strncmp(LX200String + 3, "apply", 5) == 0)
    {
        StartnewAppConfig = TRUE;
        SaveAppConfig(&newAppConfig);
        Reset();
    }

    LX200Response[0] = '1';
    LX200Response[1] = '\0';
    return;

ConfigFailure:
    StartnewAppConfig = TRUE;
    LX200Response[0] = '0';
    LX200Response[1] = '\0';
}

typedef struct
{
    char *Cmd;
    char Length;
    void (*f)();
} LX200Command;

void DumpSpeedList();

LX200Command LX200CommandTab[] ={
    { "GR", 2, GetTelescopeRA},
    { "Gr", 2, GetCurrentTargetRA},
    { "GD", 2, GetTelescopeDeclination},
    { "Gd", 2, GetCurrentTargetDeclination},
    { "GZ", 2, GetTelescopeAzimuth},
    { "GA", 2, GetTelescopeAltitude},
    { "GS", 2, GetSideralTime},
    { "GL", 2, GetLocalTime},
    { "GG", 2, GetUTCOffsetTime},
    { "GC", 2, GetCurrentDate},
    { "Gc", 2, GetCalendarFormat},

    { "Gg", 2, GetCurrentSiteLongitude},
    { "Gt", 2, GetCurrentSiteLatitude},
    { "Gu", 2, GetCurrentSiteAltitude},

    { "CM", 2, SyncWithCurrentTarget},

    { "GVD", 3, GetTelescopeFirmwareDate},
    { "GVN", 3, GetTelescopeFirmwareNumber},
    { "GVP", 3, GetTelescopeProductName},
    { "GVT", 3, GetTelescopeFirmwareTime},
    
    { "GPV", 3, GetPowerVoltage},

    { "Q", 1, Halt},
    { "Me", 2, MoveEast},
    { "Mn", 2, MoveNorth},
    { "Ms", 2, MoveSouth},
    { "Mw", 2, MoveWest},
    { "Mge", 3, GuideEast},
    { "Mgn", 3, GuideNorth},
    { "Mgs", 3, GuideSouth},
    { "Mgw", 3, GuideWest},
    { "MS", 2, SlewToTarget},

    { "RC", 2, SetCenteringRate},
    { "RG", 2, SetGuidingRate},
    { "RM", 2, SetFindRate},
    { "RS", 2, SetMaxRate},
    { "Rs", 2, SetSpecificRate},

    { "Sr", 2, SetTargetObjectRA},
    { "Sd", 2, SetTargetObjectDeclination},

    { "Sa", 2, SetTargetObjectAltitude},
    { "Sz", 2, SetTargetObjectAzimuth},

    { "SC", 2, SetDate},
    { "SL", 2, SetLocalTime},
    { "SG", 2, SetUTCOffsetTime},

    { "Sg", 2, SetCurrentSiteLongitude},
    { "St", 2, SetCurrentSiteLatitude},
    { "Su", 2, SetCurrentSiteAltitude},

    { "hS", 2, homeSetParkPosition},
    { "hP", 2, homeSlewToParkPosition},
    { "hW", 2, homeUnpark},
    { "GpH", 3, GetHomeData},

    { "PO", 2, homeUnpark}, // ASTRO-PHYSICS GTO compatibility
    { "pS", 2, GetSideOfPier}, // ASTRO-PHYSICS GTO compatibility
    { "ps", 2, SetSideOfPier},
    { "pF", 2, FlipSideOfPier},

    { "rG", 2, GetGuidingRate},
    { "rC", 2, GetCenteringRate},
    { "rS", 2, GetCurrentMaxSpeed},
    { "rM", 2, GetMaxSpeed},

    { "g+", 2, GPSon},
    { "g-", 2, GPSoff},
    { "gps", 2, GPSforward},

    { "B+", 2, IncreaseReticuleBrightness},
    { "B-", 2, DecreaseReticuleBrightness},

    { "ZGR", 3, GetStepRA},
    { "ZGD", 3, GetStepDeclination},
    { "ZGr", 3, GetStepTargetRA},
    { "ZGd", 3, GetStepTargetDeclination},

    { "U", 1, PrecisionToggle},
    { "P", 1, GetPrecision},
    { "V", 1, GetTelescopeFirmwareNumber},
    //    { "V", 1, AstroPhysicsVersion},

    { "TL", 2, SelectLunarTracking},
    { "TQ", 2, SelectSideralTracking},
    { "TS", 2, SelectSolarTracking},
    { "TH", 2, RAStop},
    
    
    { "GIP", 3, GetIPConfig},
    { "SIP", 3, SetIPConfig},

    { NULL, 0, NULL}
};

void LX200ProcessCommand(char *LX200Cmd_P)
{
    BYTE i;
    BOOL trouve = FALSE;

    for (i = 0; LX200CommandTab[i].Length != 0 && !trouve; i++)
    {
        if (strncmp(LX200Cmd_P, LX200CommandTab[i].Cmd, LX200CommandTab[i].Length) == 0)
        {
            GPS.Forward = 0;
            strncpy(LX200String, LX200Cmd_P, sizeof (LX200String));
            LX200CommandTab[i].f();
            trouve = TRUE;
        }
    }
}
