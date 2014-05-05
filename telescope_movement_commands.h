/*********************************************************************
 *
 *      ~ OpenGoto ~
 *
 *  LX200 : M ? Telescope Movement Commands
 *          Q ? Movement Commands
 *
 *********************************************************************
 * FileName:        telescope_movement_commands.h
 * Processor:       PIC24FJ256GB106
 * Compiler:        Microchip XC16 v1.21 or higher
 *
 * Copyright © 2014 Sylvain Girard
 *
 * Author               Date        Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Sylvain Girard   	3 mai 2014 Creation
 ********************************************************************/

#ifndef TELESCOPE_MOVEMENT_COMMANDS_H
#define	TELESCOPE_MOVEMENT_COMMANDS_H

#define MOVE_TO_EAST    0x1
#define MOVE_TO_WEST    0x2
#define MOVE_TO_NORTH   0x4
#define MOVE_TO_SOUTH   0x8
#define MOVE_RA         (MOVE_TO_EAST | MOVE_TO_WEST)
#define MOVE_DEC        (MOVE_TO_NORTH | MOVE_TO_SOUTH)

void Halt();
void MoveEast();
void MoveNorth();
void MoveSouth();
void MoveWest();
void SlewToTarget();

extern char CurrentMove;
extern BOOL SlewingToTarget;
//extern long StartRAStepPosition;
//extern long StartDecStepPosition;
//extern long NumberRAStep;
//extern long NumberDecStep;

#endif	/* TELESCOPE_MOVEMENT_COMMANDS_H */
