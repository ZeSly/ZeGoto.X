/*********************************************************************
 *
 *      ~ OpenGoto ~
 *
 *  TODO add description
 *
 *********************************************************************
 * FileName:        gps.h
 * Processor:       PIC24FJ256GB106
 * Compiler:        Microchip XC16 v1.21 or higher
 *
 * Copyright © 2014 Sylvain Girard
 *
 * Author               Date        Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Sylvain Girard       17 mai 2014 Creation
 ********************************************************************/

#ifndef GPS_H
#define	GPS_H

typedef struct
{
    int Elevation;
    int Azimuth;
    int SNR;
    char Id[3];
} gps_satellite_t;

typedef struct
{
    char ON;
    char Available;
    char UTCTime[13];
    char Latitude[11];
    char NSIndicator;
    char Longitude[12];
    char EWIndicator;
    char PositionFixIndicator;
    char SatellitesUsed;
    char SatellitesInView;
    char Status;
    char Date[9];
    //double MSLAltitude;
    char MSLAltitude[8];
    gps_satellite_t Satellites[32];
} gps_t;

extern gps_t GPS;

void GPSStart();
void GPSTCPServer(void);
void GPSDecodeFrame();
int DMSToDec(double *dec, char *str);

void GPSon();
void GPSoff();

#endif	/* GPS_H */
