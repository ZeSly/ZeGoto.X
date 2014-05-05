/*********************************************************************
 *
 *      ~ OpenGoto ~
 *
 *  Right Ascension motor setup and control
 *
 *********************************************************************
 * FileName:        ra_motor.c
 * Processor:       PIC24FJ256GB106
 * Compiler:        Microchip XC16 v1.21 or higher
 *
 * Copyright © 2014 Sylvain Girard
 *
 * Author               Date        Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Sylvain Girard   	01/05/2011  Creation
 ********************************************************************/

#define RA_MOTOR_C

/* Device header file */
#include <xc.h>

#include "HardwareProfile.h"
#include "ra_motor.h"
#include "GenericTypeDefs.h"
#include "rtcc.h"
#include "dec_motor.h"

/* Mount specific settings */
int32_t NbStepMax = 8640000UL;
int32_t RAStepPerSec;

uint32_t SideralPeriod = 159563UL;
uint32_t SideralHalfPeriod = 159563UL / 2;
uint16_t MaxSpeed = 120;
uint16_t CenteringSpeed = 10;

/* Acceleration/decelation varibles and constant */
int32_t AccelTime = 4; // seconds
int32_t DecelTime = 1; // seconds
int32_t AccelPeriod;
int32_t DecelPeriod;
uint16_t CurrentMaxSpeed;

/* Position variables */
int32_t RAStepPosition;
int32_t RAStepStart;
int32_t RAStepTarget;
int32_t NumberRAStep;
int32_t RADecelPositon;

uint8_t WestDirection = 0;
uint8_t EastDirection = 1;

/* static for RA motor */
static int32_t RARelativeStepPosition;
static uint8_t RADirection;
static uint8_t RANextDirection;

static motor_state_t RAState = MOTOR_STOP; // = sideral rate for RA motor

static uint32_t MotorTimerPeriod;
static uint16_t CurrentSpeed;
static uint16_t tmodulo;
static uint16_t tlap;
static uint16_t tint_cnt;
static int32_t accel_decel_cnt;

void Timer2Init(void)
{
    T2CON = 0x0000; // 16 bit time, 1:1 prescale, internal clock
    if (MotorTimerPeriod > 0xFFFF)
    {
        tlap = MotorTimerPeriod / 0xFFFF;
        tint_cnt = tlap;
        tmodulo = MotorTimerPeriod % 0xFFFF;
        PR2 = 0xFFFF;
    }
    else
    {
        tint_cnt = 0;
        tmodulo = 0;
        PR2 = MotorTimerPeriod;
    }
    IPC1bits.T2IP = 6; // Interrupt priority 6 (high)
    IFS0bits.T2IF = 0;
    IEC0bits.T2IE = 1;
}

void __attribute__((interrupt, no_auto_psv)) _T2Interrupt(void)
{
//    static BYTE fullspeed = 0;

    if (tmodulo != 0)
    {
        tint_cnt--;
        if (tint_cnt == 0)
        {
            PR2 = tmodulo;
        }
        else if (tint_cnt == 0xFFFF)
        {
            RA_STEP_IO ^= 1;
            if (RAState != MOTOR_STOP)
            {
                RARelativeStepPosition++;
                if (NumberRAStep)
                {
                    if (NumberRAStep <= RADecelPositon && RAState != MOTOR_DECEL)
                    {
                        RAState = MOTOR_DECEL;
                        accel_decel_cnt = AccelPeriod - accel_decel_cnt;
                    }
                    NumberRAStep--;
                    if (NumberRAStep == 0)
                    {
                        CurrentSpeed = 1;
                        MotorTimerPeriod = SideralHalfPeriod / CurrentSpeed;
                        RAState = MOTOR_STOP;
                    }
                }
            }

            // if SideralHalfPeriod is odd
            if (SideralPeriod & 1) tint_cnt = tlap | RA_STEP_IO;

            PR2 = 0xFFFF;
        }
    }
    else
    {
        RA_STEP_IO ^= 1;
        if (RAState != MOTOR_STOP)
        {
            RARelativeStepPosition++;
            if (NumberRAStep)
            {
                if (NumberRAStep <= RADecelPositon && RAState != MOTOR_DECEL)
                {
                    RAState = MOTOR_DECEL;
                    accel_decel_cnt = AccelPeriod - accel_decel_cnt;
                }
                NumberRAStep--;
                if (NumberRAStep == 0)
                {
                    CurrentSpeed = 1;
                    MotorTimerPeriod = SideralHalfPeriod / CurrentSpeed;
                    RAState = MOTOR_STOP;
                }
            }
        }
    }

    switch (RAState)
    {
    case MOTOR_STOP:
//        fullspeed = 0;
        break;

    case MOTOR_ACCEL:
        accel_decel_cnt -= PR2;
        if (accel_decel_cnt <= 0)
        {
            accel_decel_cnt = AccelPeriod;
            CurrentSpeed++;
            if (CurrentSpeed >= MaxSpeed)
            {
                RADecelPositon = RARelativeStepPosition;
                RAState = MOTOR_NOACC;
            }

            MotorTimerPeriod = SideralHalfPeriod / CurrentSpeed;
            if (MotorTimerPeriod > 0xFFFF)
            {
                tlap = MotorTimerPeriod / 0xFFFF;
                tint_cnt = tlap;
                tmodulo = MotorTimerPeriod % 0xFFFF;
                PR2 = 0xFFFF;
            }
            else
            {
                tint_cnt = 0;
                tmodulo = 0;
                PR2 = MotorTimerPeriod;
            }

        }
        break;

    case MOTOR_DECEL:
        accel_decel_cnt -= PR2;
        if (accel_decel_cnt <= 0)
        {
            accel_decel_cnt = NumberRAStep ? AccelPeriod : DecelPeriod;

            CurrentSpeed--;
            if (CurrentSpeed == 1)
            {
                RAState = MOTOR_STOP;
            }

            
            MotorTimerPeriod = SideralHalfPeriod / CurrentSpeed;
            if (MotorTimerPeriod > 0xFFFF)
            {
                tlap = MotorTimerPeriod / 0xFFFF;
                tint_cnt = tlap;
                tmodulo = MotorTimerPeriod % 0xFFFF;
                PR2 = 0xFFFF;
            }
            else
            {
                tint_cnt = 0;
                tmodulo = 0;
                PR2 = MotorTimerPeriod;
            }
        }
        break;

    case MOTOR_NOACC:
//        fullspeed = 1;
        break;
    }

    // Reset interrupt flag
    IFS0bits.T2IF = 0;
}

