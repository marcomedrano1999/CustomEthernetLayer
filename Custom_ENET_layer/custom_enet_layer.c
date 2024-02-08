/*
 * custom_enet_layer.c
 *
 *  Created on: 31 ene. 2024
 *      Author: marco
 */


#include "custom_enet_layer.h"

/*  SDK Included Files */
#include "Driver_ETH_MAC.h"
#include "pin_mux.h"
#include "board.h"
#include "fsl_debug_console.h"
#include "fsl_enet.h"
#include "fsl_enet_cmsis.h"
#include "fsl_enet_phy_cmsis.h"
#include "fsl_phy.h"
#include "stdlib.h"

#include "fsl_common.h"
#include "fsl_sysmpu.h"
#include "fsl_phyksz8081.h"
#include "fsl_enet_mdio.h"
#include "RTE_Device.h"
#include "fsl_crc.h"
#include "aes.h"



/*******************************************************************************
 * Variables
 ******************************************************************************/
uint8_t g_frame[ENET_DATA_LENGTH + 14];
volatile uint32_t g_testTxNum  = 0;
uint8_t source_macAddr[6]           = SOURCE_MAC_ADDRESS;
uint8_t destination_macAddr[6]      = DESTINATION_MAC_ADDRESS;
volatile uint32_t g_rxIndex    = 0;
volatile uint32_t g_rxCheckIdx = 0;
volatile uint32_t g_txCheckIdx = 0;

receive_cb_function *User_ENET_Receive_Cb;

// CRC32 variables
CRC_Type *base = CRC0;
uint32_t checksum32;

// AES128 variables
uint8_t key[] = AES128_KEY;
uint8_t iv[] = AES128_IV;
struct AES_ctx ctx;

/*******************************************************************************
 * Code
 ******************************************************************************/
mdio_handle_t mdioHandle = {.ops = &enet_ops};
phy_handle_t phyHandle   = {.phyAddr = RTE_ENET_PHY_ADDRESS, .mdioHandle = &mdioHandle, .ops = &phyksz8081_ops};

/*******************************************************************************
 * Static functions
 ******************************************************************************/
uint32_t ENET0_GetFreq(void)
{
    return CLOCK_GetFreq(kCLOCK_CoreSysClk);
}


uint32_t Compute_Padding(uint8_t *data, uint32_t len)
{
	uint32_t len_wo_padding = len;

	// Eliminate padding only when the size is the minimum
	if(len == 60){
		for(uint8_t i=len-1; i>0;i--)
		{
			if(data[i]!=0)
			{
				break;
			}
			len_wo_padding--;
		}
	}

	return len_wo_padding;
}


static void aes128_init()
{
	// Init the AES context structure
	AES_init_ctx_iv(&ctx, key, iv);

}


static uint32_t aes128_encrypt(uint8_t *data, uint32_t len, uint8_t *output_array)
{
	uint32_t padded_len = len + (16 - (len%16));

	// Copy data to final array
	memcpy(output_array, data, len);

	// Encrypt the data
	AES_CBC_encrypt_buffer(&ctx, output_array, padded_len);

	return padded_len;
}


/*!
 * @brief Init for CRC-32.
 * @details Init CRC peripheral module for CRC-32 protocol.
 *          width=32 poly=0x04c11db7 init=0xffffffff refin=true refout=true xorout=0xffffffff check=0xcbf43926
 *          name="CRC-32"
 *          http://reveng.sourceforge.net/crc-catalogue/
 */
static void InitCrc32(CRC_Type *base, uint32_t seed)
{
    crc_config_t config;

    config.polynomial         = 0x04C11DB7U;
    config.seed               = seed;
    config.reflectIn          = true;
    config.reflectOut         = true;
    config.complementChecksum = true;
    config.crcBits            = kCrcBits32;
    config.crcResult          = kCrcFinalChecksum;

    CRC_Init(base, &config);
}

