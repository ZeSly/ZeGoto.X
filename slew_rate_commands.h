/*********************************************************************
 *
 *      ~ OpenGoto ~
 *
 *  LX200 : R ? Slew Rate Commands
 *
 *********************************************************************
 * FileName:        slew_rate_commands.h
 * Processor:       PIC24FJ256GB106
 * Compiler:        Microchip XC16 v1.21 or higher
 *
 * Copyright © 2014 Sylvain Girard
 *
 * Author               Date        Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Sylvain Girard       4 mai 2014 Creation
 ********************************************************************/

#ifndef SLEW_RATE_COMMANDS_H
#define	SLEW_RATE_COMMANDS_H

void SetCenteringRate();
void SetGuidingRate();
void SetFindRate();
void SetMaxRate();
void SetSpecificRate();

void GetCenteringRate();
void GetGuidingRate();
void GetCurrentMaxSpeed();
void GetMaxSpeed();

#endif	/* SLEW_RATE_COMMANDS_H */
