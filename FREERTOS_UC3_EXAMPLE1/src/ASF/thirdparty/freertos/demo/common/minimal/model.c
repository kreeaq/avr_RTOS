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

/* Demo program include files. */
#include "partest.h"
#include "model.h"

#define ledSTACK_SIZE		configMINIMAL_STACK_SIZE
#define ledNUMBER_OF_LEDS	( 3 )
#define ledFLASH_RATE_BASE	( ( portTickType ) 333 )

extern xQueueHandle xQueueRcv;
extern xQueueHandle xQueueTrx;

/* Variable used by the created tasks to calculate the LED number to use, and
the rate at which they should flash the LED. */
static volatile unsigned portBASE_TYPE uxFlashTaskNumber = 0;

/* The task that is created three times. */
static portTASK_FUNCTION_PROTO( vModelTask, pvParameters );

/*-----------------------------------------------------------*/

void vStartModelTask( unsigned portBASE_TYPE uxPriority )
{
signed portBASE_TYPE xLEDTask;

	/* Create the three tasks. */
	//for( xLEDTask = 0; xLEDTask < ledNUMBER_OF_LEDS; ++xLEDTask )
	{
		/* Spawn the task. */
		xTaskCreate( vModelTask, ( signed char * ) "LEDx", ledSTACK_SIZE, NULL, uxPriority, ( xTaskHandle * ) NULL );
	}
}
/*-----------------------------------------------------------*/
int DataToReadModel=0;
int DataToSendModel=0;
float y = 0;
float c1 =50;
float c = 3 ;
float dt = 1;
static portTASK_FUNCTION( vModelTask, pvParameters )
{


	/* The parameters are not used. */
	( void ) pvParameters;

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


	for(;;)
	{
		if(xQueueReceive(xQueueRcv, &DataToReadModel, 10))
		{			
			y += (DataToReadModel/c1 - ((y/c1*c)/10))*dt;
			DataToSendModel = (int)y;
			xQueueSend(xQueueTrx, &DataToSendModel, 10);
		}
		
		
		vTaskDelay(5);
	}
} /*lint !e715 !e818 !e830 Function definition must be standard for task creation. */

