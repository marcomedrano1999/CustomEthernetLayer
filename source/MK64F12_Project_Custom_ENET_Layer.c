/*
 * Copyright 2016-2024 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/**
 * @file    MK64F12_Project_Custom_ENET_Layer.c
 * @brief   Application entry point.
 */
#include <stdio.h>
#include "board.h"
#include "peripherals.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "MK64F12.h"
#include "fsl_debug_console.h"
/* TODO: insert other include files here. */
#include "custom_enet_layer.h"
/* TODO: insert other definitions and declarations here. */

void application_callback(uint8_t *data, uint32_t len)
{
	// Print the receive buffer on the console
	PRINTF((char*)data);
}


/*
 * @brief   Application entry point.
 */
int main(void) {

    /* Init board hardware. */
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitBootPeripherals();
#ifndef BOARD_INIT_DEBUG_CONSOLE_PERIPHERAL
    /* Init FSL debug console. */
    BOARD_InitDebugConsole();
#endif

    // Initialize the layer
    Custom_ENET_Later_Init(application_callback);

	// Send first package
	char pck1[] = "01234567890123456789";
	Custom_ENET_Later_Transmit((uint8_t*)pck1, sizeof(pck1));

	// Send second package
	char pck2[] = "Hola Mundo";
	Custom_ENET_Later_Transmit((uint8_t*)pck2, sizeof(pck2));

	// Send third package
	char pck3[] = "Al que madruga, dios le ayuda";
	Custom_ENET_Later_Transmit((uint8_t*)pck3, sizeof(pck3));

	// Send fourth package
	char pck4[] = "Mas vale tarde que nunca";
	Custom_ENET_Later_Transmit((uint8_t*)pck4, sizeof(pck4));

	// Send fifth package
	char pck5[] = "Otoño lluvioso, año copioso";
	Custom_ENET_Later_Transmit((uint8_t*)pck5, sizeof(pck5));

	// Send sixth package
	char pck6[] = "Mas vale pajaro en mano que ciento volando";
	Custom_ENET_Later_Transmit((uint8_t*)pck6, sizeof(pck6));

	// Send seventh package
	char pck7[] = "Me han dicho un dicho, que dicen que he dicho yo. Ese dicho está mal dicho, pues si yo lo hubiera dicho, estaría mejor dicho, que ese dicho que dicen que algún día dije yo.";
	Custom_ENET_Later_Transmit((uint8_t*)pck7, sizeof(pck7));

	// Send eighth package
	char pck8[] = "Pablito clavó un clavito en la calva de un calvito. Un clavito clavó Pablito en la calva de un calvito. ¿Qué clavito clavó Pablito?";
	Custom_ENET_Later_Transmit((uint8_t*)pck8, sizeof(pck8));


    /* Enter an infinite loop, just incrementing a counter. */
    while(1) {};

    return 0 ;
}
