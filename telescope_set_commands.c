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
#include <string.h>

#include "lx200_protocol.h"
#include "get_telescope_information.h"
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
    uint8_t p = 2;
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

    RAStepTarget = hours * 3600L + minutes * 60L + seconds;
    RAStepTarget *= RAStepPerSec;

    if (RAStepTarget < 0 || RAStepTarget > NbStepMax)
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
    RAStepTarget = atol(LX200String + 3);

    if (RAStepTarget < 0 || RAStepTarget > NbStepMax)
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
    uint8_t p = 2;
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

    DecStepTarget = degrees * DecStepPerDegree;
    DecStepTarget += minutes * DecStepPerMinute;
    DecStepTarget += seconds * DecStepPerSecond;

    if (LX200String[p] == '-')
    {
        DecStepTarget = -DecStepTarget;
    }

    if (DecStepTarget < -NbStepMax / 4L || DecStepTarget > NbStepMax / 4L)
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
    DecStepTarget = atol(LX200String + 3);

    if (DecStepTarget < -NbStepMax / 4L || DecStepTarget > NbStepMax / 4L)
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
    RAStepPosition = RAStepTarget;
    DecStepPosition = DecStepTarget;

    RTCCWriteArray(RTCC_RAM, (BYTE*) & RAStepPosition, sizeof (RAStepPosition));
    RTCCWriteArray(RTCC_RAM + sizeof (int32_t), (BYTE*) & DecStepPosition, sizeof (DecStepPosition));

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
void SelDate()
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
void SelLocalTime()
{
    RTCCMapTimekeeping Timekeeping;
    BYTE p = 2;

    LX200Response[0] = '0';
    if (RTCCGetTimekeeping(&Timekeeping) == TRUE)
    {
        Timekeeping.rtchour.B12_24 = 0;
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
