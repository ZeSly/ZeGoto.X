/*********************************************************************
 *
 *      ~ OpenGoto ~
 *
 *  TODO add description
 *
 *********************************************************************
 * FileName:        telescope_set_commands.h
 * Processor:       PIC24FJ256GB106
 * Compiler:        Microchip XC16 v1.21 or higher
 *
 * Copyright © 2014 Sylvain Girard
 *
 * Author               Date        Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Sylvain Girard       4 mai 2014 Creation
 ********************************************************************/

#ifndef TELESCOPE_SET_COMMANDS_H
#define	TELESCOPE_SET_COMMANDS_H

void SetTargetObjectRA();
void SetTargetObjectDeclination();
void SyncWithCurrentTarget();

void SetStepTargetRA();
void SetStepTargetDeclination();

#endif	/* TELESCOPE_SET_COMMANDS_H */
