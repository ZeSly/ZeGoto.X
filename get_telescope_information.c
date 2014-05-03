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
 * Sylvain Girard   	2 mai 2014 Creation
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
 * Input:		    None
 * Output:		    None
 * Side Effects:    None
 * Overview:		Toggle between low/hi precision positions
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
 * Function:        void SendRA(long StepPosition_P)
 * PreCondition:    None
 * Input:		    long StepPosition_P : position in number of steps
 * Output:		    None
 * Side Effects:    None
 * Overview:		Convert step position to right ascension and send it to
 *                  host
 *****************************************************************************/
void SendRA(long StepPosition_P)
{
    char RAPosition_L[16];
    long a = 3600L * RAStepPerSec;
    long b = 60L * RAStepPerSec;

    long hours = StepPosition_P / a;
    if (LX200Precise)
    {
        long minutes = (StepPosition_P % a) / b;
        long seconds = ((StepPosition_P % a) % b) / 100;

        sprintf(RAPosition_L, "%02li:%02li:%02li#", hours, minutes, seconds);
    }
    else
    {
        long minutes = 10L * (StepPosition_P % a) / (float) b;

        sprintf(RAPosition_L, "%02li:%02li.%01li#", hours, minutes / 10, minutes % 10);
    }
    strcpy(LX200Response, RAPosition_L);
}

/******************************************************************************
 * Function:        void GetTelescopeRA()
 * PreCondition:    None
 * Input:		    None
 * Output:		    None
 * Side Effects:    None
 * Overview:		Get Telescope Right Ascension
 *****************************************************************************/
void GetTelescopeRA()
{
    SendRA(RAStepPosition);
}

/******************************************************************************
 * Function:        void GetCurrentTargetRA()
 * PreCondition:    None
 * Input:		    None
 * Output:		    None
 * Side Effects:    None
 * Overview:		Get current/target object Right Ascension
 *****************************************************************************/
void GetCurrentTargetRA()
{
    SendRA(RAStepTarget);
}

/******************************************************************************
 * Function:        void SendDeclination()
 * PreCondition:    None
 * Input:		    long StepPosition_P : position in number of steps
 * Output:		    None
 * Side Effects:    None
 * Overview:		Convert step position to declinaison and send it to host
 *****************************************************************************/
void SendDeclination(long StepPosition_P)
{
    char DecPosition_L[16];
    long DecPos_L;
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

    long degrees = DecPos_L / DecStepPerDegree;
    if (LX200Precise)
    {
        long minutes = (DecPos_L % DecStepPerDegree) / DecStepPerMinute;
        long seconds = ((DecPos_L % DecStepPerDegree) % DecStepPerMinute) / DecStepPerSecond;

        sprintf(DecPosition_L, "%c%02li:%02li:%02li#", signe, degrees, minutes, seconds);
    }
    else
    {
        long minutes = 10L * (DecPos_L % DecStepPerDegree) / DecStepPerMinute;
        sprintf(DecPosition_L, "%c%02li:%02li.%01li#", signe, degrees, minutes / 10, minutes % 10);
    }
    strcpy(LX200Response, DecPosition_L);
}

/******************************************************************************
 * Function:        void GetTelescopeDeclination()
 * PreCondition:    None
 * Input:		    None
 * Output:		    None
 * Side Effects:    None
 * Overview:		Get Telescope Right Declination
 *****************************************************************************/
void GetTelescopeDeclination()
{
    SendDeclination(DecStepPosition);
}

/******************************************************************************
 * Function:        void GetTelescopeDeclination()
 * PreCondition:    None
 * Input:		    None
 * Output:		    None
 * Side Effects:    None
 * Overview:		Get Telescope Right Declination
 *****************************************************************************/
void GetCurrentTargetDeclination()
{
    SendDeclination(DecStepTarget);
}

/******************************************************************************
 * Function:        void GetTelescopeAzimuth()
 * PreCondition:    None
 * Input:		    None
 * Output:		    None
 * Side Effects:    None
 * Overview:		Get Telescope Azimuth. Not supported, send 0
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
 * Input:		    None
 * Output:		    None
 * Side Effects:    None
 * Overview:		Get Telescope Altitude. Not supported, send 0
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
 * Input:		    None
 * Output:		    None
 * Side Effects:    None
 * Overview:		Get Sedireal Time. Not supported, send 0
 *****************************************************************************/
void GetSiderealTime()
{
    strcpy(LX200Response, "00:00:00#");
}

