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

#include "get_telescope_information.h"
#include "telescope_movement_commands.h"
#include "slew_rate_commands.h"
#include "telescope_set_commands.h"
#include "gps.h"
#include "reticule.h"

char LX200String[16];
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
    strcpy(LX200Response, "OpenGoto#");
}

typedef struct
{
    char *Cmd;
    char Length;
    void (*f)();
} LX200Command;

void DumpSpeedList();

LX200Command LX200CommandTab[] =
{
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
    { "Ge", 2, GetCurrentSiteAltitude},

    { "U", 1, PrecisionToggle},
    { "P", 1, GetPrecision},

    { "CM", 2, SyncWithCurrentTarget},

    { "GVD", 3, GetTelescopeFirmwareDate},
    { "GVN", 3, GetTelescopeFirmwareNumber},
    { "GVP", 3, GetTelescopeProductName},
    { "GVT", 3, GetTelescopeFirmwareTime},

    { "Q", 1, Halt},
    { "Me", 2, MoveEast},
    { "Mn", 2, MoveNorth},
    { "Ms", 2, MoveSouth},
    { "Mw", 2, MoveWest},
    { "MS", 2, SlewToTarget},

    { "RC", 2, SetCenteringRate},
    { "RG", 2, SetGuidingRate},
    { "RM", 2, SetFindRate},
    { "RS", 2, SetMaxRate},

    { "Sr", 2, SetTargetObjectRA},
    { "Sd", 2, SetTargetObjectDeclination},

    { "SC", 2, SetDate},
    { "SL", 2, SetLocalTime},
    { "SG", 2, SetUTCOffsetTime},

    { "Sg", 2, SetCurrentSiteLongitude},
    { "St", 2, SetCurrentSiteLatitude},
    { "Se", 2, SetCurrentSiteAltitude},

    { "g+", 2, GPSon},
    { "g-", 2, GPSoff},

    { "B+", 2, IncreaseReticuleBrightness},
    { "B-", 2, DecreaseReticuleBrightness},

    { "ZGR", 3, GetStepRA},
    { "ZGD", 3, GetStepDeclination},
    { "ZGr", 3, GetStepTargetRA},
    { "ZGd", 3, GetStepTargetDeclination},

    { "ZSr", 3, SetStepTargetRA},
    { "ZSd", 3, SetStepTargetDeclination},

//    { "ZGs", 3, DumpSpeedList },

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
            strncpy(LX200String, LX200Cmd_P, sizeof (LX200String));
            LX200CommandTab[i].f();
            trouve = TRUE;
        }
    }
}
