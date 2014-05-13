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

#include "main.h"
#include "ra_motor.h"
#include "dec_motor.h"
#include "lx200_protocol.h"
#include "rtcc.h"

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
    int32_t a = 3600L * RAStepPerSec;
    int32_t b = 60L * RAStepPerSec;

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
    GetRAString(RAStepPosition, LX200Precise, LX200Response);
}

void GetStepRA()
{
    sprintf(LX200Response, "%li#", RAStepPosition);
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
    GetRAString(RAStepTarget, LX200Precise, LX200Response);
}

void GetStepTargetRA()
{
    sprintf(LX200Response, "%li#", RAStepTarget);
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

    int32_t degrees = DecPos_L / DecStepPerDegree;
    int32_t modulo_degrees = DecPos_L % DecStepPerDegree;

    if (Precise_P)
    {
        int32_t minutes = modulo_degrees / DecStepPerMinute;
        int32_t modulo_minutes = modulo_degrees % DecStepPerMinute;
        int32_t seconds = modulo_minutes / DecStepPerSecond;
        int32_t modulo_seconds = modulo_minutes % DecStepPerSecond;

        if (modulo_seconds > DecStepPerSecond / 2L) seconds++;
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
        int32_t minutes = 10L * (DecPos_L % DecStepPerDegree) / DecStepPerMinute;
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
    GetDecString(DecStepPosition, LX200Precise, LX200Response);
}

void GetStepDeclination()
{
    sprintf(LX200Response, "%li#", DecStepPosition);
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
    GetDecString(DecStepTarget, LX200Precise, LX200Response);
}

void GetStepTargetDeclination()
{
    sprintf(LX200Response, "%li#", DecStepTarget);
}


#define PI 3.14159265358979323846
#define DEGREES(a) (a * PI / 180.0)

double tsmh_h;
double angle_horaire;

void ComputeAzimuthalCoord(double *Altitude, double *Azimuth)
{
    double latitude = 45.2448;
    double longitude = 5.63314;
    double ra = (double) RAStepPosition * 24.0 / (double) NbStepMax;
    double dec = (double) DecStepPosition * 360.0 / (double) NbStepMax;

    datetime_t datetime;
    GetUTCDateTime(&datetime);

    double h = datetime.hour + datetime.minute / 60.0 + datetime.second / 3600.0;
    double n = JulianDay - 2415384.5;
    double ts = 23750.3 + 236.555362 * n;
    tsmh_h = ts / 3600.0 + h + (longitude / 15.0);
    angle_horaire = tsmh_h - ra;

    double ah = angle_horaire * 15.0;
    double cos_z = sin(DEGREES(latitude)) * sin(DEGREES(dec)) + cos(DEGREES(latitude)) * cos(DEGREES(dec)) * cos(DEGREES(ah));
    double z = acos(cos_z);
    double sin_a = cos(DEGREES(dec)) * sin(DEGREES(ah)) / sin(z);

    *Altitude = 90.0 - (z * 180.0 / PI);
    *Azimuth = asin(sin_a) * 180.0 / PI + 180.0;
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
void GetSideralTime()
{
    strcpy(LX200Response, "00:00:00#");
}
