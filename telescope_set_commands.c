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
#include "gps.h"

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
    datetime_t localtime;
    BYTE p = 2;

    LX200Response[0] = '0';
    if (GetLocalDateTime(&localtime))
    {
        localtime.month = (LX200String[p++] - '0') * 10;
        localtime.month += (LX200String[p++] - '0');
        p++;
        localtime.day = (LX200String[p++] - '0') * 10;
        localtime.day += (LX200String[p++] - '0');
        p++;
        localtime.year = (LX200String[p++] - '0') * 10;
        localtime.year += (LX200String[p++] - '0');
        localtime.year += 2000;

        if (SetLocalDateTime(&localtime) == TRUE)
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
    datetime_t localtime;
    BYTE p = 2;

    LX200Response[0] = '0';
    if (GetLocalDateTime(&localtime))
    {
        localtime.hour = (LX200String[p++] - '0') * 10;
        localtime.hour += (LX200String[p++] - '0');
        p++;
        localtime.minute = (LX200String[p++] - '0') * 10;
        localtime.minute += (LX200String[p++] - '0');
        p++;
        localtime.second = (LX200String[p++] - '0') * 10;
        localtime.second += (LX200String[p++] - '0');

        if (SetLocalDateTime(&localtime) == TRUE)
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
            Mount.Config.UTCOffset = -ut;
            SaveMountConfig(&Mount.Config);
            LX200Response[0] = '1';
        }
        else if (LX200String[2] == '+')
        {
            Mount.Config.UTCOffset = ut;
            SaveMountConfig(&Mount.Config);
            LX200Response[0] = '1';
        }
    }
    LX200Response[1] = '\0';
}

/******************************************************************************
 * Function:        void SetCurrentSiteLongitude()
 * PreCondition:    None
 * Input:           None
 * Output:          None
 * Side Effects:    None
 * Overview:        :SGsHH.H#
 *                  Set current site longitude
 *****************************************************************************/
void SetCurrentSiteLongitude()
{
    int p = 2;

    while (LX200String[p] == ' ')
        p++;
    LX200Response[0] = DMSToDec(&Mount.Config.Longitude, LX200String + p) + '0';
    if (LX200Response[0] == '1') SaveMountConfig(&Mount.Config);
    LX200Response[1] = '\0';
}

/******************************************************************************
 * Function:        void SetCurrentSiteLatitude()
 * PreCondition:    None
 * Input:           None
 * Output:          None
 * Side Effects:    None
 * Overview:        :SGsHH.H#
 *                  Set current site latitude
 *****************************************************************************/
void SetCurrentSiteLatitude()
{
    int p = 2;

    while (LX200String[p] == ' ')
        p++;
    LX200Response[0] = DMSToDec(&Mount.Config.Latitude, LX200String + p) + '0';
    if (LX200Response[0] == '1') SaveMountConfig(&Mount.Config);
    LX200Response[1] = '\0';
}

/******************************************************************************
 * Function:        void SetCurrentSiteAltitude()
 * PreCondition:    None
 * Input:           None
 * Output:          None
 * Side Effects:    None
 * Overview:        :Sudddd#
 *                  Set current site altitude ~ OpenGoto Specific ~
 *****************************************************************************/
void SetCurrentSiteAltitude()
{
    double Elevation = 0;
    double f = 1;
    int z;

    for (z = 0; z < 4 && isdigit(LX200String[z + 2]); z++)
    {
        Elevation *= f;
        Elevation += LX200String[z + 2] - '0';
        f = 10;
    }

    if (z < 4)
    {
        LX200Response[0] = '0';
    }
    else
    {
        LX200Response[0] = '1';
        Mount.Config.Elevation = Elevation;
        SaveMountConfig(&Mount.Config);
    }
    LX200Response[1] = '\0';
}

/******************************************************************************
 * Function:        void SetTargetObjectAltitude()
 * PreCondition:    None
 * Input:           None
 * Output:          None
 * Side Effects:    None
 * Overview:        :SasDD*MM# or :SasDD*MM"SS#
 *                  Set target object altitude
 *****************************************************************************/
void SetTargetObjectAltitude()
{
    double ra, dec;
    double azimuth, altitude;
    int p = 2;

    while (LX200String[p] == ' ')
        p++;

    ComputeAzimuthalCoord(&altitude, &azimuth);

    LX200Response[0] = DMSToDec(&altitude, LX200String + p) + '0';
    LX200Response[1] = '\0';

    ComputeEquatorialCoord(altitude, azimuth, &ra, &dec);
    RA.StepTarget = (int32_t) (Mount.Config.NbStepMax * ra / 24.0);
    Dec.StepTarget = (int32_t) (Mount.Config.NbStepMax * dec / 360.0);
}

/******************************************************************************
 * Function:        void SetTargetObjectAzimuth()
 * PreCondition:    None
 * Input:           None
 * Output:          None
 * Side Effects:    None
 * Overview:        :SasDD*MM# or :SasDD*MM"SS#
 *                  Set target object altitude
 *****************************************************************************/
void SetTargetObjectAzimuth()
{
    double ra, dec;
    double azimuth, altitude;
    int p = 2;

    while (LX200String[p] == ' ')
        p++;

    ComputeAzimuthalCoord(&altitude, &azimuth);

    LX200Response[0] = DMSToDec(&azimuth, LX200String + p) + '0';
    LX200Response[1] = '\0';

    ComputeEquatorialCoord(altitude, azimuth, &ra, &dec);
    RA.StepTarget = (int32_t) (Mount.Config.NbStepMax * ra / 24.0);
    Dec.StepTarget = (int32_t) (Mount.Config.NbStepMax * dec / 360.0);
}