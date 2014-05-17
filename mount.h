/*********************************************************************
 *
 *      ~ OpenGoto ~
 *
 *  Mount settings
 *
 *********************************************************************
 * FileName:        mount.h
 * Processor:       PIC24FJ256GB106
 * Compiler:        Microchip XC16 v1.21 or higher
 *
 * Copyright © 2014 Sylvain Girard
 *
 * Author               Date        Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Sylvain Girard       16 mai 2014 Creation
 ********************************************************************/

#ifndef MOUNT_H
#define	MOUNT_H

#include <stdint.h>

typedef enum
{
    MOTOR_STOP,
    MOTOR_ACCEL,
    MOTOR_FULLSPEED,
    MOTOR_DECEL,
} motor_state_t;

typedef struct
{
    /* Mount specific settings */
    int32_t NbStepMax;
    uint32_t SideralPeriod;
    uint16_t MaxSpeed;
    uint16_t CenteringSpeed;

    /* Acceleration/decelation settings */
    int32_t AccelTime; // seconds
    int32_t DecelTime; // seconds
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


} mount_t;

extern mount_t Mount;

void MountInit();

#endif	/* MOUNT_H */
