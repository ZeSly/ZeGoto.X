/*********************************************************************
 *
 *      ~ OpenGoto ~
 *
 *  Declination motor setup and control
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

#define Dec_EI (IEC0bits.T3IE = 1)
#define Dec_DI (IEC0bits.T3IE = 0)

void DecMotorInit(void);
void DecStart(void);
void DecDecelerate(void);
void DecStop(void);
void DecSetDirection(uint8_t dir);
void UpdateDecStepPosition();

/* Mount specific variables */
extern int32_t DecStepPerDegree;
extern int32_t DecStepPerMinute;
extern int32_t DecStepPerSecond;

/* Position variables */
extern int32_t DecStepPosition;
extern int32_t DecStepStart;
extern int32_t DecStepTarget;

extern uint8_t NorthDirection;
extern uint8_t SouthDirection;

#endif	/* DEC_MOTOR_H */
