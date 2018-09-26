/*****************************************************************************
 *
 * \file
 *
 * \brief FreeRTOS Real Time Kernel example.
 *
 * Creates all the demo application tasks, then starts the scheduler.  The WEB
 * documentation provides more details of the demo application tasks.
 *
 * Main. c also creates a task called "Check".  This only executes every three
 * seconds but has the highest priority so is guaranteed to get processor time.
 * Its main function is to check that all the other tasks are still operational.
 * Each task that does not flash an LED maintains a unique count that is
 * incremented each time the task successfully completes its function.  Should
 * any error occur within such a task the count is permanently halted.  The
 * check task inspects the count of each task to ensure it has changed since
 * the last time the check task executed.  If all the count variables have
 * changed all the tasks are still executing error free, and the check task
 * toggles an LED.  Should any task contain an error at any time the LED toggle
 * will stop.
 *
 * The LED flash and communications test tasks do not maintain a count.
 *
 *****************************************************************************/

/*
    FreeRTOS V7.0.0 - Copyright (C) 2011 Real Time Engineers Ltd.


    ***************************************************************************
     *                                                                       *
     *    FreeRTOS tutorial books are available in pdf and paperback.        *
     *    Complete, revised, and edited pdf reference manuals are also       *
     *    available.                                                         *
     *                                                                       *
     *    Purchasing FreeRTOS documentation will not only help you, by       *
     *    ensuring you get running as quickly as possible and with an        *
     *    in-depth knowledge of how to use FreeRTOS, it will also help       *
     *    the FreeRTOS project to continue with its mission of providing     *
     *    professional grade, cross platform, de facto standard solutions    *
     *    for microcontrollers - completely free of charge!                  *
     *                                                                       *
     *    >>> See http://www.FreeRTOS.org/Documentation for details. <<<     *
     *                                                                       *
     *    Thank you for using FreeRTOS, and thank you for your support!      *
     *                                                                       *
    ***************************************************************************


    This file is part of the FreeRTOS distribution.

    FreeRTOS is free software; you can redistribute it and/or modify it under
    the terms of the GNU General Public License (version 2) as published by the
    Free Software Foundation AND MODIFIED BY the FreeRTOS exception.
    >>>NOTE<<< The modification to the GPL is included to allow you to
    distribute a combined work that includes FreeRTOS without being obliged to
    provide the source code for proprietary components outside of the FreeRTOS
    kernel.  FreeRTOS is distributed in the hope that it will be useful, but
    WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
    or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
    more details. You should have received a copy of the GNU General Public
    License and the FreeRTOS license exception along with FreeRTOS; if not it
    can be viewed here: http://www.freertos.org/a00114.html and also obtained
    by writing to Richard Barry, contact details for whom are available on the
    FreeRTOS WEB site.

    1 tab == 4 spaces!

    http://www.FreeRTOS.org - Documentation, latest information, license and
    contact details.

    http://www.SafeRTOS.com - A version that is certified for use in safety
    critical systems.

    http://www.OpenRTOS.com - Commercial support, development, porting,
    licensing and training services.
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Environment header files. */
#include "power_clocks_lib.h"

/* Scheduler header files. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

/* Demo file headers. */
#include "partest.h"
#include "serial.h"
#include "integer.h"
#include "comtest.h"
//#include "flash.h"

#include "model.h"
#include "USART_RCV.h"
#include "USART_TRX.h"

#include "PollQ.h"
#include "semtest.h"
#include "dynamic.h"
#include "BlockQ.h"
#include "death.h"
#include "flop.h"

/*! \name Priority definitions for most of the tasks in the demo application.
 * Some tasks just use the idle priority.
 */
//! @{
#define mainLED_TASK_PRIORITY     ( tskIDLE_PRIORITY + 1 )
#define mainCOM_TEST_PRIORITY     ( tskIDLE_PRIORITY + 2 )
#define mainQUEUE_POLL_PRIORITY   ( tskIDLE_PRIORITY + 2 )
#define mainSEM_TEST_PRIORITY     ( tskIDLE_PRIORITY + 1 )
#define mainBLOCK_Q_PRIORITY      ( tskIDLE_PRIORITY + 3 )
#define mainCHECK_TASK_PRIORITY   ( tskIDLE_PRIORITY + 4 )
#define mainCREATOR_TASK_PRIORITY ( tskIDLE_PRIORITY + 3 )
//! @}

//! Baud rate used by the serial port tasks.
#define mainCOM_TEST_BAUD_RATE    ( ( unsigned portLONG ) 57600 )

//! LED used by the serial port tasks.  This is toggled on each character Tx,
//! and mainCOM_TEST_LED + 1 is toggled on each character Rx.
#define mainCOM_TEST_LED          ( 3 )

//! LED that is toggled by the check task.  The check task periodically checks
//! that all the other tasks are operating without error.  If no errors are found
//! the LED is toggled.  If an error is found at any time the LED toggles faster.
#define mainCHECK_TASK_LED        ( 6 )

