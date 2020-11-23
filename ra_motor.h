/*********************************************************************
 *
 *      ~ ZeGoto ~
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
#define RA_MOTOR_H

#include <stdint.h>

#define RA_EI (IEC0bits.T3IE = 1)
#define RA_DI (IEC0bits.T3IE = 0)

void RAMotorInit(void);
void RAStart(void);
void RAAccelerate(void);
void RADecelerate(void);
void RAStop(void);
void RASetDirection(uint8_t dir);
void RAChangeDirection();
void UpdateRAStepPosition();
int RAIsMotorStopped();

void RAGuideWest();
void RAGuideEast();
void RAGuideStop();

/* Position variables */
typedef struct
{
    int32_t StepPerSec;
    int32_t StepPosition;
    int32_t StepStart;
    int32_t StepTarget;
    int32_t NumberStep;
    int32_t DecelPositon;
    park_mode_t IsParking;
} ra_t;

extern ra_t RA;

#endif	/* RA_MOTOR_H */

