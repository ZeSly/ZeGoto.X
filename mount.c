/*********************************************************************
 *
 *      ~ ZeGoto ~
 *
 *  Mount settings
 *
 *********************************************************************
 * FileName:        mount.c
 * Processor:       PIC24FJ256GB106
 * Compiler:        Microchip XC16 v1.21 or higher
 *
 * Copyright © 2014 Sylvain Girard
 *
 * Author               Date        Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Sylvain Girard       16 mai 2014 Creation
 ********************************************************************/

/* Device header file */
#include <xc.h>

#include "mount.h"
#include "HardwareProfile.h"
#include "TCPIP_Stack/TCPIP.h"
#include "main.h"

mount_t Mount;
DWORD adrMountConfig;
static unsigned short wOriginalMountConfigChecksum; // Checksum of the ROM defaults for Mount.Config

void MountInit()
{
    unsigned char vNeedToSaveDefaults = 0;

    while (1)
    {
        memset(&Mount.Config, 0, sizeof (Mount.Config));

        /* Mecanic setting */
        Mount.Config.NbStepMax = 8640000UL;
        Mount.Config.SideralPeriod = 159563UL;
        Mount.Config.LunarPeriod   = 163432UL;
        Mount.Config.SolarPeriod   = 160000UL;
        
        Mount.Config.MaxSpeed = 120;
        Mount.Config.CenteringSpeed = 10;
        Mount.Config.GuideSpeed = 5;    // 5 = 0.5x sideral rate on RA and 1.5x on dec

        /* Acceleration/decelation settings */
        Mount.Config.AccelTime = 4; // seconds
        Mount.Config.DecelTime = 1; // seconds

        /* Directions */
        Mount.Config.RADefaultDirection = 0;
        Mount.Config.DecDefaultDirection = 0;

        /* Park coordinates */
        Mount.Config.IsParked = 0;
        Mount.Config.ParkEast = 0;
        Mount.Config.ParkPostion = 3;   // default park position is Astro-Physics Park Position 3: pointing the pole
        Mount.Config.ParkAltitude = 0;
        Mount.Config.ParkAzimuth = 0;

        /* Site location */
        Mount.Config.Latitude = 45.2448;
        Mount.Config.Longitude = -5.63314;
        Mount.Config.Elevation = 200.0;
        Mount.Config.UTCOffset = +2.0;

        /* Reticule brightness */
        Mount.Config.ReticuleBrightness = 615;


        wOriginalMountConfigChecksum = CalcIPChecksum((BYTE *) &Mount.Config, sizeof(Mount.Config));

        NVM_VALIDATION_STRUCT NVMValidationStruct;

        // Check to see if we have a flag set indicating that we need to
        // save the ROM default Mount.Config values.
        if (vNeedToSaveDefaults)
            SaveMountConfig(&Mount.Config);

        // Read the NVMValidation record and AppConfig struct out of EEPROM/Flash
#if defined(EEPROM_CS_TRIS) || defined(EEPROM_I2CCON)
        {
            XEEReadArray(adrMountConfig, (BYTE*) & NVMValidationStruct, sizeof (NVMValidationStruct));
            XEEReadArray(adrMountConfig + sizeof (NVMValidationStruct), (BYTE*) & Mount.Config, sizeof (Mount.Config));
        }
#elif defined(SPIFLASH_CS_TRIS)
        {
            SPIFlashReadArray(adrMountConfig, (BYTE*) & NVMValidationStruct, sizeof (NVMValidationStruct));
            SPIFlashReadArray(adrMountConfig + sizeof (NVMValidationStruct), (BYTE*) & Mount.Config, sizeof (Mount.Config));
        }
#endif
        unsigned short wCalcIPChecksum = CalcIPChecksum((BYTE*) & Mount.Config, sizeof (Mount.Config));
        if ((NVMValidationStruct.wConfigurationLength != sizeof (Mount.Config)) ||
                (NVMValidationStruct.wOriginalChecksum != wOriginalMountConfigChecksum) ||
                (NVMValidationStruct.wCurrentChecksum != wCalcIPChecksum))
        {
            if (vNeedToSaveDefaults)
            {
                while (1);
            }

            // Set flag and restart loop to load ROM defaults and save them
            vNeedToSaveDefaults = 1;
            continue;
        }

        break;

    }

    Mount.CurrentMaxSpeed = Mount.Config.MaxSpeed;
    Mount.AccelPeriod = GetPeripheralClock() / Mount.Config.MaxSpeed * Mount.Config.AccelTime;
    Mount.DecelPeriod = GetPeripheralClock() / Mount.Config.MaxSpeed * Mount.Config.DecelTime;

    Mount.SideralHalfPeriod = Mount.Config.SideralPeriod / 2;
    Mount.WestDirection = Mount.Config.RADefaultDirection;
    Mount.EastDirection = !Mount.Config.RADefaultDirection;
    Mount.NorthDirection = Mount.Config.DecDefaultDirection;
    Mount.SouthDirection = !Mount.Config.DecDefaultDirection;

    Mount.AutomaticSideOfPier = 0;
    Mount.PierIsFlipping = 0;

    Mount.IsGuiding = FALSE;
}

void SaveMountConfig(const mountconfig_t *ptrMountConfig)
{
    NVM_VALIDATION_STRUCT NVMValidationStruct;    

    // Get proper values for the validation structure indicating that we can use
    // these EEPROM/Flash contents on future boot ups
    NVMValidationStruct.wOriginalChecksum = wOriginalMountConfigChecksum;
    NVMValidationStruct.wCurrentChecksum = CalcIPChecksum((BYTE*) ptrMountConfig, sizeof (mountconfig_t));
    NVMValidationStruct.wConfigurationLength = sizeof (mountconfig_t);

    // Write the validation struct and current AppConfig contents to EEPROM/Flash
#if defined(EEPROM_CS_TRIS) || defined (EEPROM_I2CCON)
    //    XEEBeginWrite(0x0000);
    XEEWriteArray(adrMountConfig, (BYTE*) & NVMValidationStruct, sizeof (NVMValidationStruct));
    XEEWriteArray(adrMountConfig + sizeof (NVMValidationStruct), (BYTE*) ptrMountConfig, sizeof (mountconfig_t));
#else
    SPIFlashBeginWrite(adrMountConfig);
    SPIFlashWriteArray((BYTE*) & NVMValidationStruct, sizeof (NVMValidationStruct));
    SPIFlashWriteArray((BYTE*) ptrAppConfig, sizeof (mountconfig_t));
#endif

}