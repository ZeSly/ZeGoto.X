/*********************************************************************
 *
 *      ~ OpenGoto ~
 *
 *  LX200 : M – Telescope Movement Commands
 *          Q – Movement Commands
 *
 *********************************************************************
 * FileName:        telescope_movement_commands.c
 * Processor:       PIC24FJ256GB106
 * Compiler:        Microchip XC16 v1.21 or higher
 *
 * Copyright © 2014 Sylvain Girard
 *
 * Author               Date        Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Sylvain Girard   	3 mai 2014 Creation
 ********************************************************************/

/* Device header file */
#include <xc.h>

#include "GenericTypeDefs.h"
#include "main.h"
#include "lx200_protocol.h"
#include "telescope_movement_commands.h"
#include "mount.h"
#include "ra_motor.h"
#include "dec_motor.h"
#include "HardwareProfile.h"
#include "utils.h"

char CurrentMove;

static int DecPulseGuideTime = 0;
static int RAPulseGuideTime = 0;

void GuidingTimerInit()
{
    OC2CON1 = 0;
    OC2CON2 = 0;
    OC2CON1bits.OCTSEL = 0x07;
    OC2R = 0;
    OC2RS = 16000;
    OC2CON2bits.OCTRIS = 1;
    OC2CON2bits.SYNCSEL = 0x1F;
    OC2CON1bits.OCM = 0;

    IPC1bits.OC2IP = 5;
    IFS0bits.OC2IF = 0;
    IEC0bits.OC2IE = 1;

    OC3CON1 = 0;
    OC3CON2 = 0;
    OC3CON1bits.OCTSEL = 0x07;
    OC3R = 0;
    OC3RS = 16000;
    OC3CON2bits.OCTRIS = 1;
    OC3CON2bits.SYNCSEL = 0x1F;
    OC3CON1bits.OCM = 0;

    IPC6bits.OC3IP = 5;
    IFS1bits.OC3IF = 0;
    IEC1bits.OC3IE = 1;

    TEST_PIN_TRIS = OUTPUT_PIN;
    TEST_PIN_OUT = 0;
}

void __attribute__((interrupt, no_auto_psv)) _OC2Interrupt(void)
{
    if (RAPulseGuideTime)
    {
        RAPulseGuideTime--;
        if (RAPulseGuideTime == 0)
        {
            RAGuideStop();
            OC2CON1bits.OCM = 0;
            TEST_PIN_OUT = 0;
        }
    }

    IFS0bits.OC2IF = 0;
}

void __attribute__((interrupt, no_auto_psv)) _OC3Interrupt(void)
{
    if (DecPulseGuideTime)
    {
        DecPulseGuideTime--;
        if (DecPulseGuideTime == 0)
        {
            DecGuideStop();
            OC3CON1bits.OCM = 0;
            TEST_PIN_OUT = 0;
        }
    }

    IFS1bits.OC3IF = 0;
}

void Halt()
{
    if (Mount.IsGuiding == FALSE)
    {
        switch (LX200String[1])
        {
        case 'e':
            CurrentMove &= ~MOVE_TO_EAST;
            RADecelerate();
            break;
        case 'w':
            CurrentMove &= ~MOVE_TO_WEST;
            RADecelerate();
            break;
        case 'n':
            CurrentMove &= ~MOVE_TO_NORTH;
            DecDecelerate();
            break;
        case 's':
            CurrentMove &= ~MOVE_TO_SOUTH;
            DecDecelerate();
            break;
        default:
            CurrentMove = 0;
            RADecelerate();
            DecDecelerate();
            break;
        }
    }
    else
    {
        switch (LX200String[1])
        {
        case 'e':
        case 'w':
            RAGuideStop();
            break;
        case 'n':
        case 's':
            DecGuideStop();
            break;
        default:
            RAGuideStop();
            DecGuideStop();
            break;
        }

    }
}

void GuideNorth()
{
    if (Mount.Config.IsParked) return;

    DecPulseGuideTime = atoi(LX200String + 3);
    if (DecPulseGuideTime != 0)
    {
        OC3CON1bits.OCM = 3;
        DecGuideNorth();
        TEST_PIN_OUT = 1;
    }
}