void RAMotorInit(void)
{
    RA_HOME_TRIS = INPUT_PIN;
    RA_SLEEP_TRIS = OUTPUT_PIN;
    RA_DIR_TRIS = OUTPUT_PIN;
    RA_STEP_TRIS = OUTPUT_PIN;
    RA_MODE_TRIS = OUTPUT_PIN;
    RA_FAULT_TRIS = INPUT_PIN;

    RA_SLEEP_IO = 0;
    RA_MODE_IO = 1; // 8 microsteps / step
    RA_DIR_IO = 0;
    RA_STEP_IO = 0;

    MotorTimerPeriod = SideralHalfPeriod;
    AccelPeriod = GetPeripheralClock() / MaxSpeed * AccelTime;
    DecelPeriod = GetPeripheralClock() / MaxSpeed * DecelTime;

    RTCCReadArray(RTCC_RAM, (BYTE *)&RAStepPosition, sizeof (RAStepPosition));
    if (RAStepPosition < 0 || RAStepPosition > NbStepMax)
    {
        RAStepPosition = 0; // Set default position to north celestial pole
    }

    RAStepPerSec = NbStepMax / (24L * 3600L);
    RAStepStart = RAStepPosition;

    Timer2Init();
}

void RAStart(void)
{
    LED2_IO = 0;
    RA_SLEEP_IO = 1;
    RA_FAULT_CN = 1;
    CurrentSpeed = 1;
    T2CONbits.TON = 1;
}

void RAAccelerate(void)
{
    if (RAState == MOTOR_STOP)
    {
        RADirection = RANextDirection;
        RA_DIR_IO = RANextDirection;

        RAStepStart = RAStepPosition;
        RARelativeStepPosition = 0;

        accel_decel_cnt = AccelPeriod;
        CurrentSpeed++;
        RAState = MOTOR_ACCEL;
    }
}

void RADecelerate(void)
{
    if (RAState == MOTOR_ACCEL || RAState == MOTOR_NOACC)
    {
        accel_decel_cnt = DecelPeriod;
        RAState = MOTOR_DECEL;
    }
}

void RAStop(void)
{
    T2CONbits.TON = 0;
    RA_FAULT_CN = 0;
    RA_SLEEP_IO = 0;
}

void RASetDirection(uint8_t dir)
{
    RANextDirection = dir;
}

extern BOOL NorthPoleOVerflow;

void UpdateRAStepPosition()
{
    static BOOL LastNorthPoleOVerflow = FALSE;
    int32_t p;
    static BOOL SavePosition = TRUE;

    if (RAState != MOTOR_STOP || NorthPoleOVerflow == TRUE)
    {
        RA_DI;
        p = RARelativeStepPosition;
        RA_EI;

        if (RADirection == WestDirection)
        {
            RAStepPosition = RAStepStart - p;
            if (RAStepPosition < 0) RAStepPosition += NbStepMax;
        }
        else
        {
            RAStepPosition = RAStepStart + p;
            if (RAStepPosition > NbStepMax) RAStepPosition -= NbStepMax;
        }

        if (LastNorthPoleOVerflow == FALSE && NorthPoleOVerflow == TRUE)
        {
            RAStepStart += NbStepMax / 2L;
            RAStepStart %= NbStepMax;
        }
        LastNorthPoleOVerflow = NorthPoleOVerflow;
        SavePosition = TRUE;
    }
    else if (SavePosition == TRUE)
    {
        RTCCWriteArray(RTCC_RAM, (BYTE*) &RAStepPosition, sizeof (RAStepPosition));
        SavePosition = FALSE;
    }
}