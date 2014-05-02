/*********************************************************************
 *
 *      ~ OpenGoto ~
 *
 *  TODO add description
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

#include "HardwareProfile.h"
#include "ra_motor.h"

motor_state_t DecState = MOTOR_STOP; // = sideral rate for RA motor

static uint16_t CurrentSpeed;
static uint16_t t3int_cnt;
static uint16_t accel_decel_cnt;

void Timer3Init(void)
{
    DEC_DIR_TRIS = OUTPUT_PIN;
    DEC_SLEEP_TRIS = OUTPUT_PIN;
    DEC_STEP_TRIS = OUTPUT_PIN;
    DEC_STEP_IO = 0;

    t3int_cnt = MaxSpeed;
    T3CON = 0x0000; // 16 bit time, 1:1 prescale, internal clock
    PR3 = MotorTimerPeriod;
    IPC2bits.T3IP = 6; // Interrupt priority 7 (high)
    IFS0bits.T3IF = 0;
    IEC0bits.T3IE = 1;
}

void __attribute__((interrupt, no_auto_psv)) _T3Interrupt(void)
{
    t3int_cnt--;
    switch (DecState)
    {
    case MOTOR_STOP:
        if (t3int_cnt == 0)
        {
            t3int_cnt = MaxSpeed;
            DEC_STEP_IO ^= 1;
            if (DEC_HOME_IO == 0)
            {
                DEC_STEP_IO = 0;
                T3CONbits.TON = 0;
                DEC_SLEEP_IO = 0;
            }
        }
        break;

    case MOTOR_ACCEL:
        if (t3int_cnt == 0)
        {
            t3int_cnt = CurrentSpeed;
            DEC_STEP_IO ^= 1;
        }

        accel_decel_cnt--;
        if (accel_decel_cnt == 0)
        {
            accel_decel_cnt = AccelPeriod;
            CurrentSpeed--;
            if (CurrentSpeed == 4)
            {
                DecState = MOTOR_NOACC;
            }
        }
        break;

    case MOTOR_DECEL:
        if (t3int_cnt == 0)
        {
            t3int_cnt = CurrentSpeed;
            DEC_STEP_IO ^= 1;
        }

        accel_decel_cnt--;
        if (accel_decel_cnt == 0)
        {
            accel_decel_cnt = DecelPeriod;
            CurrentSpeed++;
            if (CurrentSpeed == MaxSpeed)
            {
                DecState = MOTOR_STOP;
            }
        }
        break;

    case MOTOR_NOACC:
        if (t3int_cnt == 0)
        {
            t3int_cnt = CurrentSpeed;
            DEC_STEP_IO ^= 1;
        }
    }

    // Reset interrupt flag
    IFS0bits.T3IF = 0;
}

void DecMotorInit(void)
{
    DEC_HOME_TRIS = INPUT_PIN;
    DEC_SLEEP_TRIS = OUTPUT_PIN;
    DEC_DIR_TRIS = OUTPUT_PIN;
    DEC_STEP_TRIS = OUTPUT_PIN;
    DEC_MODE_TRIS = OUTPUT_PIN;
    DEC_FAULT_TRIS = INPUT_PIN;

    DEC_SLEEP_IO = 0;
    DEC_MODE_IO = 1; // 8 microsteps / step
    DEC_DIR_IO = 0;
    DEC_STEP_IO = 0;

    Timer3Init();
}

void DecStart(void)
{
    if (DecState == MOTOR_STOP)
    {
        DEC_SLEEP_IO = 1;
        CurrentSpeed = MaxSpeed;
        t3int_cnt = CurrentSpeed;
        accel_decel_cnt = AccelPeriod;
        DecState = MOTOR_ACCEL;
        T3CONbits.TON = 1;
    }
}

void DecDecelerate(void)
{
    if (DecState == MOTOR_ACCEL || DecState == MOTOR_NOACC)
    {
        CurrentSpeed++;
        t3int_cnt = CurrentSpeed;
        accel_decel_cnt = DecelPeriod;
        DecState = MOTOR_DECEL;
    }
}

void DecStop(void)
{
    T2CONbits.TON = 0;
    DEC_SLEEP_IO = 0;
}

void DecDirection(uint8_t dir)
{
    RA_DIR_IO = dir;
}
