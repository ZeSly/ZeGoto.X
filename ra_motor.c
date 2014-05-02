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

/* Mount specific settings */
int32_t NbStepMax = 8640000UL;
uint32_t SideralPeriod = 159563UL;
uint32_t SideralHalfPeriod = 159563UL / 2;
uint16_t MaxSpeed = 120;

/* Acceleration/decelation varibles and constant */
int32_t AccelTime = 4; // seconds
int32_t DecelTime = 1; // seconds
int32_t AccelPeriod;
int32_t DecelPeriod;

/* Position variables */
int32_t RAStepPosition = 0; // Set default position to north celestial pole
uint8_t WestDirection = 0;
uint8_t EastDirection = !WestDirection;

/* static for RA motor */
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
    IPC1bits.T2IP = 6; // Interrupt priority 7 (highest)
    IFS0bits.T2IF = 0;
    IEC0bits.T2IE = 1;
}

void __attribute__((interrupt, no_auto_psv)) _T2Interrupt(void)
{
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

            // if SideralHalfPeriod is odd
            if (SideralPeriod & 1) tint_cnt = tlap | RA_STEP_IO;

            PR2 = 0xFFFF;
        }
    }
    else
    {
        RA_STEP_IO ^= 1;
    }

    switch (RAState)
    {
    case MOTOR_STOP:
        break;

    case MOTOR_ACCEL:
        accel_decel_cnt -= PR2;
        if (accel_decel_cnt <= 0)
        {
            accel_decel_cnt = AccelPeriod;
            CurrentSpeed++;
            if (CurrentSpeed >= MaxSpeed)
            {
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
            accel_decel_cnt = DecelPeriod;
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
        accel_decel_cnt = AccelPeriod;
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

void RADirection(uint8_t dir)
{
    RA_DIR_IO = dir;
}
