/*********************************************************************
 *
 *      ~ ZeGoto ~
 *
 *  LX200 : T ? Tracking Commands
 *
 *********************************************************************
 * FileName:        tracking_commands.c
 * Processor:       PIC24FJ256GB106
 * Compiler:        Microchip XC16 v1.21 or higher
 *
 * Copyright © 2014 Sylvain Girard
 *
 * Author               Date        Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Sylvain Girard       16 mai 2015 Creation
 ********************************************************************/

/* Device header file */
#include <xc.h>
#include "mount.h"
#include "ra_motor.h"

void SelectSideralTracking()
{
    RAStop();
    Mount.SideralHalfPeriod = Mount.Config.SideralPeriod / 2;
    RAMotorInit();
    RAStart();
}

void SelectLunarTracking()
{
    RAStop();
    Mount.SideralHalfPeriod = Mount.Config.LunarPeriod / 2;
    RAMotorInit();
    RAStart();
}

void SelectSolarTracking()
{
    RAStop();
    Mount.SideralHalfPeriod = Mount.Config.SolarPeriod / 2;
    RAMotorInit();
    RAStart();
}
