/*********************************************************************
 *
 *      ~ OpenGoto ~
 *
 *  Right Ascension motor setup and control
 *
 *********************************************************************
 * FileName:        ra_motor.h
 * Processor:       PIC24FJ256GB106
 * Compiler:        Microchip XC16 v1.21 or higher
 *
 * Copyright © 2014 Sylvain Girard
 *
 * Author               Date        Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Sylvain Girard   	1 mai 2014  Creation
 ********************************************************************/

#ifndef RA_MOTOR_H
#define	RA_MOTOR_H

#include <stdint.h>

typedef enum
{
    MOTOR_STOP,
    MOTOR_ACCEL,
    MOTOR_NOACC,
    MOTOR_DECEL,
} motor_state_t;

void RAMotorInit(void);
void RAStart(void);
void RAAccelerate(void);
void RADecelerate(void);
void RAStop(void);
void RADirection(uint8_t dir);

#ifndef RA_MOTOR_C

extern uint32_t SideralPeriod;
extern uint32_t SideralHalfPeriod;
extern uint16_t MaxSpeed;

extern int32_t AccelTime;
extern int32_t DecelTime;

extern int32_t AccelPeriod;
extern int32_t DecelPeriod;

#endif

#endif	/* RA_MOTOR_H */

