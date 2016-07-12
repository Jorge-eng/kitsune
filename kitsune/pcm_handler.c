//*****************************************************************************
// pcm_handler.c
//
// PCM Handler Interface
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
//
//! \addtogroup WifiAudioPlayer
//! @{
//
//*****************************************************************************
/* Standard includes. */
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

/* Hardware & DriverLib library includes. */
#include "hw_types.h"
#include "hw_mcasp.h"
#include "hw_udma.h"
#include "hw_ints.h"
#include "rom.h"
#include "rom_map.h"
#include "debug.h"
#include "interrupt.h"
#include "hw_memmap.h"
#include "hw_ints.h"
#include "i2s.h"
#include "udma.h"
#include "pin.h"
#include "mcasp_if.h"
#include "circ_buff.h"
#include "pcm_handler.h"
#include "udma_if.h"
#include "i2c_if.h"
#include "uart_if.h"
#include "network.h"


/* FatFS include */
//#include "ff.h"
//#include "diskio.h"
#include "fatfs_cmd.h"
#include "uartstdio.h"

#include "codec_debug_config.h"
//*****************************************************************************
//                          LOCAL DEFINES
//*****************************************************************************
#define UDMA_DSTSZ_32_SRCSZ_16          0x21000000
#define mainQUEUE_SIZE		        3
#define UI_BUFFER_EMPTY_THRESHOLD  2048

#if (AUDIO_RECORD_DECIMATE==1)
#define DECIMATE_CHUNK_SIZE 2040
typedef struct {
	bool pending;
	uint32_t pending_data_per_channel;
	uint32_t pending_sum[4];
}decimation_dump_t;
int32_t decimate_16k(uint8_t* data);
#endif

//*****************************************************************************
//                          GLOBAL VARIABLES
//*****************************************************************************
#if (CODEC_ENABLE_MULTI_CHANNEL==1)
uint32_t ping[(CB_TRANSFER_SZ)];
uint32_t pong[(CB_TRANSFER_SZ)];
// Ping pong for playback
uint32_t ping_p[(CB_TRANSFER_SZ)];
uint32_t pong_p[(CB_TRANSFER_SZ)];
#else
unsigned short ping[(CB_TRANSFER_SZ)];
unsigned short pong[(CB_TRANSFER_SZ)];
#endif





volatile unsigned int guiDMATransferCountTx = 0;
volatile unsigned int guiDMATransferCountRx = 0;
extern tCircularBuffer *pTxBuffer;
extern tCircularBuffer *pRxBuffer;

extern TaskHandle_t audio_task_hndl;
#if (AUDIO_FULL_DUPLEX==1)
extern TaskHandle_t audio_task_hndl_p;
#endif

//*****************************************************************************
//
//! Callback function implementing ping pong mode DMA transfer
//!
//! \param None
//!
//! This function
//!        1. sets the primary and the secondary buffer for ping pong transfer.
//!		   2. sets the control table.
//!		   3. transfer the appropriate data according to the flags.
//!
//! \return None.
//
//*****************************************************************************
#define swap_endian(x) *(x) = ((*(x)) << 8) | ((*(x)) >> 8);
static volatile unsigned long qqbufsz=0;

