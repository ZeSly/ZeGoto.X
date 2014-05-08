/*********************************************************************
 *
 *      ~ OpenGoto ~
 *
 *  Declination motor setup and control
 *
 *********************************************************************
 * FileName:        dec_motor.c
 * Processor:       PIC24FJ256GB106
 * Compiler:        Microchip XC16 v1.21 or higher
 *
 * Copyright © 2014 Sylvain Girard
 *
 * Author               Date        Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Sylvain Girard   	1 mai 2014 Creation
 ********************************************************************/

/* Device header file */
#include <xc.h>
#include <stdint.h>

#include "GenericTypeDefs.h"
#include "HardwareProfile.h"
#include "ra_motor.h"
#include "dec_motor.h"
#include "rtcc.h"
#include "telescope_movement_commands.h"

/* Mount specific variables */
int32_t DecStepPerDegree;
int32_t DecStepPerMinute;
int32_t DecStepPerSecond;

/* Position variables */
int32_t DecStepPosition;
int32_t DecStepStart;
int32_t DecStepTarget;
int32_t NumberDecStep;
int32_t DecDecelPositon;

BOOL NorthPoleOVerflow;

uint8_t NorthDirection = 0;
uint8_t SouthDirection = 1;

/* static for dec motor */
static int32_t DecRelativeStepPosition;
static uint8_t DecDirection;
static uint8_t DecNextDirection;

static motor_state_t DecState = MOTOR_STOP;

static uint32_t MotorTimerPeriod;
static uint16_t CurrentSpeed;
static uint16_t tmodulo;
static uint16_t tlap;
static uint16_t tint_cnt;
static int32_t accel_decel_cnt;

void Timer3Init(void)
{
    T3CON = 0x0000; // 16 bit time, 1:1 prescale, internal clock
    if (MotorTimerPeriod > 0xFFFF)
    {
        tlap = MotorTimerPeriod / 0xFFFF;
        tint_cnt = tlap;
        tmodulo = MotorTimerPeriod % 0xFFFF;
        PR3 = 0xFFFF;
    }
    else
    {
        tint_cnt = 0;
        tmodulo = 0;
        PR3 = MotorTimerPeriod;
    }
    IPC2bits.T3IP = 6; // Interrupt priority 6 (high)
    IFS0bits.T3IF = 0;
    IEC0bits.T3IE = 1;
}

void __attribute__((interrupt, no_auto_psv)) _T3Interrupt(void)
{
    BOOL MakeOneStep = FALSE;
    BOOL NewMotorPeriod = FALSE;

    if (tmodulo != 0)
    {
        tint_cnt--;
        if (tint_cnt == 0)
        {
            PR3 = tmodulo;
        }
        else if (tint_cnt == 0xFFFF)
        {
            MakeOneStep = TRUE;

            tint_cnt = tlap;
            PR3 = 0xFFFF;
        }
    }
    else
    {
        MakeOneStep = TRUE;
    }

    if (MakeOneStep == TRUE)
    {
        DEC_STEP_IO ^= 1;
        DecRelativeStepPosition++;
        if (NumberDecStep)
        {
            if (NumberDecStep <= DecDecelPositon && DecState != MOTOR_DECEL)
            {
                DecState = MOTOR_DECEL;
                accel_decel_cnt = AccelPeriod - accel_decel_cnt;
            }
            NumberDecStep--;
        }
    }

    switch (DecState)
    {
    case MOTOR_STOP:
        T3CONbits.TON = 0;
        DEC_SLEEP_IO = 0;
        break;

    case MOTOR_ACCEL:
        accel_decel_cnt -= PR3;
        if (accel_decel_cnt <= 0)
        {
            accel_decel_cnt = AccelPeriod;
            CurrentSpeed++;
            NewMotorPeriod = TRUE;

            if (CurrentSpeed >= CurrentMaxSpeed)
            {
                DecDecelPositon = DecRelativeStepPosition;
                DecState = MOTOR_FULLSPEED;
            }
        }
        break;

    case MOTOR_DECEL:
        accel_decel_cnt -= PR3;
        if (accel_decel_cnt <= 0)
        {
            accel_decel_cnt = NumberDecStep ? AccelPeriod : DecelPeriod;
            CurrentSpeed--;
            NewMotorPeriod = TRUE;

            if (CurrentSpeed == 0)
            {
                DecState = MOTOR_STOP;
                CurrentMove &= ~MOVE_DEC;
            }

        }
        break;

    case MOTOR_FULLSPEED:
        break;
    }

    if (NewMotorPeriod == TRUE)
    {
        MotorTimerPeriod = SideralHalfPeriod / CurrentSpeed;
        if (MotorTimerPeriod > 0xFFFF)
        {
            tlap = MotorTimerPeriod / 0xFFFF;
            tint_cnt = tlap;
            tmodulo = MotorTimerPeriod % 0xFFFF;
            PR3 = 0xFFFF;
        }
        else
        {
            tint_cnt = 0;
            tmodulo = 0;
            PR3 = MotorTimerPeriod;
        }
    }

    // Reset interrupt flag
    IFS0bits.T3IF = 0;
}

