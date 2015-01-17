/*********************************************************************
 *
 *      ~ OpenGoto ~
 *
 *  TODO add description
 *
 *********************************************************************
 * FileName:        reticule.c
 * Processor:       PIC24FJ256GB106
 * Compiler:        Microchip XC16 v1.21 or higher
 *
 * Copyright © 2014 Sylvain Girard
 *
 * Author               Date        Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Sylvain Girard       26 mai 2014 Creation
 ********************************************************************/

/* Device header file */
#include <xc.h>
#include"mount.h"

void ReticuleInit()
{
    OC5CON1 = 0; /* It is a good practice to clear off the control bits initially */
    OC5CON2 = 0;
    OC5CON1bits.OCTSEL = 0x04; /* This selects timer1, the tick timer, as the clock input to the OC module */
    OC5R = Mount.Config.ReticuleBrightness;
    OC5RS = 1230;
    OC5CON2bits.SYNCSEL = 0x1F; /* This selects the synchronization source as itself */
    OC5CON1bits.OCM = 6; /* This selects and starts the Edge Aligned PWM mode*/
}

inline void ReticuleOn()
{
    OC5CON1bits.OCM = 6;
}

inline void ReticuleOff()
{
    OC5CON1bits.OCM = 0;
}

void IncreaseReticuleBrightness()
{
    if (OC5R < OC5RS) OC5R += 41;
    Mount.Config.ReticuleBrightness = OC5R;
    SaveMountConfig(&Mount.Config);
}

void DecreaseReticuleBrightness()
{
    if (OC5R > 0) OC5R -= 41;
    Mount.Config.ReticuleBrightness = OC5R;
    SaveMountConfig(&Mount.Config);
}
