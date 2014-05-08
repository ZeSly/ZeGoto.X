/*********************************************************************
 *
 *      ~ OpenGoto ~
 *
 *  Right Ascension motor setup and control
 *
 *********************************************************************
 * FileName:        input.c
 * Processor:       PIC24FJ256GB106
 * Compiler:        Microchip XC16 v1.21 or higher
 *
 * Copyright © 2014 Sylvain Girard
 *
 * Author               Date        Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Sylvain Girard   	01/05/2011  Creation
 ********************************************************************/

/* Device header file */
#include <xc.h>

#include "HardwareProfile.h"
#include "GenericTypeDefs.h"
#include "inputs.h"
#include "TCPIP Stack/Tick.h"
#include "ra_motor.h"
#include "dec_motor.h"
#include "telescope_movement_commands.h"

pad_t PadState;

void InputsInit(void)
{
    // PAD_S1
    TRISBbits.TRISB6 = 1;
    CNPU2bits.CN24PUE = 1; // pull-up
    // PAD_S2
    TRISBbits.TRISB1 = 1;
    CNPU1bits.CN3PUE = 1; // pull-up
    // PAD_S3
    TRISBbits.TRISB8 = 1;
    CNPU2bits.CN26PUE = 1; // pull-up
    // PAD_S4
    TRISBbits.TRISB0 = 1;
    CNPU1bits.CN2PUE = 1; // pull-up
    // PAD_S5
    TRISBbits.TRISB3 = 1;
    CNPU1bits.CN5PUE = 1; // pull-up
    // PAD_S6
    TRISBbits.TRISB7 = 1;
    CNPU2bits.CN25PUE = 1; // pull-up
    // PAD_SWITCH
    TRISBbits.TRISB2 = 1;
    CNPU1bits.CN4PUE = 1; // pull-up
    PadState.i = 0;

    IFS1bits.CNIF = 0;
    IPC4bits.CNIP = 7;
    IEC1bits.CNIE = 1;

    TRISBbits.TRISB12 = 1;  // DEC+
    TRISBbits.TRISB13 = 1;  // RA+
    TRISBbits.TRISB14 = 1;  // RA-
    TRISBbits.TRISB15 = 1;  // DEC-

}

void __attribute__((interrupt, no_auto_psv)) _CNInterrupt(void)
{
    if (RA_FAULT_IO == 0 && RA_SLEEP_IO == 1)
    {
        T2CONbits.TON = 0;
        RA_SLEEP_IO = 0;
        LED2_IO = 1;
    }
    if (DEC_FAULT_IO == 0 && DEC_SLEEP_IO == 1)
    {
        T3CONbits.TON = 0;
        DEC_SLEEP_IO = 0;
        LED2_IO = 1;
    }

    IFS1bits.CNIF = 0;
}

#define DEBOUNCE_DELAY (TICK_SECOND * 50 / 1000)

static pad_t lastPadState;
static QWORD lastDebounceTime = 0;

void UpdatePadState()
{
    pad_t readPadState;

    readPadState.i = 0;
    if (!PORTBbits.RB6) readPadState.PAD_S1 = 1;
    if (!PORTBbits.RB1) readPadState.PAD_S2 = 1;
    if (!PORTBbits.RB8) readPadState.PAD_S3 = 1;
    if (!PORTBbits.RB0) readPadState.PAD_S4 = 1;
    if (!PORTBbits.RB3) readPadState.PAD_S5 = 1;
    if (!PORTBbits.RB7) readPadState.PAD_S6 = 1;
    if (!PORTBbits.RB2) readPadState.PAD_SWITCH = 1;

    if (readPadState.i != lastPadState.i)
    {
        lastDebounceTime = TickGet();
    }

    if ((TickGet() - lastDebounceTime) > DEBOUNCE_DELAY)
    {
        if (readPadState.i != PadState.i)
        {
            uint16_t ManualMaxSpeed;

            PadState.i = readPadState.i;

            if (PadState.PAD_SWITCH == 1)
            {
                ManualMaxSpeed = MaxSpeed;
            }
            else
            {
                ManualMaxSpeed = 10; // Centering speed
            }

            if (!CurrentMove)
            {
                static uint16_t SavedMaxSpeed = 0;

                if (PadState.PAD_S1 == 0 && PadState.PAD_S2 == 0)
                {
                    if (PadState.PAD_S3 == 1)
                    {
                        SavedMaxSpeed = CurrentMaxSpeed;
                        CurrentMaxSpeed = ManualMaxSpeed;
                        RASetDirection(WestDirection);
                        RAAccelerate();

                    }
                    else if (PadState.PAD_S4 == 1)
                    {
                        SavedMaxSpeed = CurrentMaxSpeed;
                        CurrentMaxSpeed = ManualMaxSpeed;
                        RASetDirection(EastDirection);
                        RAAccelerate();

                    }
                    else if (PadState.PAD_S3 == 0 || PadState.PAD_S4 == 0)
                    {
                        RADecelerate();
                    }


                    if (PadState.PAD_S5 == 1)
                    {
                        SavedMaxSpeed = CurrentMaxSpeed;
                        CurrentMaxSpeed = ManualMaxSpeed;
                        DecSetDirection(NorthDirection);
                        DecAccelerate();

                    }
                    else if (PadState.PAD_S6 == 1)
                    {
                        SavedMaxSpeed = CurrentMaxSpeed;
                        CurrentMaxSpeed = ManualMaxSpeed;
                        DecSetDirection(SouthDirection);
                        DecAccelerate();
                    }
                    else if (PadState.PAD_S5 == 0 || PadState.PAD_S6 == 0)
                    {
                        DecDecelerate();
                    }

                    if (PadState.PAD_S3 == 0 && PadState.PAD_S4 == 0 &&
                            PadState.PAD_S5 == 0 && PadState.PAD_S6 == 0)
                    {
                        CurrentMaxSpeed = SavedMaxSpeed;
                    }
                }
                if (PadState.PAD_S2 == 1)
                {
                    NorthDirection = NorthDirection ? 0 : 1;
                    SouthDirection = SouthDirection ? 0 : 1;
                }
            }
        }
    }

    lastPadState.i = readPadState.i;
}