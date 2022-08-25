/************** ECE2049 DEMO CODE ******************/
/**************  28 March 2020   ******************/
/*********Redesigned by Hamayel Qureshi********/
/***************************************************/

#include <msp430.h>
//MASTER & SLAVE DEFINE STATEMENTS
#define PORT_SPI_SEL_S                        P4SEL//Port 4
#define PORT_SPI_DIR_S                        P4DIR
#define PORT_SPI_OUT_S                        P4OUT

#define PORT_CS_SEL_S                         P4SEL
#define PORT_CS_DIR_S                         P4DIR
#define PORT_CS_OUT_S                         P4OUT

#define PIN_SPI_SIMO_S                        BIT1//SIMO config
#define PIN_SPI_SOMI_S                        BIT2//SOMI config
#define PIN_SPI_SCLK_S                        BIT3//CLK config
#define PIN_CS_S                              BIT0//CS config

//UCB1 is slave
#define SPI_REG_CTL0    UCB1CTL0
#define SPI_REG_CTL1    UCB1CTL1
#define SPI_REG_BRL     UCB1BR0
#define SPI_REG_BRH     UCB1BR1
#define SPI_REG_IFG     UCB1IFG
#define SPI_REG_STAT    UCB1STAT
#define SPI_REG_TXBUF   UCB1TXBUF
#define SPI_REG_RXBUF   UCB1RXBUF

/* Peripherals.c and .h are where the functions that implement
 * the LEDs and keypad, etc are. It is often useful to organize
 * your code by putting like functions together in files.
 * You include the header associated with that file(s)
 * into the main file of your project. */
#include "peripherals.h"
#include <stdbool.h>
#include <stdlib.h>

// Function Prototypes
void swDelay(char numLoops);
void displayTime(long unsigned int inTime);
void floatToString(float valueToInt, unsigned char*xVolArray);
void initTimer(void);
void slaveSpiRead();
void masterSpiWrite();
void readVoltage();

//global variables
unsigned int potReading = 0;
volatile float potVolts = 0;
unsigned char volArray[5];
int state  = 0;
int counter = 0;


// Main
void main(void)
{
    //local variables

    WDTCTL = WDTPW | WDTHOLD;    // Stop watchdog timer. Always need to stop this!

    // Useful code starts here
    initLeds();
    configDisplay();
    configKeypad();
    configslave();

    //Enables global interrupts
    _BIS_SR(GIE);

    // *** Intro Screen ***
    Graphics_clearDisplay(&g_sContext); // Clear the display

    // Draw a box around everything because it looks nice
    Graphics_Rectangle box = {.xMin = 3, .xMax = 125, .yMin = 3, .yMax = 125 };
    Graphics_drawRectangle(&g_sContext, &box);

    while(1)
    {
        switch(state)
        {
        case 0:
            configADC12();
            readVoltage();//obtain reading
            floatToString(potVolts,volArray);
            Graphics_drawStringCentered(&g_sContext, volArray, 5, 50, 25, TRANSPARENT_TEXT);
            Graphics_flushBuffer(&g_sContext);
            swDelay(3);
            Graphics_clearDisplay(&g_sContext);

            break;
        }
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
    // hamayel qureshi, 28 march 2020

    volatile unsigned int i,j;  // volatile to prevent removal in optimization
    // by compiler. Functionally this is useless code

    for (j=0; j<numLoops; j++)
    {
        i = 50000 ;                 // SW Delay
        while (i > 0)               // could also have used while (i)
            i--;
    }
}

void configADC12()
{
    //unsigned int potReading = 0;
    // Turn on ADC
    ADC12CTL0 = ADC12SHT0_9 | ADC12ON;

    // Sample and Hold mode
    ADC12CTL1 = ADC12SHP;

    //ref = 3.3V, ch = A0
    ADC12MCTL0 = ADC12SREF_0 | ADC12INCH_0;

    // Port 6 pin 0 to function mode in ADC
    P6SEL |= BIT0;

    // clear start bit
    ADC12CTL0 &= ~ADC12SC;

    // enable and start single conversion
    ADC12CTL0 |= ADC12SC | ADC12ENC;

    while (ADC12CTL1 & ADC12BUSY){
        __no_operation();
    }
    //keep low 12 bits
    potReading = ADC12MEM0 & 0x0FFF;
}

void initTimer(void){
    TA2CTL = TASSEL_1+ID_0+MC_1;

    TA2CCR0 = 32767; //  1 second

    TA2CCTL0 = CCIE;
}
/*
#pragma vector=TIMER2_A0_VECTOR
__interrupt void TIMERINTERUPT(void){
    counter++;
}
*/

void configslave()
{
    // Configure SCLK and MOSI for peripheral mode
    PORT_SPI_SEL_S |= (PIN_SPI_SOMI_S|PIN_SPI_SIMO_S|PIN_CS_S|PIN_SPI_SCLK_S);//Sets pins 2,1,0,3

    // Configure the slave chip select as an output
    PORT_CS_SEL_S &=  ~PIN_CS_S;//complete
    PORT_CS_DIR_S |=   PIN_CS_S;
    PORT_CS_OUT_S &= ~PIN_CS_S;//enable

    // Now configure UCSIB1 to function as our SPI slave

    // Disable the module so we can configure it
    SPI_REG_CTL1 |= UCSWRST;

    SPI_REG_CTL0 &= ~(UCCKPH|UCCKPL|UC7BIT|UCMSB); // Reset the controller config parameters
    SPI_REG_CTL1 &= ~UCSSEL_3; // Reset the clock configuration

    SPI_REG_CTL1 |= SPI_CLK_SRC; // Select SMCLK for our clock source

    // Set SPI clock frequency (which is the same frequency as SMCLK so this can apparently be 0)
    SPI_REG_BRL  =  ((uint16_t)SPI_CLK_TICKS) & 0xFF;         // Load the low byte
    SPI_REG_BRH  = (((uint16_t)SPI_CLK_TICKS) >> 8) & 0xFF;   // Load the high byte

    // Configure for SPI master, synchronous, 3 wire SPI, MSB first, capture data on first edge,
    // and inactive low polarity
    SPI_REG_CTL0 |= (UCMST|UCSYNC|UCMODE_0|UCMSB|UCCKPH);

    // Reenable the module
    SPI_REG_CTL1 &= ~UCSWRST;
    SPI_REG_IFG  &= ~UCRXIFG;
}

void readVoltage()
{
    //convert to volts
    potVolts = (float)potReading * 3.3/ 4096;
}

void slaveSpiRead()
{
    unsigned char c;
    while(!(UCB1IFG&UCRXIFG))
    {
        c = UCB1RXBUF;
    }

    return c;
}

void masterSpiWrite()
{

}

void floatToString(float valueToInt, unsigned char*xVolArray){
    int value = (int)(valueToInt*100);
    int firstDig;
    int secondDig;
    int thirdDig;

    firstDig = value/100;
    secondDig = (value-(firstDig*100))/10;
    thirdDig = (value-(firstDig*100))%10;

    char firstNum = (char)(firstDig+'0');
    char secondNum = (char)(secondDig+'0');
    char thirdNum = (char)(thirdDig+'0');

    if (valueToInt>9.0){
        volArray[0] = firstNum;
        volArray[1] = '.';
        volArray[2] = secondNum;
        volArray[3] = thirdNum;
        volArray[4] = 'V';
    }

    else if(valueToInt<=9.0){
        volArray[0] = firstNum;
        volArray[1] = '.';
        volArray[2] = secondNum;
        volArray[3] = thirdNum;
        volArray[4] = 'V';
    }
}
