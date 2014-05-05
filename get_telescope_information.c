/*********************************************************************
 *
 *      ~ OpenGoto ~
 *
 *  LX200 : G - Get Telescope Information
 *
 *********************************************************************
 * FileName:        get_telescope_information.c
 * Processor:       PIC24FJ256GB106
 * Compiler:        Microchip XC16 v1.21 or higher
 *
 * Copyright © 2014 Sylvain Girard
 *
 * Author               Date        Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Sylvain Girard       2 mai 2014 Creation
 ********************************************************************/

/* Device header file */
#include <xc.h>
#include "GenericTypeDefs.h"
#include <stdio.h>
#include <string.h>

#include "main.h"
#include "ra_motor.h"
#include "dec_motor.h"
#include "lx200_protocol.h"

BOOL LX200Precise = FALSE;

/******************************************************************************
 * Function:        void PrecisionToggle()
 * PreCondition:    None
 * Input:           None
 * Output:          None
 * Side Effects:    None
 * Overview:        Toggle between low/hi precision positions
 *****************************************************************************/
void PrecisionToggle()
{
    LX200Precise = LX200Precise ? FALSE : TRUE;
}

void GetPrecision()
{
    if (LX200Precise)
        strcpy(LX200Response, "HIGH PRECISION#");
    else
        strcpy(LX200Response, "LOW PRECISION#");
}

/******************************************************************************
 * Function:        void SendRA(int32_t StepPosition_P)
 * PreCondition:    None
 * Input:           int32_t StepPosition_P : position in number of steps
 * Output:          None
 * Side Effects:    None
 * Overview:        Convert step position to right ascension and send it to
 *                  host
 *****************************************************************************/
void SendRA(int32_t StepPosition_P)
{
    int32_t a = 3600L * RAStepPerSec;
    int32_t b = 60L * RAStepPerSec;

    int32_t hours = StepPosition_P / a;
    int32_t diff = RAStepPosition - RAStepTarget;

    if (LX200Precise)
    {
        int32_t minutes = (StepPosition_P % a) / b;
        int32_t seconds = ((StepPosition_P % a) % b) / 100;

        sprintf(LX200Response, "%02li:%02li:%02li# %li", hours, minutes, seconds, diff);
    }
    else
    {
        int32_t minutes = 10L * (StepPosition_P % a) / (float) b;

        sprintf(LX200Response, "%02li:%02li.%01li# %li", hours, minutes / 10, minutes % 10, diff);
    }
}

/******************************************************************************
 * Function:        void GetTelescopeRA()
 * PreCondition:    None
 * Input:           None
 * Output:          None
 * Side Effects:    None
 * Overview:        Get Telescope Right Ascension
 *****************************************************************************/
void GetTelescopeRA()
{
    SendRA(RAStepPosition);
}

/******************************************************************************
 * Function:        void GetCurrentTargetRA()
 * PreCondition:    None
 * Input:           None
 * Output:          None
 * Side Effects:    None
 * Overview:        Get current/target object Right Ascension
 *****************************************************************************/
void GetCurrentTargetRA()
{
    SendRA(RAStepTarget);
}

/******************************************************************************
 * Function:        void SendDeclination()
 * PreCondition:    None
 * Input:           int32_t StepPosition_P : position in number of steps
 * Output:          None
 * Side Effects:    None
 * Overview:        Convert step position to declinaison and send it to host
 *****************************************************************************/
void SendDeclination(int32_t StepPosition_P)
{
    int32_t DecPos_L;
    char signe;

    if (StepPosition_P < 0)
    {
        DecPos_L = -StepPosition_P;
        signe = '-';
    }
    else
    {
        DecPos_L = StepPosition_P;
        signe = '+';
    }

    int32_t degrees = DecPos_L / DecStepPerDegree;
    int32_t diff = DecStepPosition - DecStepTarget;

    if (LX200Precise)
    {
        int32_t minutes = (DecPos_L % DecStepPerDegree) / DecStepPerMinute;
        int32_t seconds = ((DecPos_L % DecStepPerDegree) % DecStepPerMinute) / DecStepPerSecond;

        sprintf(LX200Response, "%c%02li:%02li:%02li# %li", signe, degrees, minutes, seconds, diff);
    }
    else
    {
        int32_t minutes = 10L * (DecPos_L % DecStepPerDegree) / DecStepPerMinute;
        sprintf(LX200Response, "%c%02li:%02li.%01li# %li", signe, degrees, minutes / 10, minutes % 10, diff);
    }
}

/******************************************************************************
 * Function:        void GetTelescopeDeclination()
 * PreCondition:    None
 * Input:           None
 * Output:          None
 * Side Effects:    None
 * Overview:        Get Telescope Right Declination
 *****************************************************************************/
void GetTelescopeDeclination()
{
    SendDeclination(DecStepPosition);
}

/******************************************************************************
 * Function:        void GetTelescopeDeclination()
 * PreCondition:    None
 * Input:           None
 * Output:          None
 * Side Effects:    None
 * Overview:        Get Telescope Right Declination
 *****************************************************************************/
void GetCurrentTargetDeclination()
{
    SendDeclination(DecStepTarget);
}

/******************************************************************************
 * Function:        void GetTelescopeAzimuth()
 * PreCondition:    None
 * Input:           None
 * Output:          None
 * Side Effects:    None
 * Overview:        Get Telescope Azimuth. Not supported, send 0
 *****************************************************************************/
void GetTelescopeAzimuth()
{
    if (LX200Precise)
    {
        strcpy(LX200Response, "000*00#");
    }
    else
    {
        strcpy(LX200Response, "000*00'00#");
    }
}

/******************************************************************************
 * Function:        void GetTelescopeAltitude()
 * PreCondition:    None
 * Input:           None
 * Output:          None
 * Side Effects:    None
 * Overview:        Get Telescope Altitude. Not supported, send 0
 *****************************************************************************/
void GetTelescopeAltitude()
{
    if (LX200Precise)
    {
        strcpy(LX200Response, "+00*00#");
    }
    else
    {
        strcpy(LX200Response, "+00*00'00#");
    }
}

/******************************************************************************
 * Function:        void GetSiderealTime()
 * PreCondition:    None
 * Input:           None
 * Output:          None
 * Side Effects:    None
 * Overview:        Get Sedireal Time. Not supported, send 0
 *****************************************************************************/
void GetSiderealTime()
{
    strcpy(LX200Response, "00:00:00#");
}

