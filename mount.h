/*********************************************************************
 *
 *      ~ ZeGoto ~
 *
 *  Mount settings
 *
 *********************************************************************
 * FileName:        mount.h
 * Processor:       PIC24FJ256GB106
 * Compiler:        Microchip XC16 v1.21 or higher
 *
 * Copyright � 2014 Sylvain Girard
 *
 * Author               Date        Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Sylvain Girard       16 mai 2014 Creation
 ********************************************************************/

#ifndef MOUNT_H
#define	MOUNT_H

#include "GenericTypeDefs.h"
#include <stdint.h>

typedef enum
{
    MOTOR_STOP,
    MOTOR_ACCEL,
    MOTOR_FULLSPEED,
    MOTOR_DECEL,
} motor_state_t;

typedef enum
{
    UNPARKED,
    PARKING,
    PARKED
} park_mode_t;

#define PIER_WEST 0
#define PIER_EAST 1

typedef struct
{
    /* Mount specific settings */
    int32_t NbStepMax;
    uint32_t SideralPeriod;
    uint32_t LunarPeriod;
    uint32_t SolarPeriod;
    uint16_t MaxSpeed;
    uint16_t CenteringSpeed;
    uint16_t GuideSpeed;

    int32_t RAStepBacklash;
    int32_t DecStepBacklash;

    /* Acceleration/decelation settings */
    int32_t AccelTime; // seconds
    int32_t DecelTime; // seconds

    /* Directions */
    unsigned RADefaultDirection : 1;
    unsigned DecDefaultDirection : 1;

    unsigned IsParked : 1;
    unsigned ParkEast : 1;
    unsigned ParkPostion : 2;
    double ParkAltitude;
    double ParkAzimuth;

    double Latitude;
    double Longitude;
    double Elevation;

    double UTCOffset;

    unsigned int ReticuleBrightness;
} mountconfig_t;

typedef struct
{
    mountconfig_t Config;

    uint32_t SideralHalfPeriod;

    /* Acceleration/decelation varibles */
    int32_t AccelPeriod;
    int32_t DecelPeriod;
    uint16_t CurrentMaxSpeed;

    /* Directions */
    unsigned WestDirection : 1;
    unsigned EastDirection : 1;
    unsigned NorthDirection : 1;
    unsigned SouthDirection : 1;

    unsigned SideOfPier : 1;
    unsigned CalculatedSideOfPier : 1;
    unsigned SideOfScope : 1;
    unsigned AutomaticSideOfPier : 1;
    unsigned PierIsFlipping : 1;

    BOOL IsGuiding;
} mount_t;

extern mount_t Mount;
extern DWORD adrMountConfig;

void MountInit();
void SaveMountConfig(const mountconfig_t *ptrMountConfig);

#endif	/* MOUNT_H */
