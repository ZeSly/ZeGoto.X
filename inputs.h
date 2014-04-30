/* 
 * File:   inputs.h
 * Author: ZeSly
 *
 * Created on 29 avril 2014, 20:24
 */

#ifndef INPUTS_H
#define	INPUTS_H

#define PAD_S1      1
#define PAD_S2      2
#define PAD_S3      4
#define PAD_S4      8
#define PAD_S5      16
#define PAD_S6      32
#define PAD_SWITCH  64

extern volatile BYTE bPadState;

void InputsInit(void);

#endif	/* INPUTS_H */

