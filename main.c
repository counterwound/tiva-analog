/******************************************************************
 * Tiva Analog
 * Developed by Counterwound Labs, Inc.
 * http://counterwound.com
 * *****************************************************************
 *
 *    _____/\\\\\\\\\\\_______/\\\\\\\\\\\____/\\\\\\\\\\\_
 *     ___/\\\/////////\\\___/\\\/////////\\\_\/////\\\///__
 *      __\//\\\______\///___\//\\\______\///______\/\\\_____
 *       ___\////\\\___________\////\\\_____________\/\\\_____
 *        ______\////\\\___________\////\\\__________\/\\\_____
 *         _________\////\\\___________\////\\\_______\/\\\_____
 *          __/\\\______\//\\\___/\\\______\//\\\______\/\\\_____
 *           _\///\\\\\\\\\\\/___\///\\\\\\\\\\\/____/\\\\\\\\\\\_
 *            ___\///////////_______\///////////_____\///////////__
 */

#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_adc.h"
#include "inc/hw_gpio.h"
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/adc.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/timer.h"
#include "driverlib/uart.h"
#include "utils/uartstdio.h"

//*****************************************************************************
// Ports Setup
//*****************************************************************************

void PortFunctionInit(void)
{
    //
    // Enable Peripheral Clocks
    //
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);

    //
    // Enable pin PE5 for ADC AIN8
    //
    GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_5);

    //
    // Enable pin PD1 for ADC AIN6
    //
    GPIOPinTypeADC(GPIO_PORTD_BASE, GPIO_PIN_1);

    //
    // Enable pin PB4 for ADC AIN10
    //
    GPIOPinTypeADC(GPIO_PORTB_BASE, GPIO_PIN_4);

    //
    // Enable pin PE2 for ADC AIN1
    //
    GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_2);

    //
    // Enable pin PE1 for ADC AIN2
    //
    GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_1);

    //
    // Enable pin PD3 for ADC AIN4
    //
    GPIOPinTypeADC(GPIO_PORTD_BASE, GPIO_PIN_3);

    //
    // Enable pin PD0 for ADC AIN7
    //
    GPIOPinTypeADC(GPIO_PORTD_BASE, GPIO_PIN_0);

    //
    // Enable pin PE3 for ADC AIN0
    //
    GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_3);

    //
    // Enable pin PE0 for ADC AIN3
    //
    GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_0);

    //
    // Enable pin PD2 for ADC AIN5
    //
    GPIOPinTypeADC(GPIO_PORTD_BASE, GPIO_PIN_2);

    //
    // Enable pin PE4 for ADC AIN9
    //
    GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_4);

    //
    // Enable pin PB5 for ADC AIN11
    //
    GPIOPinTypeADC(GPIO_PORTB_BASE, GPIO_PIN_5);

    //
    // Enable pin PF2 for GPIOOutput
    //
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_2);

    //
    // Enable pin PA0 for UART0 U0RX
    //
    GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0);

    //
    // Enable pin PA1 for UART0 U0TX
    //
    GPIOPinConfigure(GPIO_PA1_U0TX);
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_1);
}

//*****************************************************************************
// Timers Setup
//*****************************************************************************
volatile bool g_bTimer0Flag = 0;		// Timer 0 occurred flag
volatile bool g_bTimer1Flag = 0;		// Timer 1 occurred flag

void ConfigureTimers(void)
{
	// Enable the peripherals used by this example.
	SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER1);

	// Configure the two 32-bit periodic timers.
	TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);
	TimerConfigure(TIMER1_BASE, TIMER_CFG_PERIODIC);
	TimerLoadSet(TIMER0_BASE, TIMER_A, SysCtlClockGet() / 1);		// 1 Hz
	TimerLoadSet(TIMER1_BASE, TIMER_A, SysCtlClockGet() / 10);		// 10 Hz

	// Setup the interrupts for the timer timeouts.
	IntEnable(INT_TIMER0A);
	IntEnable(INT_TIMER1A);
	TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
	TimerIntEnable(TIMER1_BASE, TIMER_TIMA_TIMEOUT);

	// Enable the timers.
	TimerEnable(TIMER0_BASE, TIMER_A);
	TimerEnable(TIMER1_BASE, TIMER_A);
}

