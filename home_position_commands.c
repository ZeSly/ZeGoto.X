/*********************************************************************
 *
 *      ~ OpenGoto ~
 *
 *  LX200 : h - Home Position Commands
 *
 *********************************************************************
 * FileName:        home_position_commands.c
 * Processor:       PIC24FJ256GB106
 * Compiler:        Microchip XC16 v1.21 or higher
 *
 * Copyright � 2014 Sylvain Girard
 *
 * Author               Date        Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Sylvain Girard       28 juin 2014 Creation
 ********************************************************************/

/* Device header file */
#include <xc.h>
#include <string.h>

#include "utils.h"
#include "mount.h"
#include "ra_motor.h"
#include "dec_motor.h"
#include "lx200_protocol.h"
#include "telescope_movement_commands.h"

/******************************************************************************
 * Function:        void homeSetParkPosition()
 * PreCondition:    None
 * Input:           None
 * Output:          None
 * Side Effects:    None
 * Overview:        LX200 :hS# command : set parl position
 *                  :hSn# where <n> is the park position
 *****************************************************************************/
void homeSetParkPosition()
{
    if (LX200String[2] > '0' && LX200String[2] < '4')
    {
        Mount.Config.ParkPostion = LX200String[2] - '0';
    }
    else
    {
        Mount.Config.ParkPostion = 0;
        ComputeAzimuthalCoord(&Mount.Config.ParkAltitude, &Mount.Config.ParkAzimuth);
    }
    SaveMountConfig(&Mount.Config);
}

/******************************************************************************
 * Function:        void homeSlewToParkPosition()
 * PreCondition:    Mount is not parked, i.e. Mount.Config.IsParked is clear
 * Input:           None
 * Output:          None
 * Side Effects:    None
 * Overview:        LX200 :hP# command : slew to park postion and switch to
 *                  paked mode.
 *****************************************************************************/
void homeSlewToParkPosition()
{
    double ra, dec;

    if (Mount.Config.IsParked || Mount.PierIsFlipping) return;

    if (Mount.Config.ParkPostion == 0)
    {
        ComputeEquatorialCoord(Mount.Config.ParkAltitude, Mount.Config.ParkAzimuth, &ra, &dec);
        RA.StepTarget = (int32_t) (Mount.Config.NbStepMax * ra / 24.0);
        Dec.StepTarget = (int32_t) (Mount.Config.NbStepMax * dec / 360.0);
    }
    else if (Mount.Config.ParkPostion == 3)
    {
        // Set park position to north celestial pole
        ra = ComputeSideralTime();
        RA.StepTarget = (int32_t) (Mount.Config.NbStepMax * ra / 24.0);
        Dec.StepTarget = Mount.Config.NbStepMax / 4;
    }
    RA.IsParking = PARKING;
    Dec.IsParking = PARKING;
    SlewToTarget();
}

/******************************************************************************
 * Function:        void homeUnpark()
 * PreCondition:    Mount is parked, i.e. Mount.Config.IsParked is set
 * Input:           None
 * Output:          None
 * Side Effects:    None
 * Overview:        LX200 :hW# command : unpark, start RA trakcing
 *****************************************************************************/
void homeUnpark()
{
    //    double ra, dec;

    if (Mount.Config.IsParked)
    {
        RA.IsParking = UNPARKED;
        Dec.IsParking = UNPARKED;
        Mount.Config.IsParked = FALSE;
        SaveMountConfig(&Mount.Config);

        //        if (Mount.Config.ParkPostion == 0)
        //        {
        //            ComputeEquatorialCoord(Mount.Config.ParkAltitude, Mount.Config.ParkAzimuth, &ra, &dec);
        //            RA.StepTarget = (int32_t) (Mount.Config.NbStepMax * ra / 24.0);
        //            Dec.StepTarget = (int32_t) (Mount.Config.NbStepMax * dec / 360.0);
        //        }
        //        else if (Mount.Config.ParkPostion == 3)
        //        {
        //            // Set park position to north celestial pole
        //            ra = ComputeSideralTime();
        //            RA.StepTarget = (int32_t) (Mount.Config.NbStepMax * ra / 24.0);
        //            Dec.StepTarget = Mount.Config.NbStepMax / 4;
        //        }
        //        RA.StepPosition = RA.StepTarget;
        //        Dec.StepPosition = Dec.StepTarget;
        RAStart();
    }
}

/******************************************************************************
 * Function:        void SideOfPier()
 * PreCondition:    None
 * Input:           None
 * Output:          None
 * Side Effects:    None
 * Overview:        LX200 :pS# Get ASTRO-PHYSICS GTO side of pier
 *****************************************************************************/
void GetSideOfPier()
{
    if (Mount.SideOfPier)
    {
        strcpy(LX200Response, "West");
    }
    else
    {
        strcpy(LX200Response, "East");
    }

}

/******************************************************************************
 * Function:        void SetRealSideOfPier()
 * PreCondition:    None
 * Input:           None
 * Output:          None
 * Side Effects:    None
 * Overview:        LX200 :pR# Set actual side of pier
 *****************************************************************************/
void SetSideOfPier()
{
    if (LX200String[2] == 'E')
    {
        Mount.AutomaticSideOfPier = 0;
        Mount.SideOfPier = PIER_EAST;
    }
    else if (LX200String[2] == 'W')
    {
        Mount.AutomaticSideOfPier = 0;
        Mount.SideOfPier = PIER_WEST;
    }
    else if (LX200String[2] == 'A')
    {
        Mount.AutomaticSideOfPier = 1;
    }
}

/******************************************************************************
 * Function:        void FlipSideOfPier()
 * PreCondition:    Mount is not parked, i.e. Mount.Config.IsParked is clear
 * Input:           None
 * Output:          None
 * Side Effects:    None
 * Overview:        LX200 :pF# Move the mount to perform a side of pier flip
 *****************************************************************************/
void FlipSideOfPier()
{
    if (Mount.Config.IsParked) return;

    // number of step for 90� in dec
    int32_t ninety_deg = Mount.Config.NbStepMax / 4;

    // 180� on RA
    RA.NumberStep = Mount.Config.NbStepMax / 2L;

    // (90� - Dec) * 2 on Dec
    Dec.NumberStep = (ninety_deg - Dec.StepPosition) * 2L;

    Mount.PierIsFlipping = 1;
    if (Mount.SideOfPier == PIER_EAST)
    {
        MoveWest();
        MoveNorth(); // always move north, actul dec direction depend on RealSideOfPier
    }
    else
    {
        MoveEast();
        MoveNorth(); // always move north, actul dec direction depend on RealSideOfPier
    }
}