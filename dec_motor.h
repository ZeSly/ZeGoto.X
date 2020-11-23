/*********************************************************************
 *
 *      ~ ZeGoto ~
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

#include "mount.h"


#define Dec_EI (IEC1bits.T5IE = 1)
#define Dec_DI (IEC1bits.T5IE = 0)

void DecMotorInit(void);
void DecAccelerate(void);
void DecDecelerate(void);
void DecGuideStop(void);
void DecSetDirection(uint8_t dir);
void UpdateDecStepPosition();
int DecIsMotorStopped();

void DecGuide(BYTE dir);

void GetDecRelativeStepPosition(void);

typedef struct
{
    /* Mount specific variables */
    int32_t StepPerDegree;
    int32_t StepPerMinute;
    int32_t StepPerSecond;

    /* Position variables */
    int32_t StepPosition;
    int32_t StepStart;
    int32_t StepTarget;
    int32_t NumberStep;
    int32_t DecelPositon;

    BOOL NorthPoleOVerflow;
    park_mode_t IsParking;
} dec_t;

extern dec_t Dec;

#endif	/* DEC_MOTOR_H */
