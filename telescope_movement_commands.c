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

char CurrentMove;

void Halt()
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

void MoveEast()
{
    CurrentMove |= MOVE_TO_EAST;
    CurrentMove &= ~MOVE_TO_WEST;
    RASetDirection(Mount.EastDirection);
    RAAccelerate();
}

void MoveNorth()
{
    CurrentMove |= MOVE_TO_NORTH;
    CurrentMove &= ~MOVE_TO_SOUTH;
    DecSetDirection(Mount.NorthDirection);
    DecAccelerate();
}

void MoveSouth()
{
    CurrentMove |= MOVE_TO_SOUTH;
    CurrentMove &= ~MOVE_TO_NORTH;
    DecSetDirection(Mount.SouthDirection);
    DecAccelerate();
}

void MoveWest()
{
    CurrentMove |= MOVE_TO_WEST;
    CurrentMove &= ~MOVE_TO_EAST;
    RASetDirection(Mount.WestDirection);
    RAAccelerate();
}

int32_t int32abs(int32_t a)
{
   return a < 0 ? -a : a;
}

void SlewToTarget()
{
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
