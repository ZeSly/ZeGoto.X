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
    char UTCTime[13];
    char Latitute[11];
    char NSIndicator;
    char Longitude[12];
    char EWIndicator;
    char PositionFixIndicator;
    char SatellitesUsed;
    char Status;
    char Date[9];
    //double MSLAltitude;
    char MSLAltitude[8];
} gps_t;

extern gps_t GPS;

void GPSStart();
void GPSTCPServer(void);
void GPSDecodeFrame();


#endif	/* GPS_H */
