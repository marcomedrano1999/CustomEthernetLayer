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





/*******************************************************************************
 * Variables
 ******************************************************************************/
uint8_t g_frame[ENET_DATA_LENGTH + 14];
volatile uint32_t g_testTxNum  = 0;
uint8_t g_macAddr[6]           = MAC_ADDRESS;
volatile uint32_t g_rxIndex    = 0;
volatile uint32_t g_rxCheckIdx = 0;
volatile uint32_t g_txCheckIdx = 0;

receive_cb_function *User_ENET_Receive_Cb;

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

/*! @brief Build Frame for transmit. */
static void ENET_BuildBroadCastFrame(uint8_t *data, uint32_t data_len)
{
    uint32_t count  = 0;
    uint32_t length = ENET_DATA_LENGTH - 14;

    for (count = 0; count < 6U; count++)
    {
        g_frame[count] = 0xFFU;
    }
    memcpy(&g_frame[6], &g_macAddr[0], 6U);
    g_frame[12] = (length >> 8) & 0xFFU;
    g_frame[13] = length & 0xFFU;

    for(count = 0; count < data_len; count++)
    {
    	g_frame[count+14] = data[count];
    }

    for (count = 0; count < (length - data_len); count++)
    {
        g_frame[count + data_len + 14] = count % 0xFFU;
    }
}


void Custom_ENET_Layer_Receive_Cb(uint32_t event)
{

	if (event == ARM_ETH_MAC_EVENT_RX_FRAME)
	{
		uint32_t size;
		uint32_t len;

		/* Get the Frame size */
		size = EXAMPLE_ENET.GetRxFrameSize();
		/* Call ENET_ReadFrame when there is a received frame. */
		if (size != 0)
		{
			/* Received valid frame. Deliver the rx buffer with the size equal to length. */
			uint8_t *data = (uint8_t *)malloc(size);
			if (data)
			{
				len = EXAMPLE_ENET.ReadFrame(data, size);
				if (size == len)
				{
					// Verify the data with CRC32


					// Decode the message


					// Call the user receive callback
					User_ENET_Receive_Cb(data, len);
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
void Custom_ENET_Later_Init(receive_cb_function cb_func)
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
    EXAMPLE_ENET.SetMacAddress((ARM_ETH_MAC_ADDR *)g_macAddr);

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
void Custom_ENET_Later_Transmit(uint8_t *data, uint32_t len)
{
	ENET_BuildBroadCastFrame(data, len);


	// Encode the message using AES128


	// Add CRC32


	/* Send a multicast frame when the PHY is link up. */
	if (EXAMPLE_ENET.SendFrame(&g_frame[0], ENET_DATA_LENGTH, ARM_ETH_MAC_TX_FRAME_EVENT) == ARM_DRIVER_OK)
	{
		SDK_DelayAtLeastUs(1000, SDK_DEVICE_MAXIMUM_CPU_CLOCK_FREQUENCY);
	}
	else
	{
		PRINTF(" \r\nTransmit frame failed!\r\n");
	}

}
