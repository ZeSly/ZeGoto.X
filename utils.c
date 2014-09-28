/*********************************************************************
 *
 *      ~ OpenGoto ~
 *
 *  Utilities functions
 *
 *********************************************************************
 * FileName:        utils.c
 * Processor:       PIC24FJ256GB106
 * Compiler:        Microchip XC16 v1.21 or higher
 *
 * Copyright © 2014 Sylvain Girard
 *
 * Author               Date        Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Sylvain Girard       28 juin 2014 Creation
 ********************************************************************/

/* Device header file */
#include <xc.h>
#include <stdint.h>
#include <math.h>

#include "mount.h"
#include "ra_motor.h"
#include "dec_motor.h"
#include "rtcc.h"

/******************************************************************************
 * Function:        int32_t int32abs(int32_t a)
 * PreCondition:    None
 * Input:           int32_t
 * Output:          int32_t
 * Side Effects:    None
 * Overview:        compute absolute value
 *****************************************************************************/
inline int32_t int32abs(int32_t a)
{
    return a < 0 ? -a : a;
}

/******************************************************************************
 * Constant for time and coordinate calculation
 *****************************************************************************/

#define PI 3.14159265358979323846
#define DEGREES(a) (a * PI / 180.0)

/******************************************************************************
 * Function:        void ComputeAzimuthalCoord(double *Altitude, double *Azimuth)
 * PreCondition:    None
 * Input:           double *Altitude, double *Azimuth
 * Output:          None
 * Side Effects:    Update JulianDay
 * Overview:        Compute local sideral time based on current UTC time
 *****************************************************************************/
double ComputeSideralTime()
{
    datetime_t datetime;
    GetUTCDateTime(&datetime);
    JulianDay = DateToJulianDay(&datetime);

    double h = datetime.hour + datetime.minute / 60.0 + datetime.second / 3600.0;
    double n = JulianDay - 2415384.5;
    double ts = 23750.3 + 236.555362 * n;
    double tsmh_h = ts / 3600.0 + h - (Mount.Config.Longitude / 15.0);

    return fmod(tsmh_h, 24.0);
}

/******************************************************************************
 * Function:        void ComputeAzimuthalCoord(double *Altitude, double *Azimuth)
 * PreCondition:    None
 * Input:           double *Altitude, double *Azimuth
 * Output:          None
 * Side Effects:    Update JulianDay
 * Overview:        Compute azimuthal coordinates depending of current UTC time
 *****************************************************************************/
void ComputeAzimuthalCoord(double *Altitude, double *Azimuth)
{
    double ra = (double) RA.StepPosition * 24.0 / (double) Mount.Config.NbStepMax;
    double dec = (double) Dec.StepPosition * 360.0 / (double) Mount.Config.NbStepMax;

    double tsmh_h = ComputeSideralTime();
    double hour_angle = tsmh_h - ra;

    double ah = hour_angle * 15.0;
    double cos_z = sin(DEGREES(Mount.Config.Latitude)) * sin(DEGREES(dec)) + cos(DEGREES(Mount.Config.Latitude)) * cos(DEGREES(dec)) * cos(DEGREES(ah));
    double z = acos(cos_z);
    double sin_a = cos(DEGREES(dec)) * sin(DEGREES(ah)) / sin(z);

    *Altitude = 90.0 - (z * 180.0 / PI);
    *Azimuth = asin(sin_a) * 180.0 / PI + 180.0;
}

/******************************************************************************
 * Function:        ComputeEquatorialCoord
 * PreCondition:    None
 * Input:           double Altitude, double Azimuth, double *ra, double *dec
 * Output:          values pointed by double *ra and double *dec
 * Side Effects:    Update JulianDay
 * Overview:        Compute equatorial coordinates depending of current UTC time
 *****************************************************************************/
void ComputeEquatorialCoord(double Altitude, double Azimuth, double *ra, double *dec)
{
    double tsmh_h = ComputeSideralTime();

    Azimuth -= 180.0;
    double sin_dec = sin(DEGREES(Mount.Config.Latitude)) * sin(DEGREES(Altitude)) - cos(DEGREES(Mount.Config.Latitude)) * cos(DEGREES(Altitude)) * cos(DEGREES(Azimuth));
    double sin_ha = cos(DEGREES(Altitude)) * sin(DEGREES(Azimuth)) / cos(asin(sin_dec));

    double hour_angle = asin(sin_ha) * 180.0 / PI;
    hour_angle /= 15.0;
    *ra = fmod(tsmh_h - hour_angle, 24.0);
    *dec = asin(sin_dec) * 180.0 / PI;
}

/******************************************************************************
 * Function:        Dec2DMS
 * PreCondition:    None
 * Input:           double d, char *s
 * Output:          int : length of the string *s
 * Side Effects:    None
 * Overview:        Convert an angle as double to string
 *****************************************************************************/
int _Dec2DMS(double d, char *s, int h)
{
    double fract;
    double deg, min, sec;
    int ret;

    fract = fabs(modf(d, &deg));
    fract = modf(fract * 60.0, &min);
    sec = fract * 60.0;
    if (h)
    {
        ret = sprintf(s, "%02.0f:%02.0f:%02.0f", deg, min, sec);
    }
    else
    {
        ret = sprintf(s, "%.0f&deg;%.0f'%.2f''", deg, min, sec);
    }
    return ret;
}
