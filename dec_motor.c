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
 * Copyright � 2014 Sylvain Girard
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

#include "mount.h"
#include "dec_motor.h"
#include "rtcc.h"
#include "telescope_movement_commands.h"
#include "ra_motor.h"

dec_t Dec;

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
    IPC2bits.T3IP = 6; // Interrupt priority 6 (high)
    IFS0bits.T3IF = 0;
    IEC0bits.T3IE = 1;
}

void __attribute__((interrupt, no_auto_psv)) _T3Interrupt(void)
{
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
            DEC_STEP_IO ^= 1;

            tint_cnt = tlap;
            PR3 = 0xFFFF;
        }
    }
    else
    {
        DEC_STEP_IO ^= 1;
    }

    if (DEC_STEP_IO == 1)
    {

        DecRelativeStepPosition++;
        if (Dec.NumberStep)
        {
            if (Dec.NumberStep <= Dec.DecelPositon && DecState != MOTOR_DECEL)
            {
                DecState = MOTOR_DECEL;
                accel_decel_cnt = Mount.AccelPeriod - accel_decel_cnt;
            }
            Dec.NumberStep--;
        }
    }

    switch (DecState)
    {
    case MOTOR_STOP:
        break;

    case MOTOR_ACCEL:
        accel_decel_cnt -= PR3;
        if (accel_decel_cnt <= 0)
        {
            accel_decel_cnt = Mount.AccelPeriod;
            CurrentSpeed++;
            NewMotorPeriod = TRUE;

            if (CurrentSpeed >= Mount.CurrentMaxSpeed)
            {
                Dec.DecelPositon = DecRelativeStepPosition;
                DecState = MOTOR_FULLSPEED;
            }
        }
        break;

    case MOTOR_DECEL:
        accel_decel_cnt -= PR3;
        if (accel_decel_cnt <= 0)
        {
            accel_decel_cnt = Dec.NumberStep ? Mount.AccelPeriod : Mount.DecelPeriod;
            CurrentSpeed--;
            NewMotorPeriod = TRUE;

            if (CurrentSpeed == 0)
            {
                DecState = MOTOR_STOP;
                CurrentMove &= ~MOVE_DEC;
                T3CONbits.TON = 0;
                DEC_SLEEP_IO = 0;
                DEC_FAULT_CN = 0;
            }

        }
        break;

    case MOTOR_FULLSPEED:
        break;
    }

    if (NewMotorPeriod == TRUE)
    {
        MotorTimerPeriod = Mount.SideralHalfPeriod / CurrentSpeed;
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

static void UpdateMotorTimerPeriod()
{
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

void DecMotorInit(void)
{
    DEC_SLEEP_IO = 0;
    DEC_MODE_IO = 1; // 8 microsteps / step
    DEC_DIR_IO = 0;
    DEC_STEP_IO = 0;

    MotorTimerPeriod = Mount.SideralHalfPeriod;

    Dec.StepPerDegree = Mount.Config.NbStepMax / 360L;
    Dec.StepPerMinute = Dec.StepPerDegree / 60L;
    Dec.StepPerSecond = Dec.StepPerMinute / 60L;

    RTCCReadArray(RTCC_RAM + sizeof (int32_t), (BYTE *) & Dec.StepPosition, sizeof (Dec.StepPosition));
    if (Dec.StepPosition < -Mount.Config.NbStepMax / 4L || Dec.StepPosition > Mount.Config.NbStepMax / 4L)
    {
        Dec.StepPosition = Mount.Config.NbStepMax / 4; // Set default position to north celestial pole
    }
    Dec.StepTarget = Dec.StepPosition;

    Dec.StepStart = Dec.StepPosition;
    Dec.NorthPoleOVerflow = FALSE;

    Timer3Init();

    if (Mount.Config.IsParked == 1)
    {
        Dec.IsParking = PARKED;
    }
    else
    {
        Dec.IsParking = UNPARKED;
    }
}

void DecStart(void)
{
    DEC_SLEEP_IO = 1;
    DEC_FAULT_CN = 1;
    CurrentSpeed = 1;
    T3CONbits.TON = 1;
}

void DecAccelerate(void)
{
    if (DecState == MOTOR_STOP)
    {
        DecDirection = DecNextDirection;
        DEC_DIR_IO = DecNextDirection;

        Dec.StepStart = Dec.StepPosition;
        DecRelativeStepPosition = 0;
        Dec.NorthPoleOVerflow = FALSE;

        DEC_SLEEP_IO = 1;
        DEC_FAULT_CN = 1;

        CurrentSpeed = 1;
        MotorTimerPeriod = Mount.SideralHalfPeriod / CurrentSpeed;
        UpdateMotorTimerPeriod();

        accel_decel_cnt = Mount.AccelPeriod;
        DecState = MOTOR_ACCEL;

        T3CONbits.TON = 1;
    }
}

void DecDecelerate(void)
{
    if (DecState == MOTOR_ACCEL || DecState == MOTOR_FULLSPEED)
    {
        accel_decel_cnt = Mount.DecelPeriod;
        DecState = MOTOR_DECEL;
    }
}

void DecStop(void)
{
    T3CONbits.TON = 0;
    DEC_SLEEP_IO = 0;
    DEC_FAULT_CN = 0;
}

void DecSetDirection(uint8_t dir)
{
    DecNextDirection = dir;
}

void UpdateDecStepPosition()
{
    int32_t DecStepMax = Mount.Config.NbStepMax / 4L;
    int32_t p;
    static BOOL SavePosition = TRUE;

    if (DecState != MOTOR_STOP)
    {
        Dec_DI;
        p = DecRelativeStepPosition;
        Dec_EI;

        if (DecDirection == Mount.NorthDirection)
        {
            if (Dec.NorthPoleOVerflow == FALSE) Dec.StepPosition = Dec.StepStart + p;
            else Dec.StepPosition = Dec.StepStart + p;

        }
        else
        {
            if (Dec.NorthPoleOVerflow == FALSE) Dec.StepPosition = Dec.StepStart - p;
            else Dec.StepPosition = Dec.StepStart + p;
        }

        if ((Dec.StepPosition < -Mount.Config.NbStepMax / 4L) || (Dec.StepPosition > Mount.Config.NbStepMax / 4L))
        {
            Dec.NorthPoleOVerflow = TRUE;
            Dec.StepPosition = DecStepMax - (Dec.StepPosition - DecStepMax);
        }

        SavePosition = TRUE;
    }
    else if (SavePosition == TRUE)
    {
        RTCCWriteArray(RTCC_RAM + sizeof (int32_t), (BYTE*) & Dec.StepPosition, sizeof (Dec.StepPosition));
        SavePosition = FALSE;
    }

    if (Dec.IsParking == PARKING && DecState == MOTOR_STOP)
    {
        Dec.IsParking = PARKED;
    }
}

inline int DecIsMotorStop()
{
    return (DecState == MOTOR_STOP);
}

void DecGuideNorth()
{
    MotorTimerPeriod = Mount.SideralHalfPeriod * (Mount.Config.GuideSpeed) / 10;
    UpdateMotorTimerPeriod();
    DEC_DIR_IO = Mount.NorthDirection;
    DecStart();
}

void DecGuideSouth()
{
    MotorTimerPeriod = Mount.SideralHalfPeriod * (Mount.Config.GuideSpeed) / 10;
    UpdateMotorTimerPeriod();
    DEC_DIR_IO = Mount.SouthDirection;
    DecStart();
}
