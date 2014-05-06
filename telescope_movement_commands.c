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
#include "ra_motor.h"
#include "dec_motor.h"

//#include <stdio.h>

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
    RASetDirection(EastDirection);
    RAAccelerate();
}

void MoveNorth()
{
    CurrentMove |= MOVE_TO_NORTH;
    CurrentMove &= ~MOVE_TO_SOUTH;
    DecSetDirection(NorthDirection);
    DecAccelerate();
}

void MoveSouth()
{
    CurrentMove |= MOVE_TO_SOUTH;
    CurrentMove &= ~MOVE_TO_NORTH;
    DecSetDirection(SouthDirection);
    DecAccelerate();
}

void MoveWest()
{
    CurrentMove |= MOVE_TO_WEST;
    CurrentMove &= ~MOVE_TO_EAST;
    RASetDirection(WestDirection);
    RAAccelerate();
}

int32_t int32abs(int32_t a)
{
   return a < 0 ? -a : a;
}

void SlewToTarget()
{
    if (RAStepTarget)
    {
        NumberRAStep = int32abs(RAStepTarget - RAStepPosition);
        RADecelPositon = NumberRAStep / 2L;
        if (NumberRAStep % 2L == 0)
        {
            RADecelPositon--;
        }

        if (RAStepPosition < RAStepTarget)
        {
            MoveEast();
        }
        else if (RAStepPosition > RAStepTarget)
        {
            MoveWest();
        }
    }

    if (DecStepTarget)
    {
        NumberDecStep = int32abs(DecStepTarget - DecStepPosition);
        DecDecelPositon = NumberDecStep / 2L;
//        if (NumberDecStep % 2L == 0)
//        {
//            DecDecelPositon++;
//        }

        if (DecStepPosition < DecStepTarget)
        {
            MoveNorth();
        }
        else if (DecStepPosition > DecStepTarget)
        {
            MoveSouth();
        }
    }

    LX200Response[0] = '0';
    LX200Response[1] = '\0';
}
