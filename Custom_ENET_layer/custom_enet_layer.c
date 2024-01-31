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



/*!
 * @brief Initializes the Custom Ethernet layer
 *
 * Call this API to initialize the Ethernet custom later
 *
 * @param cb_funct     Function pointer to the receive callback
 */
status_t Custom_ENET_Later_Init(receive_cb_function cb_func)
{

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
status_t Custom_ENET_Later_Transmit(uint8_t *data, uint32_t len)
{

}
