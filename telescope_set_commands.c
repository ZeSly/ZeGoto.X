/*********************************************************************
 *
 *      ~ OpenGoto ~
 *
 *  LX200 : S ? Telescope Set Commands
 *
 *********************************************************************
 * FileName:        telescope_set_commands.c
 * Processor:       PIC24FJ256GB106
 * Compiler:        Microchip XC16 v1.21 or higher
 *
 * Copyright © 2014 Sylvain Girard
 *
 * Author               Date        Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Sylvain Girard       4 mai 2014 Creation
 ********************************************************************/

/* Device header file */
#include <xc.h>
#include <stdint.h>
#include <stdlib.h>
#include <ctype.h>

#include "lx200_protocol.h"
#include "get_telescope_information.h"
#include "mount.h"
#include "ra_motor.h"
#include "dec_motor.h"
#include "rtcc.h"
#include "stdlib.h"

/******************************************************************************
 * Function:        void SetTargetObjectRA()
 * PreCondition:    None
 * Input:		    None
 * Output:		    None
 * Side Effects:    None
 * Overview:		Set target object Right Ascension
 *****************************************************************************/
void SetTargetObjectRA()
{
    int p = 2;
    while (LX200String[p] == ' ')
        p++;

    int32_t hours = (int32_t) (LX200String[p] - '0') * 10 + (int32_t) (LX200String[p + 1] - '0');
    int32_t minutes = (int32_t) (LX200String[p + 3] - '0') * 10 + (int32_t) (LX200String[p + 4] - '0');
    int32_t seconds;

    if (LX200String[p + 5] == '.')
    {
        seconds = 60L * (int32_t) (LX200String[p + 6] - '0') / 10L;
        LX200Precise = FALSE;
    }
    else
    {
        seconds = (int32_t) (LX200String[p + 6] - '0') * 10 + (int32_t) (LX200String[p + 7] - '0');
        LX200Precise = TRUE;
    }

    RA.StepTarget = hours * 3600L + minutes * 60L + seconds;
    RA.StepTarget *= RA.StepPerSec;

    if (RA.StepTarget < 0 || RA.StepTarget > Mount.Config.NbStepMax)
    {
        LX200Response[0] = '0';
    }
    else
    {
        LX200Response[0] = '1';
    }
    LX200Response[1] = '\0';
}

void SetStepTargetRA()
{
    RA.StepTarget = atol(LX200String + 3);

    if (RA.StepTarget < 0 || RA.StepTarget > Mount.Config.NbStepMax)
    {
        LX200Response[0] = '0';
    }
    else
    {
        LX200Response[0] = '1';
    }
    LX200Response[1] = '\0';
}

/******************************************************************************
 * Function:        void SetTargetObjectDeclination()
 * PreCondition:    None
 * Input:		    None
 * Output:		    None
 * Side Effects:    None
 * Overview:		Set target object Right Ascension
 *****************************************************************************/
void SetTargetObjectDeclination()
{
    int p = 2;
    while (LX200String[p] == ' ')
        p++;

    int32_t degrees = (int32_t) (LX200String[p + 1] - '0') * 10 + (int32_t) (LX200String[p + 2] - '0');
    int32_t minutes = (int32_t) (LX200String[p + 4] - '0') * 10 + (int32_t) (LX200String[p + 5] - '0');
    int32_t seconds;

    if (LX200String[p + 6] != '#')
    {
        seconds = (int32_t) (LX200String[p + 7] - '0') * 10 + (int32_t) (LX200String[p + 8] - '0');
        LX200Precise = TRUE;
    }
    else
    {
        seconds = 0L;
        LX200Precise = FALSE;
    }

    Dec.StepTarget = degrees * Dec.StepPerDegree;
    Dec.StepTarget += minutes * Dec.StepPerMinute;
    Dec.StepTarget += seconds * Dec.StepPerSecond;

    if (LX200String[p] == '-')
    {
        Dec.StepTarget = -Dec.StepTarget;
    }

    if (Dec.StepTarget < -Mount.Config.NbStepMax / 4L || Dec.StepTarget > Mount.Config.NbStepMax / 4L)
    {
        LX200Response[0] = '0';
    }
    else
    {
        LX200Response[0] = '1';
    }
    LX200Response[1] = '\0';
}

void SetStepTargetDeclination()
{
    Dec.StepTarget = atol(LX200String + 3);

    if (Dec.StepTarget < -Mount.Config.NbStepMax / 4L || Dec.StepTarget > Mount.Config.NbStepMax / 4L)
    {
        LX200Response[0] = '0';
    }
    else
    {
        LX200Response[0] = '1';
    }
    LX200Response[1] = '\0';
}

/******************************************************************************
 * Function:        void SyncWithCurrentTarget()
 * PreCondition:    None
 * Input:		    None
 * Output:		    None
 * Side Effects:    None
 * Overview:		Synchronizes the telescope's position with the current
 *                  target coordinates
 *****************************************************************************/
void SyncWithCurrentTarget()
{
    RA.StepPosition = RA.StepTarget;
    Dec.StepPosition = Dec.StepTarget;

    RTCCWriteArray(RTCC_RAM, (BYTE*) & RA.StepPosition, sizeof (RA.StepPosition));
    RTCCWriteArray(RTCC_RAM + sizeof (int32_t), (BYTE*) & Dec.StepPosition, sizeof (Dec.StepPosition));

    LX200Response[0] = '#';
    LX200Response[1] = '\0';
}

/******************************************************************************
 * Function:        void SelDate()
 * PreCondition:    None
 * Input:           None
 * Output:          None
 * Side Effects:    None
 * Overview:        :SCMM/DD/YY# Set RTCC date to MM/DD/YY
 *****************************************************************************/