#if (CODEC_ENABLE_MULTI_CHANNEL==1)
void DMAPingPongCompleteAppCB_opt()
{
    unsigned long ulPrimaryIndexTx = 0x4, ulAltIndexTx = 0x24;
    unsigned long ulPrimaryIndexRx = 0x5, ulAltIndexRx = 0x25;
    tDMAControlTable *pControlTable;
    tCircularBuffer *pAudOutBuf = pRxBuffer;
    tCircularBuffer *pAudInBuf = pTxBuffer;
    unsigned char *pucDMADest;
    unsigned char *pucDMASrc;
    unsigned int dma_status;
    unsigned int i2s_status;
    int i = 0;

    traceISR_ENTER();

	i2s_status = I2SIntStatus(I2S_BASE);
	dma_status = uDMAIntStatus();

	// I2S RX
	if (dma_status & 0x00000010) {
		//UARTprintf("r");
		qqbufsz = GetBufferSize(pAudInBuf);
		HWREG(0x4402609c) = (1 << 10);
		//
		// Get the base address of the control table.
		//
		//pControlTable = (tDMAControlTable *)HWREG(DMA_BASE + UDMA_O_CTLBASE);
		pControlTable = MAP_uDMAControlBaseGet();

		//PRIMARY part of the ping pong
		if ((pControlTable[ulPrimaryIndexTx].ulControl & UDMA_CHCTL_XFERMODE_M)
				== 0) {
			guiDMATransferCountTx += CB_TRANSFER_SZ*4; //bytes
			#define END_PTR_DEBUG (CB_TRANSFER_SZ*4)-1
			pucDMADest = (unsigned char *) ping;
			pControlTable[ulPrimaryIndexTx].ulControl |= CTRL_WRD;
			pControlTable[ulPrimaryIndexTx].pvDstEndAddr = (void *) (pucDMADest
					+ END_PTR_DEBUG);
			MAP_uDMAChannelEnable(UDMA_CH4_I2S_RX);

			for (i = 0; i < CB_TRANSFER_SZ; i++) {
			uint16_t pong_lsb = pong[i] & 0xFFFF;
			uint16_t pong_msb = (pong[i] & 0xFFFF0000) >> 16;
			swap_endian(&pong_lsb);
			swap_endian(&pong_msb);
			pong[i] = ((uint32_t)pong_msb << 16 ) | pong_lsb;

			}
			// TODO DKH - Decimation here?
#if (AUDIO_RECORD_DECIMATE==1)
			if(decimate_16k(pong))
			{
				UARTprintf("DECIMATE CHUNK SIZE INCORRECT\n");
			}
#endif
			FillBuffer(pAudInBuf, (unsigned char*)pong, CB_TRANSFER_SZ*4);

		} else {
			//ALT part of the ping pong
			if ((pControlTable[ulAltIndexTx].ulControl & UDMA_CHCTL_XFERMODE_M)
					== 0) {
				guiDMATransferCountTx += CB_TRANSFER_SZ*4; //bytes

				pucDMADest = (unsigned char *) pong;
				pControlTable[ulAltIndexTx].ulControl |= CTRL_WRD;
				pControlTable[ulAltIndexTx].pvDstEndAddr = (void *) (pucDMADest
						+ END_PTR_DEBUG);
				MAP_uDMAChannelEnable(UDMA_CH4_I2S_RX);

				for (i = 0; i < CB_TRANSFER_SZ; i++) {
					uint16_t ping_lsb = ping[i] & 0xFFFF;
					uint16_t ping_msb = (ping[i] & 0xFFFF0000) >> 16;
					swap_endian(&ping_lsb);
					swap_endian(&ping_msb);
					ping[i] = ((uint32_t)ping_msb << 16 ) | ping_lsb;

				}
				// TODO DKH - Decimation here?
#if (AUDIO_RECORD_DECIMATE==1)
				if(decimate_16k(ping))
				{
					UARTprintf("DECIMATE CHUNK SIZE INCORRECT\n");
				}
#endif
				FillBuffer(pAudInBuf, (unsigned char*)ping, CB_TRANSFER_SZ*4);
			}
		}



		// TODO DKH this if condition is redundant?
		if (guiDMATransferCountTx >= CB_TRANSFER_SZ*4)

			{
			signed long xHigherPriorityTaskWoken;

			guiDMATransferCountTx = 0;

			if ( qqbufsz > LISTEN_WATERMARK ) {
				vTaskNotifyGiveFromISR( audio_task_hndl, &xHigherPriorityTaskWoken );
				portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
			}
		}
	}

	// I2S TX
	if (dma_status & 0x00000020) {
		//UARTprintf("p");
		qqbufsz = GetBufferSize(pAudOutBuf);
		HWREG(0x4402609c) = (1 << 11);
		pControlTable = MAP_uDMAControlBaseGet();

			#define END_PTR_DEBUG (CB_TRANSFER_SZ*4)-1

		if ((pControlTable[ulPrimaryIndexRx].ulControl & UDMA_CHCTL_XFERMODE_M)
				== 0) {
			if ( qqbufsz > CB_TRANSFER_SZ ) {
				guiDMATransferCountRx += CB_TRANSFER_SZ*4;

				memcpy(  (void*)ping_p, (void*)GetReadPtr(pAudOutBuf), CB_TRANSFER_SZ);
				UpdateReadPtr(pAudOutBuf, CB_TRANSFER_SZ);

				for (i = CB_TRANSFER_SZ/4-1; i!=-1 ; --i) {
					//the odd ones do not matter
					/*ping[(i<<1)+1] = */
					ping_p[(i<<2) + 0] = (ping_p[i] & 0xFFFFUL) << 16;
					ping_p[(i<<2) + 1] = 0;
					ping_p[(i<<2) + 2] = (ping_p[i] & 0xFFFF0000);
					ping_p[(i<<2) + 3] = 0;
				}

			} else {
				memset( ping_p, 0, sizeof(ping_p));
			}

			pucDMASrc = (unsigned char*)ping_p;

			pControlTable[ulPrimaryIndexRx].ulControl |= CTRL_WRD;
			//pControlTable[ulPrimaryIndex].pvSrcEndAddr = (void *)((unsigned long)&gaucZeroBuffer[0] + 15);
			pControlTable[ulPrimaryIndexRx].pvSrcEndAddr =
					(void *) ((unsigned long) pucDMASrc + END_PTR_DEBUG);
			//HWREG(DMA_BASE + UDMA_O_ENASET) = (1 << 5);
			MAP_uDMAChannelEnable(UDMA_CH5_I2S_TX);
		} else {
			if ((pControlTable[ulAltIndexRx].ulControl & UDMA_CHCTL_XFERMODE_M)
					== 0) {
				if ( qqbufsz > CB_TRANSFER_SZ ) {
					guiDMATransferCountRx += CB_TRANSFER_SZ*4;

					memcpy(  (void*)pong_p,  (void*)GetReadPtr(pAudOutBuf), CB_TRANSFER_SZ);
					UpdateReadPtr(pAudOutBuf, CB_TRANSFER_SZ);

				for (i = CB_TRANSFER_SZ/4-1; i!=-1 ; --i) {
					//the odd ones do not matter
					/*ping[(i<<1)+1] = */
					pong_p[(i<<2) + 0] = (pong_p[i] & 0xFFFFUL) << 16;
					pong_p[(i<<2) + 1] = 0;
					pong_p[(i<<2) + 2] = (pong_p[i] & 0xFFFF0000);
					pong_p[(i<<2) + 3] = 0;
				}
				} else {
					memset( pong_p, 0, sizeof(pong_p));
				}

				pucDMASrc = (unsigned char*)pong_p;

				pControlTable[ulAltIndexRx].ulControl |= CTRL_WRD;
				pControlTable[ulAltIndexRx].pvSrcEndAddr =
						(void *) ((unsigned long) pucDMASrc + END_PTR_DEBUG);
				//HWREG(DMA_BASE + UDMA_O_ENASET) = (1 << 5);
				MAP_uDMAChannelEnable(UDMA_CH5_I2S_TX);
			}
		}

		if (guiDMATransferCountRx >= CB_TRANSFER_SZ) {
			signed long xHigherPriorityTaskWoken;
			qqbufsz = GetBufferSize(pAudOutBuf);
			guiDMATransferCountRx = 0;

			if ( qqbufsz < PLAY_WATERMARK ) {
#if (AUDIO_FULL_DUPLEX==1)
				vTaskNotifyGiveFromISR( audio_task_hndl_p, &xHigherPriorityTaskWoken );
#else
				vTaskNotifyGiveFromISR( audio_task_hndl, &xHigherPriorityTaskWoken );
#endif
				portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
			}
		}
	}
	uDMAIntClear(dma_status);
	I2SIntClear(I2S_BASE, i2s_status );

	traceISR_EXIT();
}

