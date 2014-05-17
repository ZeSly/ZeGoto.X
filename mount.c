/*********************************************************************
 *
 *      ~ OpenGoto ~
 *
 *  Mount settings
 *
 *********************************************************************
 * FileName:        mount.c
 * Processor:       PIC24FJ256GB106
 * Compiler:        Microchip XC16 v1.21 or higher
 *
 * Copyright © 2014 Sylvain Girard
 *
 * Author               Date        Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Sylvain Girard       16 mai 2014 Creation
 ********************************************************************/

/* Device header file */
#include <xc.h>

#include "mount.h"
#include "HardwareProfile.h"

mount_t Mount;

void MountInit()
{
    /* Mecanic setting */
    Mount.Config.NbStepMax = 8640000UL;
    Mount.Config.SideralPeriod = 159563UL;
    Mount.SideralHalfPeriod = Mount.Config.SideralPeriod  / 2;
    Mount.Config.MaxSpeed = 120;
    Mount.Config.CenteringSpeed = 10;

    /* Acceleration/decelation settings */
    Mount.Config.AccelTime = 4; // seconds
    Mount.Config.DecelTime = 1; // seconds

    Mount.CurrentMaxSpeed = Mount.Config.MaxSpeed;
    Mount.AccelPeriod = GetPeripheralClock() / Mount.Config.MaxSpeed * Mount.Config.AccelTime;
    Mount.DecelPeriod = GetPeripheralClock() / Mount.Config.MaxSpeed * Mount.Config.DecelTime;

    /* Directions */
    Mount.WestDirection = 0;
    Mount.EastDirection = 1;
    Mount.NorthDirection = 0;
    Mount.SouthDirection = 1;

}