void SetDate()
{
    RTCCMapTimekeeping Timekeeping;
    BYTE p = 2;

    LX200Response[0] = '0';
    if (RTCCGetTimekeeping(&Timekeeping) == TRUE)
    {

        Timekeeping.rtcmth.MTHTEN = LX200String[p++] - '0';
        Timekeeping.rtcmth.MTHONE = LX200String[p++] - '0';
        p++;
        Timekeeping.rtcdate.DATETEN = LX200String[p++] - '0';
        Timekeeping.rtcdate.DATEONE = LX200String[p++] - '0';
        p++;
        Timekeeping.rtcyear.YRTEN = LX200String[p++] - '0';
        Timekeeping.rtcyear.YRONE = LX200String[p++] - '0';

        if (RTCCSetTimekeeping(&Timekeeping) == TRUE)
        {
            LX200Response[0] = '1';
        }
    }
    LX200Response[1] = '\0';
}

/******************************************************************************
 * Function:        void SelLocalTime()
 * PreCondition:    None
 * Input:           None
 * Output:          None
 * Side Effects:    None
 * Overview:        :SLHH:MM:SS# Set RTCC local Time
 *****************************************************************************/
void SetLocalTime()
{
    RTCCMapTimekeeping Timekeeping;
    BYTE p = 2;

    LX200Response[0] = '0';
    if (RTCCGetTimekeeping(&Timekeeping) == TRUE)
    {
        Timekeeping.rtchour.B12_24 = 0; // The RTCC wil always be set in 24h format
        Timekeeping.rtchour.HRTEN = LX200String[p++] - '0';
        Timekeeping.rtchour.HRONE = LX200String[p++] - '0';
        p++;
        Timekeeping.rtcmin.MINTEN = LX200String[p++] - '0';
        Timekeeping.rtcmin.MINONE = LX200String[p++] - '0';
        p++;
        Timekeeping.rtcsec.SECTEN = LX200String[p++] - '0';
        Timekeeping.rtcsec.SECONE = LX200String[p++] - '0';

        if (RTCCSetTimekeeping(&Timekeeping) == TRUE)
        {
            LX200Response[0] = '1';
        }
    }
    LX200Response[1] = '\0';
}

/******************************************************************************
 * Function:        void SetUTCOffsetTime()
 * PreCondition:    None
 * Input:           None
 * Output:          None
 * Side Effects:    None
 * Overview:        :SGsHH.H#
 *                  Set the number of hours added to local time to yield UTC
 *****************************************************************************/
void SetUTCOffsetTime()
{
    double ut;

    LX200Response[0] = '0';
    ut = (LX200String[3] - '0') * 10.0;
    ut += (LX200String[4] - '0');
    ut += (LX200String[6] - '0') * 0.1;
    if (ut <= 12)
    {
        if (LX200String[2] == '-')
        {
            UTCOffset = -ut;
            LX200Response[0] = '1';
        }
        else if (LX200String[2] == '+')
        {
            UTCOffset = ut;
            LX200Response[0] = '1';
        }
    }
    LX200Response[1] = '\0';
}

/******************************************************************************
 * Function:        void LX200DMSToDec(double *dec)
 * PreCondition:    None
 * Input:           double *dec : pointer to the decimal number destination
 * Output:          None
 * Side Effects:    None
 * Overview:        Convert a degreee, minutes, second string from a
 *                  LX200 command to decimal
 *                  If the convertion fail, *dec is not modified
 *****************************************************************************/
void LX200DMSToDec(double *dec)
{
    double degrees;
    double minutes;
    double seconds;
    double sign;
    int p = 2;

    LX200Response[0] = '0';
    LX200Response[1] = '\0';

    while (LX200String[p] == ' ')
        p++;

    if (LX200String[p++] == '-')
    {
        sign = -1.0;
    }
    else
    {
        sign = 1.0;
    }


    degrees = (double) (LX200String[p++] - '0');
    while (isdigit(LX200String[p]))
    {
        degrees *= 10.0;
        degrees += (double) (LX200String[p++] - '0');

    }
    if (degrees > 0.0 && degrees < 360.0)
    {
        p++; // skip the seperator
        minutes = (double) (LX200String[p++] - '0') * 10.0;
        minutes += (double) (LX200String[p++] - '0');

        if (LX200String[p] != '#')
        {
            p++;
            seconds = (double) (LX200String[p++] - '0') * 10.0;
            seconds += (double) (LX200String[p++] - '0');
            LX200Precise = TRUE;
        }
        else
        {
            seconds = 0L;
            LX200Precise = FALSE;
        }

        *dec = degrees + minutes / 60.0 + seconds / 3600.0;
        *dec *= sign;
        LX200Response[0] = '1';
    }
}

/******************************************************************************
 * Function:        void SetCurrentSiteLongitude()
 * PreCondition:    None
 * Input:           None
 * Output:          None
 * Side Effects:    None
 * Overview:        :SGsHH.H#
 *                  Set the number of hours added to local time to yield UTC
 *****************************************************************************/
void SetCurrentSiteLongitude()
{
    LX200DMSToDec(&Longitude);
}

/******************************************************************************
 * Function:        void SetCurrentSiteLatitude()
 * PreCondition:    None
 * Input:           None
 * Output:          None
 * Side Effects:    None
 * Overview:        :SGsHH.H#
 *                  Set the number of hours added to local time to yield UTC
 *****************************************************************************/
void SetCurrentSiteLatitude()
{
    LX200DMSToDec(&Latitude);
}