#else

void DMAPingPongCompleteAppCB_opt()
{
    unsigned long ulPrimaryIndexTx = 0x4, ulAltIndexTx = 0x24;
    unsigned long ulPrimaryIndexRx = 0x5, ulAltIndexRx = 0x25;
    tDMAControlTable *pControlTable;
    tCircularBuffer *pAudOutBuf = pRxBuffer;
    tCircularBuffer *pAudInBuf = pTxBuffer;
    unsigned char *pucDMADest;
    unsigned char *pucDMASrc;
    unsigned int dma_status;
    unsigned int i2s_status;
    int i = 0;

    traceISR_ENTER();

	i2s_status = I2SIntStatus(I2S_BASE);
	dma_status = uDMAIntStatus();

	// I2S RX
	if (dma_status & 0x00000010) {
		//UARTprintf("r");
		qqbufsz = GetBufferSize(pAudInBuf);
		HWREG(0x4402609c) = (1 << 10);
		//
		// Get the base address of the control table.
		//
		//pControlTable = (tDMAControlTable *)HWREG(DMA_BASE + UDMA_O_CTLBASE);
		pControlTable = MAP_uDMAControlBaseGet();

		//PRIMARY part of the ping pong
		if ((pControlTable[ulPrimaryIndexTx].ulControl & UDMA_CHCTL_XFERMODE_M)
				== 0) {
#if (CODEC_ENABLE_MULTI_CHANNEL==1)
			guiDMATransferCountTx += CB_TRANSFER_SZ*4; //bytes
			#define END_PTR_DEBUG (CB_TRANSFER_SZ*4)-1
#else
			guiDMATransferCountTx += CB_TRANSFER_SZ*2; //bytes
			#define END_PTR_DEBUG  END_PTR
#endif
			pucDMADest = (unsigned char *) ping;
			pControlTable[ulPrimaryIndexTx].ulControl |= CTRL_WRD;
			pControlTable[ulPrimaryIndexTx].pvDstEndAddr = (void *) (pucDMADest
					+ END_PTR_DEBUG);
			MAP_uDMAChannelEnable(UDMA_CH4_I2S_RX);

			for (i = 0; i < CB_TRANSFER_SZ; i++) {
#if (CODEC_ENABLE_MULTI_CHANNEL==1)
			uint16_t pong_lsb = pong[i] & 0xFFFF;
			uint16_t pong_msb = (pong[i] & 0xFFFF0000) >> 16;
			swap_endian(&pong_lsb);
			swap_endian(&pong_msb);
			pong[i] = ((uint32_t)pong_msb << 16 ) | pong_lsb;
#else
				swap_endian(pong+i);
#endif
			}
#if (CODEC_ENABLE_MULTI_CHANNEL==1)
			FillBuffer(pAudInBuf, (unsigned char*)pong, CB_TRANSFER_SZ*4);
#else
			FillBuffer(pAudInBuf, (unsigned char*)pong, CB_TRANSFER_SZ*2);
#endif
		} else {
			//ALT part of the ping pong
			if ((pControlTable[ulAltIndexTx].ulControl & UDMA_CHCTL_XFERMODE_M)
					== 0) {
#if (CODEC_ENABLE_MULTI_CHANNEL==1)
				guiDMATransferCountTx += CB_TRANSFER_SZ*4; //bytes
#else
				guiDMATransferCountTx += CB_TRANSFER_SZ*2;
#endif
				pucDMADest = (unsigned char *) pong;
				pControlTable[ulAltIndexTx].ulControl |= CTRL_WRD;
				pControlTable[ulAltIndexTx].pvDstEndAddr = (void *) (pucDMADest
						+ END_PTR_DEBUG);
				MAP_uDMAChannelEnable(UDMA_CH4_I2S_RX);

				for (i = 0; i < CB_TRANSFER_SZ; i++) {
#if (CODEC_ENABLE_MULTI_CHANNEL==1)
					uint16_t ping_lsb = ping[i] & 0xFFFF;
					uint16_t ping_msb = (ping[i] & 0xFFFF0000) >> 16;
					swap_endian(&ping_lsb);
					swap_endian(&ping_msb);
					ping[i] = ((uint32_t)ping_msb << 16 ) | ping_lsb;
#else
					swap_endian(ping+i);
#endif
				}
#if (CODEC_ENABLE_MULTI_CHANNEL==1)
				FillBuffer(pAudInBuf, (unsigned char*)ping, CB_TRANSFER_SZ*4);
#else
				FillBuffer(pAudInBuf, (unsigned char*)ping, CB_TRANSFER_SZ*2);
#endif
			}
		}

		// TODO DKH - Decimation here?
#if (AUDIO_RECORD_DECIMATE==1)
		if(decimate_16k(samples,DECIMATE_CHUNK_SIZE))
		{
			UARTprintf("DECIMATE CHUNK SIZE INCORRECT\n");
		}
#endif

#if (CODEC_ENABLE_MULTI_CHANNEL==1)
		// TODO DKH this if condition is redundant?
		if (guiDMATransferCountTx >= CB_TRANSFER_SZ*4)
#else
		// TODO DKH this if condition is redundant?
		if (guiDMATransferCountTx >= CB_TRANSFER_SZ*2)
#endif
			{
			signed long xHigherPriorityTaskWoken;

			guiDMATransferCountTx = 0;

			if ( qqbufsz > LISTEN_WATERMARK ) {
				vTaskNotifyGiveFromISR( audio_task_hndl, &xHigherPriorityTaskWoken );
				portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
			}
		}
	}

	// I2S TX
	if (dma_status & 0x00000020) {
		//UARTprintf("p");
		qqbufsz = GetBufferSize(pAudOutBuf);
		HWREG(0x4402609c) = (1 << 11);
		pControlTable = MAP_uDMAControlBaseGet();

#if (CODEC_ENABLE_MULTI_CHANNEL==1)
			#define END_PTR_DEBUG (CB_TRANSFER_SZ*4)-1
#else
			#define END_PTR_DEBUG  END_PTR
#endif

		if ((pControlTable[ulPrimaryIndexRx].ulControl & UDMA_CHCTL_XFERMODE_M)
				== 0) {
			if ( qqbufsz > CB_TRANSFER_SZ ) {
				guiDMATransferCountRx += CB_TRANSFER_SZ*4;

				memcpy(  (void*)ping_p, (void*)GetReadPtr(pAudOutBuf), CB_TRANSFER_SZ);
				UpdateReadPtr(pAudOutBuf, CB_TRANSFER_SZ);

#if (CODEC_ENABLE_MULTI_CHANNEL==1)
				for (i = CB_TRANSFER_SZ/4-1; i!=-1 ; --i) {
					//the odd ones do not matter
					/*ping[(i<<1)+1] = */
					ping_p[(i<<2) + 0] = (ping_p[i] & 0xFFFFUL) << 16;
					ping_p[(i<<2) + 1] = 0;
					ping_p[(i<<2) + 2] = (ping_p[i] & 0xFFFF0000);
					ping_p[(i<<2) + 3] = 0;
				}
#else
				for (i = CB_TRANSFER_SZ/2-1; i!=-1 ; --i) {
					//the odd ones do not matter
					ping[(i<<1)+1] = 0;
					ping[i<<1] = ping[i];
				}
#endif
			} else {
				memset( ping_p, 0, sizeof(ping_p));
			}

			pucDMASrc = (unsigned char*)ping_p;

			pControlTable[ulPrimaryIndexRx].ulControl |= CTRL_WRD;
			//pControlTable[ulPrimaryIndex].pvSrcEndAddr = (void *)((unsigned long)&gaucZeroBuffer[0] + 15);
			pControlTable[ulPrimaryIndexRx].pvSrcEndAddr =
					(void *) ((unsigned long) pucDMASrc + END_PTR_DEBUG);
			//HWREG(DMA_BASE + UDMA_O_ENASET) = (1 << 5);
			MAP_uDMAChannelEnable(UDMA_CH5_I2S_TX);
		} else {
			if ((pControlTable[ulAltIndexRx].ulControl & UDMA_CHCTL_XFERMODE_M)
					== 0) {
				if ( qqbufsz > CB_TRANSFER_SZ ) {
					guiDMATransferCountRx += CB_TRANSFER_SZ*4;

					memcpy(  (void*)pong_p,  (void*)GetReadPtr(pAudOutBuf), CB_TRANSFER_SZ);
					UpdateReadPtr(pAudOutBuf, CB_TRANSFER_SZ);
#if (CODEC_ENABLE_MULTI_CHANNEL==1)
				for (i = CB_TRANSFER_SZ/4-1; i!=-1 ; --i) {
					//the odd ones do not matter
					/*ping[(i<<1)+1] = */
					pong_p[(i<<2) + 0] = (pong_p[i] & 0xFFFFUL) << 16;
					pong_p[(i<<2) + 1] = 0;
					pong_p[(i<<2) + 2] = (pong_p[i] & 0xFFFF0000);
					pong_p[(i<<2) + 3] = 0;
				}
#else
					for (i = CB_TRANSFER_SZ/2-1; i!=-1 ; --i) {
						//the odd ones do not matter
						pong[(i<<1)+1] = 0;
						pong[i<<1] = pong[i];
					}
#endif
				} else {
					memset( pong_p, 0, sizeof(pong_p));
				}

				pucDMASrc = (unsigned char*)pong_p;

				pControlTable[ulAltIndexRx].ulControl |= CTRL_WRD;
				pControlTable[ulAltIndexRx].pvSrcEndAddr =
						(void *) ((unsigned long) pucDMASrc + END_PTR_DEBUG);
				//HWREG(DMA_BASE + UDMA_O_ENASET) = (1 << 5);
				MAP_uDMAChannelEnable(UDMA_CH5_I2S_TX);
			}
		}

		if (guiDMATransferCountRx >= CB_TRANSFER_SZ) {
			signed long xHigherPriorityTaskWoken;
			qqbufsz = GetBufferSize(pAudOutBuf);
			guiDMATransferCountRx = 0;

			if ( qqbufsz < PLAY_WATERMARK ) {
#if (AUDIO_FULL_DUPLEX==1)
				vTaskNotifyGiveFromISR( audio_task_hndl_p, &xHigherPriorityTaskWoken );
#else
				vTaskNotifyGiveFromISR( audio_task_hndl, &xHigherPriorityTaskWoken );
#endif
				portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
			}
		}
	}
	uDMAIntClear(dma_status);
	I2SIntClear(I2S_BASE, i2s_status );

	traceISR_EXIT();
}

