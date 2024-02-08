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


// AES128 defines

#define AES128_KEY	{0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06}
#define AES128_IV	{0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06}

/* @TEST_ANCHOR*/

#ifndef SOURCE_MAC_ADDRESS
#define SOURCE_MAC_ADDRESS {0xd4, 0xbe, 0xd9, 0x45, 0x22, 0x61}
#endif

#ifndef DESTINATION_MAC_ADDRESS
#define DESTINATION_MAC_ADDRESS {0x58, 0x11, 0x22, 0x86, 0x21, 0xe8}
//#define DESTINATION_MAC_ADDRESS {0xff, 0xff, 0xff, 0xff, 0xff, 0xff}
#endif

#endif /* CUSTOM_ENET_LAYER_CONFIG_H_ */
