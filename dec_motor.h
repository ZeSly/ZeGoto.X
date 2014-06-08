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
void DecAccelerate(void);
void DecDecelerate(void);
void DecStop(void);
void DecSetDirection(uint8_t dir);
void UpdateDecStepPosition();
int DecIsMotorStop();

void DecGuideNorth();
void DecGuideSouth();
#define DecGuideStop DecStop

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
} dec_t;

extern dec_t Dec;

#endif	/* DEC_MOTOR_H */
