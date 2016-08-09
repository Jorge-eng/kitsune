//*****************************************************************************
// middlesdc.c
//
// Configure the device pins for different peripheral signals
//
// Copyright (C) 2014 Texas Instruments Incorporated - http://www.ti.com/ 
// 
// 
//  Redistribution and use in source and binary forms, with or without 
//  modification, are permitted provided that the following conditions 
//  are met:
//
//    Redistributions of source code must retain the above copyright 
//    notice, this list of conditions and the following disclaimer.
//
//    Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the 
//    documentation and/or other materials provided with the   
//    distribution.
//
//    Neither the name of Texas Instruments Incorporated nor the names of
//    its contributors may be used to endorse or promote products derived
//    from this software without specific prior written permission.
//
//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
//  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
//  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
//  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
//  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
//  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
//  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
//  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
//  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
//  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
//  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//*****************************************************************************

//*****************************************************************************
// This file was automatically generated by the CC32xx PinMux Utility
// Version: 1.0.2
//
//*****************************************************************************

#include "pinmux.h"
#include "hw_types.h"
#include "hw_memmap.h"
#include "hw_gpio.h"
#include "pin.h"
#include "rom.h"
#include "rom_map.h"
#include "gpio.h"
#include "prcm.h"

#define REG_PAD_CONFIG_26   (0x4402E108)
#define REG_PAD_CONFIG_27   (0x4402E10C)

#define PAD_MODE_MASK        0x0000000F
#define PAD_STRENGTH_MASK    0x000000E0
#define PAD_TYPE_MASK        0x00000310


//*****************************************************************************
//
//! Configure Antenna Selection GPIOs
//!
//! \param  None
//!
//! \return None
//
//*****************************************************************************
static void SetAntennaSelectionGPIOs(void)
{
    MAP_GPIODirModeSet(GPIOA3_BASE,0xC,GPIO_DIR_MODE_OUT);

    //
    // Configure PIN_29 for GPIOOutput
    //
    HWREG(REG_PAD_CONFIG_26) = ((HWREG(REG_PAD_CONFIG_26) & ~(PAD_STRENGTH_MASK
                        | PAD_TYPE_MASK)) | (0x00000020 | 0x00000000 ));

    //
    // Set the mode.
    //
    HWREG(REG_PAD_CONFIG_26) = (((HWREG(REG_PAD_CONFIG_26) & ~PAD_MODE_MASK) |
                                                    0x00000000) & ~(3<<10));

    //
    // Set the direction
    //
    HWREG(REG_PAD_CONFIG_26) = ((HWREG(REG_PAD_CONFIG_26) & ~0xC00) | 0x00000800);


     //
    // Configure PIN_30 for GPIOOutput
    //
    HWREG(REG_PAD_CONFIG_27) = ((HWREG(REG_PAD_CONFIG_27) & ~(PAD_STRENGTH_MASK
                                | PAD_TYPE_MASK)) | (0x00000020 | 0x00000000 ));

    //
    // Set the mode.
    //
    HWREG(REG_PAD_CONFIG_27) = (((HWREG(REG_PAD_CONFIG_27) & ~PAD_MODE_MASK) |
                                        0x00000000) & ~(3<<10));

    //
    // Set the direction
    //
    HWREG(REG_PAD_CONFIG_26) = ((HWREG(REG_PAD_CONFIG_27) & ~0xC00) | 0x00000800);

}

static void ConfigCodecResetPin(void)
{
	// Set as output
    MAP_GPIODirModeSet(GPIOA3_BASE,0x4,GPIO_DIR_MODE_OUT);

    // Reset Value for REG_PAD_CONFIG_26: 0xC61

    //
    // Clear pad strength, pad type and pad mode, set drive strength to be 2mA [7:5]=001
    //
    HWREG(REG_PAD_CONFIG_26) = ((HWREG(REG_PAD_CONFIG_26) & ~(PAD_STRENGTH_MASK
                        | PAD_TYPE_MASK | PAD_MODE_MASK)) | 0x00000020 );

    //
    // Clear pad output and output buffer enable override
    //
    HWREG(REG_PAD_CONFIG_26) = (HWREG(REG_PAD_CONFIG_26) & ~(3<<10));

    //
    // Enable override of pad output buffer enable (Pg 498 of SWRU367B)
    //
    HWREG(REG_PAD_CONFIG_26) = (HWREG(REG_PAD_CONFIG_26) | 0x00000800);

}

