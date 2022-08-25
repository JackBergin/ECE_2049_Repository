/************** ECE2049 DEMO CODE ******************/
/**************  28 March 2020   ******************/
/*********Redesigned by Hamayel Qureshi********/
/***************************************************/

#include <msp430.h>
//MASTER & SLAVE DEFINE STATEMENTS
#define SLAVE_PORT_SPI_SEL                    P4SEL//Port 4
#define SLAVE_PORT_SPI_DIR                    P4DIR
#define SLAVE_PORT_SPI_OUT                    P4OUT

#define SLAVE_PORT_CS_SEL                     P4SEL
#define SLAVE_PORT_CS_DIR                     P4DIR
#define SLAVE_PORT_CS_OUT                     P4OUT
#define SLAVE_PORT_CS_REN                     P4REN

#define SLAVE_PIN_SPI_MOSI                    BIT1//MOSI config
#define SLAVE_PIN_SPI_MISO                    BIT2//MISO config
#define SLAVE_PIN_SPI_SCLK                    BIT3//SCLK config
#define SLAVE_PIN_SPI_CS                      BIT0//CS config

//UCB1 is slave
#define SLAVE_SPI_REG_CTL0    UCB1CTL0
#define SLAVE_SPI_REG_CTL1    UCB1CTL1
#define SLAVE_SPI_REG_BRL     UCB1BR0
#define SLAVE_SPI_REG_BRH     UCB1BR1
#define SLAVE_SPI_REG_IFG     UCB1IFG
#define SLAVE_SPI_REG_STAT    UCB1STAT
#define SLAVE_SPI_REG_TXBUF   UCB1TXBUF
#define SLAVE_SPI_REG_RXBUF   UCB1RXBUF

//This is needed to Configure P8.2 to use it as CS by MSP430
#define MSP_PORT_CS_SEL         P8SEL
#define MSP_PORT_CS_DIR         P8DIR
#define MSP_PORT_CS_OUT         P8OUT
#define MSP_PIN_CS              BIT2

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
unsigned char slaveSpiRead();
void masterSpiWrite(unsigned int data);
void readVoltage();
void configslave();
void configADC12();

//global variables
unsigned int potReading = 0;
float potVolts = 0;
unsigned char volArray[5];
int state  = 0;
int counter = 0;
unsigned char w = 0x55;//eventually we will floatToString the voltage and have this be our value we are passing
unsigned char v1 = 0x11;// cant we just leave this null?
unsigned char v2 = 0x11;// cant we just leave this null?
unsigned char v3 = 0x11;// cant we just leave this null?
unsigned char val;
unsigned char out[6];

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
            masterSpiWrite(volArray[0]);
            v1 = slaveSpiRead();
            masterSpiWrite(volArray[2]);
            v2 = slaveSpiRead();
            masterSpiWrite(volArray[3]);
            v3 = slaveSpiRead();

            out[0] = ' ';
            out[1] = v1;
            out[2] = '.';
            out[3] = v2;
            out[4] = v3;
            out[5] = 'V';
            out[6] = ' ';
            //Graphics_drawStringCentered(&g_sContext, volArray, AUTO_STRING_LENGTH, 50, 25, TRANSPARENT_TEXT);
            Graphics_drawStringCentered(&g_sContext, out, 6, 50, 50, TRANSPARENT_TEXT);
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
    SLAVE_PORT_SPI_SEL |= (SLAVE_PIN_SPI_MOSI|SLAVE_PIN_SPI_MISO|SLAVE_PIN_SPI_SCLK);//Sets pins 2,1,0,3

    // Configure the slave chip select as an INPUT P4.0
    SLAVE_PORT_CS_SEL &=  ~SLAVE_PIN_SPI_CS;
    SLAVE_PORT_CS_DIR &= ~SLAVE_PIN_SPI_CS;
    SLAVE_PORT_CS_REN |= SLAVE_PIN_SPI_CS;
    SLAVE_PORT_CS_OUT |= SLAVE_PIN_SPI_CS;

    //Configure the CS output of the MSP430, P8.2. It will set P4.0 high or low
    MSP_PORT_CS_SEL &= ~MSP_PIN_CS;
    MSP_PORT_CS_DIR |= MSP_PIN_CS;
    MSP_PORT_CS_OUT |= MSP_PIN_CS;

    // Disable the module so we can configure it
    SLAVE_SPI_REG_CTL1 |= UCSWRST;

    SLAVE_SPI_REG_CTL0 &= ~(0xFF); // Reset the controller config parameters
    SLAVE_SPI_REG_CTL1 &= ~UCSSEL_3; // Reset the clock configuration

    //SLAVE_SPI_REG_CTL1 |= SPI_CLK_SRC; // Select SMCLK for our clock source

    // Set SPI clock frequency (which is the same frequency as SMCLK so this can apparently be 0)
    SPI_REG_BRL  =  ((uint16_t)SPI_CLK_TICKS) & 0xFF;         // Load the low byte
    SPI_REG_BRH  = (((uint16_t)SPI_CLK_TICKS) >> 8) & 0xFF;   // Load the high byte

    // Configure for SPI master, synchronous, 3 wire SPI, MSB first, capture data on first edge,
    // and inactive low polarity
    SLAVE_SPI_REG_CTL0 |= (UCSYNC|UCMODE_2|UCMSB|UCCKPH);

    // Reenable the module
    SLAVE_SPI_REG_CTL1 &= ~UCSWRST;
    SLAVE_SPI_REG_IFG &= ~UCRXIFG;//clear interrupt flag
}

void readVoltage()
{
    //convert to volts
    potVolts = (float)potReading * 3.3/ 4096;
}

unsigned char slaveSpiRead()
{
    unsigned char c;
    while(!(SLAVE_SPI_REG_IFG & UCRXIFG))//DOUBLE CHECK
    {
        c = SLAVE_SPI_REG_RXBUF;
    }

    return (c & 0xFF);
}


void masterSpiWrite(unsigned int data)
{
    MSP_PORT_CS_OUT &= ~SLAVE_PIN_SPI_CS;//xxx0, digital I/O config

    uint8_t byte = (unsigned char) ((data)&0xFF);

    //Put byte to send in transmit TX buffer
    SPI_REG_TXBUF = byte;//writing to master here

    //Wait for the TX buffer to empty
    while(!(SPI_REG_IFG & UCTXIFG))
    {
        __no_operation();
    }

    MSP_PORT_CS_OUT |= SLAVE_PIN_SPI_CS;

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
