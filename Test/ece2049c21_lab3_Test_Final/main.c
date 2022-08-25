/***************************************************/
/************** ECE2049 DEMO CODE ******************/
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

//global variables
unsigned int month[12] = {31,28,31,30,31,30,31,31,30,31,30,31};
//out will be our array that holds all our date information, besides the month
unsigned char out[13];
int months = 1;
//unsigned char time[5];
//These variables with endings 1&2 will serve as our digits in the case that
//either the day,hour,minutes, or seconds value are double digits
int d1;
int d2;
int h1;
int h2;
int m1;
int m2;
int s1;
int s2;
char monthsASCII;
long unsigned int days = 1;//(60*60*24)
long unsigned int hour = 0;
long unsigned int minutes = 0;
long unsigned int seconds = 0;
long unsigned int remainder = 0;



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
            displayTime(5184000);//March 2nd
            Graphics_drawStringCentered(&g_sContext, my_tempArray, AUTO_STRING_LENGTH, 48, 85, TRANSPARENT_TEXT);
            Graphics_flushBuffer(&g_sContext);

        }


        //Interrupt Delay
        while((counter) < (currcount +1)){}

        //Clears the LCD output
        Graphics_clearDisplay(&g_sContext);
        months = 1; //Prevent months from being changed by the loop
        currcurrcount = (currcount - (currcount-1));
        inTime = inTime + currcurrcount;


    }
}

