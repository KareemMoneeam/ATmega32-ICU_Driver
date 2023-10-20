/*
 ================================================================================================
 Name        : measure_duty_cycle.c
 Author      : Kareem Abd El-Moneam
 Description : Measure duty cycle for a signal using ICU driver
 ================================================================================================
 */

#include "lcd.h"
#include "icu.h"
#include <avr/io.h> /* To use the SREG register */

uint8 g_edgeCount = 0;
uint16 g_timeHigh = 0;
uint16 g_timePeriod = 0;
uint16 g_timePeriodPlusHigh = 0;

/* This is the call-back function */
void APP_edgeProcessing(void)
{
	g_edgeCount++;
	if(g_edgeCount == 1)
	{
		/*
		 * Clear the timer counter register to start measurements from the
		 * first detected rising edge
		 */
		ICU_clearTimerValue();
		/* Detect falling edge */
		ICU_setEdgeDetectionType(FALLING);
	}
	else if(g_edgeCount == 2)
	{
		/* Store the High time value */
		g_timeHigh = ICU_getInputCaptureValue();    /* T1 */
		/* Detect rising edge */
		ICU_setEdgeDetectionType(RISING);
	}
	else if(g_edgeCount == 3)
	{
		/* Store the Period time value */
		g_timePeriod = ICU_getInputCaptureValue();  /* T2 */
		/* Detect falling edge */
		ICU_setEdgeDetectionType(FALLING);
	}
	else if(g_edgeCount == 4)
	{
		/* Store the Period time value + High time value */
		g_timePeriodPlusHigh = ICU_getInputCaptureValue();   /*  T3  */
		/* Clear the timer counter register to start measurements again */
		ICU_clearTimerValue();
		/* Detect rising edge */
		ICU_setEdgeDetectionType(RISING);
	}
}

int main(void)
{
	uint32 dutyCycle = 0;

	/* Create configuration structure for ICU driver */
	ICU_ConfigType ICU_Configurations = {F_CPU_CLOCK,RISING};

	/* Enable Global Interrupt I-Bit */
	SREG |= (1<<7);

	/* Set the Call back function pointer in the ICU driver */
	ICU_setCallBack(APP_edgeProcessing);

	/* Initialize both the LCD and ICU driver */
	LCD_init();
	ICU_init(&ICU_Configurations);

	while(1)
	{
		if(g_edgeCount == 4)
		{
			ICU_deInit(); /* Disable ICU Driver */
			g_edgeCount = 0;
			LCD_displayString("Duty = ");
			/* calculate the dutyCycle */
			dutyCycle = ((float)(g_timePeriodPlusHigh-g_timePeriod) / (g_timePeriodPlusHigh - g_timeHigh)) * 100;
			/* display the dutyCycle on LCD screen */
			LCD_intgerToString(dutyCycle);
			LCD_displayCharacter('%');
		}
	}
}
