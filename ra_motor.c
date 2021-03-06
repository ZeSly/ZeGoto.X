/*********************************************************************
 *
 *      ~ ZeGoto ~
 *
 *  Right Ascension motor setup and control
 *
 *********************************************************************
 * FileName:        ra_motor.c
 * Processor:       PIC24FJ256GB106
 * Compiler:        Microchip XC16 v1.21 or higher
 *
 * Copyright � 2014 Sylvain Girard
 *
 * Author               Date        Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Sylvain Girard   	01/05/2011  Creation
 ********************************************************************/

#define RA_MOTOR_C

/* Device header file */
#include <xc.h>
#include "USB\usb_function_cdc.h"
#include "HardwareProfile.h"
#include "GenericTypeDefs.h"

#include "mount.h"
#include "ra_motor.h"
#include "rtcc.h"
#include "dec_motor.h"
#include "lx200_protocol.h"
#include "telescope_movement_commands.h"
#include "utils.h"
#include "home_position_commands.h"

/* Position structure */
ra_t RA;

/* static for RA motor */
static int32_t RARelativeStepPosition;
static uint8_t RADirection;
static uint8_t RANextDirection;

static motor_state_t RAState = MOTOR_STOP; // = sideral rate for RA motor

static uint32_t MotorTimerPeriod;
static uint16_t CurrentSpeed;
static BOOL JustStarted;
//static BOOL FullStep;

static int32_t accel_decel_cnt;

void Timer23Init(void)
{
    T2CON = 0x0008; // 32 bit time, 1:1 prescale, internal clock
    T3CON = 0;
    IPC2bits.T3IP = 6; // Interrupt priority 6 (high)
    IFS0bits.T3IF = 0;
    IEC0bits.T3IE = 1;
}

//typedef struct s_speedlist
//{
//    int32_t speed;
//    int32_t position;
//    uint32_t MotorTimerPeriod;
//    struct s_speedlist *next;
//} t_speedlist;
//
//t_speedlist *lastspeed = NULL;
//t_speedlist *speedlist = NULL;

void __attribute__((interrupt, no_auto_psv)) _T3Interrupt(void)
{
    BOOL NewMotorPeriod = FALSE;

    RA_STEP_IO ^= 1;

    if (RA_STEP_IO == 1)
    {
        if (RAState != MOTOR_STOP)
        {
            RARelativeStepPosition++;
            if (RA.NumberStep)
            {
                if (RA.NumberStep <= RA.DecelPositon && RAState != MOTOR_DECEL)
                {
                    RAState = MOTOR_DECEL;
                    accel_decel_cnt = Mount.AccelPeriod - accel_decel_cnt;
                }
//                if (FullStep == TRUE)
//                {
//                    RA.NumberStep -= 8;
//                }
//                else
                {
                    RA.NumberStep--;
                }
            }
        }
    }

    switch (RAState)
    {
    case MOTOR_STOP:
        break;

    case MOTOR_ACCEL:
        accel_decel_cnt -= MotorTimerPeriod;
        if (accel_decel_cnt <= 0)
        {
            accel_decel_cnt = Mount.AccelPeriod;
            CurrentSpeed++;
            NewMotorPeriod = TRUE;

            //            t_speedlist *newspeed = malloc(sizeof (*newspeed));
            //            newspeed->speed = CurrentSpeed;
            //            newspeed->position = RARelativeStepPosition;
            //            newspeed->MotorTimerPeriod = MotorTimerPeriod;
            //            newspeed->next = NULL;
            //            lastspeed->next = newspeed;
            //            lastspeed = newspeed;

            if (CurrentSpeed >= Mount.CurrentMaxSpeed)
            {
                RA.DecelPositon = RARelativeStepPosition;
                RAState = MOTOR_FULLSPEED;
            }
        }
        break;

    case MOTOR_DECEL:
        accel_decel_cnt -= MotorTimerPeriod;
        if (accel_decel_cnt <= 0)
        {
            accel_decel_cnt = RA.NumberStep ? Mount.AccelPeriod : Mount.DecelPeriod;
            CurrentSpeed--;
            NewMotorPeriod = TRUE;

            //            t_speedlist *newspeed = malloc(sizeof (*newspeed));
            //            newspeed->speed = CurrentSpeed;
            //            newspeed->position = RARelativeStepPosition;
            //            newspeed->MotorTimerPeriod = MotorTimerPeriod;
            //            newspeed->next = NULL;
            //            lastspeed->next = newspeed;
            //            lastspeed = newspeed;

            if (CurrentSpeed == 1)
            {
                RAState = MOTOR_STOP;
                CurrentMove &= ~MOVE_RA;
                RA_DIR_IO = Mount.WestDirection;
                if (RA.IsParking == PARKING)
                {
                    // stop motor after slewing to park position
                    T2CONbits.TON = 0;
                    RA_SLEEP_IO = 0;
                }
            }


        }
        break;

    case MOTOR_FULLSPEED:
        break;
    }

    if (NewMotorPeriod == TRUE)
    {
        MotorTimerPeriod = Mount.SideralHalfPeriod / CurrentSpeed;
//        if (CurrentSpeed >= 4)
//        {
//            MotorTimerPeriod *= 8;
//            if (FullStep == FALSE)
//            {
//                FullStep = TRUE;
//                RA_MODE_IO = 0;
//            }
//        }
//        else if (FullStep == TRUE)
//        {
//            FullStep = FALSE;
//            RA_MODE_IO = 1;
//        }
        PR2 = MotorTimerPeriod & 0xFFFF;
        PR3 = (MotorTimerPeriod >> 16) & 0xFFFF;
        TMR2 = 0;
        TMR3 = 0;
    }

    // Reset interrupt flag
    IFS0bits.T3IF = 0;
}