uint32_t ComputeCRC32(uint8_t *testData, uint32_t size)
{
	CRC_WriteData(base, (uint8_t *)&testData[0], size);
	return CRC_Get32bitResult(base);
}


uint8_t verifyDataCRC(uint8_t *data, uint32_t size)
{
	uint8_t status = CRC_NOT_OK;
	uint32_t dataCRC32 = 0;
	uint32_t computedCRC32 = 0;

	// Extract data CRC32
	dataCRC32 |= data[size-4] << 24;
	dataCRC32 |= data[size-3] << 16;
	dataCRC32 |= data[size-2] << 8;
	dataCRC32 |= data[size-1];

	// Compute CRC for data. Subtract 14 for the header and 4 for the CRC32
	computedCRC32 = ComputeCRC32(&data[14], size-14-4);

	// Compare the CRC32s
	if(computedCRC32 == dataCRC32)
	{
		status = CRC_OK;
	}

	return status;
}



/*! @brief Build Frame for transmit. */
static uint32_t ENET_BuildBroadCastFrame(uint8_t *data, uint32_t len)
{
	uint32_t data_len = 0;

	// Clean the output frame
	memset(&g_frame[0], 0, sizeof(g_frame));

    // Copy destination MAC address
    memcpy(&g_frame[0], &destination_macAddr[0], 6U);

    // Copy source MAC address
    memcpy(&g_frame[6], &source_macAddr[0], 6U);

    // Encode the message using AES128
    data_len = aes128_encrypt(data, len, &g_frame[14]);

	// Disable interrupts in the CRC32 computation to prevent
	//  data corruption if a frame is received in the process
	__disable_irq();

	// Add CRC32
	uint32_t dataCRC32 = ComputeCRC32(&g_frame[14], data_len);

	// Enable all interrupts again
	__enable_irq();

    // Copy CRC frame to output array
    g_frame[data_len + 14] = (dataCRC32 >> 24) & 0xFFU;
    g_frame[data_len + 15] = (dataCRC32 >> 16) & 0xFFU;
    g_frame[data_len + 16] = (dataCRC32 >> 8) & 0xFFU;
    g_frame[data_len + 17] = dataCRC32 & 0xFFU;

    // Add byte length of CRC32 to the data length
    data_len += 4;

    // Set data length in EtherType field
    g_frame[12] = (len >> 8) & 0xFFU;
    g_frame[13] = len & 0xFFU;


    return (data_len + 18);
}





void Custom_ENET_Layer_Receive_Cb(uint32_t event)
{
	uint32_t size;
	uint32_t len;
	uint32_t len_wo_padding;
	uint32_t data_len;
	uint8_t *data;

	if (event == ARM_ETH_MAC_EVENT_RX_FRAME)
	{

		/* Get the Frame size */
		size = EXAMPLE_ENET.GetRxFrameSize();
		/* Call ENET_ReadFrame when there is a received frame. */
		if (size != 0)
		{
			/* Received valid frame. Deliver the rx buffer with the size equal to length. */
			data = (uint8_t *)malloc(size);
			if (data)
			{
				len = EXAMPLE_ENET.ReadFrame(data, size);
				if (size == len)
				{

					// Compute length without padding
					len_wo_padding = Compute_Padding(data, len);

					// Verify the data with CRC32
					if( verifyDataCRC(data, len_wo_padding) == CRC_OK)
					{
						// Subtract the MAC header and the CRC32 size
						data_len = len_wo_padding-14-4;

						// Decode the message
						AES_CBC_decrypt_buffer(&ctx, &data[14], data_len);

						// Extract the decrypted data len from the EtherType field
						data_len = (data[12] << 8) + data[13];

						// Call the user receive callback
						User_ENET_Receive_Cb(&data[14], data_len);
					}
					else
					{
						PRINTF("Data present errors in CRC32 verification!\r\n");
					}
				}
				free(data);
			}
		}
	}


}



