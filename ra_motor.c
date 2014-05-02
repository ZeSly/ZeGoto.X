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

uint32_t SideralPeriod = 159563;
uint16_t MaxSpeed = 120;
uint16_t MotorTimerPeriod;

uint32_t AccelTime = 4; // seconds
uint32_t DecelTime = 1; // seconds

uint16_t AccelPeriod;
uint16_t DecelPeriod;

motor_state_t RAState = MOTOR_STOP; // = sideral rate for RA motor

static uint16_t CurrentSpeed;
static uint16_t t2modulo;
static uint16_t t2int_cnt;
static uint16_t accel_decel_cnt;

void Timer2Init(void)
{
    t2modulo = SideralPeriod % MaxSpeed / 2;
    t2int_cnt = MaxSpeed;
    T2CON = 0x0000; // 16 bit time, 1:1 prescale, internal clock
    PR2 = MotorTimerPeriod;
    IPC1bits.T2IP = 7; // Interrupt priority 7 (highest)
    IFS0bits.T2IF = 0;
    IEC0bits.T2IE = 1;
}

void __attribute__((interrupt, no_auto_psv)) _T2Interrupt(void)
{
    t2int_cnt--;

    switch (RAState)
    {
    case MOTOR_STOP:
        if (t2int_cnt == 0)
        {
            PR2 = t2modulo;
        }
        else if (t2int_cnt == 0xFFFF)
        {
            t2int_cnt = CurrentSpeed;
            RA_STEP_IO ^= 1;
            PR2 = MotorTimerPeriod;
        }
        break;

    case MOTOR_ACCEL:
        if (t2int_cnt == 0)
        {
            t2int_cnt = CurrentSpeed;
            RA_STEP_IO ^= 1;
        }

        accel_decel_cnt--;
        if (accel_decel_cnt == 0)
        {
            accel_decel_cnt = AccelPeriod;
            CurrentSpeed--;
            if (CurrentSpeed == 4)
            {
                RAState = MOTOR_NOACC;
            }
        }
        break;

    case MOTOR_DECEL:
        if (t2int_cnt == 0)
        {
            t2int_cnt = CurrentSpeed;
            RA_STEP_IO ^= 1;
        }

        accel_decel_cnt--;
        if (accel_decel_cnt == 0)
        {
            accel_decel_cnt = DecelPeriod;
            CurrentSpeed++;
            if (CurrentSpeed == MaxSpeed)
            {
                RAState = MOTOR_STOP;
            }
        }
        break;

    case MOTOR_NOACC:
        if (t2int_cnt == 0)
        {
            t2int_cnt = CurrentSpeed;
            RA_STEP_IO ^= 1;
        }
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

    MotorTimerPeriod = SideralPeriod / MaxSpeed / 2;
    AccelPeriod = GetPeripheralClock() / MotorTimerPeriod / MaxSpeed * AccelTime;
    DecelPeriod = GetPeripheralClock() / MotorTimerPeriod / MaxSpeed * DecelTime;
    CurrentSpeed = MaxSpeed;

    Timer2Init();
}

void RAStart(void)
{
    RA_SLEEP_IO = 1;
    T2CONbits.TON = 1;
}

void RAAccelerate(void)
{
    if (RAState == MOTOR_STOP)
    {
        CurrentSpeed = MaxSpeed;
        t2int_cnt = CurrentSpeed;
        accel_decel_cnt = AccelPeriod;
        RAState = MOTOR_ACCEL;
    }
}

void RADecelerate(void)
{
    if (RAState == MOTOR_ACCEL || RAState == MOTOR_NOACC)
    {
        CurrentSpeed++;
        t2int_cnt = CurrentSpeed;
        accel_decel_cnt = DecelPeriod;
        RAState = MOTOR_DECEL;
    }
}

void RAStop(void)
{
    T2CONbits.TON = 0;
    RA_SLEEP_IO = 0;
}

void RADirection(uint8_t dir)
{
    RA_DIR_IO = dir;
}