static void UpdateMotorTimerPeriod()
{
    TMR3HLD = 0;
    TMR2 = 0;
    PR3 = (MotorTimerPeriod >> 16) & 0xFFFF;
    PR2 = MotorTimerPeriod & 0xFFFF;
    TMR2 = 0;
    TMR3 = 0;
}

void RAMotorInit(void)
{
    RA_SLEEP_IO = 0;
    RA_MODE_IO = 1; // 8 microsteps / step
    RA_DIR_IO = Mount.Config.RADefaultDirection;
    RA_STEP_IO = 0;
    RA_DECAY_IO = 1; // fast decay by default
    RA_FAULT_CN = 1;

    MotorTimerPeriod = Mount.SideralHalfPeriod;
    UpdateMotorTimerPeriod();
    RA.StepPerSec = Mount.Config.NbStepMax / (24L * 3600L);

    RTCCReadArray(RTCC_RAM, (BYTE *) & RA.StepPosition, sizeof (RA.StepPosition));
    if (RA.StepPosition < 0 || RA.StepPosition > Mount.Config.NbStepMax)
    {
        RA.StepPosition = 0; // Set default position to north celestial pole
    }
    RA.StepTarget = RA.StepPosition;

    RA.StepStart = RA.StepPosition;

    Timer23Init();

    if (Mount.Config.IsParked)
    {
        RA.IsParking = PARKED;
    }
    else
    {
        RA.IsParking = UNPARKED;
    }
}

void RAStart(void)
{
    RA.IsParking = UNPARKED;
    RA_SLEEP_IO = 1;
    CurrentSpeed = 1;
//    FullStep = FALSE;
    T2CONbits.TON = 1;
    JustStarted = TRUE;
}

void RAAccelerate(void)
{
    if (RAState == MOTOR_STOP)
    {
        RADirection = RANextDirection;
        RA_DIR_IO = RANextDirection;

        RA.StepStart = RA.StepPosition;
        RARelativeStepPosition = 0;

        CurrentSpeed++;
        MotorTimerPeriod = Mount.SideralHalfPeriod / CurrentSpeed;
        UpdateMotorTimerPeriod();

        accel_decel_cnt = Mount.AccelPeriod;
        RAState = MOTOR_ACCEL;
   }
}

void RADecelerate(void)
{
    if (RAState == MOTOR_ACCEL || RAState == MOTOR_FULLSPEED)
    {
        accel_decel_cnt = Mount.DecelPeriod;
        RAState = MOTOR_DECEL;
    }
}

void RAStop(void)
{
    T2CONbits.TON = 0;
    RA_SLEEP_IO = 0;
    //    RA_FAULT_CN = 0;
}

