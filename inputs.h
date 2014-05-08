/* 
 * File:   inputs.h
 * Author: ZeSly
 *
 * Created on 29 avril 2014, 20:24
 */

#ifndef INPUTS_H
#define	INPUTS_H

typedef union
{
    int i;
    struct
    {
        unsigned PAD_S1 : 1;
        unsigned PAD_S2 : 1;
        unsigned PAD_S3 : 1;
        unsigned PAD_S4 : 1;
        unsigned PAD_S5 : 1;
        unsigned PAD_S6 : 1;
        unsigned PAD_SWITCH : 1;
    };
} pad_t;

extern pad_t PadState;

void InputsInit(void);
void UpdatePadState();

#endif	/* INPUTS_H */

