/*
 * model.c
 *
 * Created: 23.05.2018 18:42:13
 *  Author: Maciej Kuśnierz
 */ 
#include <stdlib.h>

/* Scheduler include files. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include <avr32/io.h>
#include "compiler.h"
#include "gpio.h"
#include "power_clocks_lib.h"
#include "board.h"
#include "conf_example1.h"
#include "eic.h"
/* Demo program include files. */
#include "partest.h"
#include "USART_RCV.h"
#include "usart.h"

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



extern xQueueHandle xQueueRcv;

/* Variable used by the created tasks to calculate the LED number to use, and
the rate at which they should flash the LED. */
static volatile unsigned portBASE_TYPE uxFlashTaskNumber = 0;

/* The task that is created three times. */
static portTASK_FUNCTION_PROTO( vRcvTask, pvParameters );

/*-----------------------------------------------------------*/

/** \brief Toggle LED 0 for a short while.
 *
 */
static void toggle_led(void)
{
	volatile uint32_t i;
	volatile uint32_t j = 30;
	do {
		j--;
		i = 100000;
		while (i--);

		gpio_tgl_gpio_pin(LED0_GPIO); // Toggle the LED0.
	} while (j);
}

void vStartReceiveTask( unsigned portBASE_TYPE uxPriority )
{
signed portBASE_TYPE xLEDTask;

	/* Create the three tasks. */
	//for( xLEDTask = 0; xLEDTask < ledNUMBER_OF_LEDS; ++xLEDTask )
	{
		/* Spawn the task. */
		xTaskCreate( vRcvTask, ( signed char * ) "LEDx", ledSTACK_SIZE, NULL, uxPriority, ( xTaskHandle * ) NULL );
	}
}
/*-----------------------------------------------------------*/
int receivedChar=100;
float DataToSend=0.0;
int res = 45;
int tmp = 200;
int tmp1=0;
static portTASK_FUNCTION( vRcvTask, pvParameters )
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
	
	
	//// interrupts
	//// Structure holding the configuration parameters
	//// of the EIC module.
	//eic_options_t eic_options;
//
	//// Activate LED0 pin in GPIO output mode and switch LED0 off.
	//gpio_set_gpio_pin(LED0_GPIO);
//
	//// Enable level-triggered interrupt.
	//eic_options.eic_mode   = EIC_MODE_LEVEL_TRIGGERED;
	//// Interrupt will trigger on low-level.
	//eic_options.eic_level  = EIC_LEVEL_LOW_LEVEL;
	//// Enable filter.
	//eic_options.eic_filter = EIC_FILTER_ENABLED;
	//// For Wake Up mode, initialize in asynchronous mode
	//eic_options.eic_async  = EIC_ASYNCH_MODE;
	//// Choose External Interrupt Controller Line
	//eic_options.eic_line   = EXT_INT_EXAMPLE_LINE;
//
	//// Map the interrupt line to the GPIO pin with the right peripheral function.
	//gpio_enable_module_pin(EXT_INT_EXAMPLE_PIN_LINE, EXT_INT_EXAMPLE_FUNCTION_LINE);
 	///*
	 //* Enable the internal pull-up resistor on that pin (because the EIC is
	 //* configured such that the interrupt will trigger on low-level, see
	 //* eic_options.eic_level).
	 //*/
	//gpio_enable_pin_pull_up(EXT_INT_EXAMPLE_PIN_LINE);
//
	//// Init the EIC controller with the options
	//eic_init(&AVR32_EIC, &eic_options,1);
//
	//// Enable External Interrupt Controller Line
	//eic_enable_line(&AVR32_EIC, EXT_INT_EXAMPLE_LINE);
	///*
	 //* Switch the CPU to static sleep mode.
	 //* When the CPU is idle, it is possible to switch off the CPU clock and optionally other
	 //* clock domains to save power. This is activated by the sleep instruction, which takes the sleep
	 //* mode index number as argument. SLEEP function is defined in \DRIVERS\PM\pm.h.
	 //* In static mode, all oscillators, including 32KHz and RC oscillator are stopped.
	 //* Bandgap voltage reference BOD detector is turned off.
	 //*/
	//SLEEP(AVR32_PM_SMODE_STATIC);
	///*
	 //* Cpu now is in static sleep mode. When the wake-up external interrupt occurs,
	 //* the CPU resumes execution here and enter the while(1) loop.
	 //*/
	//
	
	
	
	
	
	
	
	
	//const TickType_t xBlockTime = pdMSTOTICKS(200);
	for(;;)
	{
		res = usart_read_char(EXAMPLE_USART, &receivedChar);
		
		if ( res == USART_RX_ERROR)
		{
			usart_reset_status(EXAMPLE_USART);
		}
		else if(res !=USART_RX_EMPTY)
		{ 
			if(!xQueueSend(xQueueRcv, (void*) &receivedChar, 10));
			{
							
			}
		}
		//else 		
			//xQueueSend(xQueueRcv, (void*) &tmp1, 10);
			//while(usart_putchar(EXAMPLE_USART, 47));
		vTaskDelay(5);
		
		// Toggle LED0 for a short while
		//toggle_led();
//
		//// Interrupt Line must be cleared to enable next SLEEP action
		//eic_clear_interrupt_line(&AVR32_EIC, EXT_INT_EXAMPLE_LINE);
//
		//// re-enter sleep mode.
		//SLEEP(AVR32_PM_SMODE_STATIC);
		/*
		 * Cpu now is in static sleep mode. When the wake-up external interrupt occurs,
		 * the CPU resumes execution back from the top of the while loop.
		 */
		
	}
} 