//*****************************************************************************
#include "hw_ver.h"
void PinMuxConfig_hw_dep() {
	int hw_ver = get_hw_ver();

	switch( hw_ver ) {
	case DVT:
	case PVT:
	    SetAntennaSelectionGPIOs();
	    break;
	case EVT1_1p5:
		ConfigCodecResetPin();
		break;
	}
}

void
PinMuxConfig(void)
{
    //
    // Enable Peripheral Clocks
    //
    MAP_PRCMPeripheralClkEnable(PRCM_I2S, PRCM_RUN_MODE_CLK);
    MAP_PRCMPeripheralClkEnable(PRCM_UARTA0, PRCM_RUN_MODE_CLK);
    MAP_PRCMPeripheralClkEnable(PRCM_UARTA1, PRCM_RUN_MODE_CLK);
    MAP_PRCMPeripheralClkEnable(PRCM_GPIOA0, PRCM_RUN_MODE_CLK);
    MAP_PRCMPeripheralClkEnable(PRCM_GPIOA1, PRCM_RUN_MODE_CLK);
    MAP_PRCMPeripheralClkEnable(PRCM_GPIOA2, PRCM_RUN_MODE_CLK);
    MAP_PRCMPeripheralClkEnable(PRCM_GPIOA3, PRCM_RUN_MODE_CLK);
    MAP_PRCMPeripheralClkEnable(PRCM_TIMERA2, PRCM_RUN_MODE_CLK);
    MAP_PRCMPeripheralClkEnable(PRCM_I2CA0, PRCM_RUN_MODE_CLK);
    MAP_PRCMPeripheralClkEnable(PRCM_GPIOA1, PRCM_RUN_MODE_CLK);
    MAP_PRCMPeripheralClkEnable(PRCM_GSPI, PRCM_RUN_MODE_CLK);
    MAP_PRCMPeripheralClkEnable(PRCM_SDHOST, PRCM_RUN_MODE_CLK);

    //
    // Configure PIN_50 for MCASP0 McAXR1
    //
    MAP_PinTypeI2S(PIN_50, PIN_MODE_6);

    //
    // Configure PIN_55 for UART0 UART0_TX
    //
    MAP_PinTypeUART(PIN_55, PIN_MODE_3);

    //
    // Configure PIN_57 for UART0 UART0_RX
    //
    MAP_PinTypeUART(PIN_57, PIN_MODE_3);

    //
    // Configure PIN_58 for UART1 UART1_TX
    //
    MAP_PinTypeUART(PIN_58, PIN_MODE_6);

    //
    // Configure PIN_59 for UART1 UART1_RX
    //
    MAP_PinTypeUART(PIN_59, PIN_MODE_6);

    //
    // Configure PIN_61 for GPIOInput
    //
    MAP_PinTypeGPIO(PIN_61, PIN_MODE_0, false);
    MAP_GPIODirModeSet(GPIOA0_BASE, 0x40, GPIO_DIR_MODE_IN);

    //setup i2S clock
    //MAP_PRCMPeripheralClkEnable(PRCM_I2S, PRCM_RUN_MODE_CLK);
    //
    // Configure PIN_62 for GPIOInput // TODO DKH This may need to be set as output
    //
//    MAP_PinTypeGPIO(PIN_62, PIN_MODE_0, false);
 //   MAP_GPIODirModeSet(GPIOA0_BASE, 0x80, GPIO_DIR_MODE_IN);
    MAP_PinTypeUART(PIN_62, PIN_MODE_1); //special TI debug logs

    //
    // Configure PIN_63 for MCASP0 McAFSX
    //
    MAP_PinTypeI2S(PIN_63, PIN_MODE_7);

    //
    // Configure PIN_64 for GPIOOutput
    //
    MAP_PinTypeGPIO(PIN_64, PIN_MODE_0, false);
    MAP_GPIODirModeSet(GPIOA1_BASE, 0x2, GPIO_DIR_MODE_OUT);
#if 0
    {
        (*((unsigned volatile int*)0x4402E0C4)) = 0; //see page 498 of the cc3200 trm section 16.8.1.1.1
    }
#endif

    //
    // Configure PIN_01 for I2C0 I2C_SCL
    //
    MAP_PinTypeI2C(PIN_01, PIN_MODE_1);
    //i2c on pin 4
    MAP_PinTypeI2C(PIN_04, PIN_MODE_5);

	//DVT uses camera clock for codec's master clock
	MAP_PRCMPeripheralClkEnable(PRCM_CAMERA, PRCM_RUN_MODE_CLK);
	HWREG(0x44025000) = 0x0000;
	MAP_CameraXClkConfig(CAMERA_BASE, 120000000ul,12000000ul); // 12MHz

	// Configure PIN_02 for CAMERA0 CAM_pXCLK
    PinModeSet(PIN_02,PIN_MODE_4);
    PinConfigSet(PIN_02,PIN_STRENGTH_6MA|PIN_STRENGTH_2MA|PIN_STRENGTH_4MA,PIN_TYPE_STD);

    //
    // Configure PIN_03 for MCASP0 McACLK
    //
    MAP_PinTypeI2S(PIN_03, PIN_MODE_3);

    //
    // Configure PIN_05 for SPI0 GSPI_CLK
    //
    MAP_PinTypeSPI(PIN_05, PIN_MODE_7);

    //
    // Configure PIN_06 for SDHOST0 SDHost_D0
    //
    MAP_PinTypeSDHost(PIN_06, PIN_MODE_8);
    MAP_PinConfigSet(PIN_06,PIN_STRENGTH_6MA|PIN_STRENGTH_2MA|PIN_STRENGTH_4MA, PIN_TYPE_STD);

    //
    // Configure PIN_07 for SDHOST0 SDHost_CLK
    //
    MAP_PinTypeSDHost(PIN_07, PIN_MODE_8);
    MAP_PinConfigSet(PIN_07,PIN_STRENGTH_6MA|PIN_STRENGTH_2MA|PIN_STRENGTH_4MA, PIN_TYPE_STD_PU);

    //
    // Configure PIN_08 for SDHOST0 SDHost_CMD
    //
    MAP_PinTypeSDHost(PIN_08, PIN_MODE_8);
    MAP_PinConfigSet(PIN_08,PIN_STRENGTH_6MA|PIN_STRENGTH_2MA|PIN_STRENGTH_4MA, PIN_TYPE_STD_PU);

    //
    // Configure PIN_53 for SPI0 GSPI_MISO
    //
    MAP_PinTypeSPI(PIN_53, PIN_MODE_7);

    //
    // Configure PIN_45 for MCASP0 McAXR0
    //
    MAP_PinTypeI2S(PIN_45, PIN_MODE_6);

    //
    // Configure PIN_52 for SPI0 GSPI_MOSI
    //
    MAP_PinTypeSPI(PIN_52, PIN_MODE_8);

    // Configure PIN_04 for GPIOOutput
    //
    MAP_PinTypeGPIO(PIN_04, PIN_MODE_0, false);
    MAP_GPIODirModeSet(GPIOA1_BASE, 0x20, GPIO_DIR_MODE_OUT);

    //
    // Configure PIN_15 for GPIOOutput
    //
    MAP_PinTypeGPIO(PIN_15, PIN_MODE_0, false);
    MAP_GPIODirModeSet(GPIOA2_BASE, 0x40, GPIO_DIR_MODE_OUT);

    //
    // Configure PIN_16 for GPIOInput
    //
    MAP_PinTypeGPIO(PIN_16, PIN_MODE_0, false);
    MAP_GPIODirModeSet(GPIOA2_BASE, 0x80, GPIO_DIR_MODE_IN);

    //
    // Configure PIN_17 for GPIOOutput
    //
    MAP_PinTypeGPIO(PIN_17, PIN_MODE_0, false);
    MAP_GPIODirModeSet(GPIOA3_BASE, 0x1, GPIO_DIR_MODE_OUT);
    {
        (*((unsigned volatile int*)0x4402E100)) |= 0x670; //see page 498 of the cc3200 trm section 16.8.1.1.1
    }

    //SOP2
    //
	//
	// Configure PIN_21 for GPIOOutput
	//
	MAP_PinTypeGPIO(PIN_21, PIN_MODE_0, false);
	MAP_GPIODirModeSet(GPIOA3_BASE, 0x2, GPIO_DIR_MODE_OUT);

	// todo PVT reverse this
	//drive high by default
    MAP_GPIOPinWrite(GPIOA3_BASE, 0x2, 0x2);

}
