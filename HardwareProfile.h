/*********************************************************************
 *
 *	Hardware specific definitions for OpenGoto
 *
 *********************************************************************
 * FileName:        HardwareProfile.h
 * Dependencies:    None
 * Processor:       PIC24FJ128GB110
 * Compiler:        Microchip C32 v1.10 or higher
 *					Microchip C30 v3.12 or higher
 *					Microchip C18 v3.34 or higher
 *					HI-TECH PICC-18 PRO 9.63PL2 or higher
 * Company:         ZeSly
 *
 * Software License Agreement
 *
 * Copyright (C) 2002-2010 Microchip Technology Inc.  All rights
 * reserved.
 *
 * Microchip licenses to you the right to use, modify, copy, and
 * distribute:
 * (i)  the Software when embedded on a Microchip microcontroller or
 *      digital signal controller product ("Device") which is
 *      integrated into Licensee's product; or
 * (ii) ONLY the Software driver source files ENC28J60.c, ENC28J60.h,
 *		ENCX24J600.c and ENCX24J600.h ported to a non-Microchip device
 *		used in conjunction with a Microchip ethernet controller for
 *		the sole purpose of interfacing with the ethernet controller.
 *
 * You should refer to the license agreement accompanying this
 * Software for additional information regarding your rights and
 * obligations.
 *
 * THE SOFTWARE AND DOCUMENTATION ARE PROVIDED "AS IS" WITHOUT
 * WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT
 * LIMITATION, ANY WARRANTY OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * MICROCHIP BE LIABLE FOR ANY INCIDENTAL, SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF
 * PROCUREMENT OF SUBSTITUTE GOODS, TECHNOLOGY OR SERVICES, ANY CLAIMS
 * BY THIRD PARTIES (INCLUDING BUT NOT LIMITED TO ANY DEFENSE
 * THEREOF), ANY CLAIMS FOR INDEMNITY OR CONTRIBUTION, OR OTHER
 * SIMILAR COSTS, WHETHER ASSERTED ON THE BASIS OF CONTRACT, TORT
 * (INCLUDING NEGLIGENCE), BREACH OF WARRANTY, OR OTHERWISE.
 *
 *
 * Author               Date		Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Sylvain Girard       09/03/2014      Creation
 ********************************************************************/

#ifndef HARDWARE_PROFILE_H
#define HARDWARE_PROFILE_H

#include "Compiler.h"

// Set configuration fuses (but only in MainDemo.c where THIS_IS_STACK_APPLICATION is defined)
#if defined(THIS_IS_STACK_APPLICATION)
// See configuration_bits.c
//	#if defined(__PIC24F__)
//		// All other PIC24F PIMs
//		_CONFIG2(FNOSC_PRIPLL & POSCMOD_XT)		// Primary XT OSC with 4x PLL
//		_CONFIG1(JTAGEN_OFF & FWDTEN_OFF)		// JTAG off, watchdog timer off
//        #endif
#endif

// Clock frequency values
// Create a PIC dependant macro for the maximum supported internal clock
#if defined(__PIC24F__) || defined(__PIC24FK__)
	#define MAXIMUM_PIC_FREQ		(32000000ul)
#else	// dsPIC33F, PIC24H
	#define MAXIMUM_PIC_FREQ		(80000000ul)
#endif

// These directly influence timed events using the Tick module.  They also are used for UART and SPI baud rate generation.
#define GetSystemClock()        (MAXIMUM_PIC_FREQ)			// Hz
#define GetInstructionClock()   (GetSystemClock()/2)	// Normally GetSystemClock()/4 for PIC18, GetSystemClock()/2 for PIC24/dsPIC, and GetSystemClock()/1 for PIC32.  Might need changing if using Doze modes.
#define GetPeripheralClock()    (GetSystemClock()/2)	// Normally GetSystemClock()/4 for PIC18, GetSystemClock()/2 for PIC24/dsPIC, and GetSystemClock()/1 for PIC32.  Divisor may be different if using a PIC32 since it's configurable.


// Hardware I/O pin mappings

// LEDs
#define LED1_TRIS       (TRISBbits.TRISB9)
#define LED1_IO         (LATBbits.LATB9)
#define LED2_TRIS       (TRISBbits.TRISB11)
#define LED2_IO         (LATBbits.LATB11)

/** ENC28J60 *******************************************************/
#define ENC_CS_TRIS         (TRISGbits.TRISG9)	// Comment this line out if you are using the ENC424J600/624J600, MRF24WB0M, or other network controller.
#define ENC_CS_IO           (LATGbits.LATG9)

#define ENC_SPI_IF          (IFS0bits.SPI1IF)
#define ENC_SSPBUF          (SPI1BUF)
#define ENC_SPISTAT         (SPI1STAT)
#define ENC_SPISTATbits     (SPI1STATbits)
#define ENC_SPICON1         (SPI1CON1)
#define ENC_SPICON1bits     (SPI1CON1bits)
#define ENC_SPICON2         (SPI1CON2)

// I/O pins
/** I2C EEPROM *****************************************************/
#define EEPROM_I2CCON       (I2C1CON)
#define EEPROM_I2CCONbits   (I2C1CONbits)
#define EEPROM_I2CBRG       (I2C1BRG)
#define EEPROM_I2CSTAT      (I2C1STAT)
#define EEPROM_I2CSTATbits  (I2C1STATbits)
#define EEPROM_I2CRCV       (I2C1RCV)
#define EEPROM_I2CTRN       (I2C1TRN)

/*******************************************************************/
/******** USB stack hardware selection options *********************/
/*******************************************************************/
//This section is the set of definitions required by the MCHPFSUSB
//  framework.  These definitions tell the firmware what mode it is
//  running in, and where it can find the results to some information
//  that the stack needs.
//These definitions are required by every application developed with
//  this revision of the MCHPFSUSB framework.  Please review each
//  option carefully and determine which options are desired/required
//  for your application.