/*******************************************************************************
 * Custom Layer APIs
 ******************************************************************************/

/*!
 * @brief Initializes the Custom Ethernet layer
 *
 * Call this API to initialize the Ethernet custom later
 *
 * @param cb_funct     Function pointer to the receive callback
 */
void Custom_ENET_Layer_Init(receive_cb_function cb_func)
{
	// Save user's receive callback
	if(cb_func != NULL)
	{
		User_ENET_Receive_Cb = cb_func;
	}

    ARM_ETH_LINK_INFO linkInfo;

    /* Disable SYSMPU. */
    SYSMPU_Enable(SYSMPU, false);

    mdioHandle.resource.base        = ENET;
    mdioHandle.resource.csrClock_Hz = ENET0_GetFreq();

    PRINTF("\r\nENET example start.\r\n");

    /* Initialize the ENET module. */
    EXAMPLE_ENET.Initialize(Custom_ENET_Layer_Receive_Cb);
    EXAMPLE_ENET.PowerControl(ARM_POWER_FULL);
    EXAMPLE_ENET.SetMacAddress((ARM_ETH_MAC_ADDR *)source_macAddr);

    PRINTF("Wait for PHY init...\r\n");
    while (EXAMPLE_ENET_PHY.PowerControl(ARM_POWER_FULL) != ARM_DRIVER_OK)
    {
        PRINTF("PHY Auto-negotiation failed, please check the cable connection and link partner setting.\r\n");
    }

    EXAMPLE_ENET.Control(ARM_ETH_MAC_CONTROL_RX, 1);
    EXAMPLE_ENET.Control(ARM_ETH_MAC_CONTROL_TX, 1);
    PRINTF("Wait for PHY link up...\r\n");
    do
    {
        if (EXAMPLE_ENET_PHY.GetLinkState() == ARM_ETH_LINK_UP)
        {
            linkInfo = EXAMPLE_ENET_PHY.GetLinkInfo();
            EXAMPLE_ENET.Control(ARM_ETH_MAC_CONFIGURE, linkInfo.speed << ARM_ETH_MAC_SPEED_Pos |
                                                            linkInfo.duplex << ARM_ETH_MAC_DUPLEX_Pos |
                                                            ARM_ETH_MAC_ADDRESS_BROADCAST);
            break;
        }
    } while (1);

#if defined(PHY_STABILITY_DELAY_US) && PHY_STABILITY_DELAY_US
    /* Wait a moment for PHY status to be stable. */
    SDK_DelayAtLeastUs(PHY_STABILITY_DELAY_US, SDK_DEVICE_MAXIMUM_CPU_CLOCK_FREQUENCY);
#endif


    // Initialize the CRC module
    InitCrc32(base, 0xFFFFFFFFU);

    // Init the AES128 module
    aes128_init();

}


/*!
 * @brief Transmits a message using the custom Ethernet layer
 *
 * Call this API to transmit data through the custom Ethernet layer.
 * 	The message will be encrypted using AES128 and will have an extra
 * 	verification step by using CRC32
 *
 * @param data     	Pointer to the data array.
 * @param len   	Length of the data to be transmitted
 */
void Custom_ENET_Layer_Transmit(uint8_t *data, uint32_t len)
{
	uint32_t BroadCastFrame_Size=0;

	// Assembly output frame
	BroadCastFrame_Size = ENET_BuildBroadCastFrame(data, len);

	/* Send a multicast frame when the PHY is link up. */
	if (EXAMPLE_ENET.SendFrame(&g_frame[0], BroadCastFrame_Size, ARM_ETH_MAC_TX_FRAME_EVENT) == ARM_DRIVER_OK)
	{
		SDK_DelayAtLeastUs(1000, SDK_DEVICE_MAXIMUM_CPU_CLOCK_FREQUENCY);
	}
	else
	{
		PRINTF(" \r\nTransmit frame failed!\r\n");
	}

}
