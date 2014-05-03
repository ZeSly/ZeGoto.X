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

#include <stdlib.h>

char CurrentMove;
BOOL SlewingToTarget = FALSE;

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
    DecDecelerate();
}

void MoveSouth()
{
    CurrentMove |= MOVE_TO_SOUTH;
    CurrentMove &= ~MOVE_TO_NORTH;
    DecSetDirection(SouthDirection);
    DecStart();
}

void MoveWest()
{
    CurrentMove |= MOVE_TO_WEST;
    CurrentMove &= ~MOVE_TO_EAST;
    RASetDirection(WestDirection);
    DecStart();
}
/*
long StartRAStepPosition;
long StartDecStepPosition;
long NumberRAStep;
long NumberDecStep;

void SlewToTarget()
{
//    Serial.print("1");

    StartRAStepPosition = RAStepPosition;
    StartDecStepPosition = DecStepPosition;
    NumberRAStep = abs(RAStepTarget - StartRAStepPosition);
    NumberDecStep = abs(DecStepTarget - StartDecStepPosition);

//    char trace[64];
//    sprintf(trace, "\r\nRAStepPosition=%li DecStepPosition=%li", RAStepPosition, DecStepPosition);
//    Serial.print(trace);
//    sprintf(trace, "\r\nRAStepTarget=%li DecStepTarget=%li", RAStepTarget, DecStepTarget);
//    Serial.print(trace);
//    sprintf(trace, "\r\nNumberRAStep=%li NumberDecStep=%li\r\n", NumberRAStep, NumberDecStep);
//    Serial.print(trace);

    SlewingToTarget = TRUE;

    if (DecStepPosition < DecStepTarget)
    {
        MoveNorth();
    }
    else if (DecStepPosition > DecStepTarget)
    {
        MoveSouth();
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
*/