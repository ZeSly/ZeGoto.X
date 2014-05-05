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
#include "ra_motor.h"

void SetCenteringRate()
{
    CurrentMaxSpeed = CenteringSpeed;
}

void SetGuidingRate()
{
}

void SetFindRate()
{
    CurrentMaxSpeed = MaxSpeed / 2;
}

void SetMaxRate()
{
    CurrentMaxSpeed = MaxSpeed;
}


