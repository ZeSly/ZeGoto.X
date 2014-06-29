/*********************************************************************
 *
 *      ~ OpenGoto ~
 *
 *  Input PAD and Guide port
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
#include "mount.h"
#include "ra_motor.h"
#include "dec_motor.h"
#include "telescope_movement_commands.h"
#include "reticule.h"
#include "home_position_commands.h"

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

    GUIDE_RAP_TRIS = INPUT_PIN;
    GUIDE_RAM_TRIS = INPUT_PIN;
    GUIDE_DECP_TRIS = INPUT_PIN;
    GUIDE_DECM_TRIS = INPUT_PIN;

    GUIDE_RAP_PULLUP = 1;
    GUIDE_RAM_PULLUP = 1;
    GUIDE_DECP_PULLUP = 1;
    GUIDE_DECM_PULLUP = 1;

    GUIDE_RAP_CN = 1;
    GUIDE_RAM_CN = 1;
    GUIDE_DECP_CN = 1;
    GUIDE_DECM_CN = 1;

    IFS1bits.CNIF = 0;
    IPC4bits.CNIP = 7;
    IEC1bits.CNIE = 1;
}

void __attribute__((interrupt, no_auto_psv)) _CNInterrupt(void)
{
    BOOL fault = FALSE;

    if (RA_FAULT_IO == 0 && RA_SLEEP_IO == 1)
    {
        T2CONbits.TON = 0;
        RA_SLEEP_IO = 0;
        LED2_IO = 1;
        fault = TRUE;
    }
    if (DEC_FAULT_IO == 0 && DEC_SLEEP_IO == 1)
    {
        T3CONbits.TON = 0;
        DEC_SLEEP_IO = 0;
        LED2_IO = 1;
        fault = TRUE;
    }

    if (fault == FALSE)
    {
        if (GUIDE_RAP_IO == 0)
        {
            RAGuideWest();
        }
        if (GUIDE_RAM_IO == 0)
        {
            RAGuideEast();
        }
        if (GUIDE_RAP_IO == 1 && GUIDE_RAM_IO == 1)
        {
            RAGuideStop();
        }

        if (GUIDE_DECP_IO == 0)
        {
            DecGuideNorth();
        }
        if (GUIDE_DECM_IO == 0)
        {
            DecGuideSouth();
        }
        if (GUIDE_DECP_IO == 1 && GUIDE_DECM_IO == 1)
        {
            DecGuideStop();
        }
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
                ManualMaxSpeed = Mount.Config.MaxSpeed;
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
                    if (Mount.Config.IsParked == 1 && PadState.PAD_SWITCH == 0)
                    {
                        // Unpark when a directionnal key pad is pressed
                        homeUnpark();
                    }

                    if (PadState.PAD_S3 == 1)
                    {
                        SavedMaxSpeed = Mount.CurrentMaxSpeed;
                        Mount.CurrentMaxSpeed = ManualMaxSpeed;
                        RASetDirection(Mount.WestDirection);
                        RAAccelerate();

                    }
                    else if (PadState.PAD_S4 == 1)
                    {
                        SavedMaxSpeed = Mount.CurrentMaxSpeed;
                        Mount.CurrentMaxSpeed = ManualMaxSpeed;
                        RASetDirection(Mount.EastDirection);
                        RAAccelerate();

                    }
                    else if (PadState.PAD_S3 == 0 || PadState.PAD_S4 == 0)
                    {
                        RADecelerate();
                    }


                    if (PadState.PAD_S5 == 1)
                    {
                        SavedMaxSpeed = Mount.CurrentMaxSpeed;
                        Mount.CurrentMaxSpeed = ManualMaxSpeed;
                        DecSetDirection(Mount.NorthDirection);
                        DecAccelerate();

                    }
                    else if (PadState.PAD_S6 == 1)
                    {
                        SavedMaxSpeed = Mount.CurrentMaxSpeed;
                        Mount.CurrentMaxSpeed = ManualMaxSpeed;
                        DecSetDirection(Mount.SouthDirection);
                        DecAccelerate();
                    }
                    else if (PadState.PAD_S5 == 0 || PadState.PAD_S6 == 0)
                    {
                        DecDecelerate();
                    }

                    if (PadState.PAD_S3 == 0 && PadState.PAD_S4 == 0 &&
                            PadState.PAD_S5 == 0 && PadState.PAD_S6 == 0 &&
                            SavedMaxSpeed != 0)
                    {
                        Mount.CurrentMaxSpeed = SavedMaxSpeed;
                    }
                }
                if (PadState.PAD_S2 == 1)
                {
                    Mount.Config.DecDefaultDirection = !Mount.Config.DecDefaultDirection;
                    Mount.NorthDirection = Mount.Config.DecDefaultDirection;
                    Mount.SouthDirection = !Mount.Config.DecDefaultDirection;
                    SaveMountConfig(&Mount.Config);
                }
            }

            if (PadState.PAD_S1 == 1 && PadState.PAD_S5 == 1)
            {
                IncreaseReticuleBrightness();
            }
            if (PadState.PAD_S1 == 1 && PadState.PAD_S6 == 1)
            {
                DecreaseReticuleBrightness();
            }
        }
    }

    lastPadState.i = readPadState.i;
}