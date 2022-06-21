/************** ECE2049 DEMO CODE ******************/
/**************  9 January 2019   ******************/
/***************************************************/

#include <msp430.h>

/* Peripherals.c and .h are where the functions that implement
 * the LEDs and keypad, etc are. It is often useful to organize
 * your code by putting like functions together in files.
 * You include the header associated with that file(s)
 * into the main file of your project. */
#include "peripherals.h"
#include<stdio.h>
#include<stdlib.h>
#include <time.h>       /* time */
#define CALADC12_15V_30C  *((unsigned int *)0x1A1A)
// Temperature Sensor Calibration = Reading at 85 degrees C is stored at addr 1A1Ch
#define CALADC12_15V_85C  *((unsigned int *)0x1A1C)

unsigned int in_temp;

//#include<string.h>

// Function Prototypes
void swDelay(char numLoops);
void initTimer(void);
void convMonth(long unsigned int day);
void intToString(long unsigned int number,int type, unsigned char*my_array1);
void displayTime(long unsigned int inTime);


// Declare globals here
int counter = 0;
unsigned char my_tempArray[12];
long unsigned int inTime = 1058385; //2384647
unsigned int in_temp;
float potVolts = 0;
//int note;



void setupADC12()
{
    unsigned int potReading = 0;
    //    float potVolts;
    ADC12CTL0 = ADC12SHT0_9 | ADC12ON; // Turn on ADC
    ADC12CTL1 = ADC12SHP; // Sample and Hold mode
    ADC12MCTL0 = ADC12SREF_0 | ADC12INCH_0; //ref = 3.3V, ch = A0
    P6SEL |= BIT0; // Port 6 pin 0 to function mode in ADC
    ADC12CTL0 &= ~ADC12SC; // clear start bit
    // enable and start single conversion
    ADC12CTL0 |= ADC12SC | ADC12ENC;
    while (ADC12CTL1 & ADC12BUSY)
        __no_operation();
    potReading = ADC12MEM0 & 0x0FFF; //keep low 12 bits
    potVolts = (float)potReading * 3.3/ 4096; // convert to volts
    //return potVolts;
}

void initTimer(void){
    TA2CTL = TASSEL_1+ID_0+MC_1;

    TA2CCR0 = 32767; //  1 second

    TA2CCTL0 = CCIE;
}

#pragma vector=TIMER2_A0_VECTOR
__interrupt void TIMERINTERUPT(void){
    counter++;
}


void floatToString(float valueToInt, int type, unsigned char*my_tempArray)
{

    int value = (int)(valueToInt);
    int firstdigit;
    int secdigit;

    firstdigit = value /10;
    secdigit = value %10;

    char fdigit = (char)(firstdigit+'0');
    char sdigit = (char)(secdigit+'0');

    if (type == 1)
    {
        my_tempArray[0] = fdigit;
        my_tempArray[1] = sdigit;
        my_tempArray[2] = (char)(67);
    }
    else if(type == 2)
    {
        my_tempArray[3] = (char)(32);
        my_tempArray[4] = fdigit;
        my_tempArray[5] = sdigit;
        my_tempArray[6] = (char)(70);
    }


}

void displayTemp(float inAvgTempC)
{

    volatile float temperatureDegC;
    volatile float temperatureDegF;
    volatile float degC_per_bit;
    volatile unsigned int bits30, bits85;

    //WDTCTL = WDTPW + WDTHOLD;      // Stop WDT

    REFCTL0 &= ~REFMSTR;    // Reset REFMSTR to hand over control of
    // internal reference voltages to
    // ADC12_A control registers

    ADC12CTL0 = ADC12SHT0_9 | ADC12REFON | ADC12ON;     // Internal ref = 1.5V

    ADC12CTL1 = ADC12SHP;                     // Enable sample timer

    // Using ADC12MEM0 to store reading
    ADC12MCTL0 = ADC12SREF_1 + ADC12INCH_10;  // ADC i/p ch A10 = temp sense
    // ACD12SREF_1 = internal ref = 1.5v

    __delay_cycles(100);                    // delay to allow Ref to settle
    ADC12CTL0 |= ADC12ENC;              // Enable conversion

    // Use calibration data stored in info memory
    bits30 = CALADC12_15V_30C;
    bits85 = CALADC12_15V_85C;
    degC_per_bit = ((float)(85.0 - 30.0))/((float)(bits85-bits30));



    ADC12CTL0 &= ~ADC12SC;  // clear the start bit
    ADC12CTL0 |= ADC12SC;       // Sampling and conversion start
    // Single conversion (single channel)

    // Poll busy bit waiting for conversion to complete
    while (ADC12CTL1 & ADC12BUSY)
        __no_operation();
    in_temp = ADC12MEM0;      // Read in results if conversion

    // Temperature in Celsius. See the Device Descriptor Table section in the
    // System Resets, Interrupts, and Operating Modes, System Control Module
    // chapter in the device user's guide for background information on the
    // formula.
    temperatureDegC = (float)((long)in_temp - CALADC12_15V_30C) * degC_per_bit +30.0;
    floatToString(temperatureDegC, 1, my_tempArray);

    // Temperature in Fahrenheit
    temperatureDegF = (float)(((9/5)*temperatureDegC) + 32);
    floatToString(temperatureDegF, 2, my_tempArray);

    __no_operation();                       // SET BREAKPOINT HERE
}



// Main
void main(void)
{

    WDTCTL = WDTPW | WDTHOLD;    // Stop watchdog timer. Always need to stop this!!

    initTimer();
    configDisplay();
    __enable_interrupt();
    _BIS_SR(GIE);

    int currcount = 0;
    int currcurrcount = 0;

    Graphics_clearDisplay(&g_sContext);
    Graphics_flushBuffer(&g_sContext);

    while(1){
        //Creates the counter for the interrupt
        currcount = counter;

        if((counter) < (currcount +1))
        {
            displayTemp(in_temp);
            Graphics_drawStringCentered(&g_sContext, my_tempArray, AUTO_STRING_LENGTH, 48, 75, TRANSPARENT_TEXT);
            Graphics_flushBuffer(&g_sContext);
        }


        //Interrupt Delay
        while((counter) < (currcount +1)){}

        //Clears the LCD output
        Graphics_clearDisplay(&g_sContext);
        currcurrcount = (currcount - (currcount-1));
        inTime = inTime + currcurrcount;


    }
}



void swDelay(char numLoops)
{
    // This function is a software delay. It performs
    // useless loops to waste a bit of time
    //
    // Input: numLoops = number of delay loops to execute
    // Output: none
    //
    // smj, ECE2049, 25 Aug 2013

    volatile unsigned int i,j;  // volatile to prevent removal in optimization
    // by compiler. Functionally this is useless code

    for (j=0; j<numLoops; j++)
    {
        i = 50000 ;                 // SW Delay
        while (i > 0)               // could also have used while (i)
            i--;
    }
}