// The interrupt handler for the first timer interrupt. 1 Hz
void Timer0IntHandler(void)
{
    // Clear the timer interrupt.
    TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

    g_bTimer0Flag = 1;		// Set flag to indicate Timer 0 interrupt
}

// The interrupt handler for the second timer interrupt. 10 Hz
void Timer1IntHandler(void)
{
    // Clear the timer interrupt.
    TimerIntClear(TIMER1_BASE, TIMER_TIMA_TIMEOUT);

    g_bTimer1Flag = 1;		// Set flag to indicate Timer 1 interrupt
}

//*****************************************************************************
// Interrupts Setup
//*****************************************************************************
void ConfigureInterrupts(void)
{
	// Enable processor interrupts.
	IntMasterEnable();
}

//*****************************************************************************
// Main code starts here
//*****************************************************************************
int main(void)
{
    // Set the clocking to run directly from the crystal.
    SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC | SYSCTL_OSC_MAIN |
    		SYSCTL_XTAL_16MHZ);

    // Initialize and setup the ports
    PortFunctionInit();
    ConfigureTimers();
    ConfigureInterrupts();

	// Use the internal 16MHz oscillator as the UART clock source.
	UARTClockSourceSet(UART0_BASE, UART_CLOCK_PIOSC);

	// Initialize the UART0 using uartstdio
    UARTStdioConfig(0, 115200, 16000000);

    uint32_t ADCValues0[1];
    uint32_t ADCValues1[1];
    uint32_t ADCValues2[1];
    uint32_t TempValueC;
    uint32_t TempValueF;

    ADCSequenceConfigure(ADC0_BASE, 0, ADC_TRIGGER_PROCESSOR, 0);
    ADCSequenceStepConfigure(ADC0_BASE, 0, 0, ADC_CTL_TS | ADC_CTL_IE | ADC_CTL_END);
    ADCSequenceEnable(ADC0_BASE, 0);
    ADCIntClear(ADC0_BASE, 0);

    ADCSequenceConfigure(ADC0_BASE, 1, ADC_TRIGGER_PROCESSOR, 0);
    ADCSequenceStepConfigure(ADC0_BASE, 1, 0, ADC_CTL_TS | ADC_CTL_IE | ADC_CTL_END);
    ADCSequenceEnable(ADC0_BASE, 1);
    ADCIntClear(ADC0_BASE, 1);

    ADCSequenceConfigure(ADC0_BASE, 2, ADC_TRIGGER_PROCESSOR, 0);
    ADCSequenceStepConfigure(ADC0_BASE, 2, 0, ADC_CTL_TS | ADC_CTL_IE | ADC_CTL_END);
    ADCSequenceEnable(ADC0_BASE, 2);
    ADCIntClear(ADC0_BASE, 2);

  	while(1)
	{
		//*****************************************************************************
		// Timers
		//*****************************************************************************

		// Timer 0
		if ( g_bTimer0Flag )
		{
			g_bTimer0Flag = 0;		// Clear flag

			if ( GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_2) )
			{
				GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, 0); //blink LED
			} else {
				GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, GPIO_PIN_2); //blink LED
			}
		}

		// Timer 1
		if ( g_bTimer1Flag )
		{
			g_bTimer1Flag = 0;		// Clear Timer 1 flag

			ADCProcessorTrigger(ADC0_BASE, 0);
			while(!ADCIntStatus(ADC0_BASE, 0, false))
			{
			}
			ADCIntClear(ADC0_BASE, 0);
			ADCSequenceDataGet(ADC0_BASE, 0, ADCValues0);

			ADCProcessorTrigger(ADC0_BASE, 1);
			while(!ADCIntStatus(ADC0_BASE, 1, false))
			{
			}
			ADCIntClear(ADC0_BASE, 1);
			ADCSequenceDataGet(ADC0_BASE, 1, ADCValues2);

			ADCProcessorTrigger(ADC0_BASE, 2);
			while(!ADCIntStatus(ADC0_BASE, 2, false))
			{
			}
			ADCIntClear(ADC0_BASE, 2);
			ADCSequenceDataGet(ADC0_BASE, 2, ADCValues2);

			TempValueC = (uint32_t)(147.5 - ((75.0*3.3 *(float)ADCValues2[0])) / 4096.0);
			TempValueF = ((TempValueC * 9) + 160) / 5;
			UARTprintf("Temperature = %3d*C or %3d*F\r", TempValueC, TempValueF);
		}
	}
}
