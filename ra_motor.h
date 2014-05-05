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

#define RA_EI (IEC0bits.T2IE = 1)
#define RA_DI (IEC0bits.T2IE = 0)

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
void RASetDirection(uint8_t dir);
void UpdateRAStepPosition();

//#ifndef RA_MOTOR_C

/* Mount specific settings */
extern int32_t NbStepMax;
extern int32_t RAStepPerSec;

extern uint32_t SideralPeriod;
extern uint32_t SideralHalfPeriod;
extern uint16_t MaxSpeed;
extern uint16_t CenteringSpeed;

/* Acceleration/decelation varibles and constant */
extern int32_t AccelTime;
extern int32_t DecelTime;
extern int32_t AccelPeriod;
extern int32_t DecelPeriod;
extern uint16_t CurrentMaxSpeed;

/* Position variables */
extern int32_t RAStepPosition;
extern int32_t RAStepStart;
extern int32_t RAStepTarget;
extern int32_t NumberRAStep;
extern int32_t RADecelPositon;

extern uint8_t WestDirection;
extern uint8_t EastDirection;

//#endif

#endif	/* RA_MOTOR_H */

