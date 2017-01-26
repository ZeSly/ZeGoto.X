/*********************************************************************
 *
 *      ~ ZeGoto ~
 *
 *  LX200 : R - Slew Rate Commands
 *
 *********************************************************************
 * FileName:        slew_rate_commands.c
 * Processor:       PIC24FJ256GB106
 * Compiler:        Microchip XC16 v1.21 or higher
 *
 * Copyright © 2014 Sylvain Girard
 *
 * Author               Date        Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Sylvain Girard       4 mai 2014 Creation
 ********************************************************************/

/* Device header file */
#include <xc.h>
#include <stdlib.h>
#include <stdio.h>

#include "mount.h"
#include "lx200_protocol.h"

void SetCenteringRate()
{
    uint16_t s;

    Mount.IsGuiding = FALSE;
    s = atoi(LX200String + 2);
    if (s != 0)
    {
        Mount.Config.CenteringSpeed = s;
    }
//    Mount.CurrentMaxSpeed = Mount.Config.CenteringSpeed;
}

void SetGuidingRate()
{
    char s;

    s = LX200String[2];
    if (s >= '1' && s <= '9')
    {
        Mount.Config.GuideSpeed = s - '0';
    }
    Mount.IsGuiding = TRUE;
}

void SetFindRate()
{
    Mount.IsGuiding = FALSE;
    Mount.CurrentMaxSpeed = Mount.Config.MaxSpeed / 2;
}

void SetMaxRate()
{
    Mount.IsGuiding = FALSE;
    Mount.CurrentMaxSpeed = Mount.Config.MaxSpeed;
}


void SetSpecificRate()
{
    Mount.IsGuiding = FALSE;
    Mount.CurrentMaxSpeed = atoi(LX200String + 2);
}

void GetCenteringRate()
{
    sprintf(LX200Response,"%i#", Mount.Config.CenteringSpeed);
}

void GetGuidingRate()
{
    LX200Response[0] = (char)Mount.Config.GuideSpeed + '0';
    LX200Response[1] = '#';
    LX200Response[2] = '\0';
}

void GetCurrentMaxSpeed()
{
    sprintf(LX200Response, "%i#", Mount.CurrentMaxSpeed);
}

void GetMaxSpeed()
{
    sprintf(LX200Response, "%i#", Mount.Config.MaxSpeed);
}