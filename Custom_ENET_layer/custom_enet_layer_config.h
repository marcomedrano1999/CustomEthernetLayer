/*
 * custom_enet_layer_config.h
 *
 *  Created on: 31 ene. 2024
 *      Author: marco
 */

#ifndef CUSTOM_ENET_LAYER_CONFIG_H_
#define CUSTOM_ENET_LAYER_CONFIG_H_

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/* ENET base address */
#define EXAMPLE_ENET     Driver_ETH_MAC0
#define EXAMPLE_ENET_PHY Driver_ETH_PHY0
#define ENET_DATA_LENGTH        (1000)

/* @TEST_ANCHOR*/

#ifndef MAC_ADDRESS
#define MAC_ADDRESS {0xd4, 0xbe, 0xd9, 0x45, 0x22, 0x61}
#endif

#endif /* CUSTOM_ENET_LAYER_CONFIG_H_ */
