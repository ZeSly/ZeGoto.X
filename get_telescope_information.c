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
#include <math.h>

#include "utils.h"
#include "main.h"
#include "mount.h"
#include "ra_motor.h"
#include "dec_motor.h"
#include "lx200_protocol.h"
#include "rtcc.h"
#include "gps.h"
#include "get_telescope_information.h"

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
void GetRAString(int32_t StepPosition_P, BOOL Precise_P, char *Str_P)
{
    int32_t a = 3600L * RA.StepPerSec;
    int32_t b = 60L * RA.StepPerSec;

    int32_t hours = StepPosition_P / a;
    int32_t modulo_hours = StepPosition_P % a;

    if (Precise_P)
    {
        int32_t minutes = modulo_hours / b;
        int32_t modulo_minutes = modulo_hours % b;
        int32_t seconds = modulo_minutes / 100;
        int32_t modulo_seconds = modulo_minutes % 100;
        if (modulo_seconds > 50) seconds++;

        if (seconds == 60)
        {
            seconds = 0;
            minutes++;
            if (minutes == 60)
            {
                minutes = 0;
                hours++;
                if (hours == 24) hours = 0; // it should never happens !
            }
        }

        sprintf(Str_P, "%02li:%02li:%02li#", hours, minutes, seconds);
    }
    else
    {
        int32_t minutes = 10L * modulo_hours / (float) b;
        int32_t modulo_minutes = minutes % 10;

        if (modulo_minutes >= 5)
        {
            minutes++;
            if (minutes == 60)
            {
                minutes = 0;
                hours++;
            }
            if (hours == 24) hours = 0; // it should never happens !
        }

        sprintf(Str_P, "%02li:%02li.%01li#", hours, minutes / 10, modulo_minutes);
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
    GetRAString(RA.StepPosition, LX200Precise, LX200Response);
}

void GetStepRA()
{
    sprintf(LX200Response, "%li#", RA.StepPosition);
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
    GetRAString(RA.StepTarget, LX200Precise, LX200Response);
}

void GetStepTargetRA()
{
    sprintf(LX200Response, "%li#", RA.StepTarget);
}

/******************************************************************************
 * Function:        void SendDeclination()
 * PreCondition:    None
 * Input:           int32_t StepPosition_P : position in number of steps
 * Output:          None
 * Side Effects:    None
 * Overview:        Convert step position to declinaison and send it to host
 *****************************************************************************/
void GetDecString(int32_t StepPosition_P, BOOL Precise_P, char *Str_P)
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

    int32_t degrees = DecPos_L / Dec.StepPerDegree;
    int32_t modulo_degrees = DecPos_L % Dec.StepPerDegree;

    if (Precise_P)
    {
        int32_t minutes = modulo_degrees / Dec.StepPerMinute;
        int32_t modulo_minutes = modulo_degrees % Dec.StepPerMinute;
        int32_t seconds = modulo_minutes / Dec.StepPerSecond;
        int32_t modulo_seconds = modulo_minutes % Dec.StepPerSecond;

        if (modulo_seconds > Dec.StepPerSecond / 2L) seconds++;
        if (seconds == 60)
        {
            seconds = 0;
            minutes++;
            if (minutes == 60)
            {
                minutes = 0;
                degrees++;
                if (degrees > 90) degrees = 90; // it should never happens !
            }
        }

        sprintf(Str_P, "%c%02li:%02li:%02li#", signe, degrees, minutes, seconds);
    }
    else
    {
        int32_t minutes = 10L * (DecPos_L % Dec.StepPerDegree) / Dec.StepPerMinute;
        int32_t modulo_minutes = minutes % 10;

        if (modulo_minutes >= 5)
        {
            minutes++;
            if (minutes == 60)
            {
                minutes = 0;
                degrees++;
            }
            if (degrees > 90) degrees = 90; // it should never happens !
        }

        sprintf(Str_P, "%c%02li:%02li.%01li#", signe, degrees, minutes / 10, minutes % 10);
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
    GetDecString(Dec.StepPosition, LX200Precise, LX200Response);
}

void GetStepDeclination()
{
    sprintf(LX200Response, "%li#", Dec.StepPosition);
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
    GetDecString(Dec.StepTarget, LX200Precise, LX200Response);
}

void GetStepTargetDeclination()
{
    sprintf(LX200Response, "%li#", Dec.StepTarget);
}

/******************************************************************************
 * Function:        void GetTelescopeAzimuth()
 * PreCondition:    None
 * Input:           None
 * Output:          None
 * Side Effects:    None
 * Overview:        Get Telescope Azimuth.
 *****************************************************************************/
void GetTelescopeAzimuth()
{
    double Azimuth, Altitude;
    double degrees, minutes, seconds;

    ComputeAzimuthalCoord(&Altitude, &Azimuth);

    degrees = floor(Azimuth);
    minutes = (Azimuth - degrees) * 60.0;
    seconds = (minutes - floor(minutes)) * 60.0;

    if (LX200Precise)
    {
        sprintf(LX200Response, "%03.0f*%02.0f'%02.0f#", degrees, minutes, seconds);
    }
    else
    {
        sprintf(LX200Response, "%03.0f*%02.0f#", degrees, minutes);
    }
}

/******************************************************************************
 * Function:        void GetTelescopeAltitude()
 * PreCondition:    None
 * Input:           None
 * Output:          None
 * Side Effects:    None
 * Overview:        Get Telescope Altitude.
 *****************************************************************************/
void GetTelescopeAltitude()
{
    double Azimuth, Altitude;
    double degrees, minutes, seconds;

    ComputeAzimuthalCoord(&Altitude, &Azimuth);

    degrees = floor(fabs(Altitude));
    minutes = (fabs(Altitude) - degrees) * 60.0;
    seconds = (minutes - floor(minutes)) * 60.0;

    if (LX200Precise)
    {
        sprintf(LX200Response, "%c%02.0f*%02.0f'%02.0f#", Altitude < 0 ? '-' : '+', degrees, minutes, seconds);
    }
    else
    {
        sprintf(LX200Response, "%c%02.0f*%02.0f#", Altitude < 0 ? '-' : '+', degrees, minutes);
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
void GetSideralTime()
{
    double st;
    char *p;

    p = LX200Response;
    st = ComputeSideralTime();
    p += Dec2HMS(st, p);
    *p++ = '#';
    *p = '\0';
}

/******************************************************************************
 * Function:        void GetLocalTime()
 * PreCondition:    None
 * Input:           None
 * Output:          None
 * Side Effects:    None
 * Overview:        :GL# Get Local Time in 24 hour format
 *                  Returns: HH:MM:SS#
 *****************************************************************************/
void GetLocalTime()
{
    RTCCMapTimekeeping Timekeeping;
    BYTE i = 0;

    RTCCGetTimekeeping(&Timekeeping);

    LX200Response[i++] = Timekeeping.rtchour.HRTEN + '0';
    LX200Response[i++] = Timekeeping.rtchour.HRONE + '0';
    LX200Response[i++] = ':';
    LX200Response[i++] = Timekeeping.rtcmin.MINTEN + '0';
    LX200Response[i++] = Timekeeping.rtcmin.MINONE + '0';
    LX200Response[i++] = ':';
    LX200Response[i++] = Timekeeping.rtcsec.SECTEN + '0';
    LX200Response[i++] = Timekeeping.rtcsec.SECONE + '0';
    LX200Response[i++] = '#';
    LX200Response[i] = '\0';
}

/******************************************************************************
 * Function:        void GetUTCOffsetTime()
 * PreCondition:    None
 * Input:           None
 * Output:          None
 * Side Effects:    None
 * Overview:        Get UTC offset time
 *****************************************************************************/
void GetUTCOffsetTime()
{
    int i;

    sprintf(LX200Response, "%c%04.1f#", Mount.Config.UTCOffset < 0 ? '-' : '+', fabs(Mount.Config.UTCOffset));
    for (i = 0 ; LX200Response[i] != '#' ; i++)
        ;
    if (LX200Response[i- 1] == '0')
    {
        LX200Response[i - 2] = '#';
        LX200Response[i - 1] = '\0';
    }
}

/******************************************************************************
 * Function:        void GetCurrentDate()
 * PreCondition:    None
 * Input:           None
 * Output:          None
 * Side Effects:    None
 * Overview:        :GC# Get current date.
 *                  Returns: MM/DD/YY#
 *****************************************************************************/
void GetCurrentDate()
{
    RTCCMapTimekeeping Timekeeping;
    BYTE i = 0;

    RTCCGetTimekeeping(&Timekeeping);

    LX200Response[i++] = Timekeeping.rtcmth.MTHTEN + '0';
    LX200Response[i++] = Timekeeping.rtcmth.MTHONE + '0';
    LX200Response[i++] = '/';
    LX200Response[i++] = Timekeeping.rtcdate.DATETEN + '0';
    LX200Response[i++] = Timekeeping.rtcdate.DATEONE + '0';
    LX200Response[i++] = '/';
    LX200Response[i++] = Timekeeping.rtcyear.YRTEN + '0';
    LX200Response[i++] = Timekeeping.rtcyear.YRONE + '0';
    LX200Response[i++] = '#';
    LX200Response[i] = '\0';
}

/******************************************************************************
 * Function:        void GetCalendarFormat()
 * PreCondition:    None
 * Input:           None
 * Output:          None
 * Side Effects:    None
 * Overview:       :Gc# Get Calendar Format
 *                  Returns: 24#
 *****************************************************************************/
void GetCalendarFormat()
{
    strcpy(LX200Response, "24#");
}

void LX200Dec2DMS(double d)
{
    double fract;
    double deg, min, sec;
    char s;

    fract = fabs(modf(d, &deg));
    fract = modf(fract * 60.0, &min);
    sec = fract * 60.0;
    if (sec >= 60.0)
    {
        min += 1.0;
        sec = 0.0;
    }
    s = deg < 0 ? '-' : '+';
    if (LX200Precise)
    {
        sprintf(LX200Response, "%c%02.0f*%02.0f:%02.0f#", s, fabs(deg), min, sec);
    }
    else
    {
        sprintf(LX200Response, "%c%02.0f*%02.0f#", s, fabs(deg), min);
    }
}

/******************************************************************************
 * Function:        void GetCurrentSiteLongitude()
 * PreCondition:    None
 * Input:           None
 * Output:          None
 * Side Effects:    None
 * Overview:        :Gg# Get Current Site Longitude
 *                  Returns: sDDD*MM#
 *****************************************************************************/
void GetCurrentSiteLongitude()
{
    LX200Dec2DMS(Mount.Config.Longitude);
}

/******************************************************************************
 * Function:        void GetCurrentSiteLatitude()
 * PreCondition:    None
 * Input:           None
 * Output:          None
 * Side Effects:    None
 * Overview:        :Gt# Get Current Site Latitdue
 *                  Returns: sDD*MM#
 *****************************************************************************/
void GetCurrentSiteLatitude()
{
    LX200Dec2DMS(Mount.Config.Latitude);
}
/******************************************************************************
 * Function:        void GetCurrentSiteAltitude()
 * PreCondition:    None
 * Input:           None
 * Output:          None
 * Side Effects:    None
 * Overview:        :Ge# Get Current Site Altitude ~ OpenGoto Specific ~
 *                  Returns: dddd#
 *****************************************************************************/
void GetCurrentSiteAltitude()
{
    sprintf(LX200Response,"%04.0f#", Mount.Config.Elevation);
}