#endif

//*****************************************************************************
//
//! configuring the DMA transfer
//!
//! \param psAudPlayerCtrl is the control structure managing the input data.
//!
//! This function
//!        1. setting the source and the destination for the DMA transfer.
//!		   2. setting the uDMA registers to control actual transfer.
//!
//! \return None.
//
//*****************************************************************************
void SetupPingPongDMATransferTx()
{
    unsigned int * puiTxSrcBuf = AudioCapturerGetDMADataPtr();

    memset(ping, 0, sizeof(ping));
    memset(pong, 0, sizeof(pong));

#if (CODEC_ENABLE_MULTI_CHANNEL==1)
    SetupTransfer(UDMA_CH4_I2S_RX,
                  UDMA_MODE_PINGPONG,
                  CB_TRANSFER_SZ,
                  UDMA_SIZE_32,
                  UDMA_ARB_8,
                  (void *)puiTxSrcBuf,
                  UDMA_CHCTL_SRCINC_NONE,
                  (void *)ping,
                  UDMA_CHCTL_DSTINC_32);
    SetupTransfer(UDMA_CH4_I2S_RX|UDMA_ALT_SELECT,
                  UDMA_MODE_PINGPONG,
                  CB_TRANSFER_SZ,
                  UDMA_SIZE_32,
                  UDMA_ARB_8,
                  (void *)puiTxSrcBuf,
                  UDMA_CHCTL_SRCINC_NONE,
                  (void *)pong,
                  UDMA_CHCTL_DSTINC_32);
#else
    // changed to SD card DMA UDMA_CH14_SDHOST_RX
    SetupTransfer(UDMA_CH4_I2S_RX,
                  UDMA_MODE_PINGPONG,
                  CB_TRANSFER_SZ, 
                  UDMA_SIZE_16,
                  UDMA_ARB_8,
                  (void *)puiTxSrcBuf, 
                  UDMA_CHCTL_SRCINC_NONE,
                  (void *)ping,
                  UDMA_CHCTL_DSTINC_16);
    SetupTransfer(UDMA_CH4_I2S_RX|UDMA_ALT_SELECT,
                  UDMA_MODE_PINGPONG,
                  CB_TRANSFER_SZ, 
                  UDMA_SIZE_16,
                  UDMA_ARB_8,
                  (void *)puiTxSrcBuf, 
                  UDMA_CHCTL_SRCINC_NONE,
                  (void *)pong,
                  UDMA_CHCTL_DSTINC_16);
#endif
}

