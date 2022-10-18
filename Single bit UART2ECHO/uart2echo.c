/*
 * Copyright (c) 2020, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 *  ======== uart2echo.c ========
 */
#include <stdint.h>
#include <stddef.h>

/* Driver Header files */
#include <ti/drivers/GPIO.h>
#include <ti/drivers/UART2.h>

/* Driver configuration */
#include "ti_drivers_config.h"

/*
 *  ======== mainThread ========
 */
void* mainThread(void *arg0)
{
    char input;
    const char echoPrompt[] = "Echoing characters:\r\n";
    UART2_Handle uart;
    UART2_Params uartParams;
    size_t bytesWritten = 0;
    uint32_t status = UART2_STATUS_SUCCESS;

    /* Call driver init functions */
    GPIO_init();

    /* Configure the LED pin */
    GPIO_setConfig(CONFIG_GPIO_LED_0, GPIO_CFG_OUT_STD | GPIO_CFG_OUT_LOW);

    /* Create a UART where the default read and write mode is BLOCKING */
    UART2_Params_init(&uartParams);
    uartParams.baudRate = 115200;

    uart = UART2_open(CONFIG_UART2_0, &uartParams);

    if (uart == NULL)
    {
        /* UART2_open() failed */
        while (1)
            ;
    }

    /* Turn on user LED to indicate successful initialization */
    GPIO_write(CONFIG_GPIO_LED_0, CONFIG_GPIO_LED_ON);

    UART2_write(uart, echoPrompt, sizeof(echoPrompt), &bytesWritten);

    enum LED_States
    {
        LED0_SMStart, LED0_N_OR_F, LED0_F_2, LED0_on, LED0_off
    } LED_State;    // Enumerate all LED states (declare integral constants)

    /* Loop forever echoing */
    while (1)
    {
        status = UART2_read(uart, &input, 1, &bytesWritten); // Read input with each loop
        if (status != UART2_STATUS_SUCCESS)
        {
            /*UART2_read() FAILED*/
            while (1);
        }

        switch (LED_State)
        {                 // State transitions
        case LED0_SMStart:              // current state, looking for input == 'o'
            if (input == 'o')
            {
                LED_State = LED0_N_OR_F;       // 'o' found, move to N_OR_F case
            }
            break;
        case LED0_N_OR_F:          // N_OR_F case, looking for either input == 'n' or 'f'
            if (input == 'n')
            {
                LED_State = LED0_on; // 'n' found, move to LED0_on, which will activate LED
            }
            else if (input == 'f')
            {
                LED_State = LED0_F_2; // 'f' found, move to F_2 for another input 'f'
            }
            break;
        case LED0_F_2:                       // F_2 case, looking for second 'f' in off
            if (input == 'f')
            {
                LED_State = LED0_off; // Second 'f' found, move to LED0_off, which will deactivate LED
            }
            break;
        default:
            LED_State = LED0_SMStart;     // Default state transitions to start
            break;
        }

        switch (LED_State)
        {                 // State actions
        case LED0_on:
            GPIO_write(CONFIG_GPIO_LED_0, CONFIG_GPIO_LED_ON);  // LED0 ON
            LED_State = LED0_SMStart;                 // Transition state to start
            break;

        case LED0_off:
            GPIO_write(CONFIG_GPIO_LED_0, CONFIG_GPIO_LED_OFF); // LED0 OFF
            LED_State = LED0_SMStart;                 // Transition state to start
            break;
        }
        status = UART2_write(uart, &input, 1, &bytesWritten);

        if (status != UART2_STATUS_SUCCESS)
        {
            /* UART2_write() failed */
            while (1);
        }
    }
}
