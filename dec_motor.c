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
 * Copyright � 2014 Sylvain Girard
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

motor_state_t DecState = MOTOR_STOP; // = sideral rate for Dec motor

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
    IPC2bits.T3IP = 6; // Interrupt priority 7 (high)
    IFS0bits.T3IF = 0;
    IEC0bits.T3IE = 1;
}

void __attribute__((interrupt, no_auto_psv)) _T3Interrupt(void)
{
    if (tmodulo != 0)
    {
        tint_cnt--;
        if (tint_cnt == 0)
        {
            PR3 = tmodulo;
        }
        else if (tint_cnt == 0xFFFF)
        {
            if (SideralPeriod & 1) DEC_STEP_IO ^= 1; // if SideralHalfPeriod is odd
            tint_cnt = tlap | DEC_STEP_IO;
            PR3 = 0xFFFF;
        }
    }
    else
    {
        DEC_STEP_IO ^= 1;
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
            if (CurrentSpeed >= MaxSpeed)
            {
                DecState = MOTOR_NOACC;
            }

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
        break;

    case MOTOR_DECEL:
        accel_decel_cnt -= PR3;
        if (accel_decel_cnt <= 0)
        {
            accel_decel_cnt = DecelPeriod;
            CurrentSpeed--;
            if (CurrentSpeed == 1)
            {
                DecState = MOTOR_STOP;
            }

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
        break;

    case MOTOR_NOACC:
        break;
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

    MotorTimerPeriod = SideralHalfPeriod;

    Timer3Init();
}

void DecStart(void)
{
    if (DecState == MOTOR_STOP)
    {
        DEC_SLEEP_IO = 1;
        CurrentSpeed = 1;
        accel_decel_cnt = AccelPeriod;
        DecState = MOTOR_ACCEL;
        T3CONbits.TON = 1;
    }
}

void DecDecelerate(void)
{
    if (DecState == MOTOR_ACCEL || DecState == MOTOR_NOACC)
    {
        accel_decel_cnt = DecelPeriod;
        DecState = MOTOR_DECEL;
    }
}

void DecStop(void)
{
    T3CONbits.TON = 0;
    DEC_SLEEP_IO = 0;
}

void DecDirection(uint8_t dir)
{
    DEC_DIR_IO = dir;
}
