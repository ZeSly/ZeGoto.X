/*********************************************************************
 *
 *      ~ OpenGoto ~
 *
 *  LX200 : h - Home Position Commands
 *
 *********************************************************************
 * FileName:        home_position_commands.h
 * Processor:       PIC24FJ256GB106
 * Compiler:        Microchip XC16 v1.21 or higher
 *
 * Copyright © 2014 Sylvain Girard
 *
 * Author               Date        Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Sylvain Girard       28 juin 2014 Creation
 ********************************************************************/

#ifndef HOME_POSITION_COMMANDS_H
#define	HOME_POSITION_COMMANDS_H

void homeSetParkPosition();
void homeSlewToParkPosition();
void homeUnpark();
void GetHomeData();

void GetSideOfPier();
void SetSideOfPier();
void FlipSideOfPier();

#endif	/* HOME_POSITION_COMMANDS_H */