//! LED that is set upon error.
#define mainERROR_LED             ( 7 )

//! The period between executions of the check task.
#define mainCHECK_PERIOD          ( ( portTickType ) 3000 / portTICK_RATE_MS  )

//! If an error is detected in a task, the vErrorChecks task will enter in an
//! infinite loop flashing the LED at this rate.
#define mainERROR_FLASH_RATE      ( (portTickType) 500 / portTICK_RATE_MS )

/*! \name Constants used by the vMemCheckTask() task.
 */
//! @{
#define mainCOUNT_INITIAL_VALUE   ( ( unsigned portLONG ) 0 )
#define mainNO_TASK               ( 0 )
//! @}

/*! \name The size of the memory blocks allocated by the vMemCheckTask() task.
 */
//! @{
#define mainMEM_CHECK_SIZE_1      ( ( size_t ) 51 )
#define mainMEM_CHECK_SIZE_2      ( ( size_t ) 52 )
#define mainMEM_CHECK_SIZE_3      ( ( size_t ) 15 )
//! @}


/*-----------------------------------------------------------*/

/*
 * The task that executes at the highest priority and calls
 * prvCheckOtherTasksAreStillRunning().  See the description at the top
 * of the file.
 */
static void vErrorChecks( void *pvParameters );

/*
 * Checks that all the demo application tasks are still executing without error
 * - as described at the top of the file.
 */
static portBASE_TYPE prvCheckOtherTasksAreStillRunning( void );

/*
 * A task that exercises the memory allocator.
 */
static void vMemCheckTask( void *pvParameters );

/*
 * Called by the check task following the detection of an error to set the
 * LEDs into a state that shows an error has beeen found.
 */
static void prvIndicateError( void );

/*-----------------------------------------------------------*/

xQueueHandle xQueueRcv = NULL, xQueueTrx = NULL, xQueueLCD = NULL;



int main( void )
{
	// Configure Osc0 in crystal mode (i.e. use of an external crystal source, with
	// frequency FOSC0) with an appropriate startup time then switch the main clock
	// source to Osc0.
	pcl_switch_to_osc(PCL_OSC0, FOSC0, OSC0_STARTUP);

	portDBG_TRACE("Starting the FreeRTOS AVR32 UC3 Demo...");

	/* Setup the LED's for output. */
	//vParTestInitialise();

	/* Start the standard demo tasks.  See the WEB documentation for more
	information.
	Note that the UC3L and UC3B parts do not have enough internal RAM to launch all the tasks of this example.
	That's why some of them are commented out using respectively #if (BOARD != UC3L_EK) or #if (BOARD != EVK1101)
	compiler directives. */
	
	xQueueRcv = xQueueCreate(10, sizeof(int)); //Rozmiar kolejki -> szybkoœæ transmisji, szybkoœæ procesora ~9600 -> 100ms na przygotowanie ramki dla tasku modelu ~~bufor na 11ramek
	xQueueTrx = xQueueCreate(10, sizeof(int));
	
	//Tick zegara systemowego: 1200b -> 120znaków/s ~znak na ms -> zegar systemowy tick = przyjecie jednego znaku * 2
	vStartModelTask( mainLED_TASK_PRIORITY );
	vStartReceiveTask( mainLED_TASK_PRIORITY ); //wysoki priorytet co 5ms sprawdzanie taska i baud rate 1200 -> bo wtedy co 10ms cos przychodzi
	vStartTrxTask( mainLED_TASK_PRIORITY );


	/* Start the scheduler. */
	vTaskStartScheduler();

	/* Will only get here if there was insufficient memory to create the idle
	task. */

	return 0;
}
/*-----------------------------------------------------------*/



/*!
 * \brief Checks that all the demo application tasks are still executing without error.
 */
static portBASE_TYPE prvCheckOtherTasksAreStillRunning( void )
{
static portBASE_TYPE xErrorHasOccurred = pdFALSE;

	if( xAreComTestTasksStillRunning() != pdTRUE )
	{
		xErrorHasOccurred = pdTRUE;
	}

	if( xArePollingQueuesStillRunning() != pdTRUE )
	{
		xErrorHasOccurred = pdTRUE;
	}

	#if (BOARD != UC3L_EK)
	if( xAreIntegerMathsTaskStillRunning() != pdTRUE )
	{
		xErrorHasOccurred = pdTRUE;
	}
	#endif

	#if (BOARD != UC3L_EK)
	if( xAreSemaphoreTasksStillRunning() != pdTRUE )
	{
		xErrorHasOccurred = pdTRUE;
	}
	#endif

	#if (BOARD != EVK1101) && (BOARD != UC3L_EK)
	if( xAreBlockingQueuesStillRunning() != pdTRUE )
	{
		xErrorHasOccurred = pdTRUE;
	}
	#endif

	#if (BOARD != UC3L_EK)
	if( xAreDynamicPriorityTasksStillRunning() != pdTRUE )
	{
		xErrorHasOccurred = pdTRUE;
	}
	#endif

	#if (BOARD != EVK1101) && (BOARD != UC3L_EK)
	if( xAreMathsTaskStillRunning() != pdTRUE )
	{
		xErrorHasOccurred = pdTRUE;
	}
	#endif

	if( xIsCreateTaskStillRunning() != pdTRUE )
	{
		xErrorHasOccurred = pdTRUE;
	}

	return ( xErrorHasOccurred );
}
/*-----------------------------------------------------------*/