void DecMotorInit(void)
{
    DEC_HOME_PULLUP = 1;
    DEC_FAULT_PULLUP = 1;

    DEC_HOME_TRIS = INPUT_PIN;
    DEC_FAULT_TRIS = INPUT_PIN;

    DEC_SLEEP_TRIS = OUTPUT_PIN;
    DEC_DIR_TRIS = OUTPUT_PIN;
    DEC_STEP_TRIS = OUTPUT_PIN;
    DEC_MODE_TRIS = OUTPUT_PIN;

    DEC_SLEEP_IO = 0;
    DEC_MODE_IO = 1; // 8 microsteps / step
    DEC_DIR_IO = 0;
    DEC_STEP_IO = 0;

    MotorTimerPeriod = SideralHalfPeriod;

    DecStepPerDegree = NbStepMax / 360L;
    DecStepPerMinute = DecStepPerDegree / 60L;
    DecStepPerSecond = DecStepPerMinute / 60L;

    RTCCReadArray(RTCC_RAM + sizeof (int32_t), (BYTE *) & DecStepPosition, sizeof (DecStepPosition));
    if (DecStepPosition < -NbStepMax / 4L || DecStepPosition > NbStepMax / 4L)
    {
        DecStepPosition = NbStepMax / 4; // Set default position to north celestial pole
    }

    DecStepStart = DecStepPosition;
    NorthPoleOVerflow = FALSE;

    Timer3Init();
}

void DecAccelerate(void)
{
    if (DecState == MOTOR_STOP)
    {
        DecDirection = DecNextDirection;
        DEC_DIR_IO = DecNextDirection;

        DecStepStart = DecStepPosition;
        DecRelativeStepPosition = 0;
        NorthPoleOVerflow = FALSE;

        DEC_SLEEP_IO = 1;
        DEC_FAULT_CN = 1;

        CurrentSpeed = 1;
        MotorTimerPeriod = SideralHalfPeriod / CurrentSpeed;
        if (MotorTimerPeriod > 0xFFFF)
        {
            tlap = MotorTimerPeriod / 0xFFFF;
            tint_cnt = tlap;
            tmodulo = MotorTimerPeriod % 0xFFFF;
            PR3 = 0xFFFF;
        }
        else
        {
            tint_cnt = 0;
            tmodulo = 0;
            PR3 = MotorTimerPeriod;
        }

        accel_decel_cnt = AccelPeriod;
        DecState = MOTOR_ACCEL;

        T3CONbits.TON = 1;
    }
}

void DecDecelerate(void)
{
    if (DecState == MOTOR_ACCEL || DecState == MOTOR_FULLSPEED)
    {
        accel_decel_cnt = DecelPeriod;
        DecState = MOTOR_DECEL;
    }
}

void DecStop(void)
{
    T3CONbits.TON = 0;
    DEC_FAULT_CN = 0;
    DEC_SLEEP_IO = 0;
}

void DecSetDirection(uint8_t dir)
{
    DecNextDirection = dir;
}

void UpdateDecStepPosition()
{
    int32_t DecStepMax = NbStepMax / 4L;
    int32_t p;
    static BOOL SavePosition = TRUE;

    if (DecState != MOTOR_STOP)
    {
        Dec_DI;
        p = DecRelativeStepPosition;
        Dec_EI;

        if (DecDirection == NorthDirection)
        {
            if (NorthPoleOVerflow == FALSE) DecStepPosition = DecStepStart + p;
            else DecStepPosition = DecStepStart + p;

        }
        else
        {
            if (NorthPoleOVerflow == FALSE) DecStepPosition = DecStepStart - p;
            else DecStepPosition = DecStepStart + p;
        }

        if ((DecStepPosition < -NbStepMax / 4L) || (DecStepPosition > NbStepMax / 4L))
        {
            NorthPoleOVerflow = TRUE;
            DecStepPosition = DecStepMax - (DecStepPosition - DecStepMax);
        }

        SavePosition = TRUE;
    }
    else if (SavePosition == TRUE)
    {
        RTCCWriteArray(RTCC_RAM + sizeof (int32_t), (BYTE*) & DecStepPosition, sizeof (DecStepPosition));
        SavePosition = FALSE;
    }
}

