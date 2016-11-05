/*********************************************************************
 *
 *      ~ ZeGoto ~
 *
 *  TODO add description
 *
 *********************************************************************
 * FileName:        reticule.h
 * Processor:       PIC24FJ256GB106
 * Compiler:        Microchip XC16 v1.21 or higher
 *
 * Copyright © 2014 Sylvain Girard
 *
 * Author               Date        Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Sylvain Girard       26 mai 2014 Creation
 ********************************************************************/

#ifndef RETICULE_H
#define	RETICULE_H

void ReticuleInit();

void ReticuleOn();
void ReticuleOff();
void IncreaseReticuleBrightness();
void DecreaseReticuleBrightness();

#endif	/* RETICULE_H */
