/*********************************************************************
 *
 *      ~ OpenGoto ~
 *
 *  Utilities functions
 *
 *********************************************************************
 * FileName:        utils.h
 * Processor:       PIC24FJ256GB106
 * Compiler:        Microchip XC16 v1.21 or higher
 *
 * Copyright © 2014 Sylvain Girard
 *
 * Author               Date        Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Sylvain Girard       28 juin 2014 Creation
 ********************************************************************/

#ifndef UTILS_H
#define	UTILS_H

#include <stdint.h>

int32_t int32abs(int32_t a);

double ComputeSideralTime();
void ComputeAzimuthalCoord(double *Altitude, double *Azimuth);
void ComputeEquatorialCoord(double Altitude, double Azimuth, double *ra, double *dec);
double ComputeTargetAltitude();

int _Dec2DMS(double d, char *s, int h);

#define Dec2DMS(d,s) _Dec2DMS(d,s,0)
#define Dec2HMS(d,s) _Dec2DMS(d,s,1)

#define MAX(a,b) (a > b ? a : b)
#define MIN(a,b) (a < b ? a : b)

#endif	/* UTILS_H */
