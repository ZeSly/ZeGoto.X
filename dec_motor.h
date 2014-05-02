/*********************************************************************
 *
 *      ~ OpenGoto ~
 *
 *  TODO add description
 *
 *********************************************************************
 * FileName:        dec_motor.h
 * Processor:       PIC24FJ256GB106
 * Compiler:        Microchip XC16 v1.21 or higher
 *
 * Copyright © 2014 Sylvain Girard
 *
 * Author               Date        Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Sylvain Girard   	1 mai 2014 Creation
 ********************************************************************/

#ifndef DEC_MOTOR_H
#define	DEC_MOTOR_H

void DecMotorInit(void);
void DecStart(void);
void DecDecelerate(void);
void DecStop(void);
void DecDirection(uint8_t dir);

#endif	/* DEC_MOTOR_H */
