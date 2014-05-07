/*********************************************************************
 *
 *      ~ OpenGoto ~
 *
 *  TODO add description
 *
 *********************************************************************
 * FileName:        get_telescope_information.h
 * Processor:       PIC24FJ256GB106
 * Compiler:        Microchip XC16 v1.21 or higher
 *
 * Copyright © 2014 Sylvain Girard
 *
 * Author               Date        Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Sylvain Girard   	2 mai 2014 Creation
 ********************************************************************/

#ifndef GET_TELESCOPE_INFORMATION_H
#define	GET_TELESCOPE_INFORMATION_H

#include <stdint.h>
#include "GenericTypeDefs.h"

extern BOOL LX200Precise;

void PrecisionToggle();
void GetPrecision();
void GetTelescopeRA();
void GetCurrentTargetRA();
void GetTelescopeDeclination();
void GetCurrentTargetDeclination();
void GetTelescopeAzimuth();
void GetTelescopeAltitude();
void GetSiderealTime();

void GetStepRA();
void GetStepDeclination();
void GetStepTargetRA();
void GetStepTargetDeclination();

void GetRAString(int32_t StepPosition_P, BOOL Precise_P, char *Str_P);
void GetDecString(int32_t StepPosition_P, BOOL Precise_P, char *Str_P);

#endif	/* GET_TELESCOPE_INFORMATION_H */
