/*********************************************************************
 *
 *      ~ OpenGoto ~
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
#include "GenericTypeDefs.h"
#include "main.h"
#include "Compiler.h"
#include <string.h>

void GetTelescopeFirmwareDate()
{
    strcpy(USB_In_Buffer, __DATE__"#");
}

void GetTelescopeFirmwareNumber()
{
    strcpy(USB_In_Buffer, VERSION"#");
}

void GetTelescopeFirmwareTime()
{
    strcpy(USB_In_Buffer, __TIME__"#");
}

void GetTelescopeProductName()
{
    strcpy(USB_In_Buffer, "OpenGoto#");
}

typedef struct
{
    char *Cmd;
    char Length;
    void (*f)();
} LX200Command;

LX200Command LX200CommandTab[] =
{
//    { "GR", 2, GetTelescopeRA},
//    { "Gr", 2, GetCurrentTargetRA},
//    { "GD", 2, GetTelescopeDeclination},
//    { "Gd", 2, GetCurrentTargetDeclination},
//    { "GZ", 2, GetTelescopeAzimuth},
//    { "GA", 2, GetTelescopeAltitude},
//    { "GS", 2, GetSiderealTime},
//
//    { "U", 1, PrecisionToggle},
//    { "P", 1, GetPrecision},
//
//    { "Sr", 2, SetTargetObjectRA},
//    { "Sd", 2, SetTargetObjectDeclination},
//
//    { "CM", 2, SyncWithCurrentTarget},

    { "GVD", 3, GetTelescopeFirmwareDate},
    { "GVN", 3, GetTelescopeFirmwareNumber},
    { "GVP", 3, GetTelescopeProductName},
    { "GVT", 3, GetTelescopeFirmwareTime},

//    { "Q", 1, Halt},
//    { "Me", 2, MoveEast},
//    { "Mn", 2, MoveNorth},
//    { "Ms", 2, MoveSouth},
//    { "Mw", 2, MoveWest},
//    { "MS", 2, SlewToTarget},
//
//    { "RC", 2, SetCenteringRate},
//    { "RG", 2, SetGuidingRate},
//    { "RM", 2, SetFindRate},
//    { "RS", 2, SetMaxRate},

    { NULL, 0, NULL}
};

char LX200String[16];

void LX200ProcessCommand(char *LX200Cmd_P)
{
    BYTE i;
    BOOL trouve = FALSE;

    for (i = 0; LX200CommandTab[i].Length != 0 && !trouve; i++)
    {
        if (strncmp(LX200Cmd_P, LX200CommandTab[i].Cmd, LX200CommandTab[i].Length) == 0)
        {
            strncpy(LX200String, LX200Cmd_P, sizeof (LX200String));
            //delay(100);
            LX200CommandTab[i].f();
            trouve = TRUE;
        }
    }
}