#define USE_SELF_POWER_SENSE_IO
//#define tris_self_power     TRISFbits.TRISF3    // Input
#define self_power          1

#define USE_USB_BUS_SENSE_IO
//#define tris_usb_bus_sense  TRISFbits.TRISF3    // Input
#define USB_BUS_SENSE       U1OTGSTATbits.SESVD

//Uncomment this to make the output HEX of this project
//   to be able to be bootloaded using the HID bootloader
#define PROGRAMMABLE_WITH_USB_HID_BOOTLOADER

//If the application is going to be used with the HID bootloader
//  then this will provide a function for the application to
//  enter the bootloader from the application (optional)
#if defined(PROGRAMMABLE_WITH_USB_HID_BOOTLOADER)
    #define EnterBootloader() __asm__("goto 0x400")
#endif

/** LED ************************************************************/
#define mLED_1              LED1_IO
#define mLED_2              LED2_IO

#define mGetLED_1()         mLED_1
#define mGetLED_2()         mLED_2

#define mLED_1_On()         mLED_1 = 1;
#define mLED_2_On()         mLED_2 = 1;

#define mLED_1_Off()        mLED_1 = 0;
#define mLED_2_Off()        mLED_2 = 0;

#define mLED_1_Toggle()     mLED_1 = !mLED_1;
#define mLED_2_Toggle()     mLED_2 = !mLED_2;

/** I/O pin definitions ********************************************/
#define INPUT_PIN 1
#define OUTPUT_PIN 0

/** RA motor control ***********************************************/
#define RA_HOME_TRIS        (TRISEbits.TRISE4)
#define RA_HOME_IO          (PORTEbits.RE4)
#define RA_HOME_PULLUP      (CNPU4bits.CN62PUE)

#define RA_SLEEP_TRIS       (TRISEbits.TRISE6)
#define RA_SLEEP_IO         (LATEbits.LATE6)

#define RA_DECAY_TRIS       (TRISEbits.TRISE7)
#define RA_DECAY_IO         (LATEbits.LATE7)

#define RA_DIR_TRIS         (TRISEbits.TRISE1)
#define RA_DIR_IO           (LATEbits.LATE1)

#define RA_STEP_TRIS        (TRISEbits.TRISE2)
#define RA_STEP_IO          (LATEbits.LATE2)

#define RA_MODE_TRIS        (TRISEbits.TRISE3)
#define RA_MODE_IO          (LATEbits.LATE3)

#define RA_FAULT_TRIS       (TRISEbits.TRISE5)
#define RA_FAULT_IO         (PORTEbits.RE5)
#define RA_FAULT_CN         (CNEN4bits.CN63IE)
#define RA_FAULT_PULLUP     (CNPU4bits.CN63PUE)

/** Dec motor control ***********************************************/
#define DEC_HOME_TRIS       (TRISDbits.TRISD1)
#define DEC_HOME_IO         (PORTDbits.RD1)
#define DEC_HOME_PULLUP     (CNPU4bits.CN50PUE)

#define DEC_SLEEP_TRIS      (TRISFbits.TRISF1)
#define DEC_SLEEP_IO        (LATFbits.LATF1)

#define DEC_DECAY_TRIS       (TRISCbits.TRISC14)
#define DEC_DECAY_IO         (LATCbits.LATC14)

#define DEC_DIR_TRIS        (TRISDbits.TRISD4)
#define DEC_DIR_IO          (LATDbits.LATD4)

#define DEC_STEP_TRIS       (TRISDbits.TRISD3)
#define DEC_STEP_IO         (LATDbits.LATD3)

#define DEC_MODE_TRIS       (TRISDbits.TRISD2)
#define DEC_MODE_IO         (LATDbits.LATD2)

#define DEC_FAULT_TRIS      (TRISEbits.TRISE0)
#define DEC_FAULT_IO        (PORTEbits.RE0)
#define DEC_FAULT_CN        (CNEN4bits.CN58IE)
#define DEC_FAULT_PULLUP    (CNPU4bits.CN58PUE)

/** Autoguiding port */
#define GUIDE_RAP_TRIS      (TRISBbits.TRISB13)
#define GUIDE_RAP_IO        (PORTBbits.RB13)
#define GUIDE_RAP_CN        (CNEN2bits.CN31IE)
#define GUIDE_RAP_PULLUP    (CNPU2bits.CN31PUE)

#define GUIDE_RAM_TRIS      (TRISBbits.TRISB14)
#define GUIDE_RAM_IO        (PORTBbits.RB14)
#define GUIDE_RAM_CN        (CNEN3bits.CN32IE)
#define GUIDE_RAM_PULLUP    (CNPU3bits.CN32PUE)

#define GUIDE_DECP_TRIS     (TRISBbits.TRISB12)
#define GUIDE_DECP_IO       (PORTBbits.RB12)
#define GUIDE_DECP_CN       (CNEN2bits.CN30IE)
#define GUIDE_DECP_PULLUP   (CNPU2bits.CN30PUE)

#define GUIDE_DECM_TRIS     (TRISBbits.TRISB15)
#define GUIDE_DECM_IO       (PORTBbits.RB15)
#define GUIDE_DECM_CN       (CNEN1bits.CN12IE)
#define GUIDE_DECM_PULLUP   (CNPU1bits.CN12PUE)

/** A pin for testing things... *************************************/
#define TEST_PIN_TRIS       (TRISDbits.TRISD7)
#define TEST_PIN_OUT        (LATDbits.LATD7)
#define TEST_PIN_IN         (PORTDbits.RD7);


#endif //HARDWARE_PROFILE_H