void SetupPingPongDMATransferRx()
{

	unsigned int * puiRxDestBuf = AudioRendererGetDMADataPtr();

    memset(ping_p, 0, sizeof(ping_p));
    memset(pong_p, 0, sizeof(pong_p));

#if (CODEC_ENABLE_MULTI_CHANNEL==1)
    SetupTransfer(UDMA_CH5_I2S_TX,
                  UDMA_MODE_PINGPONG,
                  CB_TRANSFER_SZ,
                  UDMA_SIZE_32,
                  UDMA_ARB_8,
                  (void *)ping_p,
                  UDMA_CHCTL_SRCINC_32,
                  (void *)puiRxDestBuf,
                  UDMA_DST_INC_NONE);
    SetupTransfer(UDMA_CH5_I2S_TX|UDMA_ALT_SELECT,
                  UDMA_MODE_PINGPONG,
                  CB_TRANSFER_SZ,
                  UDMA_SIZE_32,
                  UDMA_ARB_8,
                  (void *)pong_p,
                  UDMA_CHCTL_SRCINC_32,
                  (void *)puiRxDestBuf,
                  UDMA_DST_INC_NONE);
#else
    SetupTransfer(UDMA_CH5_I2S_TX,
                  UDMA_MODE_PINGPONG,
                  CB_TRANSFER_SZ,
                  UDMA_SIZE_16,
                  UDMA_ARB_8,
                  (void *)ping,
                  UDMA_CHCTL_SRCINC_16,
                  (void *)puiRxDestBuf, 
                  UDMA_DST_INC_NONE);
    SetupTransfer(UDMA_CH5_I2S_TX|UDMA_ALT_SELECT,
                  UDMA_MODE_PINGPONG,
                  CB_TRANSFER_SZ, 
                  UDMA_SIZE_16,
                  UDMA_ARB_8,
                  (void *)pong,
                  UDMA_CHCTL_SRCINC_16,
                  (void *)puiRxDestBuf, 
                  UDMA_DST_INC_NONE);
#endif
    
}