void displayTime(long unsigned int inTime)
{
    bool pass = true;
    int i = 0;
    long unsigned int date[5];
    /*
    long unsigned int days = 1;//(60*60*24)
    long unsigned int hour = 0;
    long unsigned int minutes = 0;
    long unsigned int seconds = 0;
    long unsigned int remainder = 0;
     */
    //char test[10];
    //char hh[5];
    //char dd[5];
    //char hh[3];


    days = 1 + (inTime/(86400));// the 1 accounts for Jan 1st
    remainder = inTime - ((86400)*(days-1));//-86400
    hour = remainder/(3600);
    remainder = remainder - (3600)*hour;
    minutes = remainder/60;
    remainder = remainder - (60*minutes);
    seconds = remainder;

    unsigned int sum = month[0];
    //determine month
    while(pass==true)
    {
        if(days>sum)
        {
            sum+=month[i+1];
            months++;
            i++;
        }

        else
        {
            days = days-sum+month[i];
            pass = false;
        }
    }

    date[0]=months;
    date[1]=days;
    date[2]=hour;
    date[3]=minutes;
    date[4]=seconds;
    /*
    time[0]=(char)months+0x30;//was +'0'
    time[1]=(char)days+0x30;
    time[2]=(char)hour+0x30;
    time[3]=(char)minutes+0x30;
    time[4]=(char)seconds+0x30;//could + 48 also work?
     */

    //All comparisons determine how to save and write to our array "out"
    //out is the array we must write to
    if(days>9)
    {
        d1 = days/10;
        d2 = days%10;
        out[0] = ' ';
        out[1] = (char)d1+48;
        out[2] = (char)d2+48;
        out[3] = ':';

    }
    else
    {
        out[0] = ' ';
        out[1] = '0';
        out[2] =(char)days+48;
        out[3] = ':';
    }

    if(hour>9)
    {
        h1 = hour/10;
        h2 = hour%10;
        out[4] = (char)h1+48;
        out[5] = (char)h2+48;
        out[6] = ':';
    }

    else
    {
        out[4] = '0';
        out[5] = (char)hour+48;
        out[6] = ':';
    }

    if(minutes>9)
    {
        m1 = minutes/10;
        m2 = minutes%10;
        out[7] = (char)m1+48;
        out[8] = (char)m2+48;
        out[9] = ':';
    }

    else
    {
        out[7] = '0';
        out[8] = (char)minutes+48;
        out[9] = ':';

    }

    if(seconds>9)
    {
        s1 = seconds/10;
        s2 = seconds%10;
        out[10] = (char)s1+48;
        out[11] = (char)s2+48;
        out[12] = ' ';
    }

    else
    {
        out[10] = '0';
        out[11] = (char)seconds+48;
        out[12] = ' ';
    }

    //DETERMINE & WRITE MONTH

    if(months == 1)
    {
        Graphics_drawStringCentered(&g_sContext, "JAN", AUTO_STRING_LENGTH, 20, 60, TRANSPARENT_TEXT);
    }
    if(months == 2)
    {
        Graphics_drawStringCentered(&g_sContext, "FEB", AUTO_STRING_LENGTH, 20, 60, TRANSPARENT_TEXT);
    }
    if(months == 3)
    {
        Graphics_drawStringCentered(&g_sContext, "MAR", AUTO_STRING_LENGTH, 20, 60, TRANSPARENT_TEXT);
    }
    if(months == 4)
    {
        Graphics_drawStringCentered(&g_sContext, "APR", AUTO_STRING_LENGTH, 20, 60, TRANSPARENT_TEXT);
    }
    if(months == 5)
    {
        Graphics_drawStringCentered(&g_sContext, "MAY", AUTO_STRING_LENGTH, 20, 60, TRANSPARENT_TEXT);
    }
    if(months == 6)
    {
        Graphics_drawStringCentered(&g_sContext, "JUN", AUTO_STRING_LENGTH, 20, 60, TRANSPARENT_TEXT);
    }
    if(months == 7)
    {
        Graphics_drawStringCentered(&g_sContext, "JUL", AUTO_STRING_LENGTH, 20, 60, TRANSPARENT_TEXT);
    }
    if(months == 8)
    {
        Graphics_drawStringCentered(&g_sContext, "AUG", AUTO_STRING_LENGTH, 20, 60, TRANSPARENT_TEXT);
    }
    if(months == 9)
    {
        Graphics_drawStringCentered(&g_sContext, "SEP", AUTO_STRING_LENGTH, 20, 60, TRANSPARENT_TEXT);
    }
    if(months == 10)
    {
        Graphics_drawStringCentered(&g_sContext, "OCT", AUTO_STRING_LENGTH, 20, 60, TRANSPARENT_TEXT);
    }
    if(months == 11)
    {
        Graphics_drawStringCentered(&g_sContext, "NOV", AUTO_STRING_LENGTH, 20, 60, TRANSPARENT_TEXT);
    }
    if(months == 12)
    {
        Graphics_drawStringCentered(&g_sContext, "DEC", AUTO_STRING_LENGTH, 20, 60, TRANSPARENT_TEXT);
    }
    //JAn 11: 03:33:34
    //JAN: 11: 0:
    //DISPLAY DATE
    //Graphics_drawStringCentered(&g_sContext, dd, AUTO_STRING_LENGTH, 40, 60, TRANSPARENT_TEXT);
    Graphics_drawStringCentered(&g_sContext, out, AUTO_STRING_LENGTH, 65, 60, TRANSPARENT_TEXT);
    //Graphics_drawStringCentered(&g_sContext, hh, AUTO_STRING_LENGTH, 60, 60, TRANSPARENT_TEXT);
    //Graphics_drawStringCentered(&g_sContext, time[2], AUTO_STRING_LENGTH, 64, 70, TRANSPARENT_TEXT);
    //Graphics_drawStringCentered(&g_sContext, time[3], AUTO_STRING_LENGTH, 64, 80, TRANSPARENT_TEXT);
    //Graphics_drawStringCentered(&g_sContext, time[4], AUTO_STRING_LENGTH, 64, 90, TRANSPARENT_TEXT);

    //NOTE: the display of hours(hh) is causing a format error. Make sure to look into the warning produced!
    //TESTING, instead of multiple prints, use a single array that displays the date and time

    Graphics_flushBuffer(&g_sContext);




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
