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

volatile BYTE bPadState = 0;

void InputsInit(void)
{
    // PAD_S1
    TRISBbits.TRISB6 = 1;
    CNEN2bits.CN24IE = 1; // interrupt change notification enable
    CNPU2bits.CN24PUE = 1; // pull-up
    // PAD_S2
    TRISBbits.TRISB1 = 1;
    CNEN1bits.CN3IE = 1; // interrupt change notification enable
    CNPU1bits.CN3PUE = 1; // pull-up
    // PAD_S3
    TRISBbits.TRISB8 = 1;
    CNEN2bits.CN26IE = 1; // interrupt change notification enable
    CNPU2bits.CN26PUE = 1; // pull-up
    // PAD_S4
    TRISBbits.TRISB0 = 1;
    CNEN1bits.CN2IE = 1; // interrupt change notification enable
    CNPU1bits.CN2PUE = 1; // pull-up
    // PAD_S5
    TRISBbits.TRISB3 = 1;
    CNEN1bits.CN5IE = 1; // interrupt change notification enable
    CNPU1bits.CN5PUE = 1; // pull-up
    // PAD_S6
    TRISBbits.TRISB7 = 1;
    CNEN2bits.CN25IE = 1; // interrupt change notification enable
    CNPU2bits.CN25PUE = 1; // pull-up
    // PAD_SWITCH
    TRISBbits.TRISB2 = 1;
    CNEN1bits.CN4IE = 1; // interrupt change notification enable
    CNPU1bits.CN4PUE = 1; // pull-up

    IFS1bits.CNIF = 0;
    IPC4bits.CNIP = 7;
    IEC1bits.CNIE = 1;

    TRISBbits.TRISB12 = 1;  // DEC+
    TRISBbits.TRISB13 = 1;  // RA+
    TRISBbits.TRISB14 = 1;  // RA-
    TRISBbits.TRISB15 = 1;  // DEC-

//    if (!PORTBbits.RB6) bPadState |= PAD_S1;
//    if (!PORTBbits.RB1) bPadState |= PAD_S2;
//    if (!PORTBbits.RB8) bPadState |= PAD_S3;
//    if (!PORTBbits.RB0) bPadState |= PAD_S4;
//    if (!PORTBbits.RB3) bPadState |= PAD_S5;
//    if (!PORTBbits.RB7) bPadState |= PAD_S6;
//    if (PORTBbits.RB2) bPadState |= PAD_SWITCH;
}

void __attribute__((interrupt, no_auto_psv)) _CNInterrupt(void)
{
    static unsigned int dwLastCNTick = 0;
    unsigned int dwCNTick = TMR1;
    unsigned int dwCNInterval = dwLastCNTick < dwCNTick ? dwCNTick - dwLastCNTick : dwLastCNTick - dwCNTick;

    if (dwCNInterval > 1000)
    {
        bPadState = 0;
        if (!PORTBbits.RB6) bPadState |= PAD_S1;
        if (!PORTBbits.RB1) bPadState |= PAD_S2;
        if (!PORTBbits.RB8) bPadState |= PAD_S3;
        if (!PORTBbits.RB0) bPadState |= PAD_S4;
        if (!PORTBbits.RB3) bPadState |= PAD_S5;
        if (!PORTBbits.RB7) bPadState |= PAD_S6;
        if (!PORTBbits.RB2) bPadState |= PAD_SWITCH;
    }
    dwLastCNTick = dwCNTick;

    IFS1bits.CNIF = 0;
}