/*!
 * \brief Dynamically created and deleted during each cycle of the vErrorChecks()
 * task.  This is done to check the operation of the memory allocator.
 * See the top of vErrorChecks for more details.
 *
 * \param *pvParameters Parameters for the task (can be of any kind)
 */
static void vMemCheckTask( void *pvParameters )
{
unsigned portLONG *pulMemCheckTaskRunningCounter;
void *pvMem1, *pvMem2, *pvMem3;
static portLONG lErrorOccurred = pdFALSE;

	/* This task is dynamically created then deleted during each cycle of the
	vErrorChecks task to check the operation of the memory allocator.  Each time
	the task is created memory is allocated for the stack and TCB.  Each time
	the task is deleted this memory is returned to the heap.  This task itself
	exercises the allocator by allocating and freeing blocks.

	The task executes at the idle priority so does not require a delay.

	pulMemCheckTaskRunningCounter is incremented each cycle to indicate to the
	vErrorChecks() task that this task is still executing without error. */

	pulMemCheckTaskRunningCounter = ( unsigned portLONG * ) pvParameters;

	for( ;; )
	{
		if( lErrorOccurred == pdFALSE )
		{
			/* We have never seen an error so increment the counter. */
			( *pulMemCheckTaskRunningCounter )++;
		}
		else
		{
			/* There has been an error so reset the counter so the check task
			can tell that an error occurred. */
			*pulMemCheckTaskRunningCounter = mainCOUNT_INITIAL_VALUE;
		}

		/* Allocate some memory - just to give the allocator some extra
		exercise.  This has to be in a critical section to ensure the
		task does not get deleted while it has memory allocated. */

		vTaskSuspendAll();
		{
			pvMem1 = pvPortMalloc( mainMEM_CHECK_SIZE_1 );

			if( pvMem1 == NULL )
			{
				lErrorOccurred = pdTRUE;
			}
			else
			{
				memset( pvMem1, 0xaa, mainMEM_CHECK_SIZE_1 );
				vPortFree( pvMem1 );
			}
		}
		xTaskResumeAll();

		/* Again - with a different size block. */
		vTaskSuspendAll();
		{
			pvMem2 = pvPortMalloc( mainMEM_CHECK_SIZE_2 );

			if( pvMem2 == NULL )
			{
				lErrorOccurred = pdTRUE;
			}
			else
			{
				memset( pvMem2, 0xaa, mainMEM_CHECK_SIZE_2 );
				vPortFree( pvMem2 );
			}
		}
		xTaskResumeAll();

		/* Again - with a different size block. */
		vTaskSuspendAll();
		{
			pvMem3 = pvPortMalloc( mainMEM_CHECK_SIZE_3 );
			if( pvMem3 == NULL )
			{
				lErrorOccurred = pdTRUE;
			}
			else
			{
				memset( pvMem3, 0xaa, mainMEM_CHECK_SIZE_3 );
				vPortFree( pvMem3 );
			}
		}
		xTaskResumeAll();
	}
}
/*-----------------------------------------------------------*/

static void prvIndicateError( void )
{
	/* The check task has found an error in one of the other tasks.
	Set the LEDs to a state that indicates this. */
	vParTestSetLED(mainERROR_LED,pdTRUE);

	#if BOARD == EVK1101 || BOARD == EVK1104 || BOARD == EVK1105 || BOARD == UC3C_EK || BOARD == UC3L_EK
		/* Set all LEDs to the same initial state before toggling them. */
		vParTestSetLED( 0, pdTRUE );
		vParTestSetLED( 1, pdTRUE );
		vParTestSetLED( 2, pdTRUE );
		vParTestSetLED( 3, pdTRUE );
	#endif

	for(;;)
	{
		#if BOARD == EVK1100
			vParTestToggleLED( mainCHECK_TASK_LED );
			vTaskDelay( mainERROR_FLASH_RATE );
		#elif BOARD == EVK1101 || BOARD == EVK1104 || BOARD == EVK1105 || BOARD == UC3C_EK || BOARD == UC3L_EK
			vParTestToggleLED( 0 );
			vParTestToggleLED( 1 );
			vParTestToggleLED( 2 );
			vParTestToggleLED( 3 );
			vTaskDelay( mainERROR_FLASH_RATE );
		#else
			#error You should define here the way of indicating an error on your board
		#endif
	}
}
