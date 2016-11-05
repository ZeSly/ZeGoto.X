/*********************************************************************
 *
 *      ~ ZeGoto ~
 *
 *  Processing LX200 commands
 *
 *********************************************************************
 * FileName:        lx200_protocol.h
 * Processor:       PIC24FJ256GB106
 * Compiler:        Microchip XC16 v1.21 or higher
 *
 * Copyright © 2014 Sylvain Girard
 *
 * Author               Date        Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Sylvain Girard   	2 mai 2014 Creation
 ********************************************************************/

#ifndef LX200_PROTOCOL_H
#define	LX200_PROTOCOL_H

void LX200ProcessCommand(char *LX200Cmd_P);

extern char LX200String[];
extern char LX200Response[];

#endif	/* LX200_PROTOCOL_H */
