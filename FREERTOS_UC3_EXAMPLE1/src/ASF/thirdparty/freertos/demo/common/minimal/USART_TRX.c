/*
 * model.c
 *
 * Created: 23.05.2018 18:42:13
 *  Author: Maciej Kuśnierz
 */ 
#include <stdlib.h>
#include <avr32/io.h>
#include "compiler.h"
#include "board.h"
#include "power_clocks_lib.h"
#include "gpio.h"
#include "usart.h"


/* Scheduler include files. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

/* Demo program include files. */
#include "partest.h"
#include "USART_TRX.h"

#define ledSTACK_SIZE		configMINIMAL_STACK_SIZE

#  define EXAMPLE_TARGET_PBACLK_FREQ_HZ FOSC0  // PBA clock target frequency, in Hz

#if BOARD == EVK1100
#  define EXAMPLE_USART                 (&AVR32_USART0)
#  define EXAMPLE_USART_RX_PIN          AVR32_USART0_RXD_0_0_PIN
#  define EXAMPLE_USART_RX_FUNCTION     AVR32_USART0_RXD_0_0_FUNCTION
#  define EXAMPLE_USART_TX_PIN          AVR32_USART0_TXD_0_0_PIN
#  define EXAMPLE_USART_TX_FUNCTION     AVR32_USART0_TXD_0_0_FUNCTION
#  define EXAMPLE_USART_CLOCK_MASK      AVR32_USART0_CLK_PBA
#  define EXAMPLE_PDCA_CLOCK_HSB        AVR32_PDCA_CLK_HSB
#  define EXAMPLE_PDCA_CLOCK_PB         AVR32_PDCA_CLK_PBA
#endif

#if !defined(EXAMPLE_USART)             || \
!defined(EXAMPLE_USART_RX_PIN)      || \
!defined(EXAMPLE_USART_RX_FUNCTION) || \
!defined(EXAMPLE_USART_TX_PIN)      || \
!defined(EXAMPLE_USART_TX_FUNCTION)
#  error The USART configuration to use in this example is missing.
#endif

extern xQueueHandle xQueueTrx;

/* Variable used by the created tasks to calculate the LED number to use, and
the rate at which they should flash the LED. */
static volatile unsigned portBASE_TYPE uxFlashTaskNumber = 0;

/* The task that is created three times. */
static portTASK_FUNCTION_PROTO( vTrxTask, pvParameters );

/*-----------------------------------------------------------*/

void vStartTrxTask( unsigned portBASE_TYPE uxPriority )
{
signed portBASE_TYPE xLEDTask;

	/* Create the three tasks. */
	//for( xLEDTask = 0; xLEDTask < ledNUMBER_OF_LEDS; ++xLEDTask )
	{
		/* Spawn the task. */
		xTaskCreate( vTrxTask, ( signed char * ) "LEDx", ledSTACK_SIZE, NULL, uxPriority, ( xTaskHandle * ) NULL );
	}
}
/*-----------------------------------------------------------*/
int mama=(int)'a';
int DataToReadTRX=0;
int resTrx = 20;
static portTASK_FUNCTION( vTrxTask, pvParameters )
{

	/* Calculate the LED and flash rate. */
	portENTER_CRITICAL();
	{
		/* See which of the eight LED's we should use. */
		//uxLED = uxFlashTaskNumber;
		//uxLED = 1;
		/* Update so the next task uses the next LED. */
		//uxFlashTaskNumber++;
	}
	portEXIT_CRITICAL();
	
	// Configure Osc0 in crystal mode (i.e. use of an external crystal source, with
	// frequency FOSC0) with an appropriate startup time then switch the main clock
	// source to Osc0.
	pcl_switch_to_osc(PCL_OSC0, FOSC0, OSC0_STARTUP);
	//const TickType_t xBlockTime = pdMSTOTICKS(200);
	static const gpio_map_t USART_GPIO_MAP =
	{
		{EXAMPLE_USART_RX_PIN, EXAMPLE_USART_RX_FUNCTION},
		{EXAMPLE_USART_TX_PIN, EXAMPLE_USART_TX_FUNCTION}
	};

	// USART options.
	static const usart_options_t USART_OPTIONS =
	{
		.baudrate     = 115200,
		.charlength   = 8,
		.paritytype   = USART_NO_PARITY,
		.stopbits     = USART_1_STOPBIT,
		.channelmode  = USART_NORMAL_CHMODE
	};

	// Assign GPIO to USART.
	gpio_enable_module(USART_GPIO_MAP,
	sizeof(USART_GPIO_MAP) / sizeof(USART_GPIO_MAP[0]));

	// Initialize USART in RS232 mode.
	usart_init_rs232(EXAMPLE_USART, &USART_OPTIONS, EXAMPLE_TARGET_PBACLK_FREQ_HZ);
	
	for(;;)
	{
		resTrx= uxQueueMessagesWaiting(xQueueTrx);
		if(xQueueTrx != 0)
		{
			if(!xQueueReceive(xQueueTrx, (void*) &DataToReadTRX, 10))
			{
				if(resTrx != 0);
				{
				while(usart_putchar(EXAMPLE_USART, DataToReadTRX) != USART_SUCCESS);
				}
			} 

			//while(usart_putchar(EXAMPLE_USART, 45));			
		}
		vTaskDelay(5);
	}
} 

