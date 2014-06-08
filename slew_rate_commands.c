/*********************************************************************
 *
 *      ~ OpenGoto ~
 *
 *  LX200 : R ? Slew Rate Commands
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
#include "mount.h"

void SetCenteringRate()
{
    Mount.IsGuiding = FALSE;
    Mount.CurrentMaxSpeed = Mount.Config.CenteringSpeed;
}

void SetGuidingRate()
{
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