void MoveNorth()
{
    if (Mount.Config.IsParked) return;

    if (Mount.IsGuiding == FALSE)
    {
        CurrentMove |= MOVE_TO_NORTH;
        CurrentMove &= ~MOVE_TO_SOUTH;
        DecSetDirection(Mount.NorthDirection);
        DecAccelerate();
    }
    else
    {
        DecGuideNorth();
    }
}

void GuideSouth()
{
    if (Mount.Config.IsParked) return;

    DecPulseGuideTime = atoi(LX200String + 3);
    if (DecPulseGuideTime != 0)
    {
        OC3CON1bits.OCM = 3;
        DecGuideSouth();
        TEST_PIN_OUT = 1;
    }
}

void MoveSouth()
{
    if (Mount.Config.IsParked) return;

    if (Mount.IsGuiding == FALSE)
    {
        CurrentMove |= MOVE_TO_SOUTH;
        CurrentMove &= ~MOVE_TO_NORTH;
        DecSetDirection(Mount.SouthDirection);
        DecAccelerate();
    }
    else
    {
        DecGuideSouth();
    }
}

void GuideEast()
{
    if (Mount.Config.IsParked) return;

    RAPulseGuideTime = atoi(LX200String + 3);
    if (RAPulseGuideTime != 0)
    {
        OC2CON1bits.OCM = 3;
        RAGuideEast();
        TEST_PIN_OUT = 1;
    }
}

void MoveEast()
{
    if (Mount.Config.IsParked) return;

    if (Mount.IsGuiding == FALSE)
    {
        CurrentMove |= MOVE_TO_EAST;
        CurrentMove &= ~MOVE_TO_WEST;
        RASetDirection(Mount.EastDirection);
        RAAccelerate();
    }
    else
    {
        RAGuideEast();
    }
}

void GuideWest()
{
    if (Mount.Config.IsParked) return;

    RAPulseGuideTime = atoi(LX200String + 3);
    if (RAPulseGuideTime != 0)
    {
        OC2CON1bits.OCM = 3;
        RAGuideWest();
        TEST_PIN_OUT = 1;
    }
}

void MoveWest()
{
    if (Mount.Config.IsParked) return;

    if (Mount.IsGuiding == FALSE)
    {
        CurrentMove |= MOVE_TO_WEST;
        CurrentMove &= ~MOVE_TO_EAST;
        RASetDirection(Mount.WestDirection);
        RAAccelerate();
    }
    else
    {
        RAGuideWest();
    }
}

/******************************************************************************
 * Function:        void SlewToTarget()
 * PreCondition:    None
 * Input:           None
 * Output:          None
 * Side Effects:    None
 * Overview:        Slew to RA and dec target position
 *                  LX200 MS and hP (park) command
 *****************************************************************************/
void SlewToTarget()
{
    if (Mount.Config.IsParked) return;

    if (RA.StepTarget)
    {
        RA.NumberStep = int32abs(RA.StepTarget - RA.StepPosition);
        RA.DecelPositon = RA.NumberStep / 2L;
        //        if (RA.NumberStep % 2L == 0)
        //        {
        //            RA.DecelPositon--;
        //        }

        if (RA.StepPosition < RA.StepTarget)
        {
            MoveEast();
        }
        else if (RA.StepPosition > RA.StepTarget)
        {
            MoveWest();
        }
    }

    if (Dec.StepTarget)
    {
        Dec.NumberStep = int32abs(Dec.StepTarget - Dec.StepPosition);
        Dec.DecelPositon = Dec.NumberStep / 2L;
        //        if (Dec.NumberStep % 2L == 0)
        //        {
        //            Dec.DecelPositon++;
        //        }

        if (Dec.StepPosition < Dec.StepTarget)
        {
            MoveNorth();
        }
        else if (Dec.StepPosition > Dec.StepTarget)
        {
            MoveSouth();
        }
    }

    LX200Response[0] = '0';
    LX200Response[1] = '\0';
}