void RASetDirection(uint8_t dir)
{
    RANextDirection = dir;
}

void RAChangeDirection()
{
    if (RAState == MOTOR_STOP)
    {
        RA_DIR_IO = Mount.WestDirection;
    }
}

void UpdateRAStepPosition()
{
    static BOOL LastNorthPoleOVerflow = FALSE;
    int32_t p;
    static BOOL SavePosition = TRUE;
    double ra, lst;
    //unsigned char SideOfPierBefore = Mount.SideOfPier;

    //if (Mount.PierIsFlipping) return;

    if (RAState != MOTOR_STOP || Dec.NorthPoleOVerflow == TRUE)
    {
        RA_DI;
        p = RARelativeStepPosition;
        RA_EI;

        if (RADirection == Mount.WestDirection)
        {
            RA.StepPosition = RA.StepStart - p;
            if (RA.StepPosition < 0) RA.StepPosition += Mount.Config.NbStepMax;
        }
        else
        {
            RA.StepPosition = RA.StepStart + p;
            if (RA.StepPosition > Mount.Config.NbStepMax) RA.StepPosition -= Mount.Config.NbStepMax;
        }

        if (LastNorthPoleOVerflow == FALSE && Dec.NorthPoleOVerflow == TRUE)
        {
            RA.StepStart += Mount.Config.NbStepMax / 2L;
            RA.StepStart %= Mount.Config.NbStepMax;
        }
        LastNorthPoleOVerflow = Dec.NorthPoleOVerflow;
        SavePosition = TRUE;
    }
    else if (SavePosition == TRUE)
    {
        RTCCWriteArray(RTCC_RAM, (BYTE*) &RA.StepPosition, sizeof (RA.StepPosition));
        SavePosition = FALSE;
    }

    if (RA.IsParking == PARKING && RAState == MOTOR_STOP)
    {
        RA.IsParking = PARKED;
        if (Dec.IsParking == PARKED)
        {
            Mount.Config.IsParked = TRUE;
            SaveMountConfig(&Mount.Config);
        }
    }

    lst = ComputeSideralTime();

    ra = lst - (double)RA.StepPosition / (3600.0 * (double)RA.StepPerSec);
    if (ra < 0.0) ra +=24.0;

    if (ra > 18.0)
    {
        Mount.SideOfScope = PIER_WEST;
        Mount.CalculatedSideOfPier = PIER_WEST;
    }
    else if (ra > 12)
    {
        Mount.SideOfScope = PIER_EAST;
        Mount.CalculatedSideOfPier = PIER_WEST;
    }
    else if (ra > 6)
    {
        Mount.SideOfScope = PIER_WEST;
        Mount.CalculatedSideOfPier = PIER_EAST;
    }
    else
    {
        Mount.SideOfScope = PIER_EAST;
        Mount.CalculatedSideOfPier = PIER_EAST;
    }

    if (Mount.AutomaticSideOfPier == 1 && (JustStarted || !RAIsMotorStopped() || !DecIsMotorStopped()))
    {
        Mount.SideOfPier = Mount.CalculatedSideOfPier;
        JustStarted = FALSE;
    
        /*if (Mount.SideOfPier != SideOfPierBefore)
        {
            FlipSideOfPier();
        }*/
    }
    else if (Mount.CalculatedSideOfPier != Mount.SideOfPier)
    {
        Mount.SideOfScope = Mount.SideOfScope == PIER_EAST ? PIER_WEST : PIER_EAST;
    }
}

inline int RAIsMotorStopped()
{
    return (RAState == MOTOR_STOP);
}

void RAGuideWest()
{
    MotorTimerPeriod = Mount.SideralHalfPeriod * 10 / (10 + Mount.Config.GuideSpeed);
    UpdateMotorTimerPeriod();
}

void RAGuideEast()
{
    MotorTimerPeriod = Mount.SideralHalfPeriod * 10 / (10 - Mount.Config.GuideSpeed);
    UpdateMotorTimerPeriod();
}

void RAGuideStop()
{
    MotorTimerPeriod = Mount.SideralHalfPeriod;
    UpdateMotorTimerPeriod();
}
