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
	PRINTF(data);
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
    char pck1[] = "Hola 1";
    Custom_ENET_Later_Transmit(pck1, sizeof(pck1));


    /* Enter an infinite loop, just incrementing a counter. */
    while(1) {};

    return 0 ;
}