#if (AUDIO_RECORD_DECIMATE==1)
	
static decimation_dump_t decimation_dump;
#define DECIMATION_FACTOR 3

#if (CODEC_ENABLE_MULTI_CHANNEL==1)
#define DECIMATION_CHUNK_SIZE (CB_TRANSFER_SZ*2)
#else
#define DECIMATE_CHUNK_SIZE CB_TRANSFER_SZ
#endif

#if (CODEC_ENABLE_MULTI_CHANNEL==1)
#define NUMEBR_OF_AUDIO_CHANNEL 4
#else
#define NUMEBR_OF_AUDIO_CHANNEL 2
#endif

// Decimate from 48k to 16k
int32_t decimate_16k(uint8_t* data)
{
	uint16_t* u16p_data = (uint16_t*)data;
	uint32_t total_words = DECIMATION_CHUNK_SIZE;

	uint32_t i, channel_index, array_index;
	uint32_t array_max;

	uint32_t start_index = 0;

	// data should be a valid pointer
	if(!data) return -1;
	
	// Check if there is data from the previous DMA chunk
	if(decimation_dump.pending)
	{
		// These are words from previous DMA transfer that need to be decimated
		for(i=0;i<(DECIMATION_FACTOR-decimation_dump.pending_data_per_channel);i++)
		{
			for(channel_index=0;channel_index < NUMEBR_OF_AUDIO_CHANNEL; channel_index++)
			{
				decimation_dump.pending_sum[channel_index] += u16p_data[i+channel_index];
			}

			total_words -= NUMEBR_OF_AUDIO_CHANNEL;
			start_index += NUMEBR_OF_AUDIO_CHANNEL;
		}

		for(channel_index=0;channel_index < NUMEBR_OF_AUDIO_CHANNEL; channel_index++)
		{
			u16p_data[channel_index] = decimation_dump.pending_sum[channel_index] / DECIMATION_FACTOR;
			decimation_dump.pending_sum[channel_index] = 0;
		}
	}

	// Converting to a multiple of 12. Might seem pointless but it is not.
	array_max = (total_words/12) * 12;

	// Here total words should be a multiple of 12
	for(array_index=start_index;array_index<array_max/3;array_index += 4)
	{
		for(channel_index=0;channel_index<NUMEBR_OF_AUDIO_CHANNEL;channel_index++)
		{
			for(i=0;i<DECIMATION_FACTOR;i++)
			{
				u16p_data[array_index+channel_index] += u16p_data[3*array_index+channel_index+i*4];
			}

			// Divide by decimation factor
			u16p_data[array_index+channel_index] /= 3;
		}
	}

	// Add the remaining data to the decimation dump
	if(total_words - array_max)
	{
		decimation_dump.pending = true;
		decimation_dump.pending_data_per_channel = 0;

		for(i=array_max;i<(total_words - array_max)/4;i++)
		{
			decimation_dump.pending_data_per_channel++;
			for(channel_index=0;channel_index<NUMEBR_OF_AUDIO_CHANNEL;channel_index++)
			{
				decimation_dump.pending_sum[channel_index] += u16p_data[i+channel_index];

			}

		}

	}


	// Return number of bytes/words after decimation
	return 0;
}
#endif
//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************

