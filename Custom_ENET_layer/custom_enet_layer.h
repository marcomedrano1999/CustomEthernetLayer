/*
 * custom_enet_layer.h
 *
 *  Created on: 31 ene. 2024
 *      Author: marco
 */

#ifndef CUSTOM_ENET_LAYER_H_
#define CUSTOM_ENET_LAYER_H_

#include "custom_enet_layer_config.h"


typedef void receive_cb_function(uint32_t event);

/* Custom layer initialization function */
status_t Custom_ENET_Later_Init(receive_cb_function cb_func);
/* Custom layer transmit function */
status_t Custom_ENET_Later_Transmit(uint8_t *data, uint32_t len);

#endif /* CUSTOM_ENET_LAYER_H_ */
